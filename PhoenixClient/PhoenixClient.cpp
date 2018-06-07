// PhoenixClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Communication.h"
#include "PhoenixClient.h"
#include <process.h>

int _tmain() {
  HANDLE hThreadDataReceiver;
  DWORD threadDataReceiverId = 0;
  BOOL fSuccess = FALSE;
  HANDLE runningEvent;
  DWORD dwMode;
  Client client;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  client.threadContinue = TRUE;
  client.readerAlive = FALSE;
  client.OverlWr = {0};

  if (!isGatewayRunning()) {
    error(TEXT("There's no gateway instance running! Start gateway first!"));
    system("pause");
    return FALSE;
  }

  debug(
      TEXT("Client started! Trying to establish connection with gateway...\n"));

  /**
   * Make connection
   */
  // if (!makeConnection(&client)) {
  //   return FALSE;
  // }
  while (TRUE) {
    client.hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE,
                              0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                              OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);

    if (client.hPipe != INVALID_HANDLE_VALUE) {
      break;
    }

    if (GetLastError() != ERROR_PIPE_BUSY) {
      error(TEXT("Can't create file"));
    }

    /**
     * Wait 30 sec
     */
    if (!WaitNamedPipe(PIPE_NAME, 30000)) {
      error(TEXT("Timeout! Exiting..."));
    }
  }

  debug(TEXT("Pipe connected!"));

  /**
   * Connected! Change pipe mode to read
   */
  dwMode = PIPE_READMODE_MESSAGE;
  fSuccess = SetNamedPipeHandleState(client.hPipe, &dwMode, NULL, NULL);

  if (!fSuccess) {
    error(TEXT("Can't set named pipe handle state"));
    return FALSE;
  }

  /**
   * Create thread to receive info from gateway
   */
  hThreadDataReceiver = CreateThread(NULL, 0, dataReceiver, (LPVOID)&client, 0,
                                     &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    error(TEXT("Creating data receiver thread"));
    return -1;
  }

  client.writeReady = CreateEvent(NULL, TRUE, FALSE, NULL);

  if (client.writeReady == NULL) {
    error(TEXT("Can't create write event. Exiting..."));
    return FALSE;
  }

  /**
   * Login, send credentials
   */
  clientLogin(&client);

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

  handleClose(&client);

  client.threadContinue = FALSE;
  if (client.readerAlive) {
    WaitForSingleObject(hThreadDataReceiver, 3000);
    error(TEXT("Timeout! Closing thread..."));
  }

  CloseHandle(client.writeReady);
  CloseHandle(client.hPipe);
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

VOID handleClose(Client *client) {
  Message msg;
  msg.cmd = CLIENT_CLOSING;
  _stprintf_s(msg.text, TEXT("%d"), _getpid());
  writeGatewayAsync(client, msg);
}