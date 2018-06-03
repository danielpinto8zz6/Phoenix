#pragma once

int initMessageZone();
DWORD peekMessageData(MessageData *data);
DWORD WINAPI receiveFromGateway(LPVOID lpParam);