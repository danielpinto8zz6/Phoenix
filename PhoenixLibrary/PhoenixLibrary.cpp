// Phoenix_Library.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

extern "C" {
	__declspec(dllexport) void CheckDLL() {
		_tprintf(TEXT("DLL Loaded !\n"));
		return;
	}
}