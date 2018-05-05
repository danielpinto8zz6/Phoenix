// Phoenix.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef void(__cdecl *DLLFUNC)(LPTSTR);

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  HINSTANCE hinstLib;
  DLLFUNC MessageHandler;

  // Get a handle to the DLL module.
  hinstLib = LoadLibrary(TEXT("PhoenixLibrary.dll"));

  // If the handle is valid, try to get the function address.
  if (hinstLib != NULL) {
    MessageHandler = (DLLFUNC)GetProcAddress(hinstLib, "MessageHandler");

    // If the function address is valid, call the function.
    if (NULL != MessageHandler) {
      MessageHandler((LPTSTR)TEXT("DLL LOADED"));
    } else {
      _tprintf(TEXT("Could not load DLL\n"));
    }

    // Free the DLL module.
    FreeLibrary(hinstLib);
  }

  system("pause");

  return 0;
}