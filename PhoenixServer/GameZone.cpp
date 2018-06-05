#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"

BOOL sendGameToGateway(GameData *gameData, Game *game) {
  writeDataToSharedMemory(gameData->sharedGame, game, sizeof(Game),
                          &gameData->hMutex);
  if (!SetEvent(gameData->gameUpdateEvent)) {
    error(TEXT("SetEvent failed"));
  }
  debug(TEXT("%d Bytes written in shared memory"), sizeof(Game));
  return TRUE;
}