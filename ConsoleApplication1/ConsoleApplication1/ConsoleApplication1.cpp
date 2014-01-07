// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header.h"
using namespace std;




int _tmain(int argc, TCHAR* argv[])
{
	try{
		// initialise
		if (argc != 4){ cout << "Usage: startId stopId numIterations" << endl;  exit(0); }
		int              historyStep = 1;
		int              startProductId  = argc > 1 ? _ttoi(argv[1]) : 363;
		int              stopProductId   = argc > 2 ? _ttoi(argv[2]) : 363;
		int              numMcIterations = argc > 3 ? _ttoi(argv[3]) : 100;
		const int        maxUls(100);
		const int        bufSize(1000);
		SQLHENV          hEnv = NULL;		    // Env Handle from SQLAllocEnv()
		SQLHDBC          hDBC = NULL;         // Connection handle
		RETCODE          retcode;
		char             lineBuffer[1000], charBuffer[1000];
		char             **szAllPrices = new char*[maxUls];
		vector<int>      allProductIds; allProductIds.reserve(1000);
		vector<string>   payoffType = { "", "fixed", "call", "put", "twinWin", "switchable", "basketCall", "lookbackCall" };
		vector<int>::iterator intIterator, intIterator1;
		for (int i = 0; i < maxUls; i++){
			szAllPrices[i] = new char[bufSize];
		}
		srand(time(0)); // reseed rand


		// open database
		MyDB  mydb((char **)szAllPrices), mydb1((char **)szAllPrices);

		// get list of productIds
		sprintf(lineBuffer, "%s%d%s%d%s", "select ProductId from product where ProductId>='", startProductId, "' and ProductId<='", stopProductId, "' and Matured='0'"); 	mydb.prepare((SQLCHAR *)lineBuffer, 1); 	retcode = mydb.fetch(true);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			int x(atoi(szAllPrices[0])); allProductIds.push_back(x);
			retcode = mydb.fetch(false);
		}

		// loop through each product
		for (int productIndx = 0; productIndx < allProductIds.size(); productIndx++) {
			int              oldProductBarrierId = 0, productBarrierId = 0;
			int              numBarriers = 0, thisIteration = 0;
			int              i, j, len, numUl, numMonPoints,totalNumDays, totalNumReturns, uid;
			int              productId, anyTypeId, thisPayoffId;
			double           barrier, uBarrier, payoff, strike, cap, participation, fixedCoupon, AMC, issuePrice, bidPrice, askPrice, midPrice;
			string           couponFrequency, productStartDateString, word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate, description;
			bool             capitalOrIncome, above, at;
			vector<int>      monDateIndx, accrualMonDateIndx;
			vector<UlTimeseries>  ulOriginalPrices(maxUls), ulPrices(maxUls); // underlying prices	

			productId = allProductIds.at(productIndx);
		
			// get general info:  productType, barrierType
			// ...productType
			map<int, string> productTypeMap;
			sprintf(lineBuffer, "%s", "select * from producttype"); 	mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				productTypeMap[atoi(szAllPrices[0])] = szAllPrices[1];
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
				colProductCounterpartyId = 2, colProductStrikeDate = 6, colProductFixedCoupon = 28, colProductFrequency, colProductBid, colProductAsk,
				colProductAMC = 43, colProductMaxIterations=55,colProductDepositGtee, colProductDealCheckerId, colProductAssetTypeId, colProductIssuePrice, colProductCouponPaidOut, colProductCollateralised,colProductLast
			};
			sprintf(lineBuffer, "%s%d%s", "select * from product where ProductId='", productId, "'");
			mydb.prepare((SQLCHAR *)lineBuffer, colProductLast);
			retcode = mydb.fetch(true);
			int  thisNumIterations  = atoi(szAllPrices[colProductMaxIterations]); 
			if (numMcIterations < thisNumIterations){ thisNumIterations = numMcIterations; }
			if (thisNumIterations<1)                { thisNumIterations = 1; }
			int  counterpartyId     = atoi(szAllPrices[colProductCounterpartyId]);
			bool depositGteed       = atoi(szAllPrices[colProductDepositGtee]   ) == 1;
			bool couponPaidOut      = atoi(szAllPrices[colProductCouponPaidOut] ) == 1;
			bool collateralised     = atoi(szAllPrices[colProductCollateralised]) == 1;
			productStartDateString  = szAllPrices[colProductStrikeDate];
			fixedCoupon             = atof(szAllPrices[colProductFixedCoupon]);
			bidPrice                = atof(szAllPrices[colProductBid]);
			askPrice                = atof(szAllPrices[colProductAsk]);
			AMC                     = atof(szAllPrices[colProductAMC]);
			issuePrice              = atof(szAllPrices[colProductIssuePrice]);
			midPrice                = ((bidPrice > 99.999) && (askPrice > 99.999) && (bidPrice < 100.001) && (askPrice < 100.001)) ? 1.0 : (bidPrice + askPrice) / (2.0*issuePrice);
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
			splitCounterpartyName(counterpartyNames, counterpartyName);
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

			// get underlyingids for this product from DB
			vector<int> ulIds;
			vector<int> ulIdNameMap(1000);  // underlyingId -> arrayIndex, so ulIdNameMap[uid] gives the index into ulPrices vector
			sprintf(lineBuffer, "%s%d%s", "select distinct UnderlyingId from productbarrier join barrierrelation using (ProductBarrierId) where ProductId='", productId, "'");
			mydb.prepare((SQLCHAR *)lineBuffer, 1);
			retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
				uid = atoi(szAllPrices[0]);
				if (find(ulIds.begin(), ulIds.end(), uid) == ulIds.end()) {      // build list of uids
					ulIds.push_back(uid);
				}
				ulIdNameMap.at(uid) = ulIds.size() - 1;
				// next record
				retcode = mydb.fetch(false);
			}
			numUl = ulIds.size();

			// read underlying prices
			vector<double>   ulReturns[maxUls];
			for (i = 0; i < numUl; i++) {
				ulReturns[i].reserve(10000); 
				ulOriginalPrices[i].date.reserve(10000);
				ulOriginalPrices[i].price.reserve(10000);
				ulOriginalPrices[i].nonTradingDay.reserve(10000);
			}
			char ulSql[10000]; // enough for around 100 underlyings...
			// ...form sql joins
			sprintf(ulSql, "%s", "select p0.Date Date");
			for (i = 0; i<numUl; i++) { sprintf(lineBuffer, "%s%d%s%d", ",p", i, ".price Price", i); strcat(ulSql, lineBuffer); }
			strcat(ulSql, " from prices p0 ");
			if (numUl > 1) { for (i = 1; i < numUl; i++) { sprintf(lineBuffer, "%s%d%s", " join prices p", i, " using (Date) "); strcat(ulSql, lineBuffer); } }
			sprintf(lineBuffer, "%s%d%s", " where p0.underlyingId = '", ulIds.at(0), "'");
			strcat(ulSql, lineBuffer);
			if (numUl > 1) { for (i = 1; i < numUl; i++) { sprintf(lineBuffer, "%s%d%s%d%s", " and p", i, ".underlyingId='", ulIds.at(i), "'"); strcat(ulSql, lineBuffer); } }
			if (thisNumIterations>1) { strcat(ulSql, " and Date >='1992-12-31'"); }
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
			totalNumDays    = ulOriginalPrices.at(0).price.size();
			totalNumReturns = totalNumDays - 1;
			ulPrices        = ulOriginalPrices; // copy constructor called
			vector<double> thesePrices(numUl), startLevels(numUl);
			boost::gregorian::date  bLastDataDate(boost::gregorian::from_simple_string(ulOriginalPrices.at(0).date[totalNumDays - 1]));
			cerr << "NumPrices:\t" << totalNumDays << "FirstDataDate:\t" << ulOriginalPrices.at(0).date[0] << endl;
			int daysExtant = (bLastDataDate - bProductStartDate).days(); if (daysExtant < 0){ daysExtant = 0; }
			int tradingDaysExtant(0);
			for (i = 0; ulOriginalPrices.at(0).date[totalNumDays - 1 - i] > productStartDateString; i++){
				tradingDaysExtant += 1;
			}

			// create product
			SProduct spr(productId, ulOriginalPrices.at(0),bProductStartDate, fixedCoupon, couponFrequency, couponPaidOut, AMC, depositGteed, 
				collateralised,daysExtant, midPrice);
			numBarriers = 0;

			// get barriers from DB
			enum {
				colProductBarrierId = 0, colProductId,
				colCapitalOrIncome, colNature, colPayoff, colTriggered, colSettlementDate, colDescription, colPayoffId, colParticipation,
				colStrike, colAvgTenor, colAvgFreq, colAvgType, colCap, colUnderlyingFunctionId, colParam1, colMemory, colIsAbsolute, colProductBarrierLast
			};
			sprintf(lineBuffer, "%s%d%s", "select * from productbarrier where ProductId='", productId, "' order by SettlementDate,ProductBarrierId");
			mydb.prepare((SQLCHAR *)lineBuffer, colProductBarrierLast);
			retcode = mydb.fetch(true);
			map<char, int> avgTenor; avgTenor['d'] = 1; avgTenor['w'] = 7; avgTenor['m'] = 30; avgTenor['q'] = 91; avgTenor['y'] = 365;
			map<char, int>::iterator curr, end;
			// ...parse each productbarrier row
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
				int tenorPeriodDays = 0;
				int avgDays = 0;
				int numTenor = 0;
				int avgFreq = 0;
				bool isMemory = atoi(szAllPrices[colMemory]) == 1;;
				if (strlen(szAllPrices[colAvgTenor]) && strlen(szAllPrices[colAvgFreq])){
					char avgChar1 = szAllPrices[colAvgTenor][0];
					numTenor = (avgChar1 - '0');
					char avgChar2 = tolower(szAllPrices[colAvgTenor][1]);
					/*
					for (found = false, curr = avgTenor.begin(), end = avgTenor.end(); !found && curr != end; curr++) {
						if (curr->first == avgChar2){ found = true; tenorPeriodDays = curr->second; }
					}*/
					tenorPeriodDays = avgTenor[avgChar2];

					avgDays = numTenor * tenorPeriodDays + 1;  // add 1 since averaging invariably includes both end dates
					avgChar2 = tolower(szAllPrices[colAvgFreq][0]);
					/*
					for (found = false, curr = avgTenor.begin(), end = avgTenor.end(); !found && curr != end; curr++) {
						if (curr->first == avgChar2){ found = true; avgFreq = curr->second; }
					}*/
					avgFreq = avgTenor[avgChar2];

				}
				int barrierId = atoi(szAllPrices[colProductBarrierId]);
				int avgType = atoi(szAllPrices[colAvgType]);
				bool isAbsolute = atoi(szAllPrices[colIsAbsolute]) == 1;
				capitalOrIncome = atoi(szAllPrices[colCapitalOrIncome]) == 1;
				nature = szAllPrices[colNature];
				payoff = atof(szAllPrices[colPayoff]) / 100.0;
				settlementDate = szAllPrices[colSettlementDate];
				description = szAllPrices[colDescription];
				thisPayoffId = atoi(szAllPrices[colPayoffId]); thisPayoffType = payoffType[thisPayoffId];
				participation = atof(szAllPrices[colParticipation]);
				strike = atof(szAllPrices[colStrike]);
				cap = atof(szAllPrices[colCap]);
				int     underlyingFunctionId = atoi(szAllPrices[colUnderlyingFunctionId]);
				double  param1 = atof(szAllPrices[colParam1]);
				spr.barrier.push_back(SpBarrier(barrierId, capitalOrIncome, nature, payoff, settlementDate, description,
					thisPayoffType, thisPayoffId, strike, cap, underlyingFunctionId, param1, participation, ulIdNameMap, avgDays, avgType,
					avgFreq, isMemory, isAbsolute, daysExtant, bProductStartDate));
				SpBarrier &thisBarrier(spr.barrier.at(numBarriers));
				// update vector of monitoring dates
				// DOME: for now only use endDates, as all American barriers are detected below as extremum bariers
				double thisEndDays = thisBarrier.getEndDays();
				if (thisEndDays <0){
					if (find(accrualMonDateIndx.begin(), accrualMonDateIndx.end(), thisEndDays) == accrualMonDateIndx.end()) {
						accrualMonDateIndx.push_back(thisEndDays);
					}
				}
				else {
					if (find(monDateIndx.begin(), monDateIndx.end(), thisEndDays) == monDateIndx.end()) {
						monDateIndx.push_back(thisEndDays);
					}
				}

				// get barrier relations from DB
				enum {
					brcolBarrierRelationId = 0, brcolProductBarrierId,
					brcolUnderlyingId, brcolBarrier, brcolBarrierTypeId, brcolAbove, brcolAt, brcolStartDate, brcolEndDate,
					brcolTriggered, brcolIsAbsolute, brcolUpperBarrier, brcolWeight, colBarrierRelationLast
				};
				// ** SQL fetch block
				sprintf(lineBuffer, "%s%d%s", "select * from barrierrelation where ProductBarrierId='", barrierId, "'");
				mydb1.prepare((SQLCHAR *)lineBuffer, colBarrierRelationLast);
				retcode = mydb1.fetch(false);
				// ...parse each barrierrelation row
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
					double weight = atof(szAllPrices[brcolWeight]);
					bool isAbsolute = atoi(szAllPrices[brcolIsAbsolute]) == 1;
					uid = atoi(szAllPrices[brcolUnderlyingId]);
					barrier = atof(szAllPrices[brcolBarrier]);
					uBarrier = atof(szAllPrices[brcolUpperBarrier]);
					if (uBarrier > 999999 && uBarrier < 1000001.0) { uBarrier = NULL; } // using 1000000 as a quasiNULL, since C++ SQLFetch ignores NULL columns
					above = atoi(szAllPrices[brcolAbove]) == 1;
					at = atoi(szAllPrices[brcolAt]) == 1;
					startDateString = szAllPrices[brcolStartDate];
					endDateString = szAllPrices[brcolEndDate];
					anyTypeId = atoi(szAllPrices[brcolBarrierTypeId]);
					thisBarrier.isContinuous = barrierTypeMap[anyTypeId] == "continuous";
					// express absolute levels as %ofSpot
					double thisStrikeDatePrice = ulPrices.at(ulIdNameMap[uid]).price[totalNumDays - 1 - daysExtant];
					// ...DOME only works with single underlying, for now...the issue is whether to add FixedStrike fields to each brel
					if (thisBarrier.isAbsolute)	{ 		// change fixed strike levels to percentages of spot
						thisBarrier.cap = thisBarrier.cap / thisStrikeDatePrice - 1.0;
						thisBarrier.strike /= thisStrikeDatePrice;
					}
					if (isAbsolute){
						barrier /= thisStrikeDatePrice;
						if (uBarrier != NULL) { uBarrier /= thisStrikeDatePrice; }
					}



					if (uid) {
						// create barrierRelation
						thisBarrier.brel.push_back(SpBarrierRelation(uid, barrier, uBarrier, isAbsolute, startDateString, endDateString,
							above, at, weight, daysExtant, thisBarrier.strike, ulPrices.at(ulIdNameMap[uid]), avgType, avgDays, avgFreq, productStartDateString));
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


			// further initialisation, given product info
			spr.productDays = *max_element(monDateIndx.begin(), monDateIndx.end());
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

			// get accrued coupons
			double accruedCoupon(0.0);
			spr.evaluate(totalNumDays, totalNumDays - 1, totalNumDays, 1, historyStep, ulPrices, ulReturns,
				numBarriers, numUl, ulIdNameMap, accrualMonDateIndx, recoveryRate, hazardCurve, mydb, accruedCoupon, true);

			// finally evaluate the product
			spr.evaluate(totalNumDays, daysExtant, totalNumDays - spr.productDays, thisNumIterations, historyStep, ulPrices, ulReturns,
				numBarriers, numUl, ulIdNameMap, monDateIndx, recoveryRate, hazardCurve, mydb, accruedCoupon, false);
			// tidy up

		} // for each product
	} // try

	// tidy up
	catch (out_of_range){
		cerr << "Out of Range error \n";
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

