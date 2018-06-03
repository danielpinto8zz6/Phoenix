#pragma once

int initGameData();
DWORD WINAPI threadEnemyShip(LPVOID lpParam);
DWORD WINAPI threadManageEnemyShips(LPVOID lpParam);