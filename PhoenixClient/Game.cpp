#include "stdafx.h"

#include "Communication.h"
#include "Game.h"

int getPlayerScore(Client *client) {
  for (int i = 0; i < client->game.totalPlayers; i++) {
    if (client->game.player[i].id == client->id) {
      return client->game.player[i].score;
    }
  }

  return -1;
}