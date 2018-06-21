#pragma once

BOOL connectPipes(Client *client);
BOOL handleCommand(Client *client, Message message);
BOOL joinGame(Client *client);
DWORD WINAPI messageReceiver(LPVOID lpParam);
DWORD WINAPI gameReceiver(LPVOID lpParam);