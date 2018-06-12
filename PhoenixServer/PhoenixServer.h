#pragma once

int _tmain(int argc, LPTSTR argv[]);
BOOL WINAPI CtrlHandler(DWORD dwCtrlType);
VOID handleClose(MessageData *messageData);
VOID initGameVariables(Game *game);