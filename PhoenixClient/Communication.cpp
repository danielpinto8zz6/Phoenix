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
    client->hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE,
                               0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);

    if (client->hPipe != INVALID_HANDLE_VALUE) {
      break;
    }

    if (GetLastError() != ERROR_PIPE_BUSY) {
      errorGui(TEXT("Can't create file"));
      return FALSE;
    }

    /**
     * Wait 30 sec
     */
    if (!WaitNamedPipe(PIPE_NAME, 30000)) {
      errorGui(TEXT("Timeout! Exiting..."));
      return FALSE;
    }
  }

  debug(TEXT("Pipe connected!"));

  /**
   * Connected! Change pipe mode to read
   */
  dwMode = PIPE_READMODE_MESSAGE;
  fSuccess = SetNamedPipeHandleState(client->hPipe, &dwMode, NULL, NULL);

  if (!fSuccess) {
    errorGui(TEXT("Can't set named pipe handle state"));
    return FALSE;
  }
  return TRUE;
}

BOOL clientLogin(Client *client) {
  BOOL fSuccess;
  Message msg;
  msg.cmd = LOGIN;
  _stprintf_s(msg.text, TEXT("%d"), _getpid());
  fSuccess = writeGatewayAsync(client, msg);
  return fSuccess;
}

BOOL makeConnection(Client *client) {
  while (TRUE) {
    client->hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE,
                               0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);

    if (client->hPipe != INVALID_HANDLE_VALUE) {
      break;
    }

    if (GetLastError() != ERROR_PIPE_BUSY) {
      errorGui(TEXT("Can't create file"));
      return FALSE;
    }

    /**
     * Wait 30 sec
     */
    if (!WaitNamedPipe(PIPE_NAME, 30000)) {
      errorGui(TEXT("Timeout! Exiting..."));
      return FALSE;
    }
  }
  return TRUE;
}

BOOL writeGatewayAsync(Client *client, Message message) {
  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  if (client->hPipe == NULL) {
    errorGui(TEXT("Pipe is NULL"));
    return FALSE;
  }

  ZeroMemory(&client->OverlWr, sizeof(client->OverlWr));
  ResetEvent(client->writeReady);

  client->OverlWr.hEvent = client->writeReady;

  fSuccess = WriteFile(client->hPipe, &message, sizeof(Message), &nBytes,
                       &client->OverlWr);

  WaitForSingleObject(client->writeReady, INFINITE);

  debug(TEXT("%d Bytes written"), nBytes);

  GetOverlappedResult(client->hPipe, &client->OverlWr, &nBytes, FALSE);

  if (nBytes < sizeof(Message)) {
    errorGui(TEXT("Write file didn't wrote all the info"));
    return FALSE;
  }

  if (!fSuccess) {
    errorGui(TEXT("Can't write file"));
    return FALSE;
  }

  return TRUE;
}

DWORD WINAPI dataReceiver(LPVOID lpParam) {
  Client *client = (Client *)lpParam;

  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;
  HANDLE readReady;

  Game game;

  OVERLAPPED OverlRd = {0};

  if (client->hPipe == NULL) {
    errorGui(TEXT("Pipe is NULL"));
    return FALSE;
  }

  readReady = CreateEvent(NULL, TRUE, FALSE, NULL);

  if (readReady == NULL) {
    errorGui(TEXT("Can't create read event. Exiting..."));
    return FALSE;
  }

  client->readerAlive = TRUE;

  while (client->threadContinue) {
    ZeroMemory(&OverlRd, sizeof(OverlRd));
    OverlRd.hEvent = readReady;
    ResetEvent(readReady);

    fSuccess = ReadFile(client->hPipe, &game, sizeof(Game), &nBytes, &OverlRd);

    WaitForSingleObject(readReady, INFINITE);

    GetOverlappedResult(client->hPipe, &OverlRd, &nBytes, FALSE);

    if (nBytes < sizeof(Game)) {
      errorGui(TEXT("Can't read file"));
    }

    debug(TEXT("%d bytes received"), nBytes);
  }
  client->readerAlive = FALSE;
  return FALSE;
}

BOOL initClient(HINSTANCE hInstance, HWND hWnd, Client *client) {
  HANDLE hThreadDataReceiver;
  DWORD threadDataReceiverId = 0;
  HANDLE runningEvent;

  /**
   * Create thread to receive info from gateway
   */
  hThreadDataReceiver = CreateThread(NULL, 0, dataReceiver, (LPVOID)&client, 0,
                                     &threadDataReceiverId);
  if (hThreadDataReceiver == NULL) {
    errorGui(TEXT("Creating data receiver thread"));
    return FALSE;
  }

  client->writeReady = CreateEvent(NULL, TRUE, FALSE, NULL);

  if (client->writeReady == NULL) {
    errorGui(TEXT("Can't create write event. Exiting..."));
    return FALSE;
  }

  DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MENU), hWnd, Menu);

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

  handleClose(client);

  client->threadContinue = FALSE;
  if (client->readerAlive) {
    WaitForSingleObject(hThreadDataReceiver, 3000);
    errorGui(TEXT("Timeout! Closing thread..."));
  }

  CloseHandle(client->writeReady);
  CloseHandle(client->hPipe);
  CloseHandle(runningEvent);

  return TRUE;
}

VOID handleClose(Client *client) {
  Message msg;
  msg.cmd = CLIENT_CLOSING;
  msg.number = _getpid();
  _tcscpy_s(msg.text, client->username);
  writeGatewayAsync(client, msg);
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

BOOL checkLogin(Client *client, Message message) {
  if ((_tcscmp(message.text, client->username) == 0) &&
      message.number == GetCurrentProcessId()) {
    return TRUE;
  }
  return FALSE;
}