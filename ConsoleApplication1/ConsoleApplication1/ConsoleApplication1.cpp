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
	int              anyInt, i, j, k, len, callOrPut, thisPoint, thisBarrier, thisMonIndx, thisMonPoint, numUl, numMonPoints, lastPoint, productDays, totalNumDays, totalNumReturns, uid,numBrel;
	int              anyTypeId,thisPayoffId, thisMonDays;
	double           anyDouble, barrier, uBarrier, payoff, strike, cap, participation,fixedCoupon,AMC;
	string           couponFrequency,productStartDateString, word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate, description;
	char             lineBuffer[1000], charBuffer[1000];
	bool             found,capitalOrIncome, above, at;
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


	// open database
	MyDB  mydb((char **)szAllPrices), mydb1((char **)szAllPrices);

	// get general info
	// ...productType
	vector<MapType> productTypes;
	sprintf(lineBuffer, "%s", "select * from producttype"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		MapType x(atoi(szAllPrices[0]),szAllPrices[1]); productTypes.push_back(x);
		retcode = mydb.fetch(false);
	}
	// ...barrierType
	vector<MapType> barrierTypes;
	sprintf(lineBuffer, "%s", "select * from barriertype"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		MapType x(atoi(szAllPrices[0]), szAllPrices[1]); barrierTypes.push_back(x);
		retcode = mydb.fetch(false);
	}


	// get product table data
	enum {
		colProductCounterpartyId = 2, colProductStrikeDate = 6, colProductFixedCoupon = 28, colProductFrequency, colProductBid, colProductAMC = 43, colProductDepositGtee = 56, colProductLast
	};
	sprintf(lineBuffer, "%s%d%s", "select * from product where ProductId='", productId, "'");
	mydb.prepare((SQLCHAR *)lineBuffer, colProductLast);
	retcode = mydb.fetch(true);
	int counterpartyId     = atoi(szAllPrices[colProductCounterpartyId]); 
	bool depositGteed      = atoi(szAllPrices[colProductDepositGtee])  == 1;
	productStartDateString =      szAllPrices[colProductStrikeDate];
	fixedCoupon            = atof(szAllPrices[colProductFixedCoupon]);
	AMC                    = atof(szAllPrices[colProductAMC]);
	if (strlen(szAllPrices[colProductFrequency])){ couponFrequency = szAllPrices[colProductFrequency];	}
	boost::gregorian::date  bProductStartDate(boost::gregorian::from_simple_string(productStartDateString));

	// get counterparty info
	// ...mult-issuer product's have comma-separated issuers...ASSUMED equal weight
	sprintf(lineBuffer, "%s%d%s", "select EntityName from institution where institutionid='",counterpartyId,"' ");
	mydb.prepare((SQLCHAR *)lineBuffer, 1);
	retcode = mydb.fetch(true);
	string counterpartyName = szAllPrices[0];
	vector<string> counterpartyNames;
	splitCounterpartyName(counterpartyNames, counterpartyName);
	sprintf(charBuffer, "%s%s%s", "'", counterpartyNames.at(0).c_str(), "'");
	for (i = 1; i < counterpartyNames.size(); i++){
		sprintf(charBuffer, "%s%s%s%s",charBuffer, ",'", counterpartyNames.at(i).c_str(), "'");
	}
	sprintf(lineBuffer, "%s%s%s", "select Maturity,avg(Spread) Spread from cdsspread join institution using (institutionid) where EntityName in (",charBuffer,
		") and spread is not null group by Maturity order by Maturity");

	mydb.prepare((SQLCHAR *)lineBuffer, 2);
	retcode = mydb.fetch(false);
	vector<double> cdsTenor, cdsSpread;
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		cdsTenor.push_back( atof(szAllPrices[0]));
		cdsSpread.push_back(atof(szAllPrices[1])/10000.0);
		retcode = mydb.fetch(false);
	}


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
	boost::gregorian::date  bLastDataDate(boost::gregorian::from_simple_string(ulOriginalPrices.at(0).date[totalNumDays-1]));
	cerr << "NumPrices:\t" << totalNumDays << "FirstDataDate:\t" << ulOriginalPrices.at(0).date[0] << endl;
	int daysExtant = (bLastDataDate - bProductStartDate).days(); if (daysExtant < 0){ daysExtant = 0; }

	// create product
	SProduct spr(productId, bProductStartDate, fixedCoupon, couponFrequency, AMC, depositGteed, daysExtant);
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
			tenorPeriodDays, avgFreq, isMemory, daysExtant, bProductStartDate));
		SpBarrier &thisBarrier(spr.barrier.at(numBarriers));
		// update monitoring dates
		// DOME: for now only use endDates, as all American barriers are detected below as extremum bariers
		anyInt = thisBarrier.getEndDays();
		if (find(monDateIndx.begin(), monDateIndx.end(), anyInt) == monDateIndx.end()) {
			monDateIndx.push_back(anyInt);
		}

		// barrier relations
		// table barrierrelation
		enum {
			brcolBarrierRelationId = 0, brcolProductBarrierId,
			brcolUnderlyingId, brcolBarrier, brcolBarrierTypeId, brcolAbove, brcolAt, brcolStartDate, brcolEndDate,
			brcolTriggered, brcolIsAbsolute, brcolUpperBarrier, brcolWeight,colBarrierRelationLast
		};
		// ** SQL fetch block
		sprintf(lineBuffer, "%s%d%s", "select * from barrierrelation where ProductBarrierId='", barrierId, "'");
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
			anyTypeId        = atoi(szAllPrices[brcolBarrierTypeId]);
			// get barrierType name
			for (found = false,i = 0; !found && i < barrierTypes.size(); i++){ if (anyTypeId == barrierTypes[i].id) { found = true; } }
			if (found && barrierTypes.at(i-1).name != "continuous") { thisBarrier.isContinuous = false; }

			if (uid) {
				thisBarrier.brel.push_back(SpBarrierRelation(uid, barrier, uBarrier, startDateString, endDateString, 
					above, at, weight, daysExtant, productStartDateString));
			}
			// next barrierRelation record
			retcode = mydb1.fetch(false);
		}

		// detect extremumBarriers
		// DOME: for now ANY barrier with a brel which has different Start/End dates
		// NOTE: different brels can have different start dates
		// DOME:  need to check all brels have the same endDate
		bool isExtremumBarrier = false;
		for (i = 0; i < thisBarrier.brel.size(); i++) {
			if (thisBarrier.brel[i].startDate != thisBarrier.brel[i].endDate) {
				isExtremumBarrier = true;
			}
		}
		thisBarrier.isExtremum = isExtremumBarrier;


		// next barrier record
		numBarriers += 1;
		retcode = mydb.fetch(false);
	}
	spr.productDays = *max_element(monDateIndx.begin(), monDateIndx.end());
	vector<int> numBarrierHits(numBarriers, 0);
	numMonPoints = monDateIndx.size();
	double maxYears = 0; for (i = 0; i<numBarriers; i++) { double t = spr.barrier.at(i).yearsToBarrier;   if (t > maxYears){ maxYears = t; } }
	vector<double> fullCurve; // populate a full annual CDS curve
	for (j = 0; j<maxYears + 1; j++) {
		fullCurve.push_back(interpCurve(cdsTenor,cdsSpread, j + 1));
	}
	vector<double> dpCurve, hazardCurve; // annual default probability curve
	const double recoveryRate(0.4);
	bootstrapCDS(fullCurve, dpCurve, recoveryRate);
	for (j = 0, len = fullCurve.size(); j<len; j++) {
		hazardCurve.push_back(dpCurve[j]);
	}

	// deal with any accruals/moneyness/averaging
	if (daysExtant){
	
	}

	// finally evaluate the product
	spr.evaluate(totalNumDays, numMcIterations, historyStep, ulPrices, ulReturns, 
	numBarriers, numUl, ulIdNameMap, monDateIndx, recoveryRate, hazardCurve,mydb);
	// tidy up
	return 0;
}

