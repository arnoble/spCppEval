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


int _tmain(int argc, TCHAR* argv[])
{
	const int bufSize(256);
	SQLHENV               hEnv       = NULL;		   // Env Handle from SQLAllocEnv()
	SQLHDBC               hDBC       = NULL;           // Connection handle
	HSTMT                 hStmt = NULL, hStmt1 = NULL, hStmt2 = NULL, hStmt3 = NULL;		   // Statement handle
	SQLWCHAR              szDSN[]    = L"newSp";       // Data Source Name buffer
	SQLWCHAR              szUID[]    = L"root";		   // User ID buffer
	SQLWCHAR              szPasswd[] = L"ragtinmor";   // Password buffer
	char                  szDate[bufSize];	
	char                  szPrice[bufSize],szAllPrices[bufSize][100];
	SQLLEN                cbModel;		               // Model buffer bytes recieved
	RETCODE               retcode;
	SQLRETURN             fsts;
	SQLCHAR*              thisSQL;

	// variables
	srand(time(0)); // reseed rand
	int productId = argc > 1 ? _ttoi(argv[1]) : 363;
	int numMcIterations = argc>2 ? _ttoi(argv[2]) : 100;
	string productStartDateString;
	cout << "Iterations:" << numMcIterations << " ProductId:" << productId << endl;


	unsigned	uI;
	int oldProductBarrierId = 0, productBarrierId = 0;
	int historyStep = 1;
	int numBarriers = 0, thisIteration = 0;
	int anyInt, i, j, k, len, callOrPut, thisPoint, thisBarrier, thisMonIndx, thisMonPoint, numUl, numMonPoints, lastPoint, productDays, totalNumDays, totalNumReturns, uid;
	int thisPayoffId, thisMonDays;
	double anyDouble, barrier, uBarrier, payoff, strike, cap, participation;
	string word, word1, thisPayoffType, startDateString, endDateString, nature, settlementDate, description;
	bool capitalOrIncome, above, at;
	vector<double> ulReturns[50];
	vector<int>    monDateIndx;
	vector<string> payoffType = { "", "fixed", "call", "put", "twinWin", "switchable", "basketCall", "lookbackCall" };
	vector<int>::iterator intIterator;
	char lineBuffer[1000], charBuffer[1000];
	vector<UlTimeseries> ulOriginalPrices(100), ulPrices(100); // underlying prices

	

	// open database
	SQLAllocEnv(&hEnv);        	             // Allocate memory for ODBC Environment handle
	fsts = dbConn(hEnv, &hDBC);              // connect
	if (fsts != SQL_SUCCESS && fsts != SQL_SUCCESS_WITH_INFO) {	exit(1);}
	retcode = SQLAllocStmt(hDBC, &hStmt); 	 // Allocate memory for statement handle
	retcode = SQLAllocStmt(hDBC, &hStmt1); 	 // Allocate memory for statement handle
	retcode = SQLAllocStmt(hDBC, &hStmt2); 	 // Allocate memory for statement handle
	retcode = SQLAllocStmt(hDBC, &hStmt3); 	 // Allocate memory for statement handle

	// get product from DB
	// ** SQL fetch block
	sprintf(lineBuffer, "%s%d%s", "select StrikeDate from product where ProductId='", productId, "'");
	thisSQL = (SQLCHAR *)lineBuffer;
	retcode = SQLPrepareA(hStmt2, thisSQL, SQL_NTS);                 // Prepare the SQL statement	
	fsts = SQLExecute(hStmt2);                                     // Execute the SQL statement
	if (!SQL_SUCCEEDED(fsts))	{ extract_error("SQLExecute get basic info ", hStmt2, SQL_HANDLE_STMT);	exit(1); }
	// ** end SQL fetch block
	SQLBindCol(hStmt2, 1, SQL_C_CHAR, lineBuffer, bufSize, &cbModel); // bind columns
	retcode = SQLFetch(hStmt2);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)	{ extract_error("SQLExecute", hStmt2, SQL_HANDLE_STMT);	exit(1); }
	productStartDateString = lineBuffer;
	boost::gregorian::date  bProductStartDate(boost::gregorian::from_simple_string(productStartDateString));

	// get underlyingids for this product from DB
	vector<int> ulIds;
	vector<int> ulIdNameMap(1000);
	// ** SQL fetch block
	sprintf(lineBuffer, "%s%d%s", "select distinct UnderlyingId from productbarrier join barrierrelation using (ProductBarrierId) where ProductId='", productId, "'");
	thisSQL = (SQLCHAR *)lineBuffer;
	retcode = SQLPrepareA(hStmt3, thisSQL, SQL_NTS);                 // Prepare the SQL statement	
	fsts = SQLExecute(hStmt3);                                     // Execute the SQL statement
	if (!SQL_SUCCEEDED(fsts))	{ extract_error("SQLExecute get underlying ids ", hStmt3, SQL_HANDLE_STMT);	exit(1); }
	// ** end SQL fetch block
	SQLBindCol(hStmt3, 1, SQL_C_CHAR, lineBuffer, bufSize, &cbModel); // bind columns
	retcode = SQLFetch(hStmt3);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		uid = atoi(lineBuffer);
		if (find(ulIds.begin(), ulIds.end(), uid) == ulIds.end()) {      // build list of uids
			ulIds.push_back(uid);
		}
		ulIdNameMap[uid] = ulIds.size()-1;
		// next record
		retcode = SQLFetch(hStmt3);
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
	if (numMcIterations) { strcat(ulSql, " and Date >='1992-12-31'"); }
	strcat(ulSql," order by Date");


	// bind DB prices to szAllPrices
	//sprintf(lineBuffer, "%s", "select Date,Price from prices where UnderlyingId='1' ");
	//if (numMcIterations) { strcat(lineBuffer, " and Date >='1992-12-31'"); }
	//thisSQL = (SQLCHAR *)lineBuffer;
	thisSQL = (SQLCHAR *)ulSql;
	retcode = SQLPrepareA(hStmt, thisSQL, SQL_NTS);                 // Prepare the SQL statement	
	fsts = SQLExecute(hStmt);                                       // Execute the SQL statement
	if (!SQL_SUCCEEDED(fsts))	{ extract_error("SQLExecute", hStmt, SQL_HANDLE_STMT);	exit(1); }		
	SQLBindCol(hStmt, 1, SQL_C_CHAR, szDate,  bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, 2, SQL_C_CHAR, szAllPrices[0], bufSize, &cbModel); // bind columns
	if (numUl > 1) { for (i = 1; i < numUl; i++) { SQLBindCol(hStmt, 2+i, SQL_C_CHAR, szAllPrices[i], bufSize, &cbModel); } }

	// Get row of data from the result set defined above in the statement
	bool   firstTime(true);
	double previousPrice[100];
	boost::gregorian::date lastDate;
	retcode = SQLFetch(hStmt);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		for (i = 0; i < numUl; i++) {
			int    numDayDiff;
			double thisPrice;
			thisPrice = atof(szAllPrices[i]);
			// pad non-trading days
			{
				using namespace boost::gregorian;
				date bDate(from_simple_string(szDate));
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
			// printf("%s\t%s\n",szDate,szPrice);  // Print row (model)
			ulOriginalPrices.at(i).date.push_back(szDate);
			ulOriginalPrices.at(i).price.push_back(thisPrice);
		}

		// Fetch next row from result set
		retcode = SQLFetch(hStmt);  
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
	enum {colProductBarrierId=1,colProductId,
		colCapitalOrIncome, colNature, colPayoff, colTriggered, colSettlementDate, colDescription, colPayoffId, colParticipation,
		colStrike, colAvgTenor, colAvgFreq, colAvgType, colCap
	};
	char szProductBarrierId[bufSize], szCapitalOrIncome[bufSize], szNature[bufSize], szPayoff[bufSize], szSettlementDate[bufSize], szDescription[bufSize];
	char szPayoffId[bufSize], szParticipation[bufSize], szStrike[bufSize], szCap[bufSize];
	// ** SQL fetch block
	sprintf(lineBuffer, "%s%d%s", "select * from productbarrier where ProductId='", productId, "'");
	thisSQL = (SQLCHAR *)lineBuffer;
	retcode = SQLPrepareA(hStmt, thisSQL, SQL_NTS);                 // Prepare the SQL statement	
	fsts = SQLExecute(hStmt);                                     // Execute the SQL statement
	if (!SQL_SUCCEEDED(fsts))	{ extract_error("SQLExecute", hStmt, SQL_HANDLE_STMT);	exit(1); }
	// ** end SQL fetch block
	SQLBindCol(hStmt, colProductBarrierId, SQL_C_CHAR, szProductBarrierId, bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colCapitalOrIncome,  SQL_C_CHAR, szCapitalOrIncome,  bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colNature,           SQL_C_CHAR, szNature,           bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colPayoff,           SQL_C_CHAR, szPayoff,           bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colSettlementDate,   SQL_C_CHAR, szSettlementDate,   bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colDescription,      SQL_C_CHAR, szDescription,      bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colPayoffId,         SQL_C_CHAR, szPayoffId,         bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colParticipation,    SQL_C_CHAR, szParticipation,    bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colStrike,           SQL_C_CHAR, szStrike,           bufSize, &cbModel); // bind columns
	SQLBindCol(hStmt, colCap,              SQL_C_CHAR, szCap,              bufSize, &cbModel); // bind columns
	
	// Get row of data from the result set defined above in the statement
	retcode = SQLFetch(hStmt);
	while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
		capitalOrIncome = atoi(szCapitalOrIncome) == 1;
		nature = szNature;
		payoff          = atof(szPayoff)/100.0;
		settlementDate  = szSettlementDate;
		description     = szDescription;
		thisPayoffId    = atoi(szPayoffId); thisPayoffType = payoffType[thisPayoffId];// PayoffTypeId
		participation   = atof(szParticipation);
		strike          = atof(szStrike);
		cap             = atof(szCap);
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
			brcolBarrierRelationId = 1, brcolProductBarrierId,
			brcolUnderlyingId, brcolBarrier, brcolBarrierTypeId, brcolAbove, brcolAt, brcolStartDate, brcolEndDate,
			brcolTriggered, brcolIsAbsolute, brcolUpperBarrier, brcolWeight
		};
		char szbrBarrierRelationId[bufSize], szbrProductBarrierId[bufSize], szbrUnderlyingId[bufSize], szbrBarrier[bufSize], szbrBarrierTypeId[bufSize], szbrAbove[bufSize];
		char szbrAt[bufSize], szbrStartDate[bufSize], szbrEndDate[bufSize], szbrTrigered[bufSize], szbrIsAbsolute[bufSize], szbrUpperBarrier[bufSize], szbrWeight[bufSize];
		// ** SQL fetch block
		sprintf(lineBuffer, "%s%s%s", "select * from barrierrelation where ProductBarrierId='", szProductBarrierId, "'");
		thisSQL = (SQLCHAR *)lineBuffer;
		retcode = SQLPrepareA(hStmt1, thisSQL, SQL_NTS);                 // Prepare the SQL statement	
		fsts = SQLExecute(hStmt1);                                     // Execute the SQL statement
		if (!SQL_SUCCEEDED(fsts))	{ extract_error("SQLExecute", hStmt1, SQL_HANDLE_STMT);	exit(1); }
		// ** end SQL fetch block
		SQLBindCol(hStmt1, brcolUnderlyingId, SQL_C_CHAR, szbrUnderlyingId, bufSize, &cbModel); // bind columns
		SQLBindCol(hStmt1, brcolBarrier,      SQL_C_CHAR, szbrBarrier,      bufSize, &cbModel); // bind columns
		SQLBindCol(hStmt1, brcolAbove,        SQL_C_CHAR, szbrAbove,        bufSize, &cbModel); // bind columns
		SQLBindCol(hStmt1, brcolAt,           SQL_C_CHAR, szbrAt,           bufSize, &cbModel); // bind columns
		SQLBindCol(hStmt1, brcolStartDate,    SQL_C_CHAR, szbrStartDate,    bufSize, &cbModel); // bind columns
		SQLBindCol(hStmt1, brcolEndDate,      SQL_C_CHAR, szbrEndDate,      bufSize, &cbModel); // bind columns
		SQLBindCol(hStmt1, brcolUpperBarrier, SQL_C_CHAR, szbrUpperBarrier, bufSize, &cbModel); // bind columns
		
		retcode = SQLFetch(hStmt1);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
			uid              = atoi(szbrUnderlyingId);
			barrier          = atof(szbrBarrier);
			above            = atoi(szbrAbove) == 1;
			at               = atoi(szbrAt) == 1;
			startDateString  = szbrStartDate;
			endDateString    = szbrEndDate;
			uBarrier         = atof(szbrUpperBarrier);
			if (uid) {
				spr.barrier.at(numBarriers - 1).brel.push_back(SpBarrierRelation(uid, barrier, uBarrier, startDateString, endDateString, above, at, productStartDateString));
			}

			// next record
			retcode = SQLFetch(hStmt1);
		}
		// next record
		retcode = SQLFetch(hStmt);
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
	SQLFreeStmt(hStmt, SQL_DROP); 	SQLFreeStmt(hStmt1, SQL_DROP);  // Free the allocated statement handle
	SQLDisconnect(hDBC);           // Disconnect from datasource
	SQLFreeConnect(hDBC); // Free the allocated connection handle
	SQLFreeEnv(hEnv);    // Free the allocated ODBC environment handle
	return 0;
}

