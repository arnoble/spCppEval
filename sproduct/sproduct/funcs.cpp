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

