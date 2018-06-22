#include "stdafx.h"

#include "Communication.h"
#include "Game.h"

ScoreBoard getPlayerScoreBoard(Client *client) {
  ScoreBoard scoreBoard;
  for (int i = 0; i < client->game.maxPlayers; i++) {
    if (!client->game.player[i].isEmpty) {
      if (client->game.player[i].id == client->id) {
        scoreBoard.score = client->game.player[i].score;
        scoreBoard.lives = client->game.player[i].lives;
        scoreBoard.level = client->game.level;
        return scoreBoard;
      }
    }
  }

  scoreBoard.score = -1;
  scoreBoard.lives = -1;
  scoreBoard.level = -1;

  return scoreBoard;
}