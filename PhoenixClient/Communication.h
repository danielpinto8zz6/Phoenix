#pragma once

DWORD WINAPI dataReceiver(LPVOID lpParam);
BOOL connectPipes(Pipes *clientPipes);
BOOL clientLogin(const TCHAR username[], HANDLE hPipe);