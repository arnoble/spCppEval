// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Header.h"
using namespace std;


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
	SQLWCHAR              szDSN[] = L"newSp";       // Data Source Name buffer
	SQLWCHAR              szUID[] = L"root";		   // User ID buffer
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


int _tmain(int argc, char* argv[])
{
	const int bufSize(256);
	SQLHENV               hEnv       = NULL;		   // Env Handle from SQLAllocEnv()
	SQLHDBC               hDBC       = NULL;           // Connection handle
	HSTMT                 hStmt      = NULL;		   // Statement handle
	SQLWCHAR              szDSN[]    = L"newSp";       // Data Source Name buffer
	SQLWCHAR              szUID[]    = L"root";		   // User ID buffer
	SQLWCHAR              szPasswd[] = L"ragtinmor";   // Password buffer
	char                  szDate[bufSize];		       // Model buffer
	char                  szPrice[bufSize];		       // Model buffer
	SQLLEN                cbModel;		               // Model buffer bytes recieved
	RETCODE               retcode;
	SQLRETURN             fsts;
	
	// variables
	int numMcIterations = argc>1 ? atoi(argv[1]) : 100;
	int productId = 363;
	string productStartDateString("2014-01-03");
	cout << "Iterations:" << numMcIterations << " ProductId:" << productId << endl;


	boost::gregorian::date  bProductStartDate(boost::gregorian::from_simple_string(productStartDateString));
	unsigned	uI;
	int oldProductBarrierId = 0, productBarrierId = 0;
	int historyStep = 1;
	int numBarriers = 0, thisIteration = 0;
	int anyInt, i, j, k, len, callOrPut, thisPoint, thisBarrier, thisMonIndx, thisMonPoint, numUl, numMonPoints, lastPoint, productDays, totalNumDays, totalNumReturns, uid;
	int thisPayoffId, thisMonDays;
	double anyDouble, barrier, uBarrier, payoff, strike, cap, participation;
	string word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate, description;
	bool capitalOrIncome, above, at;
	vector<double> ulReturns;
	vector<int>    monDateIndx;
	vector<string> payoffType = { "", "fixed", "call", "put", "twinWin", "switchable", "basketCall", "lookbackCall" };
	vector<int>::iterator intIterator;
	char lineBuffer[1000], charBuffer[1000];

	// get underlying prices
	vector<UlTimeseries> ulOriginalPrices(1), ulPrices(1);

















	// open database
	SQLAllocEnv(&hEnv);        	  // Allocate memory for ODBC Environment handle
	fsts = dbConn(hEnv, &hDBC);   // connect
	if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO) {	exit(1);}



	// read underlying prices
	retcode = SQLAllocStmt(hDBC, &hStmt); 			// Allocate memory for statement handle
	SQLCHAR*  thisQ = (SQLCHAR *)"select Date,Price from prices where UnderlyingId='1' ";
	retcode = SQLPrepareA(hStmt, thisQ, SQL_NTS);   // Prepare the SQL statement	
	fsts = SQLExecute(hStmt);                       // Execute the SQL statement
	if (!SQL_SUCCEEDED(fsts))	{ extract_error("SQLExecute", hStmt, SQL_HANDLE_STMT);	exit(1); }		
	SQLBindCol(hStmt, 1, SQL_C_CHAR, szDate,  bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, 2, SQL_C_CHAR, szPrice, bufSize, &cbModel); // bind columns

	// Get row of data from the result set defined above in the statement
	bool firstTime(true);
	double previousPrice;
	boost::gregorian::date lastDate;
	retcode = SQLFetch(hStmt);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		int numDayDiff;
		double thisPrice;
		thisPrice = atof(szPrice);
		// pad non-trading days
		{
			using namespace boost::gregorian;
			date bDate(from_simple_string(szDate));
			if (!firstTime) {
				double thisReturn = thisPrice / previousPrice;
				ulReturns.push_back(thisReturn);
				date_duration dateDiff(bDate - lastDate);
				numDayDiff = dateDiff.days();
				while (numDayDiff>1){
					ulOriginalPrices.at(0).date.push_back(word);
					ulOriginalPrices.at(0).price.push_back(thisPrice);
					ulReturns.push_back(1.0);
					numDayDiff -= 1;
				}
			}
			else{ firstTime = false; }
			lastDate = bDate;
			previousPrice = thisPrice;
		}
		// printf("%s\t%s\n",szDate,szPrice);  // Print row (model)
		ulOriginalPrices.at(0).date.push_back(szDate);
		ulOriginalPrices.at(0).price.push_back(thisPrice);
		retcode = SQLFetch(hStmt);  // Fetch next row from result set
	}
	totalNumDays = ulOriginalPrices.at(0).price.size();
	totalNumReturns = totalNumDays - 1;
	numUl = ulOriginalPrices.size();
	ulPrices = ulOriginalPrices; // copy onstructor called
	vector<double> thesePrices(numUl), startLevels(numUl);
	vector<int> ulIdNameMap = { -1, 0 };




	// get product
	SProduct spr;
	numBarriers = 0;
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
				pbFile.getline(lineBuffer, 1000);  description = lineBuffer;
				pbFile.getline(lineBuffer, 1000);  thisPayoffId = atoi(lineBuffer); thisPayoffType = payoffType[thisPayoffId];// PayoffTypeId
				pbFile.getline(lineBuffer, 1000);  participation = atof(lineBuffer); // Participation
				pbFile.getline(lineBuffer, 1000);  strike = atof(lineBuffer);  // PayoffStrike
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
			double thisReturn = ulReturns[thisIndx];
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
	SQLFreeStmt(hStmt, SQL_DROP);  // Free the allocated statement handle
	SQLDisconnect(hDBC);           // Disconnect from datasource
	SQLFreeConnect(hDBC); // Free the allocated connection handle
	SQLFreeEnv(hEnv);    // Free the allocated ODBC environment handle
	return 0;
}

