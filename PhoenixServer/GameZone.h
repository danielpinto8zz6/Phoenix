#pragma once

DWORD WINAPI threadEnemyShip(LPVOID lpParam);
DWORD WINAPI threadManageEnemyShips(LPVOID lpParam);
BOOL sendGameToGateway(GameData *gameData, Game *game);