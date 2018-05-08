#pragma once
#include "../phoenix_dll/phoenix_dll.h"
#include "stdafx.h"

#define ENEMYSHIPS 20

HANDLE EnemyShipsMutex;

DWORD WINAPI ThreadEnemyShip(LPVOID);
DWORD WINAPI ThreadManageEnemyShips(LPVOID lpParam);