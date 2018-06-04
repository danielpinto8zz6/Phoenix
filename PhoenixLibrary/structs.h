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
  DWORD num;
  Player player[PLAYERS];
  EnemyShip enemyShip[ENEMYSHIPS];
  TCHAR map[HEIGHT][WIDTH];
  int totalEnemyShips;
} Game;

typedef struct {
  HANDLE inboundPipe;
  HANDLE outboundPipe;
} Pipes;

typedef struct {
  Command cmd;
  TCHAR text[50];
  DWORD num;
} Message;

typedef struct {
  Game *sharedGame;
  Game game;
  BOOL ThreadMustConinue;
  HANDLE hMapFile;
  HANDLE hMutex;
  HANDLE gameUpdateEvent;
  /**
   * Hack
   */
  int position;
} GameData;

typedef struct {
  Message *sharedMessage;
  Message message;
  HANDLE hMapFile;
  HANDLE hMutex;
  BOOL STOP;
  DWORD currrentMessage;
  Pipes *pipes;
  HANDLE messageUpdateEvent;
} MessageData;

typedef struct {
  MessageData *messageData;
  GameData *gameData;
} Data;