#pragma once

DWORD WINAPI dataReceiver(LPVOID lpParam);
BOOL connectPipes(Pipes *clientPipes);
BOOL clientLogin(CONST TCHAR username[], HANDLE hPipe);