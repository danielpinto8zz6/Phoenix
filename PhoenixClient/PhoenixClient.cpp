// PhoenixClient.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "Communication.h"
#include "Game.h"
#include "PhoenixClient.h"
#include <process.h>
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                     // current instance
WCHAR szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

HBITMAP hNaveBasic = NULL;
HBITMAP hNaveDefender = NULL;
HBITMAP hNaveDodge = NULL;
HBITMAP hBomb = NULL;
HBITMAP hShot = NULL;
HBITMAP hPower1 = NULL;
HBITMAP hPower2 = NULL;
HBITMAP hPower3 = NULL;
HBITMAP hPower4 = NULL;

int x, y;
HDC hdc = NULL, auxDC = NULL;
HBRUSH bg = NULL;
HBITMAP auxBM = NULL;
int nX = 0, nY = 0;

TCHAR login[40] = TEXT("\0");
TCHAR key[10] = TEXT("\0");

HBITMAP bitmaps[20];

Client client;

BOOL started = FALSE;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  HANDLE hThreadMessageReceiver;
  DWORD threadMessageReceiverId = 0;
  HANDLE hThreadGameReceiver;
  DWORD threadGameReceiverId = 0;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  loadBitmaps();

  client.logged = FALSE;
  client.inGame = FALSE;
  client.gameStarted = FALSE;

  if (!isGatewayRunning()) {
    errorGui(TEXT("There's no gateway instance running! Start gateway first!"));
    return FALSE;
  }

  if (!connectPipes(&client)) {
    return FALSE;
  }

  client.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  client.threadContinue = TRUE;

  hThreadMessageReceiver = CreateThread(NULL, 0, messageReceiver, &client, 0,
                                        &threadMessageReceiverId);
  if (hThreadMessageReceiver == NULL) {
    errorGui(TEXT("Creating data receiver thread"));
    return FALSE;
  }

  hInst = hInstance;

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_PHOENIXCLIENT, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  hThreadGameReceiver =
      CreateThread(NULL, 0, gameReceiver, &client, 0, &threadGameReceiverId);
  if (hThreadGameReceiver == NULL) {
    errorGui(TEXT("Creating data receiver thread"));
    return FALSE;
  }

  HACCEL hAccelTable =
      LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PHOENIXCLIENT));

  MSG msg;

  // Main message loop:
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PHOENIXCLIENT));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PHOENIXCLIENT);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindowW(
      szWindowClass, szTitle,
      WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, 0,
      1000, 850, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd) {
    return FALSE;
  }

  client.hWnd = hWnd;

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  HDC hdc;
  static int xi = 0, yi = 0;
  HBITMAP hBitmap;
  RECT r;
  GetWindowRect(hWnd, &r);
  PAINTSTRUCT pt;

  switch (message) {
  case WM_CREATE:
    hdc = GetDC(hWnd);
    auxDC = NULL;
    ReleaseDC(hWnd, hdc);
    break;
  case WM_COMMAND: {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId) {
    case ID_FILE_LOGIN:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hWnd, Login);
      break;
    case ID_FILE_JOINGAME:
      if (client.inGame) {
        MessageBox(NULL, TEXT("In Game"), TEXT("Already in game!"),
                   MB_OK | MB_ICONINFORMATION);
        return 0;
      }
      if (!client.logged) {
        MessageBox(NULL, TEXT("Login!"), TEXT("Please login first"),
                   MB_OK | MB_ICONINFORMATION);
        return 0;
      }
      if (client.gameStarted) {
        MessageBox(NULL, TEXT("Can't join!"), TEXT("Game already started!"),
                   MB_OK | MB_ICONINFORMATION);
        return 0;
      }
      if (!joinGame(&client)) {
        MessageBox(NULL, TEXT("Can't join!"), TEXT("Error"),
                   MB_OK | MB_ICONINFORMATION);
        return 0;
      }
      break;
    case ID_FILE_TOP10:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SCORE), hWnd, Score);
      break;
    case IDM_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
      break;
    case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  } break;
  case WM_KEYDOWN:
    Message msg;

    if (client.gameStarted && client.inGame) {
      if (wParam == VK_RIGHT) {
        msg.cmd = KEYRIGHT;
        writeDataToPipeAsync(client.hPipeMessage, client.hEvent, &msg,
                             sizeof(Message));
      }
      if (wParam == VK_LEFT) {
        msg.cmd = KEYLEFT;
        writeDataToPipeAsync(client.hPipeMessage, client.hEvent, &msg,
                             sizeof(Message));
      }
      if (wParam == VK_UP) {
        msg.cmd = KEYUP;
        writeDataToPipeAsync(client.hPipeMessage, client.hEvent, &msg,
                             sizeof(Message));
      }
      if (wParam == VK_DOWN) {
        msg.cmd = KEYDOWN;
        writeDataToPipeAsync(client.hPipeMessage, client.hEvent, &msg,
                             sizeof(Message));
      }
      if (wParam == VK_SPACE) {
        msg.cmd = KEYSPACE;
        writeDataToPipeAsync(client.hPipeMessage, client.hEvent, &msg,
                             sizeof(Message));
      }
    }
    break;
  case WM_CLOSE:
    if (MessageBox(hWnd, TEXT("Exit?"), TEXT("Exit"),
                   MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDYES)
      DestroyWindow(hWnd);
    break;
  case WM_PAINT:
    hdc = BeginPaint(hWnd, &pt);

    if (auxDC == NULL) {
      auxDC = CreateCompatibleDC(hdc);
      hBitmap = CreateCompatibleBitmap(hdc, r.right - r.left, r.bottom - r.top);
      SelectObject(auxDC, hBitmap);
    }

    if (client.gameStarted) {
      drawGame(r);
    }

    BitBlt(hdc, 0, 0, r.right - r.left, r.bottom - r.top, auxDC, 0, 0, SRCCOPY);

    EndPaint(hWnd, &pt);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    // DeleteObject(hNave);
    // DeleteDC(hdcNave);

    /**
     * Release resources in memory
     */
    DeleteObject(bg);
    DeleteObject(auxBM);
    DeleteDC(auxDC);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return (INT_PTR)FALSE;
}

