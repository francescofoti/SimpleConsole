// SimpleConsole.c : Defines the entry point for the application.
//
#include <windows.h>
#include <windef.h>
#include "strsafe.h"
#include "SimpleConsole.h"
#include "consoul.h"
#include "callbacks.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//consoul
HWND mhWndConsole;
#define TIMERID_1 1234L
UINT muiCounter = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SIMPLECONSOLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPLECONSOLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLECONSOLE));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(RGB(0,0,0));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SIMPLECONSOLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

VOID SaveConsoulBitmap(PWCHAR pszFilename) {
  HDC hdcWindow;
  HDC hdcMemDC = NULL;
  HBITMAP hbmScreen = NULL;
  BITMAP bmpScreen;

  hdcWindow = GetDC(mhWndConsole);

  // Create a compatible DC which is used in a BitBlt from the window DC
  hdcMemDC = CreateCompatibleDC(hdcWindow);

  if (!hdcMemDC)
  {
    MessageBoxW(mhWndConsole, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
    goto done;
  }

  // Get the client area for size calculation
  RECT rcClient;
  GetClientRect(mhWndConsole, &rcClient);
  // Create a compatible bitmap from the Window DC
  hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
  if (!hbmScreen)
  {
    MessageBoxW(mhWndConsole, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
    goto done;
  }

  // Select the compatible bitmap into the compatible memory DC.
  SelectObject(hdcMemDC, hbmScreen);
  // Bit block transfer into our compatible memory DC.
  if (!BitBlt(hdcMemDC,
    0, 0,
    rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
    hdcWindow,
    0, 0,
    SRCCOPY))
  {
    MessageBoxW(mhWndConsole, L"BitBlt has failed", L"Failed", MB_OK);
    goto done;
  }

  // Get the BITMAP from the HBITMAP
  GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

  BITMAPFILEHEADER   bmfHeader;
  BITMAPINFOHEADER   bi;

  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bmpScreen.bmWidth;
  bi.biHeight = bmpScreen.bmHeight;
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

  // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
  // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
  // have greater overhead than HeapAlloc.
  HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
  char *lpbitmap = (char *)GlobalLock(hDIB);

  // Gets the "bits" from the bitmap and copies them into a buffer 
  // which is pointed to by lpbitmap.
  GetDIBits(hdcWindow, hbmScreen, 0,
    (UINT)bmpScreen.bmHeight,
    lpbitmap,
    (BITMAPINFO *)&bi, DIB_RGB_COLORS);

  // A file is created, this is where we will save the screen capture.
  HANDLE hFile = CreateFileW(pszFilename,
    GENERIC_WRITE,
    0,
    NULL,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL, NULL);

  // Add the size of the headers to the size of the bitmap to get the total file size
  DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  //Offset to where the actual bitmap bits start.
  bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

  //Size of the file
  bmfHeader.bfSize = dwSizeofDIB;

  //bfType must always be BM for Bitmaps
  bmfHeader.bfType = 0x4D42; //BM   

  DWORD dwBytesWritten = 0;
  WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
  WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
  WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

  //Unlock and Free the DIB from the heap
  GlobalUnlock(hDIB);
  GlobalFree(hDIB);

  //Close the handle for the file that was created
  CloseHandle(hFile);

  //Clean up
done:
  DeleteObject(hbmScreen);
  DeleteObject(hdcMemDC);
  ReleaseDC(mhWndConsole, hdcWindow);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void ToggleUseCallbacks(HWND hWndConsole) {
  static UINT wUseCallbacks = 0;
  if (wUseCallbacks == 0) {
    CSSetOnMouseButtonCallback(hWndConsole, (void *)OnConsoulMouseButton);
    CSSetOnDrawZoneCallback(hWndConsole, (void *)OnConsoulZonePaint);
    CSSetOnWmPaintCallback(hWndConsole, WMPAINTCBK_AFTER, (void *)OnConsoulWmPaint);
    wUseCallbacks = 1;
    CSPushLine(hWndConsole, L"Switching to callbacks", 1);
  }
  else {
    /* CSSetOnWmPaintCallback works only with a callback function,
     * it has no equivalent windows (user) message.
     * Once the callback function has been set, it sticks even
     * if we change the Consoul calling mechanism to messages
     * (CSSetUseCallbacks(hWndConsole, 0).
     * So we have to set it to a NULL pointer to disable the callbacks.
     */
    CSSetOnWmPaintCallback(hWndConsole, WMPAINTCBK_AFTER, NULL);
    wUseCallbacks = 0;
    CSPushLine(hWndConsole, L"Switching to windows messages", 1);
  }
  CSSetUseCallbacks(hWndConsole, wUseCallbacks);
}

HWND CreateConsole(HWND hWndParent) {
  HWND hWndConsole;
  hWndConsole = (HWND)CSCreateLogWindow(
    hWndParent, 0, 0, 0, 0,
    RGB(0, 0, 0), RGB(128, 128, 128),
    L"Consolas", 14, 500,
    LW_RENDERMODEBYLINE | LW_TRACK_ZONES | LW_SENDMESSAGE_NOCALLBACKS);
  if (hWndConsole != 0) {
    CSSetUseCallbacks(hWndConsole, FALSE);
    CSSetTrackZones(hWndConsole, TRUE);
    ShowWindow(hWndConsole, SW_SHOW);
    CSPushLine(hWndConsole, L"\x01b[38;$33CC33mConsoul DLL C SDK sample, press:", 0);
    CSPushLine(hWndConsole, L"\x01b[100;70m\x01b[1m[ESC]\x01b[0m\x01b[100;0m to stop the timer", 0);
    CSPushLine(hWndConsole, L"\x01b[100;70m\x01b[1m[SPACE]\x01b[0m\x01b[100;0m to start the timer", 0);
    CSPushLine(hWndConsole, L"\x01b[100;70m\x01b[1m[ENTER]\x01b[0m\x01b[100;0m to toggle between callbacks and parent window messages mechanism", 0);
    CSPushLine(hWndConsole, L"\x01b[100;70m\x01b[1m[TAB]\x01b[0m\x01b[100;0m to save the current visible area of the console in a bitmap file", 0);
    CSPushLine(hWndConsole, L"\x01b[38;$3333AAm\x01b[1mUse the cursor keys and page up/down keys to scroll", 0);
  }
  return hWndConsole;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  WCHAR msg[1024];
  
  switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
 
    case WM_CREATE: {
        mhWndConsole = CreateConsole(hWnd);
        if (mhWndConsole != (HWND)NULL) {
          SetTimer(hWnd, TIMERID_1, 1000L, NULL);
        } else {
          MessageBoxW(hWnd, (PWCHAR)"Failed to create Consoul window", (PWCHAR)"Consoul Error", MB_ICONERROR + MB_OK);
          PostQuitMessage(0);
        }
      }
      break;

    case WM_KEYDOWN: {
        switch (wParam) {
          case VK_ESCAPE:
            KillTimer(hWnd, TIMERID_1);
            CSPushLine(mhWndConsole, L"(Timer stopped)", 1);
            break;
          case VK_SPACE:
            SetTimer(hWnd, TIMERID_1, 1000L, NULL);
            CSPushLine(mhWndConsole, L"(Timer started)", 1);
            break;
          case VK_RETURN:
            ToggleUseCallbacks(mhWndConsole);
            break;
          case VK_TAB:
            SaveConsoulBitmap(L"test.bmp");
            MessageBoxW(hWnd, L"Visible image saved in test.bmp", L"Save console as bitmap", MB_OK+MB_ICONINFORMATION);
            break;
          case VK_UP:
            SendMessage(mhWndConsole, WM_VSCROLL, SB_LINEUP, 0);
            break;
          case VK_DOWN:
            SendMessage(mhWndConsole, WM_VSCROLL, SB_LINEDOWN, 0);
            break;
          case VK_PRIOR:
            SendMessage(mhWndConsole, WM_VSCROLL, SB_PAGEUP, 0);
            break;
          case VK_NEXT:
            SendMessage(mhWndConsole, WM_VSCROLL, SB_PAGEDOWN, 0);
            break;
          default:
            break;
        }
      }
      break;
      
    case WM_SIZE: {
        if (mhWndConsole != (HWND)NULL) {
            UINT ixClient = LOWORD(lParam);
            UINT iyClient = HIWORD(lParam);
            MoveWindow(mhWndConsole, 0, 0, ixClient, iyClient, FALSE);
        }
      }
      break;
    case WM_TIMER: {
        if (mhWndConsole != 0) {
          StringCbPrintfW((PWCHAR)msg, sizeof(msg), L"\x1B[38;$FFFFm %u \x1B[0m Hello \x1B[98;100m World\x1B[99;100m!", ++muiCounter);
          CSPushLine(mhWndConsole, msg, 0);
        }
      }
      break;

    case WM_DESTROY: {
        if (mhWndConsole != 0) {
          CSDestroyLogWindow((HWND)mhWndConsole);
        }
        PostQuitMessage(0);
      }
      break;

    case WM_USER_MOUSEMSG: {
        PTCSMOUSEEVENT pEvtData;
        pEvtData = (PTCSMOUSEEVENT)lParam;
        if(pEvtData->hWnd == mhWndConsole) {
          OnMouseButtonMessage(mhWndConsole, pEvtData, wParam);
        }
      }
      break;

    case WM_USER_ODZONEDRAW: {
        PTCSODZONEDRAW pEvtData;
        pEvtData = (PTCSODZONEDRAW)lParam;
        if (pEvtData->hWnd == mhWndConsole) {
          OnZoneDrawMessage(mhWndConsole, pEvtData);
        }
      }
      break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
