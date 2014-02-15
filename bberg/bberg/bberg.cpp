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
	size_t origsize      = wcslen(orig) + 1;
	const size_t newsize = origsize * 2;
	char *thisDate       = new char[newsize];
	*convertedChars      = 0;
	wcstombs_s(convertedChars, thisDate, newsize, orig, _TRUNCATE);
	return thisDate;
}


int _tmain(int argc, _TCHAR* argv[])
{
	const bool doDebug(true);
	try{
		// initialise
		if (argc < 4){ std::cout << "Usage: startId stopId dateAsYYYYMMDD  <optionalArguments>" << endl;  exit(0); }
		int              startProductId  = argc > 1 ? _ttoi(argv[1]) : 34;
		int              stopProductId   = argc > 2 ? _ttoi(argv[2]) : 1000;
		size_t numChars;
		char *thisDate = WcharToChar(argv[3], &numChars);
		

		// init
		const int        maxBufs(100);
		const int        bufSize(1000);
		SQLHENV          hEnv = NULL;		  // Env Handle from SQLAllocEnv()
		SQLHDBC          hDBC = NULL;         // Connection handle
		RETCODE          retcode;
		char             lineBuffer[1000], charBuffer[1000], resultBuffer[1000];
		char             **szAllBufs = new char*[maxBufs];
		vector<int>      allProductIds; allProductIds.reserve(1000);
		for (int i = 0; i < maxBufs; i++){
			szAllBufs[i] = new char[bufSize];
		}



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




		// open database
		MyDB  mydb((char **)szAllBufs, "spIPRL"), mydb1((char **)szAllBufs, "spIPRL");  // production
		//MyDB  mydb((char **)szAllBufs, "spIPRL"), mydb1((char **)szAllBufs, "newSp"); // local

		// get list of productIds
		sprintf(lineBuffer, "%s%d%s%d%s", "select ProductId from product where ProductId>='", startProductId, "' and ProductId<='", stopProductId, "' and Matured='0'"); 	mydb.prepare((SQLCHAR *)lineBuffer, 1); 	retcode = mydb.fetch(true);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			int x(atoi(szAllBufs[0])); allProductIds.push_back(x);
			retcode = mydb.fetch(false);
		}


		//
		// get BID,ASK prices for productIds
		//
		if (doDebug){
			char *bidAskFields[] ={ "PX_BID", "PX_ASK" };
			char *lastFields[]   ={ "PX_LAST", "PX_LAST" };
			sprintf(lineBuffer, "%s%d%s%d%s", "select ProductId, p.name, p.StrikeDate, cp.name, if (p.BbergTicker != '', p.BbergTicker, Isin) Isin, BbergPriceFeed from product p join institution cp on(p.CounterpartyId=cp.institutionid) where Isin != ''  and productid>='", startProductId, "' and ProductId<='", stopProductId, "' and Matured='0' order by ProductId; ");
			mydb.prepare((SQLCHAR *)lineBuffer, 6); 	retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				int id(atoi(szAllBufs[0]));
				string tickerString = szAllBufs[4];
				bool found = tickerString.find("Equity") == std::string::npos;
				if (found) {
					sprintf(charBuffer, "%s%s%s%s", "/isin/", szAllBufs[4], "@", szAllBufs[5]);
				}
				else {
					strcpy(charBuffer, szAllBufs[4]);
				}
				std::cout << "\ngetting prices for " << id << std::endl;
				BbergData thisBbergData = getBbergBidOfferPrices(charBuffer, found ? bidAskFields : lastFields, thisDate, ref, session);
				if (thisBbergData.p[0] > 0.0 && thisBbergData.p[1] > 0.0){
					sprintf(charBuffer, "%s%.2lf%s%.2lf%s%d%s", "update product set bid='", thisBbergData.p[0], "',ask='", thisBbergData.p[1], "' where productid='", id, "';");
					mydb1.prepare((SQLCHAR *)charBuffer, 1);
				}
				else {
					std::cerr << "Failed to get prices for " << id << std::endl;
				}

				retcode = mydb.fetch(false);
			}

		}
		
		//
		// get curves
		//
		if (doDebug){
			sprintf(lineBuffer, "%s", "select ccy,tenor,bberg from curve order by ccy,Tenor;");
			mydb.prepare((SQLCHAR *)lineBuffer, 3); 	retcode = mydb.fetch(true);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				char   *ccy          = szAllBufs[0];
				char   *tenor        = szAllBufs[1];
				char   *tickerString = szAllBufs[2];
				double  thePrice     = -1.0;
				getBbergPrices(tickerString, "PX_LAST", thisDate, thisDate, ref, session, "HistoricalDataRequest", resultBuffer);
				thePrice = atof(resultBuffer);
				if (thePrice > 0.0){
					sprintf(charBuffer, "%s%.4lf%s%s%s%s%s",
						"update curve set rate='", thePrice, "' where Tenor='", tenor, "' and ccy='", ccy, "';");
					mydb1.prepare((SQLCHAR *)charBuffer, 1);
				}
				else {
					std::cerr << "Failed to get prices for " << tickerString << std::endl;
				}
				retcode = mydb.fetch(false);
			}
		}



			//
			// get CDS info
			//
		if (doDebug) {
			sprintf(lineBuffer, "%s%s%s",
				"select distinct cp.institutionid, cp.name,cds.maturity,cds.bberg,cp.bberg from  ",
				" institution cp left join cdsspread cds using (institutionid) ",
				" order by institutionId,maturity;");
			mydb.prepare((SQLCHAR *)lineBuffer, 5); 	retcode = mydb.fetch(true);
			int institutionId = -1;
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				char *cpName   = szAllBufs[1];
				char *mat      = szAllBufs[2];
				char *cdsBberg = szAllBufs[3];
				char *cpBberg  = szAllBufs[4];
				double  thePrice     = -1.0;
				// first record - get static data for institution
				if (institutionId != atoi(szAllBufs[0])){
					institutionId  = atoi(szAllBufs[0]);
					char *fields[] ={ "RTG_SP_LT_LC_ISSUER_CREDIT", "CUR_MKT_CAP", "EQY_FUND_CRNCY", "BS_TIER1_CAP_RATIO", "RTG_MOODY_LONG_TERM",
						"RTG_FITCH_LT_ISSUER_DEFAULT", "RSK_BB_ISSUER_LIKELIHOOD_OF_DFLT","" };
					char *fieldFormat[] = { "%s", "%.2lf", "%s", "%.2lf", "%s","%s", "%s" };
					double fieldScaling[] ={ 0, 1000000000.0, 0, 1, 0, 0, 0 };
					char *fieldName[] ={"SPrating",	"MarketCap","Currency","TierOne","Moody","Fitch","drsk1yprobdefault"};
					for (int i=0; strcmp(fields[i],""); i++){
						strcpy(resultBuffer,"");
						getBbergPrices(cpBberg, fields[i], thisDate, thisDate, ref, session, "ReferenceDataRequest", resultBuffer);
						if (strcmp(resultBuffer,"")){
							if (fieldScaling[i]){
								sprintf(resultBuffer, fieldFormat[i], atof(resultBuffer) / fieldScaling[i]);
							}
							sprintf(charBuffer, "%s%s%s%s%s%d%s", "update institution set ", fieldName[i], "='", resultBuffer, "' where institutionid='", institutionId, "';");
							mydb1.prepare((SQLCHAR *)charBuffer, 1);
						}
					}
				}
				// get CDS rates
				if (strcmp(cdsBberg,"")){
					strcpy(resultBuffer, "");
					getBbergPrices(cdsBberg, "PX_LAST", thisDate, thisDate, ref, session, "HistoricalDataRequest", resultBuffer);
					if (strcmp(resultBuffer, "")){
						sprintf(charBuffer, "%s%s%s%d%s%s%s", "update cdsspread set Spread='", resultBuffer, "' where institutionid='", institutionId, "' and Maturity='", mat, "';");
						mydb1.prepare((SQLCHAR *)charBuffer, 1);
					}
					strcpy(cdsBberg,""); // some institutions have no CDS...so MySQL will return a NULL, leaving cdsBberg at its old value, typically for the previous institution 
				}
				
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

