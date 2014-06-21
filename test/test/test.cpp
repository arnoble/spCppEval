// test.cpp : main project file.

#include "stdafx.h"
#include <vector>

using namespace System;

class Super {
public:
	Super(){};
	Super(bool yesno){
		if (yesno){
			ynFptr = &Super::helloYes;
		}
		else {
			ynFptr = &Super::helloNo;
		}
	}
	void (Super::*ynFptr)();
	
	void helloYes(){
		Console::WriteLine(L"HelloYes from Super");
	}
	void helloNo(){
		Console::WriteLine(L"HelloNo from Super");
	}
};


int main()
{
	Super superYes(true);
	Super *ptrYes = &superYes;
	
	ptrYes->helloYes();                 // OK
	(superYes .* (superYes.ynFptr))();  // OK ... works because pointer-to-member-function is stored in the ynFptr field of the class of superYes, accessed with the superyes.ynFptr syntax
	
    return 0;
}
