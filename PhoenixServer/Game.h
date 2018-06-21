#pragma once

BOOL addPlayer(Game *game, TCHAR username[50], int id);
int addShot(DefenderShip *defenderShip);
int compare(const void *a, const void *b);
BOOL coordinatesEqual(Coordinates c1, Coordinates c2);
Coordinates getFirstEmptyPosition(Game *game);
int getPlayerIndex(Game *game, int id);
BOOL isCoordinatesValid(Coordinates coordinates);
BOOL isPointInsideRect(Coordinates rect1Coord, Size rect1Sz, Coordinates point);
BOOL isPositionOccupied(Game *game, Coordinates coordinates);
BOOL isRectangleOverlapping(Coordinates rect1Coord, Size rect1Sz, Coordinates rect2Coord, Size rect2Sz);
BOOL joinGame(Data *data, int id);
DWORD WINAPI manageShot(LPVOID lParam);
void movePlayer(GameData *gameData, int id, Command m);
BOOL removePlayer(Game *game, int id);
BOOL removeShot(DefenderShip *defenderShip, int position);
void setUpPlayers(GameData *data);
void setupTopTen(Game *game);
void sort(int *arr, size_t len);
BOOL startGame(Data *data);
DWORD WINAPI threadEnemyShip(LPVOID lpParam);
DWORD WINAPI threadManageEnemyShips(LPVOID lpParam);