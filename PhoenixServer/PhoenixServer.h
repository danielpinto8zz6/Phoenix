#pragma once

#define ENEMYSHIPS 20

HANDLE EnemyShipsMutex;

DWORD WINAPI threadEnemyShip(LPVOID lpParam);
DWORD WINAPI threadManageEnemyShips(LPVOID lpParam);
