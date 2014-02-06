// bberg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header.h"
using namespace BloombergLP; 
using namespace blpapi;
using namespace std;



int _tmain(int argc, _TCHAR* argv[])
{
	try{
		// initialise
		if (argc < 3){ cout << "Usage: startId stopId <optionalArguments>" << endl;  exit(0); }
		int              startProductId  = argc > 1 ? _ttoi(argv[1]) : 34;
		int              stopProductId   = argc > 2 ? _ttoi(argv[2]) : 1000;
	

		// init
		const int        maxBufs(100);
		const int        bufSize(1000);
		SQLHENV          hEnv = NULL;		  // Env Handle from SQLAllocEnv()
		SQLHDBC          hDBC = NULL;         // Connection handle
		RETCODE          retcode;
		char             lineBuffer[1000], charBuffer[1000];
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
		MyDB  mydb((char **)szAllBufs), mydb1((char **)szAllBufs);

		// get list of productIds
		sprintf(lineBuffer, "%s%d%s%d%s", "select ProductId from product where ProductId>='", startProductId, "' and ProductId<='", stopProductId, "' and Matured='0'"); 	mydb.prepare((SQLCHAR *)lineBuffer, 1); 	retcode = mydb.fetch(true);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			int x(atoi(szAllBufs[0])); allProductIds.push_back(x);
			retcode = mydb.fetch(false);
		}

		// get BID,ASK prices for productIds
		char *bidAskFields[] ={ "PX_BID", "PX_ASK" };
		char *lastFields[]   ={ "PX_LAST", "PX_LAST" };
		sprintf(lineBuffer, "%s%d%s%d%s", "select ProductId, p.name, p.StrikeDate, cp.name, if (p.BbergTicker != '', p.BbergTicker, Isin) Isin, BbergPriceFeed from product p join institution cp on(p.CounterpartyId=cp.institutionid) where Isin != ''  and productid>='", startProductId, "' and ProductId<='", stopProductId, "' and Matured='0' order by ProductId; ");
		mydb.prepare((SQLCHAR *)lineBuffer, 6); 	retcode = mydb.fetch(true);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			int id(atoi(szAllBufs[0]));
			string tickerString = szAllBufs[4];
			bool found = tickerString.find("Equity") == std::string::npos;
			if (found) {
				sprintf(charBuffer, "%s%s%s%s", "/isin/",szAllBufs[4], "@", szAllBufs[5]);
			}
			else {
				strcpy(charBuffer, szAllBufs[4]);
			}
			BbergData thisBbergData = getBbergPrice(charBuffer, found ? bidAskFields : lastFields, ref, session);
			sprintf(charBuffer, "%s%.2lf%s%.2lf%s%d%s", "update product set bid='", thisBbergData.p[0], "',ask='", thisBbergData.p[1],"' where productid='", id, "';");
			mydb1.prepare((SQLCHAR *)charBuffer, 1);

			retcode = mydb.fetch(false);
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

