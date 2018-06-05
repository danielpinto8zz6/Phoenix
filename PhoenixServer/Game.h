#pragma once

Coordinates getFirstEmptyPosition(Game *game);
BOOL isPositionOccupied(Game *game, Coordinates coordinates);
BOOL coordinatesEqual(Coordinates c1, Coordinates c2);
BOOL isCoordinatesValid(Coordinates coordinates);
DWORD WINAPI threadEnemyShip(LPVOID lpParam);
DWORD WINAPI threadManageEnemyShips(LPVOID lpParam);