#include "stdafx.h"

#include "Communication.h"
#include <process.h>

DWORD WINAPI dataReceiver(LPVOID lpParam) {
  Pipes *clientPipes;
  clientPipes = (Pipes *)lpParam;

  Message msg;
  BOOL result;
  DWORD nBytes;
  while (TRUE) {
    result = ReadFile(clientPipes->outboundPipe, (LPVOID)&msg, sizeof(msg),
                      &nBytes, NULL);
    if (nBytes > 0) {
      switch (msg.cmd) {
      case LOGGED:
        _tprintf(TEXT("Succeed : %s logged\n"), msg.text);
        break;
      }
    }
  };

  return 0;
}

BOOL connectPipes(Pipes *clientPipes) {
  if (!WaitNamedPipe(PIPE_NAME_INBOUND, NMPWAIT_WAIT_FOREVER)) {
    Error(TEXT("Connecting to inbound pipe"));
    return FALSE;
  }

  clientPipes->outboundPipe =
      CreateFile(PIPE_NAME_INBOUND, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                 FILE_ATTRIBUTE_NORMAL, NULL);
  if (clientPipes->outboundPipe == NULL ||
      clientPipes->outboundPipe == INVALID_HANDLE_VALUE) {
    Error(TEXT("Connecting to outbound pipe"));
    return FALSE;
  }
  clientPipes->inboundPipe =
      CreateFile(PIPE_NAME_OUTBOUND, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                 FILE_ATTRIBUTE_NORMAL, NULL);
  if (clientPipes->inboundPipe == NULL ||
      clientPipes->inboundPipe == INVALID_HANDLE_VALUE) {
    Error(TEXT("Connecting to inbound pipe"));
    return FALSE;
  }
  return TRUE;
}

BOOL clientLogin(const TCHAR username[], HANDLE hPipe) {
  BOOL success;
  DWORD nBytes;
  Message msg;
  msg.cmd = LOGIN;
  TCHAR text[50];
  _stprintf_s(text, TEXT("%s[%d]"), username, _getpid());
  _tcscpy_s(msg.text, _tcslen(text) + 1, text);

  success = writeDataToPipe(&msg, sizeof(msg), hPipe, &nBytes);
  return success;
}