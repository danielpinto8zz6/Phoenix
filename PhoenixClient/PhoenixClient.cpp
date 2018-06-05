// PhoenixClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Communication.h"
#include "PhoenixClient.h"

int _tmain() {
  HANDLE hThreadDataReceiver;
  DWORD threadDataReceiverId;
  Pipes clientPipes;
  BOOL success = FALSE;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  if (!isGatewayRunning()) {
    error(TEXT("There's no gateway instance running! Start gateway first!"));
    system("pause");
    return FALSE;
  }

  _tprintf(TEXT("Client started! Establishing connection with gateway...\n"));

  /**
   * Connect inbound & outbound pipes
   */
  while (!success) {
    success = connectPipes(&clientPipes);
    /**
     * Keep trying to connect every 5 seconds
     */
    if (!success) {
      error(TEXT("Connection to gateway failed! Trying again in 5 sec"));
      Sleep(5000);
    }
  }

  /**
   * Create thread to receive info from gateway
   */
  hThreadDataReceiver =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dataReceiver,
                   (LPVOID)&clientPipes, 0, &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    error(TEXT("Creating data receiver thread"));
    return -1;
  }

  /**
   * Login, send credentials
   */
  clientLogin(TEXT("USER"), clientPipes.inboundPipe);

  /**
   * Now that everything is set up, set control handler
   */
  SetConsoleCtrlHandler(CtrlHandler, TRUE);

  WaitForSingleObject(hThreadDataReceiver, INFINITE);

  system("pause");

  return 0;
}

/**
 * Used before app close
 */
BOOL WINAPI CtrlHandler(DWORD dwCtrlType) {
  switch (dwCtrlType) {
  case CTRL_SHUTDOWN_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
    // TODO
    return TRUE;
  default:
    // We don't care about this event
    // Default handler is used
    return FALSE;
  }
  return FALSE;
}