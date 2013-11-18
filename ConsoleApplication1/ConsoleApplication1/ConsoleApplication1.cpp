// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <sqlext.h>
#include <stdio.h>
#include <string>


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


int _tmain(int argc, _TCHAR* argv[])
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

	SQLAllocEnv(&hEnv);        	  // Allocate memory for ODBC Environment handle
	fsts = dbConn(hEnv, &hDBC);   // connect
	if (fsts == SQL_SUCCESS || fsts == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocStmt(hDBC, &hStmt); 			// Allocate memory for statement handle
			SQLCHAR*  thisQ = (SQLCHAR *)"select Date,Price from prices where UnderlyingId='1' limit 10";
			retcode = SQLPrepareA(hStmt, thisQ, SQL_NTS);   // Prepare the SQL statement	
			fsts = SQLExecute(hStmt);                       // Execute the SQL statement
			if (!SQL_SUCCEEDED(fsts))	{
				extract_error("SQLExecute", hStmt, SQL_HANDLE_STMT);	exit(1);
			}
			
			SQLBindCol(hStmt, 1, SQL_C_CHAR, szDate,  bufSize, &cbModel); // bind columns
			SQLBindCol(hStmt, 2, SQL_C_CHAR, szPrice, bufSize, &cbModel); // bind columns

			// Get row of data from the result set defined above in the statement
			retcode = SQLFetch(hStmt);
			while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)	{
				printf("%s\t%s\n",szDate,szPrice);                                                // Print row (model)
				retcode = SQLFetch(hStmt);  // Fetch next row from result set
			}
			SQLFreeStmt(hStmt, SQL_DROP);  // Free the allocated statement handle
			SQLDisconnect(hDBC);           // Disconnect from datasource
		}
	SQLFreeConnect(hDBC); // Free the allocated connection handle
	SQLFreeEnv(hEnv);    // Free the allocated ODBC environment handle
	return 0;
}

