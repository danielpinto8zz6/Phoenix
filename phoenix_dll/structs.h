#pragma once
#include <tchar.h>
#include <windows.h>

typedef enum { SHIELD, ICE, BATTERY, PLUS, ALCOOL } PowerupType;

typedef enum { BASIC, DODGE } InvaderType;

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
	InvaderType type;
	Bomb bombs[50];
} InvasionShip;

typedef struct {
	int level;
} Game;

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