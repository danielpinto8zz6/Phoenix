// PhoenixClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Communication.h"
#include "PhoenixClient.h"

int _tmain() {
  HANDLE hThreadDataReceiver;
  DWORD threadDataReceiverId;
  ClientPipes clientPipes;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  _tprintf(TEXT("Client started! Establishing connection with gateway...\n"));

  /**
   * Connect inbound & outbound pipes
   */
  connectPipes(&clientPipes);

  /**
   * Create thread to receive info from gateway
   */
  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dataReceiver,
                   (LPVOID)&clientPipes, 0, &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    Error(TEXT("Creating data receiver thread"));
    return -1;
  }

  /**
   * Login, send credentials
   */
  clientLogin(TEXT("USER"), clientPipes.inboundPipe);

  WaitForSingleObject(hThreadDataReceiver, INFINITE);

  system("pause");

  return 0;
}