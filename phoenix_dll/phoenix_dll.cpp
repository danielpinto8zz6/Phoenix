#include "phoenix_dll.h"
#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#define TAM 256

// Para verificar ao carregar a dll que a aplicação irá ocupar mais memória
char ponteiro[40960];
// Definição da variável global
int nDLL = 1234;
// Exportar a função para ser utilizada fora da DLL
int UmaString(void) {
  TCHAR str[TAM];
  _tprintf(TEXT("Dentro da Dll\nIntroduza uma frase:"));
  _fgetts(str, TAM, stdin);
  if (_tcslen(str) > 1) // Introduzir mais caracteres do que apenas <enter>
    return 1;
  else
    return 0;
}