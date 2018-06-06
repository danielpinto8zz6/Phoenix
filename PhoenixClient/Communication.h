#pragma once

DWORD WINAPI dataReceiver(LPVOID lpParam);
BOOL connectPipes(Pipes *clientPipes);
BOOL clientLogin(LPCWSTR username, HANDLE hPipe);