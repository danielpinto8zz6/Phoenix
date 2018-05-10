// phoenix_gateway.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../phoenix_dll/phoenix_dll.h"
#include "../phoenix_dll/structs.h"

int main() {
  ControlData data;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  if (!initMemAndSync(&data)) {
    return -1;
  }

  data.game =
      (Game *)MapViewOfFile(data.hMapFile, FILE_MAP_WRITE, 0, 0, sizeof(Game));

  if (data.game == NULL) {
    _tprintf(TEXT("[Erro]Mapeamento da memória partilhada(%d)\n"),
             GetLastError());
    return -1;
  }

  while (1) {
    Sleep(1000);
    readData(&data, data.game);
    for (int i = 0; i < ENEMYSHIPS; i++) {
      _tprintf(TEXT("Position X Y: (%d , %d)\n"),
               data.game->enemy_ship[i].position.x,
               data.game->enemy_ship[i].position.y);
    }
  }

  return 0;
}
