#pragma once

Coordinates GetFirstEmptyPosition(Game *game);
BOOL isPositionOccupied(Game *game, Coordinates coordinates);
BOOL coordinatesEqual(Coordinates c1, Coordinates c2);
BOOL isCoordinatesValid(Coordinates coordinates);