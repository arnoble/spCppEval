// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header.h"
using namespace std;



int _tmain(int argc, TCHAR* argv[])
{
	const int maxUls(100);
	const int bufSize(1000);
	SQLHENV               hEnv       = NULL;		   // Env Handle from SQLAllocEnv()
	SQLHDBC               hDBC       = NULL;           // Connection handle
	char                  szDate[bufSize];
	char                  szPrice[bufSize], **szAllPrices = new char*[maxUls];
	SQLLEN                cbModel;		               // Model buffer bytes recieved
	RETCODE               retcode;
	SQLCHAR*              thisSQL;

	// initialise
	srand(time(0)); // reseed rand
	int              productId = argc > 1 ? _ttoi(argv[1]) : 363;
	int              numMcIterations = argc>2 ? _ttoi(argv[2]) : 100;
	cout << "Iterations:" << numMcIterations << " ProductId:" << productId << endl;
	unsigned	     uI;
	int              oldProductBarrierId = 0, productBarrierId = 0;
	int              historyStep = 1;
	int              numBarriers = 0, thisIteration = 0;
	int              anyInt, i, j, k, len, callOrPut, thisPoint, thisBarrier, thisMonIndx, thisMonPoint, numUl, numMonPoints, lastPoint, productDays, totalNumDays, totalNumReturns, uid;
	int              thisPayoffId, thisMonDays;
	double           anyDouble, barrier, uBarrier, payoff, strike, cap, participation,fixedCoupon;
	string           couponFrequency,productStartDateString, word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate, description;
	char             lineBuffer[1000], charBuffer[1000];
	bool             capitalOrIncome, above, at;
	vector<double>   ulReturns[50];
	vector<int>      monDateIndx;
	vector<string>   payoffType = { "", "fixed", "call", "put", "twinWin", "switchable", "basketCall", "lookbackCall" };
	vector<int>::iterator intIterator;
	vector<UlTimeseries>  ulOriginalPrices(100), ulPrices(100); // underlying prices	
	for (i = 0; i < maxUls; i++){
		szAllPrices[i] = new char[bufSize];
	}
	// cout << "Press a key to continue...";  getline(cin, word);  // KEEP in case you want to attach debugger


	// open database, get product table data
	MyDB  mydb((char **)szAllPrices), mydb1((char **)szAllPrices);
	enum {
		colProductStrikeDate = 6, colProductFixedCoupon = 28, colProductFrequency,colProductBid,colProductLast
	};
	sprintf(lineBuffer, "%s%d%s", "select * from product where ProductId='", productId, "'");
	mydb.prepare((SQLCHAR *)lineBuffer, colProductLast);
	retcode = mydb.fetch(true);
	productStartDateString =      szAllPrices[colProductStrikeDate];
	fixedCoupon            = atof(szAllPrices[colProductFixedCoupon]);
	if (strlen(szAllPrices[colProductFrequency])){ couponFrequency = szAllPrices[colProductFrequency];	}
	boost::gregorian::date  bProductStartDate(boost::gregorian::from_simple_string(productStartDateString));

	// get underlyingids for this product from DB
	vector<int> ulIds;
	vector<int> ulIdNameMap(1000);
	sprintf(lineBuffer, "%s%d%s", "select distinct UnderlyingId from productbarrier join barrierrelation using (ProductBarrierId) where ProductId='", productId, "'");
	mydb.prepare((SQLCHAR *)lineBuffer,1);
	retcode = mydb.fetch(true);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		uid = atoi(szAllPrices[0]);
		if (find(ulIds.begin(), ulIds.end(), uid) == ulIds.end()) {      // build list of uids
			ulIds.push_back(uid);
		}
		ulIdNameMap[uid] = ulIds.size()-1;
		// next record
		retcode = mydb.fetch(false);
	}
	numUl = ulIds.size();



	//******* read underlying prices
	char ulSql[1000];
	// ...form sql joins
	sprintf(ulSql, "%s", "select p0.Date Date"); 
	for (i=0; i<numUl; i++) { sprintf(lineBuffer, "%s%d%s%d", ",p", i, ".price Price",i); strcat(ulSql, lineBuffer); }
	strcat(ulSql, " from prices p0 ");
	if (numUl > 1) { for (i=1; i < numUl; i++) { sprintf(lineBuffer, "%s%d%s", " join prices p", i, " using (Date) "); strcat(ulSql, lineBuffer); } }
	sprintf(lineBuffer, "%s%d%s", " where p0.underlyingId = '", ulIds.at(0), "'"); 
	strcat(ulSql, lineBuffer);
	if (numUl > 1) { for (i=1; i < numUl; i++) { sprintf(lineBuffer, "%s%d%s%d%s", " and p", i, ".underlyingId='", ulIds.at(i), "'"); strcat(ulSql, lineBuffer); } }
	if (numMcIterations>1) { strcat(ulSql, " and Date >='1992-12-31'"); }
	strcat(ulSql," order by Date");
	// ...call DB
	mydb.prepare((SQLCHAR *)ulSql, numUl+1);
	bool   firstTime(true);
	double previousPrice[maxUls];
	boost::gregorian::date lastDate;
	// .. get record <Date,price0,...,pricen>
	retcode = mydb.fetch(true);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		int    numDayDiff;
		boost::gregorian::date bDate(boost::gregorian::from_simple_string(szAllPrices[0]));
		if (!firstTime) {
			boost::gregorian::date_duration dateDiff(bDate - lastDate);
			numDayDiff = dateDiff.days();
		}
		for (i = 0; i < numUl; i++) {
			double thisPrice;
			thisPrice = atof(szAllPrices[i+1]);
			if (!firstTime) {
				ulReturns[i].push_back(thisPrice / previousPrice[i]);				
				for (j = 1; j < numDayDiff;j++){    // pad non-trading days
					ulOriginalPrices.at(i).date.push_back(szAllPrices[0]);
					ulOriginalPrices.at(i).price.push_back(thisPrice);
					ulReturns[i].push_back(1.0);
				}
			}
			previousPrice[i] = thisPrice;	
			ulOriginalPrices.at(i).date.push_back(szAllPrices[0]);
			ulOriginalPrices.at(i).price.push_back(thisPrice);
		}
		// next row
		if(firstTime){ firstTime = false; }

		lastDate  = bDate;
		retcode   = mydb.fetch(false);
	}
	totalNumDays    = ulOriginalPrices.at(0).price.size();
	totalNumReturns = totalNumDays - 1;
	ulPrices        = ulOriginalPrices; // copy constructor called
	vector<double> thesePrices(numUl), startLevels(numUl);
	cerr << "NumPrices:\t" << totalNumDays << "FirstDate:\t" << ulOriginalPrices.at(0).date[0] << endl;


	// get product
	SProduct spr(productId, bProductStartDate, fixedCoupon, couponFrequency);
	numBarriers = 0;
	// get from flat file --KEEP in case of need
	/*
	ifstream pbFile;
	pbFile.open("c:/sites/sppdf/flatfiles/363.txt");
	while (!pbFile.eof()) {
		pbFile.getline(lineBuffer, 1000);
		productBarrierId = atoi(lineBuffer);
		if (productBarrierId != 0) {
			if (productBarrierId != oldProductBarrierId) {
				oldProductBarrierId = productBarrierId;
				pbFile.getline(lineBuffer, 1000);  // productId
				pbFile.getline(lineBuffer, 1000);  capitalOrIncome = atoi(lineBuffer) == 1;
				pbFile.getline(lineBuffer, 1000);  nature = lineBuffer;
				pbFile.getline(lineBuffer, 1000);  payoff = atof(lineBuffer);
				pbFile.getline(lineBuffer, 1000);  // Triggered
				pbFile.getline(lineBuffer, 1000);  settlementDate = lineBuffer;
				pbFile.getline(lineBuffer, 1000);  description    = lineBuffer;
				pbFile.getline(lineBuffer, 1000);  thisPayoffId   = atoi(lineBuffer); thisPayoffType = payoffType[thisPayoffId];// PayoffTypeId
				pbFile.getline(lineBuffer, 1000);  participation = atof(lineBuffer); // Participation
				pbFile.getline(lineBuffer, 1000);  strike        = atof(lineBuffer);  // PayoffStrike
				pbFile.getline(lineBuffer, 1000);  // AvgTenor
				pbFile.getline(lineBuffer, 1000);  // AvgFreq
				pbFile.getline(lineBuffer, 1000);  // AvgType
				pbFile.getline(lineBuffer, 1000);  cap = atof(lineBuffer);  // Cap
				pbFile.getline(lineBuffer, 1000);  // UnderlyingFunctionId
				pbFile.getline(lineBuffer, 1000);  // Param1
				pbFile.getline(lineBuffer, 1000);  // Memory
				pbFile.getline(lineBuffer, 1000);  // IsAbsolute
				spr.barrier.push_back(SpBarrier(capitalOrIncome, nature, payoff, settlementDate, description,
					thisPayoffType, thisPayoffId, strike, cap, participation, ulIdNameMap, bProductStartDate));
				anyInt = spr.barrier.at(numBarriers).getEndDays();
				if (find(monDateIndx.begin(), monDateIndx.end(), anyInt) == monDateIndx.end()) {
					monDateIndx.push_back(anyInt);
				}
				numBarriers += 1;
			}
			// barrier relations
			pbFile.getline(lineBuffer, 1000);  // BarrierRelationId
			pbFile.getline(lineBuffer, 1000);  uid = atoi(lineBuffer);     // UnderlyingId
			pbFile.getline(lineBuffer, 1000);  barrier = atof(lineBuffer); // Barrier
			pbFile.getline(lineBuffer, 1000);  // BarrierTypeId
			pbFile.getline(lineBuffer, 1000);  above = atoi(lineBuffer) == 1; // Above
			pbFile.getline(lineBuffer, 1000);  at = atoi(lineBuffer) == 1; // At
			pbFile.getline(lineBuffer, 1000);  startDateString = lineBuffer;   // StartDate
			pbFile.getline(lineBuffer, 1000);  endDateString = lineBuffer;   // EndDate
			pbFile.getline(lineBuffer, 1000);  // Triggered.1
			pbFile.getline(lineBuffer, 1000);  // IsAbsolute.1
			pbFile.getline(lineBuffer, 1000);  uBarrier = atof(lineBuffer); // UpperBarrier
			pbFile.getline(lineBuffer, 1000);  // Weight
			if (uid) {
				spr.barrier.at(numBarriers - 1).brel.push_back(SpBarrierRelation(uid, barrier, uBarrier, startDateString, endDateString, above, at, productStartDateString));
			}
		}
	}
	pbFile.close();
	*/


	// get barriers from DB
	// table productbarrier
	enum {colProductBarrierId=0,colProductId,
		colCapitalOrIncome, colNature, colPayoff, colTriggered, colSettlementDate, colDescription, colPayoffId, colParticipation,
		colStrike, colAvgTenor, colAvgFreq, colAvgType, colCap,colProductBarrierLast
	};
	// ** SQL fetch block
	sprintf(lineBuffer, "%s%d%s", "select * from productbarrier where ProductId='", productId, "'");
	mydb.prepare((SQLCHAR *)lineBuffer, colProductBarrierLast);
	retcode = mydb.fetch(true);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		capitalOrIncome = atoi(szAllPrices[colCapitalOrIncome]) == 1;
		nature          = szAllPrices[colNature];
		payoff          = atof(szAllPrices[colPayoff]) / 100.0;
		settlementDate  = szAllPrices[colSettlementDate];
		description     = szAllPrices[colDescription];
		thisPayoffId    = atoi(szAllPrices[colPayoffId]); thisPayoffType = payoffType[thisPayoffId];
		participation   = atof(szAllPrices[colParticipation]);
		strike          = atof(szAllPrices[colStrike]);
		cap             = atof(szAllPrices[colCap]);
		spr.barrier.push_back(SpBarrier(capitalOrIncome, nature, payoff, settlementDate, description,
			thisPayoffType, thisPayoffId, strike, cap, participation, ulIdNameMap, bProductStartDate));
		anyInt = spr.barrier.at(numBarriers).getEndDays();
		if (find(monDateIndx.begin(), monDateIndx.end(), anyInt) == monDateIndx.end()) {
			monDateIndx.push_back(anyInt);
		}
		numBarriers += 1;		

		// barrier relations
		// table barrierrelation
		enum {
			brcolBarrierRelationId = 0, brcolProductBarrierId,
			brcolUnderlyingId, brcolBarrier, brcolBarrierTypeId, brcolAbove, brcolAt, brcolStartDate, brcolEndDate,
			brcolTriggered, brcolIsAbsolute, brcolUpperBarrier, brcolWeight,colBarrierRelationLast
		};
		char szbrBarrierRelationId[bufSize], szbrProductBarrierId[bufSize], szbrUnderlyingId[bufSize], szbrBarrier[bufSize], szbrBarrierTypeId[bufSize], szbrAbove[bufSize];
		char szbrAt[bufSize], szbrStartDate[bufSize], szbrEndDate[bufSize], szbrTrigered[bufSize], szbrIsAbsolute[bufSize], szbrUpperBarrier[bufSize], szbrWeight[bufSize];
		// ** SQL fetch block
		sprintf(lineBuffer, "%s%s%s", "select * from barrierrelation where ProductBarrierId='", szAllPrices[colProductBarrierId], "'");
		mydb1.prepare((SQLCHAR *)lineBuffer, colBarrierRelationLast);
		retcode = mydb1.fetch(false);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
			uid              = atoi(szAllPrices[brcolUnderlyingId]);
			barrier          = atof(szAllPrices[brcolBarrier]);
			above            = atoi(szAllPrices[brcolAbove]) == 1;
			at               = atoi(szAllPrices[brcolAt]) == 1;
			startDateString  = szAllPrices[brcolStartDate];
			endDateString    = szAllPrices[brcolEndDate];
			uBarrier         = atof(szAllPrices[brcolUpperBarrier]);
			if (uid) {
				spr.barrier.at(numBarriers - 1).brel.push_back(SpBarrierRelation(uid, barrier, uBarrier, startDateString, endDateString, above, at, productStartDateString));
			}

			// next record
			retcode = mydb1.fetch(false);
		}
		// next record
		retcode = mydb.fetch(false);
	}
	spr.productDays = *max_element(monDateIndx.begin(), monDateIndx.end());
	vector<int> numBarrierHits(numBarriers, 0);
	numMonPoints = monDateIndx.size();



	// evaluate product
	productDays = spr.productDays;
	lastPoint = totalNumDays - productDays;
	// main MC loop
	for (thisIteration = 0; thisIteration < numMcIterations; thisIteration++) {
		// start a product on each date
		for (thisPoint = 0; thisPoint < lastPoint; thisPoint += historyStep) {
			// initialise product
			boost::gregorian::date bStartDate(boost::gregorian::from_simple_string(ulPrices.at(0).date.at(thisPoint)));
			bool   matured;       matured     = false;
			double couponValue;   couponValue = 0.0;
			double thisPayoff; 
	 		vector<double> lookbackLevel;
			// DOME
			for (i = 0; i < numUl; i++) { startLevels[i] = ulPrices.at(i).price.at(thisPoint); }
			for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
				SpBarrier &b(spr.barrier.at(thisBarrier));
				for (uI = 0; uI < b.brel.size(); uI++){
					SpBarrierRelation &thisBrel(b.brel.at(uI));
					thisBrel.setLevels(startLevels[uI]);
				}
			}

			// go through each monitoring date
			for (thisMonIndx = 0; !matured && thisMonIndx < numMonPoints; thisMonIndx++){
				thisMonDays  = monDateIndx.at(thisMonIndx);
				thisMonPoint = thisPoint + thisMonDays;
				const string   thisDateString(ulPrices.at(0).date.at(thisMonPoint));
				for (i = 0; i < numUl; i++) { 
					thesePrices[i] = ulPrices.at(i).price.at(thisMonPoint); 
					//cout << "Price" << i << " :" << thesePrices[i] << endl;  getline(cin, word);
				}
				
				// test each barrier
				for (thisBarrier = 0; !matured && thisBarrier<numBarriers; thisBarrier++){
					SpBarrier &b(spr.barrier.at(thisBarrier));
					// is barrier alive
					if (b.endDays == thisMonDays) {
						// is barrier hit
						if (b.isHit(thesePrices)){
							thisPayoff = b.getPayoff(startLevels, lookbackLevel, thesePrices);
							if (b.capitalOrIncome){ 
								matured     = true; 
								thisPayoff += couponValue; 
								if (spr.couponFrequency.size()) {  // add fixed coupon
									boost::gregorian::date bThisDate(boost::gregorian::from_simple_string(ulPrices.at(0).date.at(thisMonPoint)));
									double daysElapsed   =  (bThisDate - bStartDate).days();
									char   freqChar      = toupper(spr.couponFrequency[1]);
									double couponEvery   = spr.couponFrequency[0] - '0';
									double daysPerEvery  = freqChar == 'D' ? 1 : freqChar == 'M' ? 30 : 360;
									thisPayoff          += spr.fixedCoupon*floor(daysElapsed / daysPerEvery / couponEvery);
								}
							}
							else { couponValue += thisPayoff ; }
							b.storePayoff(thisDateString, thisPayoff);
							//cerr << thisDateString << "\t" << thisBarrier << endl; cout << "Press a key to continue...";  getline(cin, word);
						}
					}
				}
			}
		}

		// create new random sample for next iteration
		for (j = 1; j < totalNumReturns; j++){
			int thisIndx; thisIndx = (int)floor(((double)rand() / (RAND_MAX))*(totalNumReturns - 1));
			for (i = 0; i < numUl; i++) {
				double thisReturn; thisReturn = ulReturns[i][thisIndx];
				ulPrices.at(i).price[j] = ulPrices.at(i).price[j - 1] * thisReturn;
			}
		}
		cout << ".";
	}

	int numAllIterations(0);
	for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
		if (spr.barrier.at(thisBarrier).capitalOrIncome) {
			numAllIterations += spr.barrier.at(thisBarrier).hit.size();
		}
		
	}
	cout << endl;
	for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
		SpBarrier &b(spr.barrier.at(thisBarrier));
		double mean; mean = b.sumPayoffs / b.hit.size();
		double prob; prob = (100.0*b.hit.size()) / numAllIterations;
		/*
		printf("%20s\t%s\t%lf\t%s\t%lf\n", b.description,"Prob:", prob,"ExpectedPayoff",mean);
		*/
		cout << b.description << " Prob:" << prob << " ExpectedPayoff:" << mean << endl;
	}


	// tidy up
	return 0;
}

