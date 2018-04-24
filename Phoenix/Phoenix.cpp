// Phoenix.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

typedef void(__cdecl *DLLFUNC)();

void check_dll() {
  HINSTANCE hinstLib;
  DLLFUNC ProcAdd;

  BOOL fFreeResult, fRunTimeLinkSuccess = FALSE;

  // Get a handle to the DLL module.

  hinstLib = LoadLibrary(TEXT("PhoenixLibrary.dll"));

  // If the handle is valid, try to get the function address.

  if (hinstLib != NULL) {
    ProcAdd = (DLLFUNC)GetProcAddress(hinstLib, "CheckDLL");

    // If the function address is valid, call the function.

    if (NULL != ProcAdd) {
      fRunTimeLinkSuccess = TRUE;
      ProcAdd();
    }

    // Free the DLL module.

    fFreeResult = FreeLibrary(hinstLib);
  }

  // If unable to call the DLL function, use an alternative.
  if (!fRunTimeLinkSuccess)
    _tprintf(TEXT("Could not load DLL\n"));
}

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  check_dll();

  system("pause");

  return 0;
}