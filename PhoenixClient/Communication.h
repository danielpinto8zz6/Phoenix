#pragma once

BOOL connectPipes(Client *client);
void handleCommand(Client *client, Message message);
BOOL joinGame(Client *client);
DWORD WINAPI messageReceiver(LPVOID lpParam);
DWORD WINAPI gameReceiver(LPVOID lpParam);