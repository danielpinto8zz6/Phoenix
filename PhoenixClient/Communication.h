#pragma once

BOOL connectPipes(Client *client);
BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
DWORD WINAPI dataReceiver(LPVOID lpParam);
VOID handleClose(Client *client);
BOOL initClient(HINSTANCE hInstance, HWND hWnd, Client *client);