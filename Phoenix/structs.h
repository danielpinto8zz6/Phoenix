#pragma once
#include "stdafx.h"

typedef struct {
  int x;
  int y;
} Coordinates;

typedef struct {
  TCHAR username[50];
  Coordinates position;
} Player;

typedef struct {
  Coordinates position;
  int points;
  int velocity;
} InvasionShip;

typedef struct {
  Coordinates position;
  int velocity;
} DefenderShip;

typedef struct {
  int level;
} Game;

typedef struct {
  Coordinates position;
} Bomb;

typedef struct {
  Coordinates position;
} Shoot;

typedef struct {
  Coordinates position;
} Powerup;