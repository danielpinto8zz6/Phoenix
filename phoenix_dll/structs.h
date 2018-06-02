#pragma once

#include <tchar.h>
#include <windows.h>

#define PLAYERS 10
#define ENEMYSHIPS 20
#define WIDTH 10
#define HEIGHT 15

#define PIPE_NAME_INBOUND TEXT("\\\\.\\pipe\\phoenix-gateway-inbountd")
#define PIPE_NAME_OUTBOUND TEXT("\\\\.\\pipe\\phoenix-gateway-outbound")


typedef enum { SHIELD, ICE, BATTERY, PLUS, ALCOOL } PowerupType;

typedef enum { BASIC, DODGE } EnemyType;

typedef enum { NONE } PowerupEffect;

typedef enum { LOGIN } Command;

typedef struct {
  int x;
  int y;
} Coordinates;

typedef struct {
  int x;
  int y;
} Size;

typedef struct {
  Coordinates position;
  int velocity;
  Size size;
  int firingRate;
} DefenderShip;

typedef struct {
  TCHAR username[50];
  DefenderShip ship;
  int lifes;
  int points;
} Player;

typedef struct {
  Coordinates position;
} Bomb;

typedef struct {
  Coordinates position;
  int points;
  int velocity;
  Size size;
  EnemyType type;
  Bomb bombs[50];
} EnemyShip;

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

typedef struct {
  int level;
  unsigned num;
  Player player[PLAYERS];
  EnemyShip enemy_ship[ENEMYSHIPS];
  TCHAR map[HEIGHT][WIDTH];
} Game;

typedef struct {
  HANDLE hMapFile;
  Game *game;
  int ThreadMustConinue;
  HANDLE hMutex;
  HANDLE smRead;
  HANDLE smWrite;
} ControlData;

typedef struct {
  Command cmd;
  BOOL Stop;
} Message;

typedef struct {
  HANDLE inboundPipe;
  HANDLE outboundPipe;
} ClientPipes;
