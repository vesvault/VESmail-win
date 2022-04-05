#include "stdafx.h"
#include "VESmail-win.h"
#include <shellapi.h>
#include <CommCtrl.h>
#include "vmUsers.h"


struct {
	int mouseY;
	char scroll;
	char scrolled;
	char uHoverRsp;
	HWND uHover;
} vmUsers;

LRESULT CALLBACK vmUsersWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int r;
	void *dp;

	switch (message)
	{
	case WM_ERASEBKGND:
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		if (vmUsers.scrolled) {
			RECT rect;
			GetClientRect(hWnd, &rect);
			FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW));
			vmUsers.scrolled = 0;
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_SETCURSOR: {
		HWND uh = (HWND)wParam;
		if (uh != vmUsers.uHover) vmUsers.uHoverRsp = SendMessage(GetParent(hWnd), VMW_MSG_UHOVER, (WPARAM)((vmUsers.uHover = uh)), 0);
		return vmUsers.uHoverRsp || DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_MOUSEWHEEL:
		vmUsers.scrolled = 1;
		SendMessage(GetParent(hWnd), VMW_MSG_USCROLL, ((signed short) HIWORD(wParam)) / 8, 0);
		break;
	case WM_LBUTTONDOWN:
		vmUsers.scroll = 1;
		vmUsers.mouseY = HIWORD(lParam);
		break;
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON) {
			if (vmUsers.scroll) {
				int y = vmUsers.mouseY;
				vmUsers.mouseY = HIWORD(lParam);
				vmUsers.scrolled = 1;
				SendMessage(GetParent(hWnd), VMW_MSG_USCROLL, y - vmUsers.mouseY, 0);
			}
		}
		else {
			vmUsers.scroll = 0;
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) < VMW_ID_BTN_PROF) break;
	case WM_CTLCOLORSTATIC:
		return SendMessage(GetParent(hWnd), message, wParam, lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM vmUsersRegisterClass() {
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_PARENTDC;
	wcex.lpfnWndProc = vmUsersWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = vmInst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = VMW_USERS_CLASS;
	wcex.hIconSm = NULL;
	return RegisterClassExW(&wcex);

}

HWND vmUsersCreate(HWND hWnd, int x, int y, int w, int h) {
	vmUsers.scroll = vmUsers.scrolled = 0;
	vmUsers.uHover = NULL;
	vmUsersRegisterClass();
	return CreateWindow(VMW_USERS_CLASS, NULL, WS_VISIBLE | WS_CHILD /*| WS_VSCROLL*/, x, y, w, h, hWnd, NULL, vmInst, NULL);
}
