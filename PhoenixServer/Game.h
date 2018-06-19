#pragma once

BOOL coordinatesEqual(Coordinates c1, Coordinates c2);
Coordinates getFirstEmptyPosition(Game *game);
BOOL isCoordinatesValid(Coordinates coordinates);
BOOL isPositionOccupied(Game *game, Coordinates coordinates);
BOOL isRectangleOverlapping(Coordinates rect1Coord, Size rect1Sz, Coordinates rect2Coord, Size rect2Sz);
DWORD WINAPI threadEnemyShip(LPVOID lpParam);
DWORD WINAPI threadManageEnemyShips(LPVOID lpParam);