#pragma once
#include "stdafx.h"

#define ENEMYSHIPS 20

HANDLE EnemyShipsMutex;

DWORD WINAPI ThreadEnemyShip(LPVOID lpParam);
DWORD WINAPI ThreadManageEnemyShips(LPVOID lpParam);