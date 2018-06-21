// PhoenixServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "Game.h"
#include "GameZone.h"
#include "MessageZone.h"
#include "PhoenixServer.h"
#include "Registry.h"

#define MAX_LOADSTRING 100

#define IDC_START_GAME 101

Data data;

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

  DWORD threadReceiveMessagesFromServerId;
  HANDLE hThreadReceiveMessagesFromGateway;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

  data.gameData = &gameData;
  data.messageData = &messageData;

  data.totalClients = 0;

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

  HWND hWnd = CreateWindowW(
      szWindowClass, szTitle,
      WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, 0,
      500, 350, nullptr, nullptr, hInstance, nullptr);

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
  static HWND hwndButton;

  switch (message) {
  case WM_CREATE:
    hwndButton = CreateWindow(
        TEXT("BUTTON"),     // Predefined class; Unicode assumed
        TEXT("Start Game"), // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
        150,                                                   // x position
        50,                                                    // y position
        200,                                                   // Button width
        100,                                                   // Button height
        hWnd,                                                  // Parent window
        (HMENU)IDC_START_GAME,                                 // No menu.
        (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
        NULL); // Pointer not needed.
    break;
  case WM_COMMAND: {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId) {
    case ID_FILE_CONFIGURE:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_CONFIGURATION), hWnd,
                Configure);
      break;
    case IDC_START_GAME:
    case ID_FILE_STARTGAME:
      if (data.gameData->game.started) {
        MessageBox(hWnd, TEXT("Game already started!"), TEXT("Error"),
                   MB_OK | MB_ICONINFORMATION);
        return FALSE;
      }
      Message msg;
      msg.cmd = GAME_STARTED;
      data.gameData->game.started = TRUE;
      writeDataToSharedMemory(data.messageData->sharedMessage, &msg,
                              sizeof(Message), data.messageData->hMutex,
                              data.messageData->gatewayMessageUpdateEvent);
      startGame(&data);
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
  case WM_CLOSE:
    if (MessageBox(hWnd, TEXT("Exit?"), TEXT("Exit"),
                   MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDYES)
      DestroyWindow(hWnd);
    break;
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

  int eN, eV, pN, pD, pOc, maxP, eL,eD;
  BOOL fSuccess;

  switch (message) {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      if (data.gameData->game.started) {
        MessageBox(
            hDlg,
            TEXT("Game already started! Configurations won't take effect!"),
            TEXT("Game started"), MB_OK | MB_ICONINFORMATION);
      }

      eN = GetDlgItemInt(hDlg, IDC_EDIT5, &fSuccess, TRUE);
      if (eN != 0) {
        if (eN > MAX_ENEMY_SHIPS) {
          MessageBox(hDlg, TEXT("Please input a number between 1 & 20!"),
                     TEXT("Enemy Ships number"), MB_OK | MB_ICONINFORMATION);
          break;
        }
        data.gameData->game.maxEnemyShips = eN;
      }

      eV = GetDlgItemInt(hDlg, IDC_EDIT4, &fSuccess, TRUE);
      if (eV != 0) {
        if (eV > 10) {
          MessageBox(hDlg, TEXT("Please input a number between 1 & 10!"),
                     TEXT("Enemy Ships velocity"), MB_OK | MB_ICONINFORMATION);
          break;
        }
        data.gameData->game.velocityEnemyShips = eV;
      }

      maxP = GetDlgItemInt(hDlg, IDC_EDIT6, &fSuccess, TRUE);
      if (maxP != 0) {
        if (maxP > 5) {
          MessageBox(hDlg, TEXT("Please input a number between 1 & 5!"),
                     TEXT("Enemy Ships velocity"), MB_OK | MB_ICONINFORMATION);
          break;
        }
        data.gameData->game.maxPlayers = maxP;
      }
	  eD = GetDlgItemInt(hDlg, IDC_EDIT8, &fSuccess, TRUE);
	  if (maxP != 0) {
		  if (maxP > 5) {
			  MessageBox(hDlg, TEXT("Please input a number between 1 & 5!"),
				  TEXT("Enemy Ships difficulty"), MB_OK | MB_ICONINFORMATION);
			  break;
		  }
		  data.gameData->game.maxPlayers = eD;
	  }

      eL = GetDlgItemInt(hDlg, IDC_EDIT7, &fSuccess, TRUE);
      if (eL != 0) {
        data.gameData->game.earlyLives = eL;
      }

      pN = GetDlgItemInt(hDlg, IDC_EDIT3, &fSuccess, TRUE);
      if (pN != 0) {
        if (pN > 20) {
          MessageBox(hDlg, TEXT("Please input a number between 1 & 20!"),
                     TEXT("Number of Powerups"), MB_OK | MB_ICONINFORMATION);
          break;
        }
        data.gameData->game.maxPowerups = pN;
      }

      pD = GetDlgItemInt(hDlg, IDC_EDIT2, &fSuccess, TRUE);
      if (pD != 0) {
        if (pD > 5) {
          MessageBox(hDlg, TEXT("Please input a number between 1 & 5!"),
                     TEXT("Powerups duration"), MB_OK | MB_ICONINFORMATION);
          break;
        }
        data.gameData->game.maxPowerups = pD;
      }

      pOc = GetDlgItemInt(hDlg, IDC_EDIT1, &fSuccess, TRUE);
      if (pOc != 0) {
        if (pOc > 10) {
          MessageBox(hDlg, TEXT("Please input a number between 1 & 10!"),
                     TEXT("Powerups occurrence probability"),
                     MB_OK | MB_ICONINFORMATION);
          break;
        }
        data.gameData->game.powerupsProbabilityOccurrence = pOc;
      }

      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    case IDCANCEL:
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return (INT_PTR)FALSE;
}

VOID handleClose(MessageData *messageData) {
  Message msg;
  msg.cmd = SERVER_DISCONNECTED;
  writeDataToSharedMemory(messageData->sharedMessage, &msg, sizeof(Message),
                          messageData->hMutex,
                          messageData->gatewayMessageUpdateEvent);
}

VOID initGameVariables(Game *game) {
  game->level = 1;

  game->totalPlayers = 0;

  game->earlyLives = 2;

  game->totalEnemyShips = 0;

  game->maxEnemyShips = MAX_ENEMY_SHIPS;

  game->velocityEnemyShips = 1;

  game->powerupsDuration = 10;

  game->powerupsProbabilityOccurrence = 1;

  game->difficulty = 1;

  game->maxPlayers = MAX_PLAYERS;

  game->started = FALSE;

  for (int i = 0; i < 10; i++) {
    _stprintf_s(game->topTen[i].username, 50, TEXT("Daniel"));
    game->topTen[i].score = 0;
  }
}
