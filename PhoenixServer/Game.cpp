#include "stdafx.h"

#include "Game.h"

Coordinates *GetFirstEmptyPosition(Game *game) {
  Coordinates *coordinates = (Coordinates *)malloc(sizeof(Coordinates));

  for (INT y = 0; y < HEIGHT; y++) {
    for (INT x = 0; x < WIDTH; x++) {
      if (game->map[y][x] == ' ' || game->map[y][x] == '\0') {
        coordinates->x = x;
        coordinates->y = y;
        return coordinates;
      }
    }
  }
  return NULL;
}
