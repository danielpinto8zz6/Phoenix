#pragma once
#include "stdafx.h"

enum PowerupType { SHIELD, ICE, BATTERY, PLUS, ALCOOL };

enum InvaderType { BASIC, DODGE };

enum PowerupEffect {};

typedef struct {
  int x;
  int y;
} Coordinates;

typedef struct {
  int x;
  int y;
} Size;

typedef struct {
  TCHAR username[50];
  DefenderShip ship;
  int lifes;
  int points;
} Player;

typedef struct {
  Coordinates position;
  int points;
  int velocity;
  Size size;
  InvaderType type;
  Bomb bombs[50];
} InvasionShip;

typedef struct {
  Coordinates position;
  int velocity;
  Size size;
  int firingRate;
} DefenderShip;

typedef struct {
  int level;
} Game;

typedef struct {
  Coordinates position;
} Bomb;

typedef struct {
  Coordinates position;
  int velocity;
} Shoot;

typedef struct {
  Coordinates position;
  int velocity;
  PowerupType type;
  PowerupEffect effect;
} Powerup;