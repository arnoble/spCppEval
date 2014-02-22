#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <windows.h>
#include <sqlext.h>
#include <stdio.h>
#include <vector>
#include <regex>
#include <blpapi_session.h> 
#include <blpapi_event.h> 
#include <blpapi_message.h> 
#include <blpapi_request.h> 
#include <iostream> 
using namespace BloombergLP;
using namespace blpapi;

// *************** STRUCTS
typedef struct bbergData { double p[2]; } BbergData;



// Bberg send request to service
BbergData getBbergBidOfferPrices(char *ticker, char **fields, char *thisDate, blpapi::Service &ref, blpapi::Session &session){
	BbergData thePrices; thePrices.p[0] = thePrices.p[1] = -1.0;
	
	blpapi::Request request = ref.createRequest("HistoricalDataRequest");
	request.getElement("securities").appendValue(ticker);
	request.getElement("fields").appendValue(fields[0]);
	request.getElement("fields").appendValue(fields[1]);

		//request.set("periodicityAdjustment", "ACTUAL");
	request.set("periodicitySelection", "DAILY");
	request.set("startDate", thisDate);
	request.set("endDate", thisDate);
	//request.set("maxDataPoints", 1);
	session.sendRequest(request);
	//request.print(std::cout);

	// process response
	bool done(false);
	while (!done) {
		Event event = session.nextEvent();
		MessageIterator msgIter(event);
		while (msgIter.next()) {
			Message msg = msgIter.message();
			if (Event::RESPONSE == event.eventType() || Event::PARTIAL_RESPONSE == event.eventType()) {
				if (msg.hasElement("securityData")) {
					done = true;
					Element secs = msg.getElement("securityData");
					std::cout << "\n" << secs.getElementAsString("security") << std::endl;
					//secs.print(std::cout);
					Element flds = secs.getElement("fieldData");
					// We get an array of value for the historical request.
					std::cout << "Date\t\tPX_LAST" << std::endl;
					for (int i = 0; i < flds.numValues(); ++i) {
						Element f = flds.getValueAsElement(i);
						f.print(std::cout);
						for (int j = 0; j <2; ++j) {
							if (f.hasElement(fields[j])){
								thePrices.p[j] = f.getElementAsFloat64(fields[j]);
								std::cout << f.getElementAsString("date") << "\t" << thePrices.p[j] << std::endl;
								// in case we only get 1 price
								if (j == 0){                    thePrices.p[1] =thePrices.p[0]; }
								else if (thePrices.p[0] < 0.0){ thePrices.p[0] =thePrices.p[1]; }
							}
							else {
								std::cout << "\n no element for " << fields[j]  << std::endl;
							}
						}
						
					}
				}
			}
			else {
				//std::cout << msg << std::endl;
			}
		}
	}
	return(thePrices);
}

void getBbergPrices(char *ticker, char *field, char *startDate, char *endDate, blpapi::Service &ref, blpapi::Session &session, char *reqType,char *result){

	bool  histDataRequest = strcmp(reqType, "HistoricalDataRequest") == 0;
	blpapi::Request request = ref.createRequest(reqType);
	request.getElement("securities").appendValue(ticker);
	request.getElement("fields").appendValue(field);

	//request.set("periodicityAdjustment", "ACTUAL");
	if (histDataRequest){
		request.set("periodicitySelection", "DAILY");
		request.set("startDate", startDate);
		request.set("endDate", endDate);
		//request.set("maxDataPoints", 1);
	}
	session.sendRequest(request);
	//request.print(std::cout);

	// process response
	bool done(false);
	while (!done) {
		Event event = session.nextEvent();
		MessageIterator msgIter(event);
		while (msgIter.next()) {
			Message msg = msgIter.message();
			msg.print(std::cout);
			if (Event::RESPONSE == event.eventType() || Event::PARTIAL_RESPONSE == event.eventType()) {
				if (msg.hasElement("securityData")) {
					done = true;
					Element secs = msg.getElement("securityData");
					secs.print(std::cout);
					Element securityData = histDataRequest ? secs : secs.getValueAsElement(0); // should only be one
					std::cout << "\n" << securityData.getElementAsString("security") << std::endl;
					Element flds = securityData.getElement("fieldData");
					flds.print(std::cout);
					std::cout << "\nNumElements" << flds.numElements();
					std::cout << "\nNumValues" << flds.numValues();
					// We get an array of value for the historical request.
					int numItems = flds.numValues();
					if (histDataRequest && numItems){
						flds = flds.getValueAsElement(0);
					}
					if (flds.hasElement(field)){
							sprintf(result, "%s", flds.getElementAsString(field));
							std::cout << field << "\t" << *result << std::endl;
						}
					else {
							std::cout << "\n no element for " << field << std::endl;
					}
				}
			}
			else {
				//std::cout << msg << std::endl;
			}
		}
	}
	return;
}




