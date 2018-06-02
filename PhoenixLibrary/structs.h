#pragma once

#include <Windows.h>
#include <tchar.h>

#include "PhoenixLibrary.h"

#define PLAYERS 10
#define ENEMYSHIPS 20
#define WIDTH 10
#define HEIGHT 15

typedef enum { SHIELD, ICE, BATTERY, PLUS, ALCOOL } PowerupType;

typedef enum { BASIC, DODGE } EnemyType;

typedef enum { NONE } PowerupEffect;

typedef enum { LOGIN, SUCCESS, LOGGED } Command;

typedef struct {
  INT x;
  INT y;
} Coordinates;

typedef struct {
  INT x;
  INT y;
} Size;

typedef struct {
  Coordinates position;
  INT velocity;
  Size size;
  INT firingRate;
} DefenderShip;

typedef struct {
  DefenderShip ship;
  INT lifes;
  INT points;
} Player;

typedef struct {
  Coordinates position;
} Bomb;

typedef struct {
  Coordinates position;
  INT points;
  INT velocity;
  Size size;
  EnemyType type;
  Bomb bombs[50];
} EnemyShip;

typedef struct {
  Coordinates position;
  INT velocity;
} Shoot;

typedef struct {
  Coordinates position;
  INT velocity;
  PowerupType type;
  PowerupEffect effect;
} Powerup;

typedef struct {
  INT level;
  unsigned num;
  Player player[PLAYERS];
  EnemyShip enemy_ship[ENEMYSHIPS];
  TCHAR map[HEIGHT][WIDTH];
} Game;

typedef struct {
  HANDLE hMapFile;
  Game *game;
  INT ThreadMustConinue;
  HANDLE hMutex;
  HANDLE smRead;
  HANDLE smWrite;
} ControlData;

typedef struct {
  Command cmd;
  BOOL Stop;
  TCHAR text[50];
} Message;

typedef struct {
  HANDLE inboundPipe;
  HANDLE outboundPipe;
} Pipes;

typedef struct {
  TCHAR username[50];
  Pipes pipes;
} Client;