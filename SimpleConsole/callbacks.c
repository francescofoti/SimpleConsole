#include "stdafx.h"
#include "strsafe.h"
#include "SimpleConsole.h"
#include "consoul.h"
#include "callbacks.h"

/*
 * Functions implementing callback/wm_messages responses.
 * Called either from the other function groups in this module
 *  1. callback function (like OnConsoulMouseButton())
 *  2. windows messages delegates (OnConsoul<xxxx>Message() functions).
 * from the console window parent window message loop.
*/

void ReverseZoneText(HWND hWndConsole, UINT wLine, UINT wZoneID) {
  #define MSG_CHARS 1024
  TCHAR msg[MSG_CHARS];

  if (wZoneID == 100) {
    CSGetZoneText(hWndConsole, wLine, wZoneID, MSG_CHARS, msg, 0);
    CSReplaceZone(hWndConsole, wLine, wZoneID, _tcsrev(msg), 1);
    CSRedrawLine(hWndConsole, wLine);
  }
}

void DrawOnZone(HWND hWndConsole, HDC hDC, UINT wLine, UINT wZoneID, LPRECT lprcZone) {
  if (hDC && (wZoneID == 100)) {
    if (hDC) {
      HPEN hPen, hOldPen;
      HBRUSH hBrush, hOldBrush;

      if (wLine % 2) {
        hPen = CreatePen(PS_SOLID, 1, RGB(200, 0, 0));
        hBrush = CreateSolidBrush(RGB(150, 0, 0));
      }
      else {
        hPen = CreatePen(PS_SOLID, 1, RGB(0, 240, 0));
        hBrush = CreateSolidBrush(RGB(0, 210, 0));
      }

      int iWidth, iHeight;

      iWidth     = (lprcZone->bottom - lprcZone->top) / 3;
      iHeight    = (lprcZone->bottom - lprcZone->top) / 3;
      hOldPen   = SelectObject(hDC, hPen);
      hOldBrush = SelectObject(hDC, hBrush);
      Rectangle(hDC,
        lprcZone->left + iWidth / 2,
        lprcZone->top + (CSGetLineHeight(hWndConsole) / 2 - iHeight / 2),
        lprcZone->left + iWidth,
        lprcZone->bottom - (CSGetLineHeight(hWndConsole) / 2 - iHeight / 2));
      SelectObject(hDC, hOldBrush);
      SelectObject(hDC, hOldPen);
      DeleteObject(hBrush);
      DeleteObject(hPen);
    }
  }
}

/*
 * OnConsoul<xxx>
 *  Callback functions for Consoul's callback mechanism.
 * (CSGetUseCallbacks()==1)
*/

UINT STDCALL OnConsoulVirtualLine(HWND hWndConsole, ULONG dwLine) {
  return 0;
}

UINT STDCALL OnConsoulZonePaint(HWND hWndConsole, HDC hDC, UINT wZoneID, ULONG wLine, UINT wLeft, UINT wTop, UINT wRight, UINT wBottom) {
  RECT rcZone;
  SetRect(&rcZone, wLeft, wTop, wRight, wBottom);
  DrawOnZone(hWndConsole, hDC, wLine, wZoneID, &rcZone);
  return 0;
}

UINT STDCALL OnConsoulWmPaint(HWND hWndConsole, UINT wCbkMode, HDC hDC, const LPRECT lprcLinePos, const LPRECT lprcLineRect, const LPRECT lprcPaint) {
  if (wCbkMode != WMPAINTCBK_AFTER) {
    return 0;
  }

  RECT rcPaint, rcLinePos, rcLineRect;
  HBRUSH hOldBrush, hBrush;
  HPEN hDotPen, hOldPen;

  CopyRect(&rcPaint, lprcPaint);
  CopyRect(&rcLinePos, lprcLinePos);
  CopyRect(&rcLineRect, lprcLineRect);

  hBrush = GetStockObject(NULL_BRUSH);
  hOldBrush = SelectObject(hDC, hBrush);
  hDotPen = CreatePen(PS_DOT, 1, RGB(60, 60, 60));
  hOldPen = SelectObject(hDC, hDotPen);

  INT iCol, x, iCharHeight, iCharWidth;

  iCharHeight = CSGetLineHeight(hWndConsole);
  iCharWidth = CSGetCharWidth(hWndConsole);

  for(iCol=0; iCol<(rcLinePos.right - rcLinePos.left); iCol++) {
    x = rcLineRect.left + (iCol * iCharWidth);
    MoveToEx(hDC, x, rcPaint.top, 0);
    LineTo(hDC, x, rcPaint.bottom);
  }

  INT iRow, y, iCount=0;

  if(iCharHeight > 0) {
    iCount = (rcLineRect.bottom - rcLineRect.top) / iCharHeight;
  }
  for(iRow = 0; iRow <= iCount; iRow++) {
    y = rcLineRect.top + (iRow * iCharHeight);
    MoveToEx(hDC, rcPaint.left, y, 0);
    LineTo(hDC, rcPaint.right, y);
  }

  SelectObject(hDC, hOldBrush);
  SelectObject(hDC, hOldPen);
  DeleteObject(hDotPen);

  return 0;
}

UINT STDCALL OnConsoulMouseButton(HWND hWndConsole, UINT wEvtCode, UINT wParam, UINT wZoneID, UINT wLine, UINT wCol, UINT wPosX, UINT wPosY) {
  WCHAR buf[255];

  switch (wEvtCode) {
    case WM_LBUTTONDOWN: {
      swprintf(buf, sizeof(buf) / sizeof(*buf),
        L"OnConsoulMouseButton(wEvtCode=%u,wParam=%u,wZoneID=%u,wLine=%u,wCol=%u,wPosX=%u,wPosY=%u",
        wEvtCode, wParam, wZoneID, wLine, wCol, wPosX, wPosY);
      CSPushLine(hWndConsole, buf, 0);
      ReverseZoneText(hWndConsole, wLine, wZoneID);
    }
    break;
  default:
    break;
  }
  return 0;
}

/*
 * On<xxx>Message
 * Function that are called when handling consoul WM_ messages
 * from the console window parent window message loop.
 * (CSGetUseCallbacks()==0)
*/

/* Handles WM_USER_MOUSEMSG message payload */
void OnMouseButtonMessage(HWND hWndConsole, PTCSMOUSEEVENT pEvtData, WPARAM wParam)
{
  WCHAR buf[255];

  switch (wParam) {
  case WM_LBUTTONDOWN: {
      swprintf(buf, sizeof(buf) / sizeof(*buf),
        L"OnMouseButtonMessage(wParam=%u,wZoneID=%u,wLine=%u,wCol=%u,wPosX=%u,wPosY=%u",
        wParam, pEvtData->wZoneID, pEvtData->wLine, pEvtData->wCol, pEvtData->position.x, pEvtData->position.y);
      CSPushLine(hWndConsole, buf, 0);
      ReverseZoneText(hWndConsole, pEvtData->wLine, pEvtData->wZoneID);
    }
    break;
  default:
    break;
  }
}

/* Handles WM_USER_ODZONEDRAW message payload */
void OnZoneDrawMessage(HWND hWndConsole, PTCSODZONEDRAW pEvtData) {
  DrawOnZone(hWndConsole, pEvtData->hDC, pEvtData->wLine, pEvtData->wZoneID, &pEvtData->rcUpdate);
}

