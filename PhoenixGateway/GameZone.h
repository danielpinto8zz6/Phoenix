#pragma once

#include "structs.h"

BOOL initGameZone(GameData *gameData);
DWORD WINAPI receiveGameDataFromServer(LPVOID lpParam);
DWORD peekGameData(GameData *data);