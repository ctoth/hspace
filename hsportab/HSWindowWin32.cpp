/**
 *
 * Hemlock Space 5 (HSpace 5)
 * Copyright (c) 2009, Bas Schouten and Shawn Sagady
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 
 *    * Redistribution in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the HSpace 5 Development Team nor the names
 *      of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS  SOFTWARE IS PROVIDED BY THE HSPACE DEVELOPMENT TEAM AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FINTESS FOR A PARTICULAR
 * PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  COPYRIGHT  OWNERS OR
 * CONTRIBUTORS  BE  LIABLE  FOR  ANY  DIRECT,  INDIRECT, INCIDENTAL, SPECIAL
 * EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED TO,
 * PRODUCEMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR  BUSINESS  INTERUPTION)  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER  IN  CONTRACT,  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)  ARISING  IN  ANY  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Original author(s):
 *   Bas Schouten
 *
 */
#include "HSWindow.h"
#include <windows.h>

#include <vector>
#include <map>

std::map<HWND, HSWindow*> sWindows;

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
    if (sWindows[hWnd]) {
      sWindows[hWnd]->MenuItemClicked(wmId);
    }
  	return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
    if (sWindows[hWnd]) {
      sWindows[hWnd]->OnPaint();
    }
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
  case WM_SIZE:
    if (sWindows[hWnd]) {
      sWindows[hWnd]->OnResize();
    }
    break;
  case WM_CHAR:
    // TODO: Obviously horrible hack, like so much of UI stuff in portab fix this.
    if (wParam == '+') {
      sWindows[hWnd]->OnKeyPress(HSKC_NUMPLUS);
    } else if (wParam == '-') {
      sWindows[hWnd]->OnKeyPress(HSKC_NUMMINUS);
    }
    break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	return 0;
}
HSWindow::HSWindow(HSWindow *aWindow)
{
  p = new HSWindowPriv();
  p->mInstance = ::GetModuleHandle(0);
  p->mParent = aWindow;
  p->mMenu = NULL;
  p->mWnd = NULL;
  p->mName = "Unnamed Window";
  p->mAlwaysOnTop = false;
}

HSWindow::~HSWindow(void)
{
  ::DestroyWindow(p->mWnd);
  delete p;
}

void
HSWindow::Show()
{
  if (!p->mWnd) {
    if (!p->mParent) {
      WNDCLASSEX wcex;
      memset(&wcex, 0, sizeof(WNDCLASSEX));
      wcex.cbSize = sizeof(WNDCLASSEX);

      wcex.style = CS_HREDRAW | CS_VREDRAW;
      wcex.lpszClassName = "TestClass";
      wcex.hInstance = p->mInstance;
      wcex.lpfnWndProc = WndProc;
      wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
      ATOM atom = RegisterClassEx(&wcex);
      wcex.lpszClassName = "TestClass2";
      atom = RegisterClassEx(&wcex);
    }
    
    CreateWin();

    if (!p->mWnd)
    {
      return;
    }
    sWindows[p->mWnd] = this;
    
    if (p->mMenu) {
      ::SetMenu(p->mWnd, p->mMenu);
    }
  }

  ShowWindow(p->mWnd, SW_SHOW);
  UpdateWindow(p->mWnd);
  OnLoad();
}

void
HSWindow::Hide()
{
  ::ShowWindow(p->mWnd, SW_HIDE);
}

void
HSWindow::CreateWin()
{
  if (p->mParent) {
    p->mWnd = CreateWindowEx(0, "TestClass", p->mName.c_str(), WS_CAPTION,
      0, 0, 50, 50, (HWND)p->mParent->Handle(), NULL, p->mInstance, NULL);
  } else {
    p->mWnd = CreateWindow("TestClass", p->mName.c_str(), WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, p->mInstance, NULL);
  }
}

void
HSWindow::SetRect(const HSRect &aRect)
{
  if (p->mWnd) {
    if (p->mParent) {
      ::SetWindowPos(p->mWnd, HWND_TOP, aRect.left, aRect.top,
        aRect.right - aRect.left, aRect.bottom - aRect.top,
        SWP_NOZORDER);
    } else {
      ::SetWindowPos(p->mWnd, HWND_TOP, aRect.left, aRect.top,
        aRect.right - aRect.left, aRect.bottom - aRect.top,
        SWP_NOZORDER);
    }
  }
}

void
HSWindow::SetName(const std::string &aName)
{
  ::SetWindowText(p->mWnd, aName.c_str());
}

void
HSWindow::SetAlwaysOnTop(bool aAlwaysOnTop)
{
  if (p->mWnd) {
    RECT rect;
    ::GetWindowRect(p->mWnd, &rect);
    if (aAlwaysOnTop) {
      ::SetWindowPos(p->mWnd, HWND_TOPMOST, 0, 0, 
        0, 0, SWP_NOSIZE | SWP_NOMOVE);
    } else {
      ::SetWindowPos(p->mWnd, HWND_NOTOPMOST, 0, 0, 0, 0, 
        SWP_NOSIZE | SWP_NOMOVE);
    }
  }
  p->mAlwaysOnTop = aAlwaysOnTop;
}

bool
HSWindow::GetAlwaysOnTop()
{
  return p->mAlwaysOnTop;
}

void*
HSWindow::Handle()
{
  return p->mWnd;
}

void*
HSWindow::AddMenu(const char *aText)
{
  if (!p->mMenu) {
    p->mMenu = ::CreateMenu();
    if (p->mWnd) {
      ::SetMenu(p->mWnd, p->mMenu);
    }
  }
  HMENU menu = ::CreatePopupMenu();
  ::AppendMenu(p->mMenu, MF_STRING | MF_POPUP, (ULONG_PTR)menu, aText);
  return (void*)menu;
}

void
HSWindow::AddMenuItem(void* aMenu, const char *aText, int aID)
{
  if (aText[0] == '-') {
    ::AppendMenu((HMENU)aMenu, MF_SEPARATOR, 0, NULL);
  } else {
    ::AppendMenu((HMENU)aMenu, MF_STRING, aID, aText);
  }
}

void
HSWindow::SetMenuItemChecked(void* aMenu, int aID, bool aChecked)
{
  ::CheckMenuItem((HMENU)aMenu, aID, aChecked ? MF_CHECKED : MF_UNCHECKED);
}

void
HSWindow::OnLoad()
{
}

void
HSWindow::OnPaint()
{
}

void
HSWindow::OnResize()
{
}
