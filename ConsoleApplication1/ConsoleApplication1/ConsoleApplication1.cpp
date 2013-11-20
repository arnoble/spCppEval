// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header.h"
using namespace std;



int _tmain(int argc, TCHAR* argv[])
{
	const int maxUls(100);
	const int bufSize(256);
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
	double           anyDouble, barrier, uBarrier, payoff, strike, cap, participation;
	string           productStartDateString,word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate, description;
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

	// open database, get productStartDate
	MyDB  mydb((char **)szAllPrices), mydb1((char **)szAllPrices);
	sprintf(lineBuffer, "%s%d%s", "select StrikeDate from product where ProductId='", productId, "'");
	mydb.prepare((SQLCHAR *)lineBuffer,1);
	retcode = mydb.fetch(true);
	productStartDateString = szAllPrices[0];
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



	// read underlying prices
	char ulSql[1000];
	sprintf(ulSql, "%s", "select p0.Date Date"); // form sql
	for (i=0; i<numUl; i++) { sprintf(lineBuffer, "%s%d%s", ",p", i, ".price "); strcat(ulSql, lineBuffer); }
	strcat(ulSql, " from prices p0 ");
	if (numUl > 1) { for (i=1; i < numUl; i++) { sprintf(lineBuffer, "%s%d%s", " join prices p", i, " using (Date) "); strcat(ulSql, lineBuffer); } }
	sprintf(lineBuffer, "%s%d%s", " where p0.underlyingId = '", ulIds.at(0), "'"); 
	strcat(ulSql, lineBuffer);
	if (numUl > 1) { for (i=1; i < numUl; i++) { sprintf(lineBuffer, "%s%d%s%d%s", " and p", i, ".underlyingId='", ulIds.at(i), "'"); strcat(ulSql, lineBuffer); } }
	if (numMcIterations>1) { strcat(ulSql, " and Date >='1992-12-31'"); }
	strcat(ulSql," order by Date");
	mydb.prepare((SQLCHAR *)ulSql, numUl+1);
	// Get row of data from the result set defined above in the statement
	bool   firstTime(true);
	double previousPrice[100];
	boost::gregorian::date lastDate;
	retcode = mydb.fetch(true);
	// retcode = SQLFetch(hStmt);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		for (i = 0; i < numUl; i++) {
			int    numDayDiff;
			double thisPrice;
			thisPrice = atof(szAllPrices[i+1]);
			// pad non-trading days
			{
				using namespace boost::gregorian;
				date bDate(from_simple_string(szAllPrices[0]));
				if (!firstTime) {
					double thisReturn = thisPrice / previousPrice[i];
					ulReturns[i].push_back(thisReturn);
					date_duration dateDiff(bDate - lastDate);
					numDayDiff = dateDiff.days();
					while (numDayDiff > 1){
						ulOriginalPrices.at(0).date.push_back(word);
						ulOriginalPrices.at(0).price.push_back(thisPrice);
						ulReturns[0].push_back(1.0);
						numDayDiff -= 1;
					}
				}
				else{ firstTime = false; }
				lastDate         = bDate;
				previousPrice[i] = thisPrice;
			}
			ulOriginalPrices.at(i).date.push_back(szAllPrices[0]);
			ulOriginalPrices.at(i).price.push_back(thisPrice);
		}

		// Fetch next row from result set
		retcode = mydb.fetch(false);
	}
	totalNumDays    = ulOriginalPrices.at(0).price.size();
	totalNumReturns = totalNumDays - 1;
	numUl           = ulOriginalPrices.size();
	ulPrices        = ulOriginalPrices; // copy constructor called
	vector<double> thesePrices(numUl), startLevels(numUl);


	// get product
	SProduct spr;
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
		colStrike, colAvgTenor, colAvgFreq, colAvgType, colCap
	};
	// ** SQL fetch block
	sprintf(lineBuffer, "%s%d%s", "select * from productbarrier where ProductId='", productId, "'");
	mydb.prepare((SQLCHAR *)lineBuffer, colCap+1);
	retcode = mydb.fetch(true);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		capitalOrIncome = atoi(szAllPrices[colCapitalOrIncome]) == 1;
		nature          = szAllPrices[colNature];
		payoff          = atof(szAllPrices[colPayoff]) / 100.0;
		settlementDate  = szAllPrices[colSettlementDate];
		description     = szAllPrices[colDescription];
		thisPayoffId    = atoi(szAllPrices[colPayoffId]); thisPayoffType = payoffType[thisPayoffId];// PayoffTypeId
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
			brcolTriggered, brcolIsAbsolute, brcolUpperBarrier, brcolWeight
		};
		char szbrBarrierRelationId[bufSize], szbrProductBarrierId[bufSize], szbrUnderlyingId[bufSize], szbrBarrier[bufSize], szbrBarrierTypeId[bufSize], szbrAbove[bufSize];
		char szbrAt[bufSize], szbrStartDate[bufSize], szbrEndDate[bufSize], szbrTrigered[bufSize], szbrIsAbsolute[bufSize], szbrUpperBarrier[bufSize], szbrWeight[bufSize];
		// ** SQL fetch block
		sprintf(lineBuffer, "%s%s%s", "select * from barrierrelation where ProductBarrierId='", szAllPrices[colProductBarrierId], "'");
		mydb1.prepare((SQLCHAR *)lineBuffer, brcolWeight+1);
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
			bool   matured(false);
			vector<double> lookbackLevel;
			// DOME
			startLevels[0] = ulPrices.at(0).price.at(thisPoint);
			for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
				SpBarrier &b(spr.barrier.at(thisBarrier));
				for (uI = 0; uI < b.brel.size(); uI++){
					SpBarrierRelation &thisBrel(b.brel.at(uI));
					thisBrel.setLevels(startLevels[0]);
				}
			}

			// go through each monitoring date
			for (thisMonIndx = 0; !matured && thisMonIndx < numMonPoints; thisMonIndx++){
				thisMonDays = monDateIndx.at(thisMonIndx);
				thisMonPoint = thisPoint + thisMonDays;
				const string   thisDateString(ulPrices.at(0).date.at(thisMonPoint));
				thesePrices[0] = ulPrices.at(0).price.at(thisMonPoint);
				// test each barrier
				for (thisBarrier = 0; !matured && thisBarrier<numBarriers; thisBarrier++){
					SpBarrier &b(spr.barrier.at(thisBarrier));
					// is barrier alive
					if (b.endDays == thisMonDays) {
						// is barrier hit
						if (b.isHit(thesePrices)){
							matured = b.capitalOrIncome;
							b.storePayoff(thisDateString, b.getPayoff(startLevels, lookbackLevel, thesePrices));
						}
					}
				}
			}
		}

		// create new random sample for next iteration
		for (i = 1; i < totalNumReturns; i++){
			int thisIndx = (int)floor(((double)rand() / (RAND_MAX))*(totalNumReturns - 1));
			double thisReturn = ulReturns[0][thisIndx];
			ulPrices.at(0).price[i] = ulPrices.at(0).price[i - 1] * thisReturn;
		}
		cout << ".";
	}

	int numAllIterations(0);
	for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
		numAllIterations += spr.barrier.at(thisBarrier).hit.size();
	}
	cout << endl;
	for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
		SpBarrier &b(spr.barrier.at(thisBarrier));
		double mean = b.sumPayoffs / b.hit.size();
		double prob = (100.0*b.hit.size()) / numAllIterations;
		/*
		printf("%20s\t%s\t%lf\t%s\t%lf\n", b.description,"Prob:", prob,"ExpectedPayoff",mean);
		*/
		cout << b.description << " Prob:" << prob << " ExpectedPayoff:" << mean << endl;
	}






	// tidy up
	return 0;
}

