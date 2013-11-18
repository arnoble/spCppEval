// test.cpp : main project file.

#include "stdafx.h"

using namespace System;

int main(array<System::String ^> ^args)
{
	ADODB::ConnectionClass  conn;
	conn.Open("MySQL ODBC 3.51 Driver", "root", "ragtinmor",3);
	ADODB::RecordsetClass  rs;
	//rs.Open("select * from producttype",conn,1,1,3);
	ADODB::_Connection c = nullptr;

    Console::WriteLine(L"Hello World");
    return 0;
}
