#pragma once

BOOL connectPipes(Client *client);
BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
DWORD WINAPI gameReceiver(LPVOID lpParam);
DWORD WINAPI messageReceiver(LPVOID lpParam);
VOID handleClose(Client *client);