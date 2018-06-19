#pragma once

BOOL connectPipes(Client *client);
DWORD WINAPI gameReceiver(LPVOID lpParam);
void handleCommand(Client *client, Message message);
DWORD WINAPI messageReceiver(LPVOID lpParam);