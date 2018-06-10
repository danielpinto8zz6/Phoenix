#include "stdafx.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#define TAM 200

typedef struct {
  int score;
  TCHAR username[50];
} Punctuation;

void Registry() {

  HKEY key;
  DWORD result, size;
  DWORD score;
  score = 10;

  if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Phoenix"), 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key,
                     &result) != ERROR_SUCCESS) {
    error(TEXT("Can't create key in registry. Game data could be "
               "affected!"));
    return;
  } else {
    if (result == REG_CREATED_NEW_KEY) {
      debug(TEXT("Key: HKEY_CURRENT_USER\\Software\\Phoenix created!"));

      RegSetValueEx(key, TEXT("score"), 0, REG_DWORD, (LPBYTE)&score,
                    sizeof(DWORD));
    } else if (result == REG_OPENED_EXISTING_KEY) {
      _tprintf(TEXT("Key: HKEY_CURRENT_USER\\Software\\Phoenix opened!\n"));
      size = 20;
      size = sizeof(score);
      RegQueryValueEx(key, TEXT("score"), NULL, NULL, (LPBYTE)&score, &size);
      score++;
      RegSetValueEx(key, TEXT("score"), 0, REG_DWORD, (LPBYTE)&score,
                    sizeof(DWORD));
    }
    RegCloseKey(key);
  }
}