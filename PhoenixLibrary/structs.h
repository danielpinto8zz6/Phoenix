#pragma once

#include <Windows.h>
#include <tchar.h>

#include "PhoenixLibrary.h"

#define PLAYERS 10
#define ENEMYSHIPS 20
#define WIDTH 1000
#define HEIGHT 1000

#define MAXCLIENTS 20

typedef enum { SHIELD, ICE, BATTERY, PLUS, ALCOOL } PowerupType;

typedef enum { LEFT, RIGHT, SHOT } MovType;


typedef enum { BASIC, DODGE } EnemyType;

typedef enum { NONE } PowerupEffect;

typedef enum {
  LOGIN,
  SUCCESS,
  LOGGED,
  UPDATE_GAME,
  PLAYER_ADDED,
  CLIENT_DISCONNECTED,
  GATEWAY_DISCONNECTED,
  SERVER_DISCONNECTED,
  KEYDOWN,
  KEYUP,
  KEYLEFT,
  KEYRIGHT,
  KEYSPACE,
  GAME_STARTED,
  JOIN_GAME,
  IN_GAME,
  CANT_JOIN,
  PLAYER_LOST
} Command;

typedef struct {
  int x;
  int y;
} Coordinates;

typedef struct {
  int width;
  int height;
} Size;

typedef struct {
  Coordinates position;
  int velocity;
  Size size;
  int firingRate;
} DefenderShip;

typedef struct {
  int id;
  TCHAR username[50];
  DefenderShip ship;
  int lifes;
  int score;
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
  HANDLE inboundPipe;
  HANDLE outboundPipe;
} Pipes;

typedef struct {
  Command cmd;
  TCHAR text[80];
  BOOL sendToAllClients;
  int clientId;
} Message;


typedef struct {
  TCHAR username[50];
  int score;
} Top;

typedef struct {
  int level;
  Player player[PLAYERS];
  int totalPlayers;
  int maxPlayers;
  EnemyShip enemyShip[ENEMYSHIPS];
  int totalEnemyShips;
  int maxEnemyShips;
  int velocityEnemyShips;
  int earlyLives;
  int maxPowerups;
  int powerupsDuration;
  int powerupsProbabilityOccurrence;
  BOOL started;
  Message message;
  Top topTen[10];
} Game;

typedef struct {
  Game *sharedGame;
  Game game;
  BOOL STOP;
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
  HANDLE serverMessageUpdateEvent;
  HANDLE gatewayMessageUpdateEvent;
} MessageData;

typedef struct {
  int id;
  TCHAR username[50];
  BOOL threadContinue;
  HANDLE hPipeGame;
  HANDLE hPipeMessage;
  HANDLE hEvent;
  BOOL readerAlive;
  Game game;
  BOOL gameStarted;
  BOOL logged;
  BOOL inGame;
  HWND hWnd;
} Client;

typedef struct {
  int id;
  TCHAR username[50];
} Clients;

typedef struct {
  MessageData *messageData;
  GameData *gameData;
  Client client[MAXCLIENTS];
  Clients clients[MAXCLIENTS];
  int totalClients;
  HANDLE tmpPipeMessage;
  HANDLE tmpPipeGame;
  HANDLE hEvent;
} Data;