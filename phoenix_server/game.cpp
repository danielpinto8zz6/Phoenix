#include "stdafx.h"
#include "game.h"

int x = 0;

Coordinates *GetFirstEmptyPosition(Game game) {
	Coordinates coordinates;

	for (int x = 0; x < HEIGHT; x++) {
		for (int y = 0; y < WIDTH; y++) {
			if (game.map[x][y] == ' ') {
				coordinates.x = x;
				coordinates.y = y;	
				return &coordinates;
			}
		}
	}
	return NULL;
}