class MyDB {
private:
	SQLHENV   hEnv;
	SQLHDBC   hDBC;
	SQLRETURN fsts;
	const int bufSize=256;
	SQLLEN    cbModel;		               // Model buffer bytes recieved
	HSTMT     hStmt;
	char     **bindBuffer;

public:
	// get info on SQL error
	void extract_error(
		char *fn,
		SQLHANDLE handle,
		SQLSMALLINT type)
	{
		SQLINTEGER  i = 0;
		SQLINTEGER  native;
		SQLWCHAR    state[7];
		SQLWCHAR    text[256];
		SQLSMALLINT len;
		SQLRETURN   ret;
		fprintf(stderr,
			"\n"
			"The driver reported the following diagnostics whilst running "
			"%s\n\n",
			fn);
		do
		{
			ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
			if (SQL_SUCCEEDED(ret))
				printf("%s:%ld:%ld:%s\n", state, i, native, text);
		} while (ret == SQL_SUCCESS);
	}
	// open connection to DataSource newSp
	SQLRETURN dbConn(SQLHENV hEnv, SQLHDBC* hDBC) {
		SQLWCHAR              szDSN[]    = L"newSp";       // Data Source Name buffer
		SQLWCHAR              szUID[]    = L"root";		   // User ID buffer
		SQLWCHAR              szPasswd[] = L"ragtinmor";   // Password buffer
		SQLRETURN             fsts;

		fsts = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, hDBC);  // Allocate memory for the connection handle
		if (!SQL_SUCCEEDED(fsts))	{
			extract_error("SQLAllocHandle for dbc", hEnv, SQL_HANDLE_ENV);
			exit(1);
		}
		// Connect to Data Source
		fsts = SQLConnect(*hDBC, szDSN, SQL_NTS, szUID, SQL_NTS, szPasswd, SQL_NTS); // use SQL_NTS for length...NullTerminatedString
		if (!SQL_SUCCEEDED(fsts))	{
			extract_error("SQLAllocHandle for connect", hDBC, SQL_HANDLE_DBC);
			exit(1);
		}
		return fsts;
	}
	// open connection to DataSource spIPRL
	SQLRETURN dbConnIPRL(SQLHENV hEnv, SQLHDBC* hDBC) {
		SQLWCHAR              szDSN[]    = L"spIPRL";       // Data Source Name buffer
		SQLWCHAR              szUID[]    = L"C85693_anoble";		   // User ID buffer
		SQLWCHAR              szPasswd[] = L"Ragtin_Mor14";   // Password buffer
		SQLRETURN             fsts;

		fsts = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, hDBC);  // Allocate memory for the connection handle
		if (!SQL_SUCCEEDED(fsts))	{
			extract_error("SQLAllocHandle for dbc", hEnv, SQL_HANDLE_ENV);
			exit(1);
		}
		// Connect to Data Source
		fsts = SQLConnect(*hDBC, szDSN, SQL_NTS, szUID, SQL_NTS, szPasswd, SQL_NTS); // use SQL_NTS for length...NullTerminatedString
		if (!SQL_SUCCEEDED(fsts))	{
			extract_error("SQLAllocHandle for connect", hDBC, SQL_HANDLE_DBC);
			exit(1);
		}
		return fsts;
	}


	MyDB(char **bindBuffer,const std::string dataSource) : bindBuffer(bindBuffer){
		SQLAllocEnv(&hEnv);
		fsts = dataSource == "spIPRL" ? dbConnIPRL(hEnv, &hDBC) : dbConn(hEnv, &hDBC);              // connect
		if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO) { exit(1); }
	};
	~MyDB(){
		SQLDisconnect(hDBC);           // Disconnect from datasource
		SQLFreeConnect(hDBC); // Free the allocated connection handle
		SQLFreeEnv(hEnv);    // Free the allocated ODBC environment handle
	}
	void prepare(SQLCHAR* thisSQL, int numCols) {
		if (hStmt != NULL) {
			SQLFreeStmt(hStmt, SQL_DROP);
		}
		fsts  =  SQLAllocStmt(hDBC, &hStmt); 	 // Allocate memory for statement handle
		fsts  =  SQLPrepareA(hStmt, thisSQL, SQL_NTS);                 // Prepare the SQL statement	
		fsts  =  SQLExecute(hStmt);                                     // Execute the SQL statement
		if (!SQL_SUCCEEDED(fsts))	{ extract_error("SQLExecute get basic info ", hStmt, SQL_HANDLE_STMT);	exit(1); }
		for (int i = 0; i < numCols; i++){
			SQLBindCol(hStmt, i + 1, SQL_C_CHAR, bindBuffer[i], bufSize, &cbModel); // bind columns
		}
	}
	void bind(int col, char *buffer) {
		SQLBindCol(hStmt, col, SQL_C_CHAR, buffer, bufSize, &cbModel); // bind columns
	}
	SQLRETURN fetch(bool checkForErrors){
		fsts = SQLFetch(hStmt);
		if (checkForErrors){
			if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO)	{ extract_error("SQLFetch", hStmt, SQL_HANDLE_STMT);	exit(1); }
		}
		return fsts;
	}
	SQLRETURN execute(bool checkForErrors){
		fsts = SQLExecute(hStmt);
		if (checkForErrors){
			if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO)	{ extract_error("SQLExecute", hStmt, SQL_HANDLE_STMT);	exit(1); }
		}
		return fsts;
	}

};
