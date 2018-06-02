#pragma once

int _tmain();
BOOL receiveData(LPVOID data, DWORD size, HANDLE hPipe, DWORD *nBytes);
BOOL sendData(LPVOID data, DWORD size, HANDLE hPipe, DWORD *nBytes);
DWORD WINAPI dataReceiver(LPVOID lpParam);