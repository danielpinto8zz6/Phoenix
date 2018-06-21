#pragma once

BOOL addPlayer(Game *game, TCHAR username[50], int id);
BOOL coordinatesEqual(Coordinates c1, Coordinates c2);
Coordinates getFirstEmptyPosition(Game *game);
int getPlayerIndex(Game *game, int id);
BOOL isCoordinatesValid(Coordinates coordinates);
BOOL isPointInsideRect(Coordinates rect1Coord, Size rect1Sz, Coordinates point);
BOOL isPositionOccupied(Game *game, Coordinates coordinates);
BOOL isRectangleOverlapping(Coordinates rect1Coord, Size rect1Sz,
                            Coordinates rect2Coord, Size rect2Sz);
BOOL removePlayer(Game *game, int id);
DWORD WINAPI threadEnemyShip(LPVOID lpParam);
DWORD WINAPI threadManageEnemyShips(LPVOID lpParam);
BOOL joinGame(Data *data, int id);
void setupTopTen(Game *game);
BOOL startGame(Data *data);