#pragma once

DWORD WINAPI dataReceiver(LPVOID lpParam);
BOOL connectPipes(ClientPipes *clientPipes);
BOOL clientLogin(const TCHAR username[], HANDLE hPipe);