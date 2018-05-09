#include "structs.h"
#include <tchar.h>
#include <windows.h>

#define EOF (-1)

#ifdef __cplusplus // If used by C++ code,
extern "C" {       // we need to export the C interface
#endif

__declspec(dllexport) int myPuts(LPWSTR lpszMsg) {
  DWORD cchWritten;
  HANDLE hConout;
  BOOL fRet;

  // Get a handle to the console output device.

  hConout = CreateFileW(L"CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (INVALID_HANDLE_VALUE == hConout)
    return EOF;

  // Write a null-terminated string to the console output device.

  while (*lpszMsg != L'\0') {
    fRet = WriteConsole(hConout, lpszMsg, 1, &cchWritten, NULL);
    if ((FALSE == fRet) || (1 != cchWritten))
      return EOF;
    lpszMsg++;
  }
  return 1;
}

#ifdef __cplusplus
}
#endif
