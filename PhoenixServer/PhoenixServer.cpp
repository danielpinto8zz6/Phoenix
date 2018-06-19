// PhoenixServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"
#include "MessageZone.h"
#include "PhoenixServer.h"
#include "Registry.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                     // current instance
WCHAR szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  MessageData messageData;
  GameData gameData;
  Data data;

  DWORD threadReceiveMessagesFromServerId;
  HANDLE hThreadReceiveMessagesFromGateway;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  data.gameData = &gameData;
  data.messageData = &messageData;

  initGameVariables(&data.gameData->game);

  /**
   * Use an event to check if the program is running
   * Start event at instance start
   */
  if (isServerRunning()) {
    errorGui(
        TEXT("There is an instance of server already running! Only 1 server "
             "at time"));
    return FALSE;
  }

  DWORD threadManageEnemyShipsId;
  HANDLE hThreadManageEnemyShips;

  if (!initGameZone(&gameData)) {
    errorGui(TEXT("Can't connect game data with server. Exiting..."));
    return FALSE;
  }

  gameData.gameUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, GAME_UPDATE_EVENT);

  if (gameData.gameUpdateEvent == NULL) {
    errorGui(TEXT("CreateEvent failed"));
    return FALSE;
  }

  if (!initMessageZone(&messageData)) {
    errorGui(TEXT("Can't connect message data with server. Exiting..."));
    return FALSE;
  }

  messageData.gatewayMessageUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, MESSAGE_GATEWAY_UPDATE_EVENT);

  if (messageData.gatewayMessageUpdateEvent == NULL) {
    errorGui(TEXT("CreateEvent failed"));
    return FALSE;
  }

  messageData.serverMessageUpdateEvent =
      CreateEventW(NULL, FALSE, FALSE, MESSAGE_SERVER_UPDATE_EVENT);

  if (messageData.serverMessageUpdateEvent == NULL) {
    errorGui(TEXT("CreateEvent failed"));
    return FALSE;
  }

  hThreadReceiveMessagesFromGateway =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveMessagesFromGateway,
                   &data, 0, &threadReceiveMessagesFromServerId);
  if (hThreadReceiveMessagesFromGateway == NULL) {
    errorGui(TEXT("Creating thread to receive data from server"));
    return FALSE;
  }

  hThreadManageEnemyShips =
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadManageEnemyShips,
                   &gameData, 0, &threadManageEnemyShipsId);
  if (hThreadManageEnemyShips == NULL) {
    errorGui(TEXT("Creating thread to manage enemy ships"));
    return FALSE;
  }

  Registry();

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_PHOENIXSERVER, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  HACCEL hAccelTable =
      LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PHOENIXSERVER));

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
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PHOENIXSERVER));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PHOENIXSERVER);
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

  HWND hWnd =
      CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                    0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd) {
    return FALSE;
  }

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
  switch (message) {
  case WM_COMMAND: {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId) {
    case ID_FILE_CONFIGURE:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_CONFIGURATION), hWnd,
                Configure);
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
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...
    EndPaint(hWnd, &ps);
  } break;
  case WM_DESTROY:
    PostQuitMessage(0);
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

INT_PTR CALLBACK Configure(HWND hDlg, UINT message, WPARAM wParam,
                           LPARAM lParam) {
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

VOID handleClose(MessageData *messageData) {
  Message msg;
  msg.cmd = SERVER_CLOSING;
  writeDataToSharedMemory(messageData->sharedMessage, &msg, sizeof(Message),
                          messageData->hMutex,
                          messageData->gatewayMessageUpdateEvent);
}

VOID initGameVariables(Game *game) {
  game->level = 1;

  game->totalPlayers = 0;

  game->totalEnemyShips = 0;

  game->started = FALSE;
}
