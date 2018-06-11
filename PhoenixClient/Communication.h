#pragma once

BOOL clientLogin(Client *client);
BOOL connectPipes(Client *client);
BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
DWORD WINAPI dataReceiver(LPVOID lpParam);
VOID handleClose(Client *client);
BOOL initClient(HINSTANCE hInstance, HWND hWnd, Client *client);
BOOL makeConnection(Client *client);
BOOL writeGatewayAsync(Client *client, Message message);
