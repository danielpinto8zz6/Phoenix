#include "stdafx.h"

#include "Game.h"

Coordinates GetFirstEmptyPosition(Game *game) {
  Coordinates coordinates;

  for (coordinates.y = 0; coordinates.y < HEIGHT; coordinates.y++) {
    for (coordinates.x = 0; coordinates.x < WIDTH; coordinates.x++) {
      if (!isPositionOccupied(game, coordinates)) {
        return coordinates;
      }
    }
  }
  coordinates.x = -1;
  coordinates.y = -1;

  return coordinates;
}

BOOL isPositionOccupied(Game *game, Coordinates coordinates) {
  for (int i = 0; i < game->totalEnemyShips; i++) {
    if (coordinatesEqual(coordinates, game->enemyShip[i].position)) {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL coordinatesEqual(Coordinates c1, Coordinates c2) {
  return c1.x == c2.x && c1.y == c2.y ? TRUE : FALSE;
}

BOOL isCoordinatesValid (Coordinates coordinates){
  return coordinates.x == -1 || coordinates.y == -1 ? FALSE : TRUE;
}