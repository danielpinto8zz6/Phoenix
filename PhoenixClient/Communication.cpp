#include "stdafx.h"

#include "Communication.h"
#include "Game.h"
#include "PhoenixClient.h"
#include "resource.h"
#include <process.h>

BOOL connectPipes(Client *client) {
  DWORD dwMode;
  BOOL fSuccess;

  while (TRUE) {
    client->hPipeMessage = CreateFile(PIPE_MESSAGE_NAME, GENERIC_READ | GENERIC_WRITE,
                       0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                       OPEN_EXISTING, 0, NULL);

    if (client->hPipeMessage != INVALID_HANDLE_VALUE) {
      break;
    }

    if (GetLastError() != ERROR_PIPE_BUSY) {
      errorGui(TEXT("Can't create file"));
      return FALSE;
    }

    // client->hPipeGame = CreateFile(
    //     PIPE_GAME_NAME, GENERIC_READ, 0 | FILE_SHARE_READ | FILE_SHARE_WRITE,
    //     NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);

    // if (client->hPipeGame != INVALID_HANDLE_VALUE) {
    //   break;
    // }

    if (GetLastError() != ERROR_PIPE_BUSY) {
      errorGui(TEXT("Can't create file"));
      return FALSE;
    }

    /**
     * Wait 30 sec
     */
    if (!WaitNamedPipe(PIPE_MESSAGE_NAME, 30000)) {
      errorGui(TEXT("Timeout! Exiting..."));
      return FALSE;
    }
    // if (!WaitNamedPipe(PIPE_GAME_NAME, 30000)) {
    //   errorGui(TEXT("Timeout! Exiting..."));
    //   return FALSE;
    // }
  }

  debug(TEXT("Pipe connected!"));

  /**
   * Connected! Change pipe mode to read
   */
  dwMode = PIPE_READMODE_MESSAGE;
  fSuccess = SetNamedPipeHandleState(client->hPipeMessage, &dwMode, NULL, NULL);

  if (!fSuccess) {
    errorGui(TEXT("Can't set named pipe handle state"));
    return FALSE;
  }
  return TRUE;
}

DWORD WINAPI dataReceiver(LPVOID lpParam) {
  Client *client = (Client *)lpParam;

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  Game game;

  if (client->hPipeGame == NULL) {
    errorGui(TEXT("Pipe is NULL"));
    return FALSE;
  }

  while (client->threadContinue) {
    fSuccess = readDataFromPipe(client->hPipeGame, (LPVOID)&game, sizeof(Game));

    if (!fSuccess) {
      error(TEXT("Can't read message data"));
      break;
    }

    // TODO
  }

  return FALSE;
}

BOOL initClient(HINSTANCE hInstance, HWND hWnd, Client *client) {
  HANDLE hThreadDataReceiver;
  DWORD threadDataReceiverId = 0;
  // HANDLE runningEvent;

  /**
   * Create thread to receive info from gateway
   */
  hThreadDataReceiver = CreateThread(NULL, 0, dataReceiver, (LPVOID)client, 0,
                                     &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    errorGui(TEXT("Creating data receiver thread"));
    return FALSE;
  }

  // /**
  //  * Now that everything is set up, set control handler
  //  */
  // SetConsoleCtrlHandler(CtrlHandler, TRUE);

  // /**
  //  * Wait running event to be released to proceed
  //  */
  // TCHAR eventName[50];
  // _stprintf_s(eventName, TEXT("%s_%d"), CLIENT_CLOSE_EVENT, _getpid());

  // runningEvent = CreateEventW(NULL, FALSE, FALSE, eventName);

  // if (runningEvent != NULL) {
  //   WaitForSingleObject(runningEvent, INFINITE);
  // }

  // handleClose(client);

  // client->threadContinue = FALSE;
  // if (client->readerAlive) {
  //   WaitForSingleObject(hThreadDataReceiver, 3000);
  //   errorGui(TEXT("Timeout! Closing thread..."));
  // }

  // CloseHandle(client->writeReady);
  // CloseHandle(client->hPipeGame);
  // CloseHandle(runningEvent);

  return TRUE;
}

VOID handleClose(Client *client) {
  Message msg;
  msg.cmd = CLIENT_CLOSING;
  _tcscpy_s(msg.text, client->username);
  writeDataToPipe(client->hPipeMessage, (LPVOID)&msg, sizeof(Message));
}

BOOL WINAPI CtrlHandler(DWORD dwCtrlType) {
  HANDLE serverRunningEvent;
  TCHAR eventName[50];
  _stprintf_s(eventName, TEXT("%s_%d"), CLIENT_CLOSE_EVENT, _getpid());

  serverRunningEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
  if (serverRunningEvent == NULL) {
    errorGui(TEXT("Can't set up close event! Client will not exit "
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
      errorGui(TEXT("Can't send close event! Client will not exit "
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