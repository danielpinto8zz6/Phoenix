// Phoenix_Library.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) void CheckDLL() {

  _tprintf(TEXT("DLL Loaded !\n"));
  return;
}

#ifdef __cplusplus
}
#endif