// bberg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header.h"
using namespace BloombergLP;
using namespace blpapi;
using namespace std;



// convert date wchar to char
// ...consult this: http://msdn.microsoft.com/en-us/library/ms235631.aspx
char *WcharToChar(const WCHAR* orig, size_t* convertedChars) {
	size_t origsize = wcslen(orig) + 1;
	const size_t newsize = origsize * 2;
	char *thisDate = new char[newsize];
	*convertedChars = 0;
	wcstombs_s(convertedChars, thisDate, newsize, orig, _TRUNCATE);
	return thisDate;
}


int _tmain(int argc, _TCHAR* argv[])
{
	const bool doDebug(false);
	try{
		// initialise
		const int        maxBufs(100);
		const int        bufSize(1000);
		char             lineBuffer[bufSize], charBuffer[bufSize], resultBuffer[bufSize], idBuffer[bufSize];
		size_t numChars;
		int commaSepList = strstr(WcharToChar(argv[1], &numChars), ",") ? 1 : 0;
		if (argc < 4 - commaSepList){ std::cout << "Usage: startId stopId (or comma-sep list of ids) dateAsYYYYMMDD  <optionalArguments: 'dbServer:'spCloud|newSp  'pricesOnly' 'prices:'y/n  'curves:'y/n 'cds:'y/n 'static:'y/n> 'tickerfeed:'n/y 'currentPrices:'n/y>" << endl;  exit(0); }
		int              startProductId,stopProductId;
		char *thisDate = WcharToChar(argv[3-commaSepList], &numChars);
		char isoDate[11]; sprintf(isoDate,"%c%c%c%c%c%c%c%c%c%c%c%c", thisDate[0], thisDate[1], thisDate[2], thisDate[3], '-', thisDate[4], thisDate[5], '-', thisDate[6], thisDate[7],'\0' );
		string anyString(thisDate); if (anyString.find("-") != string::npos){ std::cout << "Usage: enter date as YYYYMMDD " << endl;  exit(0); }
		char dbServer[100]; strcpy(dbServer, "spCloud");  // newSp for local PC
		bool doPrices(true), doCDS(true), doCurves(true), doStatic(true), doTickerfeed(false), doCurrentPrices(false);
		for (int i = 4-commaSepList; i<argc; i++){
			char *thisArg = WcharToChar(argv[i], &numChars);
			if (sscanf(thisArg, "prices:%s",        lineBuffer)){ doPrices = strcmp(lineBuffer, "y") == 0; }
			if (sscanf(thisArg, "cds:%s",           lineBuffer)){ doCDS = strcmp(lineBuffer, "y") == 0; }
			if (sscanf(thisArg, "static:%s",        lineBuffer)){ doStatic = strcmp(lineBuffer, "y") == 0; }
			if (sscanf(thisArg, "curves:%s",        lineBuffer)){ doCurves = strcmp(lineBuffer, "y") == 0; }
			if (sscanf(thisArg, "tickerfeed:%s",    lineBuffer)){ doTickerfeed = strcmp(lineBuffer, "y") == 0; }
			if (sscanf(thisArg, "currentPrices:%s", lineBuffer)){ doCurrentPrices = strcmp(lineBuffer, "y") == 0; }
			if (sscanf(thisArg, "dbServer:%s",      lineBuffer)){ strcpy(dbServer, lineBuffer); }
			if (strstr(thisArg, "pricesOnly")                  ){ doStatic = false; doCDS = false; doCurves = false; }
		}
		char *pricePrefix = doCurrentPrices ? "current" : "";

		// init
		SQLHENV          hEnv = NULL;		  // Env Handle from SQLAllocEnv()
		SQLHDBC          hDBC = NULL;         // Connection handle
		RETCODE          retcode;
		char             **szAllBufs = new char*[maxBufs];
		vector<int>      allProductIds; allProductIds.reserve(1000);
		for (int i = 0; i < maxBufs; i++){
			szAllBufs[i] = new char[bufSize];
		}


		// open database
		cerr << "opening database connectino with: " << dbServer;
		MyDB  mydb((char **)szAllBufs, dbServer), mydb1((char **)szAllBufs, dbServer), mydb2((char **)szAllBufs, dbServer);
		cerr << "OPENED database connectino with: " << dbServer;


		// Bberg Setup Session parameters: port, buffer size, etc...
		SessionOptions sessionOptions;
		sessionOptions.setServerHost("127.0.0.1");
		sessionOptions.setServerPort(8194);
		Session session(sessionOptions); // Establish session
		// Start Session
		if (!session.start()) {
			std::cerr << "Failed to start session." << std::endl;
			return 1;
		}
		// Bberg open service
		if (!session.openService("//blp/refdata")) {
			std::cerr << "Failed to open //blp/refdata" << std::endl;
			return 1;
		}
		Service ref = session.getService("//blp/refdata");




		// get list of productIds
		sprintf(lineBuffer, "%s", "select ProductId from product where ProductId ");
		if (commaSepList == 1){
			sprintf(idBuffer,"%s%s%s"," in (",WcharToChar(argv[1],&numChars),") ");
		}
		else{
			startProductId = argc > 1 ? _ttoi(argv[1]) : 34;
			stopProductId  = argc > 2 ? _ttoi(argv[2]) : 1000;
			sprintf(idBuffer, "%s%d%s%d%s", " >= '", startProductId, "' and ProductId <= '", stopProductId,"'");
		}
		sprintf(lineBuffer,"%s%s%s", lineBuffer, idBuffer, " and Matured='0'");
		mydb.prepare((SQLCHAR *)lineBuffer, 1); 	
		retcode = mydb.fetch(true);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			int x(atoi(szAllBufs[0])); allProductIds.push_back(x);
			retcode = mydb.fetch(false);
		}
		if (commaSepList == 1 && allProductIds.size() > 0){
			startProductId = allProductIds[0];
			stopProductId  = allProductIds[allProductIds.size()-1];
		}

		//
		// get BID,ASK prices for productIds
		//
		if (!doDebug && (doPrices || doCurrentPrices)){
			char *bidAskFields[] = { "PX_BID", "PX_ASK" };
			char *lastFields[] = { "PX_LAST", "PX_LAST" };
			sprintf(lineBuffer, "%s%s%s%s%s", "select ProductId, p.name, p.StrikeDate, cp.name, if (p.BbergTicker != '', p.BbergTicker, Isin) Isin, BbergPriceFeed from product p join institution cp on(p.CounterpartyId=cp.institutionid) where (Isin != '' or p.BbergTicker != '') and productid ",
			idBuffer, " and Matured='0' ", startProductId == stopProductId ? "" : " and strikedate<now() ", " order by ProductId; ");
			// std::cout << "\ngetting prices with SQL " << lineBuffer << std::endl; 
			mydb.prepare((SQLCHAR *)lineBuffer, 6);
			retcode = mydb.fetch(false);  // set to false, since there may not be any deals with ISINs
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				int id(atoi(szAllBufs[0]));
				string tickerString = szAllBufs[4];

				// get prices ... there may be more than one where a product has several ISINs eg #524
				std::cout << "\ngetting prices for " << id << std::endl;
				double bidPrice(0.0), askPrice(0.0);
				std::string  pricingDateString("");
				char *cPtr(NULL);
				bool notEquityTicker = !(tickerString.find("Equity") != std::string::npos || tickerString.find("@") != std::string::npos);
				if (notEquityTicker) {
					cPtr = strtok(szAllBufs[4], ",");
				}
				else {
					cPtr = szAllBufs[4];
				}
				bool gotAllPrices(true);
				while (cPtr && gotAllPrices){
					// make this ticker
					if (notEquityTicker) {
						sprintf(charBuffer, "%s%s%s%s", "/isin/", cPtr, "@", szAllBufs[5]);
					}
					else { sprintf(charBuffer, "%s", cPtr); }
					BbergData thisBbergData;
					int counter = 3;
					char newDateStr[9];
					strcpy(newDateStr, thisDate);
					do {
						thisBbergData = getBbergBidOfferPrices(charBuffer, notEquityTicker ? bidAskFields : lastFields, newDateStr, ref, session);
						DatePlusDays(newDateStr, -1, newDateStr);
					} while (counter-- > 0 && (thisBbergData.p[0] < 0.0 || thisBbergData.p[1] < 0.0));
					if (thisBbergData.p[0] > 0.0 && thisBbergData.p[1] > 0.0){
						pricingDateString = thisBbergData.date;
						bidPrice += thisBbergData.p[0]; askPrice += thisBbergData.p[1];
					}
					else {
						std::cerr << "Failed to get prices for " << id << std::endl;
						gotAllPrices = false;
					}
					if (notEquityTicker) {
						cPtr = strtok(NULL, ",");
					}
					else { cPtr = NULL; }
				}

				if (gotAllPrices){
					//sprintf(charBuffer, "%s%.2lf%s%.2lf%s%d%s", "update product set bid='", bidPrice, "',ask='", askPrice, "' where productid='", id, "';");
					sprintf(charBuffer, "%s%s%s%.2lf%s%s%s%.2lf%s%s%s%s%s%d%s",
						"update product set ", pricePrefix, "bid='", bidPrice, "',", pricePrefix, "ask='", askPrice, "',", pricePrefix, "bidaskdate='", pricingDateString.c_str(), "' where productid='", id, "';");
					//std::cerr << "Prices date for " << id << " is " << pricingDateString << std::endl;
					mydb1.prepare((SQLCHAR *)charBuffer, 1);
					if (doTickerfeed){
						sprintf(charBuffer, "%s%.2lf%s%.2lf%s%d%s", "insert into tickerfeed (bid,ask,productid,datetime) values ('", bidPrice, "','", askPrice, "','", id, "',now());");
						mydb1.prepare((SQLCHAR *)charBuffer, 1);
					}
				}
				retcode = mydb.fetch(false);
			}

		}

		//
		// get curves
		//
		if (!doDebug && doCurves){
			vector<string> curveTableNames ={"curve","oncurve"};
			for (auto thisName = begin(curveTableNames); thisName != end(curveTableNames); ++thisName) {
				sprintf(lineBuffer, "%s%s%s", "select ccy,tenor,bberg from ",thisName," order by ccy,Tenor;");
				mydb.prepare((SQLCHAR *)lineBuffer, 3); 	retcode = mydb.fetch(true);
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					char   *ccy = szAllBufs[0];
					char   *tenor = szAllBufs[1];
					char   *tickerString = szAllBufs[2];
					double  thePrice = -1.0;
					getBbergPrices(tickerString, "PX_LAST", thisDate, thisDate, ref, session, "HistoricalDataRequest", resultBuffer);
					thePrice = atof(resultBuffer);
					if (1 /* thePrice > 0.0 */ ){    // advent of negative rates
						sprintf(charBuffer, "%s%s%s%.4lf%s%s%s%s%s",
							"update ",thisName," set rate='", thePrice, "' where Tenor='", tenor, "' and ccy='", ccy, "';");
						mydb1.prepare((SQLCHAR *)charBuffer, 1);
					}
					else {
						std::cerr << "Failed to get prices for " << tickerString << std::endl;
					}
					retcode = mydb.fetch(false);
				}
			}
		}



		//
		// get CDS info
		//
		char   *fields[] = { "RTG_SP_LT_LC_ISSUER_CREDIT", "CUR_MKT_CAP", "EQY_FUND_CRNCY", "BS_TIER1_CAP_RATIO", "RTG_MDY_SEN_UNSECURED_DEBT",
			"RTG_FITCH_SEN_UNSECURED", "RSK_BB_ISSUER_LIKELIHOOD_OF_DFLT", "" };
		char   *sovereignFields[] ={ "RTG_SP_LT_LC_ISSUER_CREDIT", "CUR_MKT_CAP", "EQY_FUND_CRNCY", "BS_TIER1_CAP_RATIO", "RTG_MOODY_LONG_TERM",
			"RTG_FITCH_LT_LC_ISSUER_DEFAULT", "RSK_BB_ISSUER_LIKELIHOOD_OF_DFLT", "" };
		char   *drskFields[] ={ "RSK_BB_IMPLIED_CDS_SPREAD" };

		char   *fieldFormat[]   = { "%s", "%.2lf", "%s", "%.2lf", "%s", "%s", "%s" };
		double  fieldScaling[]  = { 0, 1000000000.0, 0, 1, 0, 0, 0 };
		bool    fieldEscalate[] = { false, true, false, true, false, false, true }; 
		char   *fieldName[]     = { "SPrating", "MarketCap", "Currency", "TierOne", "Moody", "Fitch", "drsk1yprobdefault" };
		if (!doDebug  && doCDS) {
			if (!doDebug) {				
				// now get info for each institution
				sprintf(lineBuffer, "%s%s%s%s",
					"select distinct cp.institutionid, cp.name,cds.maturity,cds.bberg,cp.bberg,cp.BbergIssuerPrice from  ",
					" product p join institution i on (p.CounterpartyId=i.InstitutionId),institution cp left join cdsspread cds using (institutionid)  where p.ProductId ",
					idBuffer, " and i.entityname like concat('%',cp.entityname,'%') order by institutionId,maturity;");
				mydb.prepare((SQLCHAR *)lineBuffer, 6); 	retcode = mydb.fetch(true);
				int     institutionId = -1, previousInstitutionId = -1;
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					institutionId             = atoi(szAllBufs[0]);
					char *cpName              = szAllBufs[1];
					char *mat                 = szAllBufs[2];
					char *cdsBberg            = szAllBufs[3];
					char *cpBberg             = szAllBufs[4];
					char *cpBbergIssuerPrice  = szAllBufs[5];
					double  thePrice          = -1.0;
					bool    isaSovereign      = false;
					// first record - get static data for institution
					if (doStatic && (institutionId != previousInstitutionId)){
						// detect if institution is a sovereign
						getBbergPrices(cpBberg, "COMPANY_IS_PRIVATE", thisDate, thisDate, ref, session, "ReferenceDataRequest", resultBuffer);
						if (!strcmp(resultBuffer, "")){
							isaSovereign = true;
						}
						previousInstitutionId = institutionId;

						for (int i = 0; strcmp(fields[i], ""); i++){
							char *fieldPtr = isaSovereign ? sovereignFields[i] : fields[i];
							strcpy(resultBuffer, "");
							getBbergPrices(cpBberg, fieldPtr, thisDate, thisDate, ref, session, "ReferenceDataRequest", resultBuffer);
							if (!strcmp(resultBuffer, "")) {
								// if that field does not exist, try the sovereign version of FLDS - eg InvestecBank has its MOODY credit rating there ...
								if (!isaSovereign){
									getBbergPrices(cpBberg, sovereignFields[i], thisDate, thisDate, ref, session, "ReferenceDataRequest", resultBuffer);
								}
								// if that field does not exist, try cpBbergIssuerPrice (typically the group parent)
								if (!strcmp(resultBuffer, "") && fieldEscalate[i]){
									getBbergPrices(cpBbergIssuerPrice, fieldPtr, thisDate, thisDate, ref, session, "ReferenceDataRequest", resultBuffer);
									if (!isaSovereign){
										getBbergPrices(cpBbergIssuerPrice, sovereignFields[i], thisDate, thisDate, ref, session, "ReferenceDataRequest", resultBuffer);
									}
								}
							}
							if (strcmp(resultBuffer, "")){
								if (fieldScaling[i]){
									sprintf(resultBuffer, fieldFormat[i], atof(resultBuffer) / fieldScaling[i]);
								}
								sprintf(charBuffer, "%s%s%s%s%s%d%s", "update institution set ", fieldName[i], "='", resultBuffer, "' where institutionid='", institutionId, "';");
								mydb1.prepare((SQLCHAR *)charBuffer, 1);
							}
						}
					}
					// get CDS rates
					if (strcmp(cdsBberg, "") || strcmp(cpBberg, "")){
						strcpy(resultBuffer, "");
						strcpy(charBuffer, strcmp(cdsBberg, "") ? cdsBberg:cpBberg);
						// if no CDS use DRSK-implied CDS
						anyString = strcmp(cdsBberg, "")  && strstr(cdsBberg, "Corp") != NULL ? "PX_LAST" : "RSK_BB_IMPLIED_CDS_SPREAD";
						int counter = 3;
						char newDateStr[9];
						strcpy(newDateStr, thisDate);
						do {
							getBbergPrices(charBuffer, anyString.c_str(), newDateStr, newDateStr, ref, session, "HistoricalDataRequest", resultBuffer);
							DatePlusDays(newDateStr, -1, newDateStr);
						} while (counter-- > 0 && !strcmp(resultBuffer, ""));

						if (strcmp(resultBuffer, "")){
							sprintf(charBuffer, "%s%s%s%s%s%d%s%s%s", "update cdsspread set Spread='", resultBuffer, "',LastDataDate='",isoDate,"' where institutionid='", institutionId, "' and Maturity='", mat, "';");
							mydb1.prepare((SQLCHAR *)charBuffer, 1);
						}
						strcpy(cdsBberg, ""); // some institutions have no CDS...so MySQL will return a NULL, leaving cdsBberg at its old value, typically for the previous institution 
						strcpy(cpBberg, "");
					}
					retcode = mydb.fetch(false);
				}
			}

			//
			// now handle issuers with MULTI in name - assume all sub-names equally weighted
			//
			sprintf(lineBuffer, "%s", "select institutionid,EntityName from  institution where name like 'MULTI%' ");
			mydb.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				int institutionId = atoi(szAllBufs[0]);
				char subNameBuffer[1000];
				char* multiName = szAllBufs[1];
				vector<string>  subNames;

				// split into subnames
				char *cptr = strtok(multiName, ",");
				while (cptr != NULL){
					subNames.push_back(cptr);
					cptr = strtok(NULL, ",");
				}
				// get sub-names fields
				strcpy(subNameBuffer, "");
				sprintf(lineBuffer, "%s", "select institutionid,SPrating,replace(Moody,'(P)','') Moody,Fitch,TierOne,drsk1yprobdefault drsk from institution where EntityName in (");
				for (int j = 0; j < subNames.size(); j++){
					if (j != 0){ strcat(subNameBuffer, ","); }
					sprintf(subNameBuffer, "%s%s%s%s", subNameBuffer, "'", subNames[j].c_str(), "'");
				}
				sprintf(lineBuffer, "%s%s%s", lineBuffer, subNameBuffer, ");");
				mydb1.prepare((SQLCHAR *)lineBuffer, 6); 	retcode = mydb1.fetch(true);
				vector<int> spRatings, moRatings, fiRatings;
				vector<double> tierOne, drskProbs;
				map<string, int> SPscore, MoScore, FiScore;
				SPscore["AAA"] = 1; SPscore["AA+"] = 2; SPscore["AA"] = 3;  SPscore["AA-"] = 4;  SPscore["A+"] = 5;  SPscore["A"] = 6;  SPscore["A-"] = 7;
				SPscore["BBB+"] = 8; SPscore["BBB"] = 9; SPscore["BBB-"] = 10; SPscore["BB+"] = 11; SPscore["BB"] = 12; SPscore["BB-"] = 13; SPscore["B+"] = 14;
				MoScore["Aaa"] = 1; MoScore["Aa1"] = 2; MoScore["Aa2"] = 3;  MoScore["Aa3"] = 4;  MoScore["A1"] = 5;  MoScore["A2"] = 6;  MoScore["A3"] = 7; MoScore["A"] = 7;
				MoScore["Baa1"] = 8; MoScore["Baa2"] = 9; MoScore["Baa3"] = 10; MoScore["B"] = 10;
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					if (strcmp(szAllBufs[1], "0") && strcmp(szAllBufs[1], "") && SPscore.find(szAllBufs[1]) != SPscore.end()) { spRatings.push_back(SPscore[szAllBufs[1]]); }
					if (strcmp(szAllBufs[2], "0") && strcmp(szAllBufs[2], "") && MoScore.find(szAllBufs[2]) != MoScore.end()) { moRatings.push_back(MoScore[szAllBufs[2]]); }
					if (strcmp(szAllBufs[3], "0") && strcmp(szAllBufs[3], "") && SPscore.find(szAllBufs[3]) != SPscore.end()) { fiRatings.push_back(SPscore[szAllBufs[3]]); }
					double thisTierOne = atof(szAllBufs[4]); if (thisTierOne > 0.0) { tierOne.push_back(thisTierOne); }
					double thisDrsk = atof(szAllBufs[5]); if (thisDrsk    > 0.0) { drskProbs.push_back(thisDrsk); }
					retcode = mydb1.fetch(false);
				}
				int thisSpScore = std::accumulate(spRatings.begin(), spRatings.end(), 0.0) / spRatings.size();
				string thisSpRating = FindMapKeyIndx(SPscore, thisSpScore);
				int thisMoScore = std::accumulate(moRatings.begin(), moRatings.end(), 0.0) / moRatings.size();
				string thisMoRating = FindMapKeyIndx(MoScore, thisMoScore);
				int thisFiScore = std::accumulate(fiRatings.begin(), fiRatings.end(), 0.0) / fiRatings.size();
				string thisFiRating = FindMapKeyIndx(SPscore, thisFiScore);
				double thisTierOneScore = std::accumulate(tierOne.begin(), tierOne.end(), 0.0) / tierOne.size();
				double thisDrskScore = std::accumulate(drskProbs.begin(), drskProbs.end(), 0.0) / drskProbs.size();
				// update static data for MULTI institution
				sprintf(charBuffer, "%s%s%s%s%s%s%s%lf,%s%lf%s%d%s",
					"update institution set SPrating='", thisSpRating.c_str(),
					"',Moody='", thisMoRating.c_str(),
					"',Fitch='", thisFiRating.c_str(),
					"',TierOne='", thisTierOneScore,
					"',drsk1yprobdefault='", thisDrskScore,
					"' where institutionid='", institutionId, "';");
				mydb1.prepare((SQLCHAR *)charBuffer, 1);

				// update CDS rates
				sprintf(lineBuffer, "%s", "select Maturity,avg(Spread) Spread from cdsspread join institution using (institutionid) where EntityName in (");
				sprintf(lineBuffer, "%s%s%s", lineBuffer, subNameBuffer, ") and Spread is not null group by Maturity order by Maturity;");
				mydb1.prepare((SQLCHAR *)lineBuffer, 2); 	retcode = mydb1.fetch(true);
				while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					sprintf(charBuffer, "%s%d%s%s%s%s%s%s%s",
						"replace into cdsspread values (", institutionId, ",", szAllBufs[0], ",", szAllBufs[1], ",'','", isoDate, "');");
					mydb2.prepare((SQLCHAR *)charBuffer, 1);
					retcode = mydb1.fetch(false);
				}
				// get next institution
				retcode = mydb.fetch(false);
			}
		}


		// tidy up
		session.stop();
		cout << std::endl << "DONE" << std::endl;
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



