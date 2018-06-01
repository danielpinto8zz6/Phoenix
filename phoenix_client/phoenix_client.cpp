// phoenix_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"
#include "phoenix_client.h"
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include <windows.h>

int _tmain() {

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  _tprintf(TEXT("Connecting to pipe...\n"));

  // Open the named pipe
  // Most of these parameters aren't very relevant for pipes.
  HANDLE hGatewayPipe = CreateFile(GATEWAY_PIPE_NAME,
                           GENERIC_READ, // only need read access
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hGatewayPipe == INVALID_HANDLE_VALUE) {
    _tprintf(TEXT("Failed to connect to pipe.\n"));
    // look up error code here using GetLastError()
    system("pause");
    return 1;
  }

 _tprintf(TEXT("Reading data from pipe...\n"));

  // The read operation will block until there is data to read
  TCHAR buffer[128];
  DWORD numBytesRead = 0;
  BOOL result =
      ReadFile(hGatewayPipe,
               buffer,                // the data from the pipe will be put here
               127 * sizeof(TCHAR), // number of bytes allocated
               &numBytesRead, // this will store number of bytes actually read
               NULL           // not using overlapped IO
      );

  if (result) {
    buffer[numBytesRead / sizeof(TCHAR)] = '\0'; // null terminate the string
    _tprintf(TEXT("Number of bytes read: %d\n"),numBytesRead);
    _tprintf(TEXT("Message: %s\n"), buffer);
  } else {
    _tprintf(TEXT("Failed to read data from the pipe.\n"));
  }

  // Close our pipe handle
  CloseHandle(hGatewayPipe);

  system("pause");

  return 0;
}