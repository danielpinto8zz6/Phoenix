#pragma once
#include "stdafx.h"

int _tmain();
unsigned int __stdcall threadListener(LPVOID lpParam);
DWORD WINAPI manageClients(LPVOID lpParam);