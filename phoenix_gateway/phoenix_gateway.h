#pragma once
#include "stdafx.h"

int _tmain();
unsigned int __stdcall threadListener(LPVOID lpParam);
LPVOID clientsDataReceiver();
LPVOID sendDataToClient(int clientId, LPVOID data);