﻿// PhoenixServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Game.h"
#include "GameData.h"
#include "PhoenixServer.h"

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  initGameData();

  system("pause");

  return 0;
}