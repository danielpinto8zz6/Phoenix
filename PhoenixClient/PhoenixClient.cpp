// PhoenixClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PhoenixClient.h"
#include <process.h>

int _tmain() {
  HANDLE hThreadDataReceiver;
  DWORD threadDataReceiverId;
  BOOL success;
  ClientPipes clientPipes;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  _tprintf(TEXT("Establishing connection with gateway...\n"));

  if (!WaitNamedPipe(PIPE_NAME_INBOUND, NMPWAIT_WAIT_FOREVER)) {
    Error(TEXT("Connecting to inbound pipe"));
    return -1;
  }

  _tprintf(TEXT("Connected with gateway\n"));

  clientPipes.outboundPipe =
      CreateFile(PIPE_NAME_INBOUND, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                 FILE_ATTRIBUTE_NORMAL, NULL);
  if (clientPipes.outboundPipe == NULL ||
      clientPipes.outboundPipe == INVALID_HANDLE_VALUE) {
    Error(TEXT("Connecting to outbound pipe"));
    return -1;
  }
  clientPipes.inboundPipe =
      CreateFile(PIPE_NAME_OUTBOUND, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                 FILE_ATTRIBUTE_NORMAL, NULL);
  if (clientPipes.inboundPipe == NULL ||
      clientPipes.inboundPipe == INVALID_HANDLE_VALUE) {
    Error(TEXT("Connecting to inbound pipe"));
    return -1;
  }

  /**
   * Client thread to receive info from gateway
   */
  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dataReceiver,
                   (LPVOID)&clientPipes, 0, &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    Error(TEXT("Creating data receiver thread"));
    return -1;
  }

  DWORD nBytes;
  Message msg;
  msg.cmd = LOGIN;
  TCHAR text[50];
  _stprintf_s(text, TEXT("USER-%d"), _getpid());
  _tcscpy_s(msg.text, _tcslen(text) + 1, text);

  success =
      WriteFile(clientPipes.inboundPipe, &msg, sizeof(msg), &nBytes, NULL);
  if (!success) {
    Error(TEXT("Sending data to gateway"));
  }

  WaitForSingleObject(hThreadDataReceiver, INFINITE);

  system("pause");

  return 0;
}

DWORD WINAPI dataReceiver(LPVOID lpParam) {
  ClientPipes *clientPipes;
  clientPipes = (ClientPipes *)lpParam;

  Message msg;
  BOOL result;
  DWORD nBytes;
  while (TRUE) {
    result = ReadFile(clientPipes->outboundPipe, (LPVOID)&msg, sizeof(msg),
                      &nBytes, NULL);
    if (nBytes > 0) {
      switch (msg.cmd) {
      case SUCCESS:
        _tprintf(TEXT("Succeed : %s\n"), msg.text);
        break;
      }
    }
  };

  return 0;
}
