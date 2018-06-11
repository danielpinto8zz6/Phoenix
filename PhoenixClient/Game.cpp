#include "stdafx.h"

#include "Communication.h"

BOOL CALLBACK Menu(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
  switch (messg) {
  case WM_CLOSE:
    if (MessageBox(hWnd, TEXT("Are you sure?"), TEXT("Close"), MB_YESNO) ==
        IDYES) {
      PostQuitMessage(0);
      exit(0);
    }
  }
  return 0;
}