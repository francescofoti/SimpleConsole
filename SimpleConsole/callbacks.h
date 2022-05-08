#pragma once

UINT STDCALL OnConsoulMouseButton(HWND hWndConsole, UINT wEvtCode, UINT wParam, UINT wZoneID, UINT wLine, UINT wCol, UINT wPosX, UINT wPosY);
UINT STDCALL OnConsoulVirtualLine(HWND hWnd, ULONG dwLine); // warning, know issue, should be UINT wLine
UINT STDCALL OnConsoulZonePaint(HWND hWnd, HDC hDC, UINT wZoneID, ULONG dwLine, UINT wLeft, UINT wTop, UINT wRight, UINT wBottom);
UINT STDCALL OnConsoulWmPaint(HWND hWndConsole, UINT wCbkMode, HDC hDC, const LPRECT lprcLinePos, const LPRECT lprcLineRect, const LPRECT lprcPaint);

void OnMouseButtonMessage(HWND hWndConsole, PTCSMOUSEEVENT pEvtData, WPARAM wParam);
void OnZoneDrawMessage(HWND hWndConsole, PTCSODZONEDRAW pEvtData);
