// PhoenixDll.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//DLL.cpp
#include <windows.h>
#include "PhoenixDll.h"
//Para verificar ao carregar a dll que a aplicação irá ocupar mais memória
char pointer[40960];

//Exportar a função para ser utilizada fora da DLL
void CheckDLL(void){
	  _tprintf(TEXT("Dll Loaded\n"));
}