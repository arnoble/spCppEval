// DDEExample.cpp : Defines the entry point for the console application.
// http://stackoverflow.com/questions/3306216/how-to-be-notified-of-any-update-from-dynamic-data-exchange-dde

#include "stdafx.h"
#include <windows.h> 
#include "ddeml.h"

#define PAUSE system("pause")

HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv,
	HSZ hsz1, HSZ hsz2, HDDEDATA hdata,
	ULONG_PTR dwData1, ULONG_PTR dwData2)
{
	printf("uType: %d", uType);
	switch (uType)
	{
	case XTYP_REQUEST:
		printf("XTYP_REQUEST\n");
		break;

	}
	return 0;
}

void DDERequest(DWORD idInst, HCONV hConv, char* szItem, char* sDesc)
{
	HSZ hszItem = DdeCreateStringHandle(idInst, szItem, 0);
	HDDEDATA hData = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
		XTYP_REQUEST, 5000, NULL);
	if (hData == NULL)
	{
		printf("Request failed: %s\n", szItem);
	}
	else
	{
		char szResult[255];
		DdeGetData(hData, (unsigned char *)szResult, 255, 0);
		printf("%s%s\n", sDesc, szResult);
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	char szApp[] = "EXCEL";
	char szTopic[] = "C:\\Test.xlsx";
	char szCmd1[] = "[APP.MINIMIZE()]";
	char szItem1[] = "R1C1";  char szDesc1[] = "A1 Contains: ";


	UINT uiResult;
	DWORD m_dwDDEInstance = 0;
	uiResult = DdeInitialize(&m_dwDDEInstance, (PFNCALLBACK)&DdeCallback, APPCLASS_STANDARD | APPCMD_CLIENTONLY, 0);
	if (uiResult != DMLERR_NO_ERROR)
	{
		printf("DDE Initialization Failed: 0x%04x\n", uiResult);
		return FALSE;
	}
	printf("m_dwDDEInstance: %u\n", m_dwDDEInstance);
	//PAUSE;
	HSZ hszApp, hszTopic;
	HCONV hConv;
	hszApp = DdeCreateStringHandle(m_dwDDEInstance, szApp, 0);
	hszTopic = DdeCreateStringHandle(m_dwDDEInstance, szTopic, 0);
	hConv = DdeConnect(m_dwDDEInstance, hszApp, hszTopic, NULL);
	DdeFreeStringHandle(m_dwDDEInstance, hszApp);
	DdeFreeStringHandle(m_dwDDEInstance, hszTopic);
	if (hConv == NULL)
	{
		printf("DDE Connection Failed.\n");
	}

	DDERequest(m_dwDDEInstance, hConv, szItem1, szDesc1);
	DdeDisconnect(hConv);
	DdeUninitialize(m_dwDDEInstance);

	PAUSE;
}