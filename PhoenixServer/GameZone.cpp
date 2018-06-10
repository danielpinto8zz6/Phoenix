#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"

BOOL sendGameToGateway(GameData *gameData, Game *game) {
  writeDataToSharedMemory(gameData->sharedGame, game, sizeof(Game),
                          gameData->hMutex, gameData->gameUpdateEvent);
  // debug(TEXT("%d Bytes written in shared memory"), sizeof(Game));
  return TRUE;
}
