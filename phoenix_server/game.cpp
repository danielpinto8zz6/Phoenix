#include "stdafx.h"

#include "game.h"

Coordinates *GetFirstEmptyPosition(Game *game) {
  Coordinates coordinates;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (game->map[y][x] == ' ' || game->map[y][x] == '\0') {
        coordinates.x = x;
        coordinates.y = y;
        return &coordinates;
      }
    }
  }
  return NULL;
}