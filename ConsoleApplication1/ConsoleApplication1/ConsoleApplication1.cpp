// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header.h"
using namespace std;




int _tmain(int argc, TCHAR* argv[])
{
	// initialise
	int              historyStep     = 1;
	int              productId       = argc > 1 ? _ttoi(argv[1]) : 363;
	int              numMcIterations = argc>2 ? _ttoi(argv[2]) : 100;
	const int        maxUls(100);
	const int        bufSize(1000);
	SQLHENV          hEnv       = NULL;		    // Env Handle from SQLAllocEnv()
	SQLHDBC          hDBC       = NULL;         // Connection handle
	SQLLEN           cbModel;		            // SQL buffer bytes recieved
	RETCODE          retcode;
	SQLCHAR*         thisSQL;
	char             szDate[bufSize];
	char             szPrice[bufSize], **szAllPrices = new char*[maxUls];
	unsigned	     uI;
	int              oldProductBarrierId = 0, productBarrierId = 0;
	int              numBarriers = 0, thisIteration = 0;
	int              anyInt, i, j, k, len, callOrPut, thisPoint, thisBarrier, thisMonIndx, thisMonPoint, numUl, numMonPoints, lastPoint, productDays, totalNumDays, totalNumReturns, uid;
	int              thisPayoffId, thisMonDays;
	double           anyDouble, barrier, uBarrier, payoff, strike, cap, participation,fixedCoupon,AMC;
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
	srand(time(0)); // reseed rand
	cout << "Iterations:" << numMcIterations << " ProductId:" << productId << endl;
	// cout << "Press a key to continue...";  getline(cin, word);  // KEEP in case you want to attach debugger


	// open database, get product table data
	MyDB  mydb((char **)szAllPrices), mydb1((char **)szAllPrices);
	enum {
		colProductStrikeDate = 6, colProductFixedCoupon = 28, colProductFrequency,colProductBid,colProductAMC=43,colProductLast
	};
	sprintf(lineBuffer, "%s%d%s", "select * from product where ProductId='", productId, "'");
	mydb.prepare((SQLCHAR *)lineBuffer, colProductLast);
	retcode = mydb.fetch(true);
	productStartDateString =      szAllPrices[colProductStrikeDate];
	fixedCoupon            = atof(szAllPrices[colProductFixedCoupon]);
	AMC                    = atof(szAllPrices[colProductAMC]);
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
					ulOriginalPrices.at(i).nonTradingDay.push_back(true);
					ulReturns[i].push_back(1.0);
				}
			}
			previousPrice[i] = thisPrice;	
			ulOriginalPrices.at(i).date.push_back(szAllPrices[0]);
			ulOriginalPrices.at(i).price.push_back(thisPrice);
			ulOriginalPrices.at(i).nonTradingDay.push_back(false);

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
	SProduct spr(productId, bProductStartDate, fixedCoupon, couponFrequency,AMC);
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
	enum {colProductBarrierId=0,colProductId,
		colCapitalOrIncome, colNature, colPayoff, colTriggered, colSettlementDate, colDescription, colPayoffId, colParticipation,
		colStrike, colAvgTenor, colAvgFreq, colAvgType, colCap,colUnderlyingFunctionId,colParam1,colMemory,colIsAbsolute,colProductBarrierLast
	};
	// ** SQL fetch block
	sprintf(lineBuffer, "%s%d%s", "select * from productbarrier where ProductId='", productId, "'");
	mydb.prepare((SQLCHAR *)lineBuffer, colProductBarrierLast);
	retcode = mydb.fetch(true);
	map<char, int> avgTenor; avgTenor['d'] = 1; avgTenor['w'] = 7; avgTenor['m'] = 30; avgTenor['q'] = 91; avgTenor['y'] = 365;
	map<char,int>::iterator curr, end;
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		int tenorPeriodDays;
		int avgDays   = 0; 
		int avgFreq   = 0;
		bool isMemory = atoi(szAllPrices[colMemory]) == 1;;
		if (strlen(szAllPrices[colAvgTenor])){
			char avgChar1   = szAllPrices[colAvgTenor][0];
			char avgChar2   = szAllPrices[colAvgTenor][1];
			bool foundTenor = false;
			avgFreq         = (avgChar1 - '0');
			for (curr = avgTenor.begin(), end = avgTenor.end(); !foundTenor && curr != end; curr++)
			if (curr->first == avgChar2){ foundTenor = true; tenorPeriodDays = curr->second; }
			avgDays = avgFreq * tenorPeriodDays;
		}
		int barrierId   = atoi(szAllPrices[colProductBarrierId]);
		int avgType     = atoi(szAllPrices[colAvgType]); 
		capitalOrIncome = atoi(szAllPrices[colCapitalOrIncome]) == 1;
		nature          = szAllPrices[colNature];
		payoff          = atof(szAllPrices[colPayoff]) / 100.0;
		settlementDate  = szAllPrices[colSettlementDate];
		description     = szAllPrices[colDescription];
		thisPayoffId    = atoi(szAllPrices[colPayoffId]); thisPayoffType = payoffType[thisPayoffId];
		participation   = atof(szAllPrices[colParticipation]);
		strike          = atof(szAllPrices[colStrike]);
		cap             = atof(szAllPrices[colCap]);
		spr.barrier.push_back(SpBarrier(barrierId,capitalOrIncome, nature, payoff, settlementDate, description,
			thisPayoffType, thisPayoffId, strike, cap, participation, ulIdNameMap,avgDays,avgType,
			tenorPeriodDays,avgFreq,isMemory,bProductStartDate));
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
			double weight    = atof(szAllPrices[brcolWeight]);
			uid              = atoi(szAllPrices[brcolUnderlyingId]);
			barrier          = atof(szAllPrices[brcolBarrier]);
			above            = atoi(szAllPrices[brcolAbove]) == 1;
			at               = atoi(szAllPrices[brcolAt]) == 1;
			startDateString  = szAllPrices[brcolStartDate];
			endDateString    = szAllPrices[brcolEndDate];
			uBarrier         = atof(szAllPrices[brcolUpperBarrier]);
			if (uid) {
				spr.barrier.at(numBarriers - 1).brel.push_back(SpBarrierRelation(uid, barrier, uBarrier, startDateString, endDateString, above, at, weight,productStartDateString));
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
		// start a product on each TRADING date
		for (thisPoint = 0; thisPoint < lastPoint; thisPoint += historyStep) {
			// wind forwards to next trading date
			while (ulPrices.at(0).nonTradingDay.at(thisPoint) && thisPoint < lastPoint) {
				thisPoint += 1;
			}
			// initialise product
			vector<bool> barrierWasHit(numBarriers);
			boost::gregorian::date bStartDate(boost::gregorian::from_simple_string(ulPrices.at(0).date.at(thisPoint)));
			bool   matured;       matured     = false;
			double couponValue;   couponValue = 0.0;
			double thisPayoff; 
	 		vector<double> lookbackLevel;
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
						// do any averaging/lookback
						int proportionHits = 1;
						// DOME - cater for extremum barriers, where typically averaging does not apply to barrier hit test
						// maybe replace thesePrices with their averages
						b.doAveraging(thesePrices, ulPrices,thisMonPoint);
						// is barrier hit
						if (b.isHit(thesePrices)){
							barrierWasHit[thisBarrier] = true;
							thisPayoff = b.getPayoff(startLevels, lookbackLevel, thesePrices,spr.AMC);
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
							else { 
								couponValue               += thisPayoff ; 
								barrierWasHit[thisBarrier] = true;
								if (b.isMemory) {
									for (k = 0; k<thisBarrier; k++) {
										SpBarrier &bOther(spr.barrier.at(k));
										if (!bOther.capitalOrIncome && !barrierWasHit[k]) {
											double payoffOther = bOther.payoff;
											barrierWasHit[k]   = true; 
											couponValue       += payoffOther ;
											bOther.storePayoff(thisDateString, proportionHits*payoffOther);
										}
									}
								}

							}
							b.storePayoff(thisDateString, proportionHits*thisPayoff);
							//cerr << thisDateString << "\t" << thisBarrier << endl; cout << "Press a key to continue...";  getline(cin, word);
						}
						else {
							// in case you want to see why not hit
							// b.isHit(thesePrices);
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

	int numAllEpisodes(0);
	for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
		if (spr.barrier.at(thisBarrier).capitalOrIncome) {
			numAllEpisodes += spr.barrier.at(thisBarrier).hit.size();
		}
		
	}
	cout << endl;

	// *****************
	// ** handle results
	// *****************
	bool     applyCredit        = false;
	double   projectedReturn    = (numMcIterations == 1 ? (applyCredit ? 0.05 : 0.0) : (applyCredit ? 0.02 : 1.0));
	string   lastSettlementDate = spr.barrier.at(numBarriers - 1).settlementDate;
	bool     foundEarliest      = false;
	double   probEarly(0.0),probEarliest(0.0);
	double   midPrice(1.0);        // DOME
	vector<double> allPayoffs,allAnnRets;
	int    numPosPayoffs(0),   numStrPosPayoffs(0),   numNegPayoffs(0);
	double sumPosPayoffs(0),   sumStrPosPayoffs(0),   sumNegPayoffs(0);
	double sumPosDurations(0), sumStrPosDurations(0), sumNegDurations(0);

	// ** process barrier results
	double sumPayoffs(0.0), sumAnnRets(0.0), sumDuration(0.0);
	int    numCapitalInstances(0);
	for (thisBarrier = 0; thisBarrier < numBarriers; thisBarrier++){
		SpBarrier &b(spr.barrier.at(thisBarrier));
		int    numInstances = b.hit.size();
		double thisYears    = b.yearsToBarrier;
		double mean         = b.sumPayoffs / numInstances;
		double prob         = (1.0*numInstances) / numAllEpisodes;
		double annReturn    = numInstances ? (exp(log(((b.capitalOrIncome?0.0:1.0)+mean) / midPrice) / b.yearsToBarrier) - 1.0) : 0.0;
		sumDuration        += numInstances*b.yearsToBarrier;

		if (b.capitalOrIncome) {
			if (!foundEarliest){ foundEarliest = true; probEarliest = prob; }
			if (b.settlementDate < lastSettlementDate) probEarly   += prob;
			numCapitalInstances += numInstances;
			sumPayoffs          += b.sumPayoffs;
			for (i = 0; i < b.hit.size();i++){
				double thisAmount = b.hit.at(i).amount;
				double thisAnnRet = exp(log(thisAmount / midPrice) / thisYears) - 1.0;
				allPayoffs.push_back(thisAmount);	
				allAnnRets.push_back(thisAnnRet);
				sumAnnRets += thisAnnRet;
				if (thisAmount >  1.0) { sumStrPosPayoffs += thisAmount; numStrPosPayoffs++;    sumStrPosDurations += thisYears; }
				if (thisAmount >= 1.0) { sumPosPayoffs    += thisAmount; numPosPayoffs++;       sumPosDurations    += thisYears; }
				else                   { sumNegPayoffs    += thisAmount; numNegPayoffs++;       sumNegDurations    += thisYears; }
			}
		}
		cout << b.description << " Prob:" << prob << " ExpectedPayoff:" << mean << endl;
		// ** SQL barrierProb
		sprintf(lineBuffer, "%s%.5lf%s%.5lf%s%.5lf%s%d%s%d%s%.2lf%s", "update testbarrierprob set Prob='",prob,
			"',AnnReturn='",annReturn,
			"',CondPayoff='", mean,
			"',NumInstances='", numInstances,
			"' where ProductBarrierId='", spr.barrier.at(thisBarrier).barrierId, "' and ProjectedReturn='", projectedReturn,"'");
		mydb.prepare((SQLCHAR *)lineBuffer, 1);
		retcode = mydb.execute(true);
	}
	
	// ** process product results
	int numAnnRets(allAnnRets.size());
	const double confLevel(0.1);
	sort(allPayoffs.begin(), allPayoffs.end());
	sort(allAnnRets.begin(), allAnnRets.end()); 
	double averageReturn = sumAnnRets / numAnnRets;
	double vaR95         = 100.0*allPayoffs[floor(numAnnRets*0.05)];

	// eShortfall, esVol
	int numShortfall(floor(confLevel*allAnnRets.size()));
	double eShortfall(0.0);	for (i = 0; i < numShortfall; i++){ eShortfall += allAnnRets[i]; }	eShortfall  /= numShortfall;
	double duration  = sumDuration / numAnnRets;
	double esVol     =  (log(1 + averageReturn) - log(1 + eShortfall)) / ESnorm(.1);
	double scaledVol = esVol * sqrt(duration);
	double geomReturn(0.0);	for (i = 0; i < numAnnRets; i++){ geomReturn += log(allPayoffs[i]); }
	geomReturn = exp(geomReturn / sumDuration) - 1;
	double sharpeRatio = scaledVol > 0.0 ? (geomReturn / scaledVol>1000.0 ? 1000.0 : geomReturn / scaledVol) : 1000.0;
	vector<double> cesrBuckets = { 0.0, 0.005, .02, .05, .1, .15, .25, .4 };
	double riskCategory(1.0);  for (i = 1, len = cesrBuckets.size(); i<len && scaledVol>cesrBuckets[i]; i++) { riskCategory += 1.0; }
	if (i != len) riskCategory += (scaledVol - cesrBuckets[i - 1]) / (cesrBuckets[i] - cesrBuckets[i - 1]);
	// WinLose
	double sumNegRet(0.0), sumPosRet(0.0), sumStrPosRet(0.0);
	int    numNegRet(0),   numPosRet(0),   numStrPosRet(0);
	for (j = 0; j<numAnnRets; j++) {
		double ret = allAnnRets[j];
		if (ret>0){ sumStrPosRet += ret; numStrPosRet++; }
		if (ret<0){ sumNegRet    += ret; numNegRet++; }
		else {      sumPosRet    += ret; numPosRet++; }
	}
	double strPosDuration(sumStrPosDurations / numStrPosPayoffs), posDuration(sumPosDurations / numPosPayoffs), negDuration(sumNegDurations / numNegPayoffs);
	double ecGain         =  100.0*(numPosPayoffs    ? exp(log(sumPosPayoffs    / midPrice / numPosPayoffs    ) / posDuration   ) - 1.0 : 0.0);
	double ecStrictGain   =  100.0*(numStrPosPayoffs ? exp(log(sumStrPosPayoffs / midPrice / numStrPosPayoffs) / strPosDuration) - 1.0 : 0.0);
	double ecLoss         = -100.0*(numNegPayoffs ? exp(log(sumNegPayoffs / midPrice / numNegPayoffs) / negDuration) - 1.0 : 0.0);
	double probGain       = numPosRet    ? ((double)numPosRet)    / numAnnRets : 0;
	double probStrictGain = numStrPosRet ? ((double)numStrPosRet) / numAnnRets : 0;
	double probLoss       = 1 - probGain;
	double eGainRet       = ecGain * probGain;
	double eLossRet       = ecLoss * probLoss;
	double winLose        = sumNegRet ? (eGainRet / eLossRet>1000.0 ? 1000.0 : eGainRet / eLossRet) : 1000.0;

	sprintf(lineBuffer, "%s%.5lf", "update testcashflows set ExpectedPayoff='", sumPayoffs / numAnnRets);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ExpectedReturn='", geomReturn);
	sprintf(lineBuffer, "%s%s%s",    lineBuffer, "',FirstDataDate='",  ulOriginalPrices.at(0).date[0].c_str());
	sprintf(lineBuffer, "%s%s%s",    lineBuffer, "',LastDataDate='",   ulOriginalPrices.at(0).date[totalNumDays-1].c_str());
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',SharpeRatio='",    sharpeRatio);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',RiskCategory='",   riskCategory);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',WinLose='",        winLose);
	time_t rawtime;	struct tm * timeinfo;  time(&rawtime);	timeinfo = localtime(&rawtime);
	strftime(charBuffer, 80, "%Y-%m-%d", timeinfo);
	sprintf(lineBuffer, "%s%s%s",    lineBuffer, "',WhenEvaluated='",  charBuffer);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbEarliest='",   probEarliest);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ProbEarly='",      probEarly);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',VaR='",            vaR95);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ESvol='",          esVol);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',Duration='",       duration);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',NumResamples='",   numMcIterations);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecGain='",         ecGain);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecStrictGain='",   ecStrictGain);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',ecLoss='",         ecLoss);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probGain='",       probGain);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probStrictGain='", probStrictGain);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',probLoss='",       probLoss);
	sprintf(lineBuffer, "%s%s%.5lf", lineBuffer, "',eShortfall='",     eShortfall*100.0);
	sprintf(lineBuffer, "%s%s%d",    lineBuffer, "',NumEpisodes='",    numAllEpisodes);

	sprintf(lineBuffer, "%s%s%d%s%.2lf%s", lineBuffer, "' where ProductId='", productId, "' and ProjectedReturn='", projectedReturn, "'");
	
	mydb.prepare((SQLCHAR *)lineBuffer, 1);
	retcode = mydb.execute(true);
	
	
	

	// tidy up
	return 0;
}

