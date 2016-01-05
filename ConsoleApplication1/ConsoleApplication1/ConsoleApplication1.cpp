// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header.h"
using namespace std;




int _tmain(int argc, _TCHAR* argv[])
{
	size_t numChars;
	try{
		// initialise
		if (argc < 3){ cout << "Usage: startId stopId (or a comma-separated list) numIterations <optionalArguments: 'doFAR' 'debug' 'priips' 'proto' 'dbServer:'spCloud|newSp|spIPRL   'forceIterations'  'historyStep:'nnn 'startDate:'YYYY-mm-dd 'endDate:'YYYY-mm-dd 'minSecsTaken:'nnn  'maxSecsTaken:'nnn >" << endl;  exit(0); }
		int              historyStep = 1, minSecsTaken=0, maxSecsTaken=0;
		int              commaSepList   = strstr(WcharToChar(argv[1], &numChars),",") ? 1:0;
		int              startProductId ;
		int              stopProductId ; 
		int              numMcIterations = argc > 3 - commaSepList ? _ttoi(argv[3 - commaSepList]) : 100;
		bool             doFinalAssetReturn(false), forceIterations(false), doDebug(false), doPriips(false);
		char             lineBuffer[1000], charBuffer[1000];
		char             startDate[11]      = "";
		char             endDate[11]        = "";
		char             useProto[6]        = "";
		map<char, int>   avgTenor; avgTenor['d'] = 1; avgTenor['w'] = 7; avgTenor['m'] = 30; avgTenor['q'] = 91; avgTenor['y'] = 365;
		char dbServer[100]; strcpy(dbServer, "newSp");  // on local PC: newSp for local, spIPRL for IXshared        on IXcloud: spCloud

		// build list of productIds
		if (!commaSepList == 1) {
			startProductId = argc > 1 ? _ttoi(argv[1]) : 363;
			stopProductId  = argc > 2 ? _ttoi(argv[2]) : 363;
		}
		// process optional argumants
		for (int i=4 - commaSepList; i<argc; i++){
			char *thisArg  = WcharToChar(argv[i], &numChars);
			if (strstr(thisArg, "forceIterations"   )){ forceIterations    = true; }
			if (strstr(thisArg, "priips"            )){ doPriips           = true; }
			if (strstr(thisArg, "proto"             )){ strcpy(useProto,"proto"); }
			if (strstr(thisArg, "doFAR"             )){ doFinalAssetReturn = true; }
			if (strstr(thisArg, "debug"             )){ doDebug            = true; }
			if (sscanf(thisArg, "startDate:%s",  lineBuffer)){ strcpy(startDate, lineBuffer); }
			else if (sscanf(thisArg, "endDate:%s",      lineBuffer)){ strcpy(endDate,   lineBuffer); }
			else if (sscanf(thisArg, "dbServer:%s",     lineBuffer)){ strcpy(dbServer,  lineBuffer); }
			else if (sscanf(thisArg, "minSecsTaken:%s", lineBuffer)){ minSecsTaken  = atoi(lineBuffer); }
			else if (sscanf(thisArg, "maxSecsTaken:%s", lineBuffer)){ maxSecsTaken  = atoi(lineBuffer); }
			else if (sscanf(thisArg, "historyStep:%s",  lineBuffer)){ historyStep   = atoi(lineBuffer); }
		}
		if (doPriips){
			if (strlen(startDate)){
				doPriips = false;
				cout << "Will not do PRIIPs as you have entered a startDate" << endl;
			}
		}
		const int        maxUls(100);
		const int        bufSize(1000);
		RETCODE          retcode;
		SomeCurve        anyCurve;
		time_t           startTime;
		char             **szAllPrices = new char*[maxUls];
		vector<int>      allProductIds; allProductIds.reserve(1000);
		vector<string>   payoffType ={ "", "fixed", "call", "put", "twinWin", "switchable", "basketCall", "lookbackCall", "lookbackPut", "basketPut", 
			"basketCallQuanto", "basketPutQuanto","cappuccino" };
		vector<int>::iterator intIterator, intIterator1;
		for (int i = 0; i < maxUls; i++){
			szAllPrices[i] = new char[bufSize];
		}
		srand(time(0)); // reseed rand
	
		// open database
		MyDB  mydb((char **)szAllPrices, dbServer), mydb1((char **)szAllPrices, dbServer);

		// get list of productIds
		if (commaSepList == 1) {
			sprintf(charBuffer, "%s%s%s", " where p.ProductId in (", WcharToChar(argv[1], &numChars),") ");
		} else {
			sprintf(charBuffer, "%s%d%s%d%s", " where p.ProductId >= '", startProductId, "' and p.ProductId <= '", stopProductId, "'");
		}
		sprintf(lineBuffer, "%s%s%s%s%s%s%s", "select p.ProductId from ", useProto, "product p join ", useProto, "cashflows c using (ProductId) ", charBuffer, " and Matured='0' and ProjectedReturn=1 ");
		if (minSecsTaken){
			sprintf(lineBuffer, "%s%s%d",lineBuffer, " and SecsTaken>=", minSecsTaken);
		}
		if (maxSecsTaken){
			sprintf(lineBuffer, "%s%s%d%s", lineBuffer, " and (WhenEvaluated is null or SecsTaken<=", maxSecsTaken,")");
		}
		
		mydb.prepare((SQLCHAR *)lineBuffer, 1); 	retcode = mydb.fetch(true);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			int x(atoi(szAllPrices[0])); allProductIds.push_back(x);
			retcode = mydb.fetch(false);
		}
		// cerr << "Doing:" << allProductIds.size() << " products " << lineBuffer << endl;

		// loop through each product
		int numProducts = allProductIds.size();
		for (int productIndx = 0; productIndx < numProducts; productIndx++) {
			int              oldProductBarrierId = 0, productBarrierId = 0;
			int              numBarriers = 0, thisIteration = 0;
			int              i, j, len, len1, anyInt, anyInt1, numUl, numMonPoints,totalNumDays, totalNumReturns, uid;
			int              productId, anyTypeId, thisPayoffId;
			double           anyDouble, maxBarrierDays,barrier, uBarrier, payoff, strike, cap, participation, fixedCoupon, AMC, productShapeId, issuePrice, bidPrice, askPrice, midPrice;
			string           productShape,couponFrequency, productStartDateString, productCcy,word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate, 
				description, avgInAlgebra, productTimepoints, productPercentiles,fairValueDateString,lastDataDateString;
			bool             capitalOrIncome, above, at;
			vector<int>      monDateIndx, accrualMonDateIndx;
			vector<UlTimeseries>  ulOriginalPrices(maxUls), ulPrices(maxUls); // underlying prices	

			// init
			maxBarrierDays = 0.0;
			startTime      = time(0);
			productId      = allProductIds.at(productIndx);
		
			// get general info:  productType, productShape, barrierType
			// ...productType
			map<int, string> productTypeMap;
			sprintf(lineBuffer, "%s", "select * from producttype"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				productTypeMap[atoi(szAllPrices[0])] = szAllPrices[1];
				retcode = mydb.fetch(false);
			}
			// ...productShape
			map<int, string> productShapeMap;
			sprintf(lineBuffer, "%s", "select * from productshape"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				productShapeMap[atoi(szAllPrices[0])] = szAllPrices[1];
				retcode = mydb.fetch(false);
			}


			// ...barrierType
			map<int, string> barrierTypeMap;
			sprintf(lineBuffer, "%s", "select * from barriertype"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				barrierTypeMap[atoi(szAllPrices[0])] = szAllPrices[1];
				retcode = mydb.fetch(false);
			}

			// get product table data
			enum {
				colProductCounterpartyId = 2, colProductStrikeDate = 6, colProductCcy = 14, colProductFixedCoupon = 28, colProductFrequency, colProductBid, colProductAsk,
				colProductAMC = 43, colProductShapeId,colProductMaxIterations=55, colProductDepositGtee, colProductDealCheckerId, colProductAssetTypeId, colProductIssuePrice, 
				colProductCouponPaidOut, colProductCollateralised, colProductCurrencyStruck, colProductBenchmarkId, colProductHurdleReturn, colProductBenchmarkTER,
				colProductTimepoints, colProductPercentiles, colProductDoTimepoints, colProductDoPaths, colProductStalePrice, colProductFairValue, colProductFairValueDate, colProductLast
			};
			sprintf(lineBuffer, "%s%s%s%d%s", "select * from ", useProto, "product where ProductId='", productId, "'");
			mydb.prepare((SQLCHAR *)lineBuffer, colProductLast);
			retcode = mydb.fetch(true);
			int  thisNumIterations  = forceIterations ? numMcIterations : atoi(szAllPrices[colProductMaxIterations]);
			if (numMcIterations < thisNumIterations){ thisNumIterations = numMcIterations; }
			if (thisNumIterations<1)                { thisNumIterations = 1; }
			int  counterpartyId     = atoi(szAllPrices[colProductCounterpartyId]);
			bool depositGteed       = atoi(szAllPrices[colProductDepositGtee]   ) == 1;
			bool couponPaidOut      = atoi(szAllPrices[colProductCouponPaidOut] ) == 1;
			bool collateralised     = atoi(szAllPrices[colProductCollateralised]) == 1;
			bool currencyStruck     = atoi(szAllPrices[colProductCurrencyStruck]) == 1;
			bool doTimepoints       = atoi(szAllPrices[colProductDoTimepoints]  ) == 1;
			bool doPaths            = atoi(szAllPrices[colProductDoPaths]       ) == 1;
			if ((doPaths || doTimepoints) && (numProducts > 1)){
				doTimepoints = false;
				doPaths      = false;
				cout << "We only doTimepoints/paths if #products is 1 ... if you need timepoints/paths please do each product singly as it can overburden the database..thanks" << endl;
			};
			bool stalePrice         = atoi(szAllPrices[colProductStalePrice]) == 1;
			double fairValuePrice   = atof(szAllPrices[colProductFairValue]);
			fairValueDateString     = szAllPrices[colProductFairValueDate];
			int  benchmarkId        = atoi(szAllPrices[colProductBenchmarkId]);
			double hurdleReturn     = atof(szAllPrices[colProductHurdleReturn])/100.0;
			double contBenchmarkTER = -log(1.0 - atof(szAllPrices[colProductHurdleReturn]) / 100.0);

			productStartDateString  = szAllPrices[colProductStrikeDate];
			productCcy              = szAllPrices[colProductCcy];
			fixedCoupon             = atof(szAllPrices[colProductFixedCoupon]);
			bidPrice                = atof(szAllPrices[colProductBid]);
			askPrice                = atof(szAllPrices[colProductAsk]);
			AMC                     = atof(szAllPrices[colProductAMC]);
			productShapeId          = atoi(szAllPrices[colProductShapeId]);
			productShape			= productShapeMap[productShapeId];
			// assemble timepoints request
			productTimepoints		= szAllPrices[colProductTimepoints];
			vector<string> timepoints;
			vector<int>    tempTimepointDays;
			splitCommaSepName(timepoints, productTimepoints);
			if (timepoints.size() > 0) {
				for (i=0, len=timepoints.size(); i<len; i++){
					len1               = timepoints[i].size() - 1;
					char tenorChar     = tolower(timepoints[i].at(len1));
					string tenorLength = timepoints[i].substr(0, len1);
					anyInt = atoi(tenorLength.c_str()) * avgTenor[tenorChar];
					if (std::find(tempTimepointDays.begin(), tempTimepointDays.end(), anyInt) == tempTimepointDays.end()) { tempTimepointDays.push_back(anyInt); }
				}
			}

			// assemble percentile request
			productPercentiles		= szAllPrices[colProductPercentiles];
			vector<string> uPercentiles;
			vector<double> simPercentiles;
			splitCommaSepName(uPercentiles, productPercentiles);
			if (uPercentiles.size() > 0) {
				for (i=0, len=uPercentiles.size(); i<len; i++){
					anyDouble = atof(uPercentiles[i].c_str())/100.0;
					if (std::find(simPercentiles.begin(), simPercentiles.end(), anyDouble) == simPercentiles.end()) { simPercentiles.push_back(anyDouble); }
				}
			}
			
			if (doTimepoints && simPercentiles.size() > 0 && tempTimepointDays.size() > 0){
				if (std::find(simPercentiles.begin(), simPercentiles.end(), 0.0)   == simPercentiles.end())  simPercentiles.push_back(0.0);
				if (std::find(simPercentiles.begin(), simPercentiles.end(), 0.999) == simPercentiles.end())  simPercentiles.push_back(0.999);
				sort(simPercentiles.begin(), simPercentiles.end());
			}


			issuePrice              = atof(szAllPrices[colProductIssuePrice]);
			// clean bid,ask
			if (bidPrice <= 0.0 && askPrice > 0.0){ bidPrice=askPrice; }
			if (bidPrice > 0.0  && askPrice <= 0.0){ askPrice=bidPrice; }
			if (strlen(szAllPrices[colProductFrequency])){ couponFrequency = szAllPrices[colProductFrequency]; }
			boost::gregorian::date  bProductStartDate(boost::gregorian::from_simple_string(productStartDateString));
			cout << "Iterations:" << numMcIterations << " ProductId:" << productId << endl;
			// cout << "Press a key to continue...";  getline(cin, word);  // KEEP in case you want to attach debugger


			// get counterparty info
			// ...mult-issuer product's have comma-separated issuers...ASSUMED equal weight
			sprintf(lineBuffer, "%s%d%s", "select EntityName from institution where institutionid='", counterpartyId, "' ");
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
			retcode = mydb.fetch(true);
			string counterpartyName = szAllPrices[0];
			vector<string> counterpartyNames;
			splitCommaSepName(counterpartyNames, counterpartyName);
			sprintf(charBuffer, "%s%s%s", "'", counterpartyNames.at(0).c_str(), "'");
			for (i = 1; i < counterpartyNames.size(); i++){
				sprintf(charBuffer, "%s%s%s%s", charBuffer, ",'", counterpartyNames.at(i).c_str(), "'");
			}
			sprintf(lineBuffer, "%s%s%s", "select Maturity,avg(Spread) Spread from cdsspread join institution using (institutionid) where EntityName in (", charBuffer,
				") and spread is not null group by Maturity order by Maturity");

			mydb.prepare((SQLCHAR *)lineBuffer, 2);
			retcode = mydb.fetch(false);
			vector<double> cdsTenor, cdsSpread;
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				cdsTenor.push_back(atof(szAllPrices[0]));
				cdsSpread.push_back(atof(szAllPrices[1]) / 10000.0);
				retcode = mydb.fetch(false);
			}

			// get baseCurve
			sprintf(lineBuffer, "%s%s%s", "select Tenor,Rate/100 Spread from curve where ccy='", productCcy.c_str(),
				"' order by Tenor");
			mydb.prepare((SQLCHAR *)lineBuffer, 2);
			retcode = mydb.fetch(false);
			vector<SomeCurve> baseCurve;
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				anyCurve.tenor  = atof(szAllPrices[0]);
				anyCurve.spread = atof(szAllPrices[1]);
				baseCurve.push_back(anyCurve);
				retcode = mydb.fetch(false);
			}


			// get underlyingids for this product from DB
			// they can come in any order of UnderlyingId (this is deliberate to aviod the code becoming dependent on any ordering
			vector<int> ulIds;
			vector<string> ulCcys;
			vector<int> ulIdNameMap(1000);  // underlyingId -> arrayIndex, so ulIdNameMap[uid] gives the index into ulPrices vector
			sprintf(lineBuffer, "%s%s%s%s%s%d%s", "select distinct u.UnderlyingId UnderlyingId,u.ccy ulCcy from ", useProto, "productbarrier join ", useProto, "barrierrelation using (ProductBarrierId) join underlying u using (underlyingid) where ProductId='",
				productId, "' ");
			if (benchmarkId){
				sprintf(charBuffer, "%s%d%s%s%s%d%s", " union (select ", benchmarkId, ",u.ccy from ", useProto, "product p join underlying u on (p.BenchmarkId=u.UnderlyingId) where ProductId='", productId, "') ");
				strcat(lineBuffer, charBuffer);
			}
			mydb.prepare((SQLCHAR *)lineBuffer, 2);
			retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
				ulCcys.push_back(szAllPrices[1]);
				uid = atoi(szAllPrices[0]);
				if (find(ulIds.begin(), ulIds.end(), uid) == ulIds.end()) {      // build list of uids
					ulIds.push_back(uid);
				}
				ulIdNameMap.at(uid) = ulIds.size() - 1;
				// next record
				retcode = mydb.fetch(false);
			}
			numUl = ulIds.size();

			//** currencyStruck deals will have nonZero values for $crossRateUids
			vector<int> crossRateUids ; for (i=0; i<numUl; i++) { crossRateUids.push_back(0); }
			if (currencyStruck){
				for (i=0; i < numUl; i++) {
					if (productCcy != ulCcys[i]){
						sprintf(lineBuffer, "%s%s%s%s%s", "select UnderlyingId from underlying where name=concat('", ulCcys[i].c_str(), "','", productCcy.c_str(), "')");
						mydb.prepare((SQLCHAR *)lineBuffer, 1);
						retcode = mydb.fetch(true);
						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
							crossRateUids[i] = atoi(szAllPrices[0]);
						}
					}
				}
			}



			// read underlying prices
			vector<double>   ulReturns[maxUls];
			for (i = 0; i < numUl; i++) {
				ulReturns[i].reserve(10000); 
				ulOriginalPrices[i].date.reserve(10000);
				ulOriginalPrices[i].price.reserve(10000);
				ulOriginalPrices[i].nonTradingDay.reserve(10000);
			}
			char ulSql[10000]; // enough for around 100 underlyings...
			char crossRateBuffer[100];
			
			// get PRIIPs start date to use
			char startDateBuffer[100];
			if (strlen(endDate)){ strcpy(charBuffer,endDate); }
			else {
				mydb.prepare((SQLCHAR *)"select max(date) from prices", 1);
				retcode = mydb.fetch(true);
				strcpy(charBuffer, szAllPrices[0]);
			}	
			sprintf(startDateBuffer, "%s%s%s", " and Date >= date_sub('",charBuffer,"', interval 5 year) " );
			// ...form sql joins
			sprintf(ulSql, "%s", "select p0.Date Date");
			for (i = 0; i<numUl; i++) { 
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s", "*p",(numUl + i),".price");}
				sprintf(lineBuffer, "%s%d%s%s%s%d", ",p", i, ".price", (crossRateUids[i] ? crossRateBuffer : ""), " Price", i); strcat(ulSql, lineBuffer);
			}
			strcat(ulSql, " from prices p0 ");
			if (crossRateUids[0]){ sprintf(crossRateBuffer, "%s%d%s", " join prices p", numUl, " using (Date) "); strcat(ulSql, crossRateBuffer); }
			for (i = 1; i < numUl; i++) { 
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s", " join prices p",(numUl + i)," using (Date) "); }
				sprintf(lineBuffer, "%s%d%s%s", " join prices p", i, " using (Date) ", (crossRateUids[i] ? crossRateBuffer : "")); strcat(ulSql, lineBuffer);
			}
			sprintf(lineBuffer, "%s%d%s", " where p0.underlyingId = '", ulIds.at(0), "'");
			strcat(ulSql, lineBuffer);
			if (crossRateUids[0]){ sprintf(crossRateBuffer, "%s%d%s%d%s", " and p",numUl, ".underlyingid='", crossRateUids[0], "'"); strcat(ulSql, crossRateBuffer); }
			for (i = 1; i < numUl; i++) {
				if (crossRateUids[i]){ sprintf(crossRateBuffer, "%s%d%s%d%s", " and p",(numUl + i),".underlyingid='",crossRateUids[i],"'" ); }
				sprintf(lineBuffer, "%s%d%s%d%s%s", " and p", i, ".underlyingId='", ulIds.at(i), "'", (crossRateUids[i] ? crossRateBuffer : "")); strcat(ulSql, lineBuffer);
			}
			if (strlen(startDate)) { sprintf(ulSql, "%s%s%s%s", ulSql, " and Date >='", startDate, "'"); }
			else if (thisNumIterations>1) { strcat(ulSql, doPriips ? startDateBuffer : " and Date >='1992-12-31' "); }
			if (strlen(endDate))   { sprintf(ulSql, "%s%s%s%s", ulSql, " and Date <='", endDate,   "'"); }
			strcat(ulSql, " order by Date");
			// ...call DB
			mydb.prepare((SQLCHAR *)ulSql, numUl + 1);
			bool   firstTime(true);
			vector<double> previousPrice(numUl);
			boost::gregorian::date lastDate;
			// .. parse each record <Date,price0,...,pricen>
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
					thisPrice = atof(szAllPrices[i + 1]);
					if (!firstTime) {
						ulReturns[i].push_back(thisPrice / previousPrice[i]);
						for (j = 1; j < numDayDiff; j++){    // pad non-trading days
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
				if (firstTime){ firstTime = false; }
				lastDate = bDate;
				retcode = mydb.fetch(false);
			}
			totalNumDays       = ulOriginalPrices.at(0).price.size();
			lastDataDateString = ulOriginalPrices.at(0).date[totalNumDays - 1];
			totalNumReturns    = totalNumDays - 1;
			midPrice           = stalePrice && (fairValueDateString == lastDataDateString) ? fairValuePrice / issuePrice : (bidPrice + askPrice) / (2.0*issuePrice);
			ulPrices           = ulOriginalPrices; // copy constructor called
			vector<double> thesePrices(numUl), startLevels(numUl);
			boost::gregorian::date  bLastDataDate(boost::gregorian::from_simple_string(lastDataDateString));
			cerr << "NumPrices:\t" << totalNumDays << "FirstDataDate:\t" << ulOriginalPrices.at(0).date[0] << endl;
			int daysExtant = (bLastDataDate - bProductStartDate).days(); if (daysExtant < 0){ daysExtant = 0; }
			int tradingDaysExtant(0);
			for (i = 0; ulOriginalPrices.at(0).date[totalNumDays - 1 - i] > productStartDateString; i++){
				tradingDaysExtant += 1;
			}

			// create product
			SProduct spr(productId, ulOriginalPrices.at(0),bProductStartDate, fixedCoupon, couponFrequency, couponPaidOut, AMC, 
				productShape,depositGteed, collateralised, daysExtant, midPrice, baseCurve,ulIds);
			numBarriers = 0;

			// get barriers from DB
			enum {
				colProductBarrierId = 0, colProductId,
				colCapitalOrIncome, colNature, colPayoff, colTriggered, colSettlementDate, colDescription, colPayoffId, colParticipation,
				colStrike, colAvgTenor, colAvgFreq, colAvgType, colCap, colUnderlyingFunctionId, colParam1, colMemory, colIsAbsolute, colAvgInTenor, colAvgInFreq, colStrikeReset, colStopLoss, colAvgInAlgebra, colOriginalStrike, colForfeitCoupons, colProductBarrierLast
			};
			sprintf(lineBuffer, "%s%s%s%d%s", "select * from ", useProto, "productbarrier where ProductId='", productId, "' order by SettlementDate,ProductBarrierId");
			mydb.prepare((SQLCHAR *)lineBuffer, colProductBarrierLast);
			retcode = mydb.fetch(true);
			map<char, int>::iterator curr, end;
			// ...parse each productbarrier row
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
				int avgDays = 0, avgInDays = 0;
				int avgFreq = 0, avgInFreq = 0;
				bool isMemory      = atoi(szAllPrices[colMemory]     ) == 1;
				
				if (strlen(szAllPrices[colAvgTenor]) && strlen(szAllPrices[colAvgFreq])){
					buildAveragingInfo(szAllPrices[colAvgTenor], szAllPrices[colAvgFreq], avgDays, avgFreq);
				}
				if (strlen(szAllPrices[colAvgInTenor]) && strlen(szAllPrices[colAvgInFreq])){
					buildAveragingInfo(szAllPrices[colAvgInTenor], szAllPrices[colAvgInFreq], avgInDays, avgInFreq);
				}
				int barrierId         = atoi(szAllPrices[colProductBarrierId]);
				int avgType           = atoi(szAllPrices[colAvgType]);
				bool isAbsolute       = atoi(szAllPrices[colIsAbsolute]) == 1;
				bool isStrikeReset    = atoi(szAllPrices[colStrikeReset]) == 1;
				bool isStopLoss       = atoi(szAllPrices[colStopLoss]) == 1;
				bool isForfeitCoupons = atoi(szAllPrices[colForfeitCoupons]) == 1;
				capitalOrIncome = atoi(szAllPrices[colCapitalOrIncome]) == 1;
				nature = szAllPrices[colNature];
				payoff = atof(szAllPrices[colPayoff]) / 100.0;
				settlementDate = szAllPrices[colSettlementDate];
				description = szAllPrices[colDescription];
				avgInAlgebra = szAllPrices[colAvgInAlgebra];
				thisPayoffId = atoi(szAllPrices[colPayoffId]); thisPayoffType = payoffType[thisPayoffId];
				participation = atof(szAllPrices[colParticipation]);
				strike = atof(szAllPrices[colStrike]);
				cap = atof(szAllPrices[colCap]);
				int     underlyingFunctionId = atoi(szAllPrices[colUnderlyingFunctionId]);
				double  param1 = atof(szAllPrices[colParam1]);

				/*
				* barrier creation
				*/
				spr.barrier.push_back(SpBarrier(barrierId, capitalOrIncome, nature, payoff, settlementDate, description,
					thisPayoffType, thisPayoffId, strike, cap, underlyingFunctionId, param1, participation, ulIdNameMap, avgDays, avgType,
					avgFreq, isMemory, isAbsolute, isStrikeReset, isStopLoss, isForfeitCoupons, daysExtant, bProductStartDate, doFinalAssetReturn, midPrice));
				SpBarrier &thisBarrier(spr.barrier.at(numBarriers));
	
				// get barrier relations from DB
				enum {
					brcolBarrierRelationId = 0, brcolProductBarrierId,
					brcolUnderlyingId, brcolBarrier, brcolBarrierTypeId, brcolAbove, brcolAt, brcolStartDate, brcolEndDate,
					brcolTriggered, brcolIsAbsolute, brcolUpperBarrier, brcolWeight, brcolStrikeOverride, colBarrierRelationLast
				};
				// ** SQL fetch block
				sprintf(lineBuffer, "%s%s%s%d%s", "select * from ", useProto, "barrierrelation where ProductBarrierId='", barrierId, "' order by UnderlyingId");
				mydb1.prepare((SQLCHAR *)lineBuffer, colBarrierRelationLast);
				retcode = mydb1.fetch(false);
				// ...parse each barrierrelation row
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					double weight           = atof(szAllPrices[brcolWeight]);
					double strikeOverride   = atof(szAllPrices[brcolStrikeOverride]);
					bool   isAbsolute       = atoi(szAllPrices[brcolIsAbsolute]) == 1;
					uid      = atoi(szAllPrices[brcolUnderlyingId]);
					barrier  = atof(szAllPrices[brcolBarrier]);
					uBarrier = atof(szAllPrices[brcolUpperBarrier]);
					if (uBarrier > 999999 && uBarrier < 1000001.0) { uBarrier = NULL; } // using 1000000 as a quasiNULL, since C++ SQLFetch ignores NULL columns
					above                         = atoi(szAllPrices[brcolAbove]) == 1;
					at                            = atoi(szAllPrices[brcolAt]) == 1;
					startDateString               = szAllPrices[brcolStartDate];
					endDateString                 = szAllPrices[brcolEndDate];
					anyTypeId                     = atoi(szAllPrices[brcolBarrierTypeId]);
					bool   isContinuousALL        = _stricmp(barrierTypeMap[anyTypeId].c_str(), "continuousall"  ) == 0;
					thisBarrier.isContinuousGroup = thisBarrier.isContinuousGroup || _stricmp(barrierTypeMap[anyTypeId].c_str(), "continuousgroup") == 0;
					thisBarrier.isContinuous      = thisBarrier.isContinuous      || _stricmp(barrierTypeMap[anyTypeId].c_str(), "continuous") == 0;
					// express absolute levels as %ofSpot
					double thisStrikeDatePrice = ulPrices.at(ulIdNameMap[uid]).price[totalNumDays - 1 - daysExtant];
					// ...DOME only works with single underlying, for now...the issue is whether to add FixedStrike fields to each brel
					if (thisBarrier.isAbsolute)	{ 		// change fixed strike levels to percentages of spot
						thisBarrier.cap        /= thisStrikeDatePrice;
						thisBarrier.strike     /= thisStrikeDatePrice;
					}
					if (isAbsolute){
						barrier /= thisStrikeDatePrice;
						if (uBarrier       != NULL) { uBarrier       /= thisStrikeDatePrice; }
						if (strikeOverride != 0.0)  { strikeOverride /= thisStrikeDatePrice; }
					}

					if (uid) {
						// create barrierRelation
						thisBarrier.brel.push_back(SpBarrierRelation(uid, barrier, uBarrier, isAbsolute, startDateString, endDateString,
							above, at, weight, daysExtant, strikeOverride != 0.0 ? strikeOverride : thisBarrier.strike, ulPrices.at(ulIdNameMap[uid]), 
							avgType, avgDays, avgFreq, avgInDays, avgInFreq, avgInAlgebra,productStartDateString,isContinuousALL));
					}
					// next barrierRelation record
					retcode = mydb1.fetch(false);
				}

				switch (thisBarrier.payoffTypeId) {
					// single-currency baskets
				case basketCallPayoff:	
				case basketPutPayoff:
					// quanto baskets	
				case basketCallQuantoPayoff:
				case basketPutQuantoPayoff:
					double basketFinal=0.0, basketRef=1.0;
					for (j=0; j<thisBarrier.brel.size(); j++) {
						const SpBarrierRelation &thisBrel(thisBarrier.brel[j]);
						double w          = thisBrel.weight;
						int uid           = thisBrel.underlying;
						double finalPrice = ulPrices.at(ulIdNameMap[uid]).price[totalNumDays - 1];
						double refPrice   = ulPrices.at(ulIdNameMap[uid]).price[totalNumDays - 1 - daysExtant];
						basketFinal   += finalPrice / refPrice * w;
					}
					thisBarrier.strike  /= basketFinal / basketRef;
					break;
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
				thisBarrier.isExtremum = !thisBarrier.isStopLoss && isExtremumBarrier;  // force stopLoss barriers to be monitored daily, rather than be treated as an extremum, so stopLoss barriers do not need special processing in the simulator

				// update vector of monitoring dates
				double thisEndDays = thisBarrier.getEndDays();
				if (thisEndDays <0){
					if (find(accrualMonDateIndx.begin(), accrualMonDateIndx.end(), thisEndDays) == accrualMonDateIndx.end()) {
						accrualMonDateIndx.push_back(thisEndDays);
					}
				}
				else {
					// DOME: for now only use endDates, as all American barriers are detected below as extremum bariers
					if (thisBarrier.isExtremum || !thisBarrier.isContinuous){
						if (find(monDateIndx.begin(), monDateIndx.end(), thisEndDays) == monDateIndx.end()) {
							monDateIndx.push_back(thisEndDays);
						}
					}
					else {  // daily monitoring
						for (i = 0; i < thisBarrier.brel.size(); i++) {
							const SpBarrierRelation &thisBrel(thisBarrier.brel[i]);
							if (thisBrel.startDate != thisBrel.endDate) {
								int startDays      = (thisBrel.bStartDate - bProductStartDate).days() - daysExtant;
								int endDays        = (thisBrel.bEndDate - bProductStartDate).days() - daysExtant;
								if (startDays < thisBarrier.startDays){
									thisBarrier.startDays = startDays;
								}
								for (j=startDays; j <= endDays; j++){
									if (find(monDateIndx.begin(), monDateIndx.end(), j) == monDateIndx.end()) {
										monDateIndx.push_back(j);
									}
								}
							}
						}

					}
				}	

				// any other init
				int thisEndDaysInt = (int)thisEndDays;
				if (doTimepoints && std::find(tempTimepointDays.begin(), tempTimepointDays.end(), thisEndDaysInt) == tempTimepointDays.end()) { tempTimepointDays.push_back(thisEndDaysInt); }
				if (thisEndDays > maxBarrierDays){ maxBarrierDays = thisEndDays; }

				// next barrier record
				numBarriers += 1;
				retcode = mydb.fetch(false);
			}


			// possibly pad future ulPrices for resampling into if there is not enough history
			int daysPadding = max(maxBarrierDays,maxBarrierDays + daysExtant - totalNumDays + 1);
			boost::gregorian::date  bTempDate = bLastDataDate;
			while (daysPadding>0){
				bTempDate += boost::gregorian::days(1);
				string tempString = boost::gregorian::to_iso_extended_string(bTempDate);
				sprintf(charBuffer, "%s", tempString.c_str());
				for (i = 0; i < numUl; i++) {
					ulOriginalPrices.at(i).date.push_back(charBuffer);						ulPrices.at(i).date.push_back(charBuffer);
					ulOriginalPrices.at(i).price.push_back(0.0);							ulPrices.at(i).price.push_back(0.0);
					// DOME: crudely mimic-ing weekends
					int  dayOfWeek           = daysPadding % 7;
					bool mimicNonTradingDay  = (dayOfWeek == 5) || (dayOfWeek == 6);
					ulOriginalPrices.at(i).nonTradingDay.push_back(mimicNonTradingDay);		ulPrices.at(i).nonTradingDay.push_back(mimicNonTradingDay);
				}
				daysPadding -= 1;
			}


			// remove any timepointDays 
			vector<int> timepointDays;
			if (doTimepoints){
				for (i=0; i<tempTimepointDays.size(); i++){
					if (tempTimepointDays[i] <= maxBarrierDays){
						timepointDays.push_back(tempTimepointDays[i]);
					}
				}
			}
			int numTimepoints  = timepointDays.size();
			vector<string> timepointNames;
			if (doTimepoints){
				sort(timepointDays.begin(), timepointDays.end());
				// space for timepoint stats [timepointDays][uid][]
				for (i=0; i<numTimepoints; i++){
					sprintf(lineBuffer,"%d%s",timepointDays[i],"d");
					timepointNames.push_back(lineBuffer);
				}
				// put in any barrier names
				for (i=0; i < numBarriers; i++) {
					const SpBarrier&    b(spr.barrier.at(i));
					int thisEndDays = (int)b.endDays;
					bool done = false;
					for (i=0; !done && i < timepointDays.size(); i++){
						if (thisEndDays == timepointDays[i]){
							done = true;
							sprintf(lineBuffer, "%s %s", timepointNames.at(i).c_str(), b.description.c_str());
							timepointNames[i] = lineBuffer;
						}
					}
				}
				// init database
				sprintf(lineBuffer, "%s%d", "delete from timepoints where userid=3 and productid=", productId);
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				sprintf(lineBuffer, "%s%d", "delete from path where userid=3 and productid=", productId);
				mydb.prepare((SQLCHAR *)lineBuffer, 1);
				/*
				retcode = mydb.fetch(true);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					cerr << "Problem deleting: " << lineBuffer << endl;
				}
				*/
				
			}






			// further initialisation, given product info
			spr.productDays    = *max_element(monDateIndx.begin(), monDateIndx.end());
			spr.maxProductDays = maxBarrierDays + daysExtant;
			numMonPoints = monDateIndx.size();
			// ...check product not matured
			if (!numMonPoints || (numMonPoints == 1 && monDateIndx[0] == 0)){ continue; }
			double maxYears = 0; for (i = 0; i<numBarriers; i++) { double t = spr.barrier.at(i).yearsToBarrier;   if (t > maxYears){ maxYears = t; } }
			vector<double> fullCurve;             // populate a full annual CDS curve
			for (j = 0; j<maxYears + 1; j++) {
				fullCurve.push_back(interpCurve(cdsTenor, cdsSpread, j + 1));
			}
			vector<double> dpCurve, hazardCurve;  // annual default probability curve
			const double recoveryRate(0.4);
			bootstrapCDS(fullCurve, dpCurve, recoveryRate);
			for (j = 0, len = fullCurve.size(); j<len; j++) {
				hazardCurve.push_back(dpCurve[j]);
			}

			// possibly impose user-defined view of expectedReturn: only need to bump ulReturns

			// initialise product, now we have all the state
			spr.init();

			// get accrued coupons
			double accruedCoupon(0.0);
			spr.evaluate(totalNumDays, totalNumDays - 1, totalNumDays, 1, historyStep, ulPrices, ulReturns,
				numBarriers, numUl, ulIdNameMap, accrualMonDateIndx, recoveryRate, hazardCurve, mydb, accruedCoupon, true, false, doDebug, startTime, benchmarkId, contBenchmarkTER,hurdleReturn,
				false, false, timepointDays, timepointNames, simPercentiles, doPriips, useProto);

			// finally evaluate the product...1000 iterations of a 60barrier product (eg monthly) = 60000
			spr.evaluate(totalNumDays, thisNumIterations == 1 ? daysExtant : totalNumDays - 1, thisNumIterations == 1 ? totalNumDays - spr.productDays : totalNumDays /*daysExtant + 1*/, /* thisNumIterations*numBarriers>100000 ? 100000 / numBarriers : */ min(2000000, thisNumIterations), historyStep, ulPrices, ulReturns,
				numBarriers, numUl, ulIdNameMap, monDateIndx, recoveryRate, hazardCurve, mydb, accruedCoupon, false, doFinalAssetReturn, doDebug, startTime, benchmarkId,contBenchmarkTER,hurdleReturn,
				doTimepoints, doPaths, timepointDays, timepointNames, simPercentiles, doPriips, useProto);
			// tidy up

		} // for each product
		std::cout << "timeTaken:" << difftime(time(0), startTime) << "secs" << endl;
		std::cout << "clytemnestra";
	} // try

	// tidy up
	catch (out_of_range oor){
		cerr << "Out of Range error \n" << oor.what();
	}
	catch (bad_alloc){
		cerr << "Out of Memory error \n";
	}
	catch (length_error){
		cerr << "Length error \n";
	}
	catch (...){
		cerr << "unknown error \n";
	}

	return 0;
}

