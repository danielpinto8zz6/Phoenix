#pragma once

#include "structs.h"

DWORD WINAPI receiveGameDataFromServer(LPVOID lpParam);
DWORD peekGameData(GameData *data);