BOOL CALLBACK Login(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
  Message message;

  switch (messg) {
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      GetDlgItemText(hWnd, IDC_EDIT_USERNAME, message.text, 50);
      if (_tcslen(message.text) == 0) {
        MessageBox(NULL, TEXT("Fill in all the fields first!"),
                   TEXT("Missing fields"), MB_OK | MB_ICONINFORMATION);
        break;
      }

      message.cmd = LOGIN;
      _tcscpy_s(client.username, message.text);

      if (!writeDataToPipeAsync(client.hPipeMessage, client.hEvent, &message,
                                sizeof(Message))) {
        MessageBox(NULL, TEXT("Can't communicate with gateway!"), TEXT("Error"),
                   MB_OK | MB_ICONINFORMATION);
        break;
      }

      EndDialog(hWnd, LOWORD(wParam));
      return (INT_PTR)TRUE;
    case IDCANCEL:
      EndDialog(hWnd, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return 0;
}

INT_PTR CALLBACK Score(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
  case WM_INITDIALOG:
    for (int i = 0; i < 10; i++) {
      TCHAR name[20];
      TCHAR points[20];

      _stprintf_s(nome, 20, TEXT("Daniel"));
      _stprintf_s(points, 20, TEXT("%d"), 10);

      SendDlgItemMessage(hDlg, IDC_LIST3, LB_ADDSTRING, NULL, (LPARAM)name);
      SendDlgItemMessage(hDlg, IDC_LIST2, LB_ADDSTRING, NULL, (LPARAM)points);
    }
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return 0;
}

void drawGame(RECT r) {
  HDC hdcBuff;

  for (int i = 0; i < 10; i++) {
    hdcBuff = CreateCompatibleDC(auxDC);
    SelectObject(hdcBuff, bitmaps[0]);

    StretchBlt(auxDC, 50 * i, 0, 50, 50, hdcBuff, 0, 0, 50, 50, SRCCOPY);

    DeleteObject(hdcBuff);
  }
}

void loadBitmaps() {
  bitmaps[0] =
      LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_BOMBS));
}