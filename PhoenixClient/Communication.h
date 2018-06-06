#pragma once

DWORD WINAPI dataReceiver(LPVOID lpParam);
BOOL connectPipes(Pipes *clientPipes);
BOOL clientLogin(LPCWSTR username, HANDLE hPipe);
BOOL sendMessageToGateway(HANDLE hPipe, Message *message);