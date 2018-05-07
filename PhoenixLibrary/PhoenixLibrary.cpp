// Phoenix_Library.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) void MessageHandler(LPTSTR lpszFunction) {
  _tprintf(TEXT("MESSAGE : %s\n"), lpszFunction);
}

#ifdef __cplusplus
}
#endif