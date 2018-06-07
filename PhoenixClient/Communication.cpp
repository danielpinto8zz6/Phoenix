#include "stdafx.h"

#include "Communication.h"
#include <process.h>

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
      error(TEXT("Can't create file"));
      return FALSE;
    }

    /**
     * Wait 30 sec
     */
    if (!WaitNamedPipe(PIPE_NAME, 30000)) {
      error(TEXT("Timeout! Exiting..."));
      return FALSE;
    }
  }
  return TRUE;
}

BOOL writeGatewayAsync(Client *client, Message message) {
  DWORD nBytes = 0;
  BOOL fSuccess = FALSE;

  if (client->hPipe == NULL) {
    error(TEXT("Pipe is NULL"));
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
    error(TEXT("Write file didn't wrote all the info"));
    return FALSE;
  }

  if (!fSuccess) {
    error(TEXT("Can't write file"));
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
    error(TEXT("Pipe is NULL"));
    return FALSE;
  }

  readReady = CreateEvent(NULL, TRUE, FALSE, NULL);

  if (readReady == NULL) {
    error(TEXT("Can't create read event. Exiting..."));
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
      error(TEXT("Can't read file"));
    }

    debug(TEXT("%d bytes received"), nBytes);
  }
  client->readerAlive = FALSE;
  return FALSE;
}