// PhoenixClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Communication.h"
#include "PhoenixClient.h"
#include <process.h>

int _tmain() {
  HANDLE hThreadDataReceiver;
  DWORD threadDataReceiverId;
  Pipes clientPipes;
  BOOL success = FALSE;
  HANDLE runningEvent;

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

  /**
   * Wait running event to be released to proceed
   */
  TCHAR eventName[50];
  _stprintf_s(eventName, TEXT("%s_%d"), CLIENT_CLOSE_EVENT, _getpid());

  runningEvent = CreateEventW(NULL, FALSE, FALSE, eventName);

  if (runningEvent != NULL) {
    WaitForSingleObject(runningEvent, INFINITE);
  }

  handleClose(clientPipes.inboundPipe);

  CloseHandle(runningEvent);

  return 0;
}

BOOL WINAPI CtrlHandler(DWORD dwCtrlType) {
  HANDLE serverRunningEvent;
  TCHAR eventName[50];
  _stprintf_s(eventName, TEXT("%s_%d"), CLIENT_CLOSE_EVENT, _getpid());

  serverRunningEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
  if (serverRunningEvent == NULL) {
    error(TEXT("Can't set up close event! Client will not exit "
               "properly"));
    ExitThread(0);
  }

  switch (dwCtrlType) {
  case CTRL_SHUTDOWN_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
    if (!SetEvent(serverRunningEvent)) {
      error(TEXT("Can't send close event! Client will not exit "
                 "properly"));
    }
    /**
     * Force exit after 10 sec
     */
    Sleep(10000);
    return TRUE;
  default:
    return FALSE;
  }
  return FALSE;
}

VOID handleClose(HANDLE hPipe) {
  Message msg;
  msg.cmd = CLIENT_CLOSING;
  _stprintf_s(msg.text, TEXT("%d"), _getpid());
  sendMessageToGateway(hPipe, &msg);
}