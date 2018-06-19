// PhoenixClient.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "Communication.h"
#include "PhoenixClient.h"
#include <process.h>
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                     // current instance
WCHAR szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name

HBITMAP hNave = NULL;

int x, y;
HDC hdc = NULL, auxDC = NULL;
HBRUSH bg = NULL;
HBITMAP auxBM = NULL;
int nX = 0, nY = 0;

TCHAR login[40] = TEXT("\0");
TCHAR key[10] = TEXT("\0");

Client client;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  HANDLE hThreadMessageReceiver;
  DWORD threadMessageReceiverId = 0;

#ifdef UNICODE
  _setmode(_fileno(stdin), _O_WTEXT);
  _setmode(_fileno(stdout), _O_WTEXT);
#endif

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
  static BITMAP bmNave;
  static HDC hdcNave;

  switch (message) {
  case WM_CREATE:

    hNave =
        (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_BOMBS),
                           IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
    hdc = GetDC(hWnd);
    GetObject(hNave, sizeof(bmNave), &bmNave);
    hdcNave = CreateCompatibleDC(hdc);
    SelectObject(hdcNave, hNave);
    ReleaseDC(hWnd, hdc);

    // OBTEM AS DIMENSOES DO DISPLAY...
    bg = CreateSolidBrush(RGB(255, 128, 128));
    nX = GetSystemMetrics(SM_CXSCREEN);
    nY = GetSystemMetrics(SM_CYSCREEN);

    // PREPARA 'BITMAP' E ASSOCIA A UM 'DC' EM MEMORIA...
    hdc = GetDC(hWnd);
    auxDC = CreateCompatibleDC(hdc);
    auxBM = CreateCompatibleBitmap(hdc, nX, nY);
    SelectObject(auxDC, auxBM);
    SelectObject(auxDC, GetStockObject(GRAY_BRUSH));
    PatBlt(auxDC, 0, 0, nX, nY, PATCOPY);
    ReleaseDC(hWnd, hdc);

    break;
  case WM_COMMAND: {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId) {
    case ID_FILE_LOGIN:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hWnd, Login);
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

    if (client.gameStarted) {
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
  case WM_PAINT: {
    PAINTSTRUCT ps;
    PatBlt(auxDC, 0, 0, nX, nY, PATCOPY);

    SetStretchBltMode(auxDC, BLACKONWHITE);
    StretchBlt(auxDC, 25, 80, 100, 60, hdcNave, 0, 0, bmNave.bmWidth,
               bmNave.bmHeight, SRCCOPY);

    // COPIA INFORMACAO DO 'DC' EM MEMORIA PARA O DISPLAY...
    hdc = BeginPaint(hWnd, &ps);
    BitBlt(hdc, 0, 0, nX, nY, auxDC, 0, 0, SRCCOPY);
    EndPaint(hWnd, &ps);
  } break;
  case WM_DESTROY:
    PostQuitMessage(0);
    DeleteObject(hNave);
    DeleteDC(hdcNave);

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