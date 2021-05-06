
#include "stdafx.h"
#include "VESmail-win.h"
#include <shellapi.h>
#include <CommCtrl.h>
#include "vmUsers.h"
#include "vmSpinner.h"

extern "C" {
#include "../../../srv/local.h"
#include "../../../srv/tls.h"
#include "../../../srv/daemon.h"
}


#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define VMW_SCALE(x)	((x) * vmapp.scale)
#define VMW_LOADSTW(str, id)	LoadStringW(vmInst, id, str, sizeof(str) / sizeof(*str))

HINSTANCE vmInst;
WCHAR vmTitle[80];

struct vmapp_st {
	struct {
		HWND logo;
		HWND btn;
		HWND userSt;
		HWND userList;
		HWND userWait;
//		HWND uHover;
	} hwnd;
	struct {
		HFONT prompt;
		HFONT value;
	} hfont;
	struct {
		HCURSOR arrow;
		HCURSOR hand;
	} hcursor;
	struct {
		VESmail_daemon *all[7];
		VESmail_daemon *error;
	} daemon;
	struct user {
		struct user *chain;
		const char *user;
		int stat;
		int error;
		struct {
			HWND stat;
			HWND login;
			HWND profile;
		} hwnd;
	} *users;
	struct user *uHover;
	HICON st_icons[4];
	struct {
		HICON n;
		HICON h;
		HICON e;
	} pr_icon;
	HICON main_icon;
	struct {
		WCHAR btn[40];
		WCHAR user_st0[80];
		WCHAR user_st[80];
		WCHAR fail[128];
	} txt;
	char scale;
	char blink;
	int uwidth;
	int uheight;
	int uscroll;
} vmapp;

PWCHAR vm_2wchar(PWCHAR d, const char *s) {
	PWCHAR p = d;
	while ((*p++ = *s++));
	return d;
}

void vmInit(HWND hWnd) {
	vmapp.blink = 0;
	vmapp.uwidth = VMW_SCALE(VMW_USER_W);
	int logow = VMW_SCALE(32);
	vmapp.hfont.prompt = CreateFont(VMW_SCALE(10), 0, 0, 0, 100, 1, 0, 0, DEFAULT_CHARSET, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Verdana");
	vmapp.hfont.value = CreateFont(VMW_SCALE(10), 0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Verdana");

	int lw = VMW_SCALE(32);
	vmapp.hwnd.logo = CreateWindow(L"STATIC",
		vmapp.txt.btn,
		WS_VISIBLE | WS_TABSTOP | WS_CHILD | SS_ICON,
		VMW_SCALE(VMW_PAD_L + VMW_USER_W / 2) - lw / 2,
		VMW_SCALE(VMW_PAD_T + VMW_LOGO_Y),
		lw,
		lw,
		hWnd, NULL, vmInst, NULL);
	HICON logo;
	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCE(IDI_ICON1), lw, lw, &logo);
	SendMessage(vmapp.hwnd.logo, STM_SETICON, (WPARAM)logo, NULL);

	vmapp.hwnd.btn = CreateWindow(L"BUTTON",
		vmapp.txt.btn,
		WS_VISIBLE | WS_TABSTOP | WS_CHILD | BS_PUSHBUTTON,
		VMW_SCALE(VMW_PAD_L + (VMW_USER_W - VMW_BTN_W) / 2),
		VMW_SCALE(VMW_PAD_T + VMW_BTN_Y),
		VMW_SCALE(VMW_BTN_W),
		VMW_SCALE(VMW_BTN_H),
		hWnd, (HMENU) VMW_ID_BTN_PROF, vmInst, NULL);

	vmapp.st_icons[0] = NULL;
	int icw = VMW_SCALE(8);
	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCEW(IDI_LED_C), icw, icw, vmapp.st_icons + 1);
	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCEW(IDI_LED_A), icw, icw, vmapp.st_icons + 2);
	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCEW(IDI_LED_E), icw, icw, vmapp.st_icons + 3);

	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCEW(IDI_SETTINGS), icw, icw, &vmapp.pr_icon.n);
	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCEW(IDI_SETTINGS_H), icw, icw, &vmapp.pr_icon.h);
	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCEW(IDI_SETTINGS_E), icw, icw, &vmapp.pr_icon.e);

	vmapp.daemon.all[0] = vmapp.daemon.error = NULL;
	VESmail_daemon **dp = VESmail_local_daemons;
	int i;
	for (i = 0; *dp; dp++, i++) {
		VESmail_daemon *d = *dp;
		if (!strcmp(d->type, "now")) continue;
		VESmail_daemon **adp;
		for (adp = vmapp.daemon.all; ; adp++) {
			if (!*adp) {
				if (adp < vmapp.daemon.all + sizeof(vmapp.daemon.all) / sizeof(*vmapp.daemon.all) - 1) *adp++ = d;
				*adp = NULL;
				break;
			}
			if (!strcmp((*adp)->type, d->type)) break;
		}
	}

	vmapp.hwnd.userSt = CreateWindow(L"STATIC",
		vmapp.txt.user_st0,
		WS_VISIBLE | WS_CHILD | SS_ENDELLIPSIS,
		VMW_SCALE(VMW_PAD_L + VMW_USER_ST_X),
		VMW_SCALE(VMW_PAD_T + VMW_USER_ST_Y),
		VMW_SCALE(VMW_USER_W - VMW_USER_ST_X - VMW_USER_ST_R),
		VMW_SCALE(VMW_USER_ST_H),
		hWnd, NULL, vmInst, NULL);
	SendMessageA(vmapp.hwnd.userSt, WM_SETFONT, (WPARAM)vmapp.hfont.prompt, NULL);
	vmapp.hwnd.userList = vmUsersCreate(hWnd,
		VMW_SCALE(VMW_PAD_L),
		VMW_SCALE(VMW_PAD_T + VMW_USER_ST_Y + VMW_USER_ST_H),
		VMW_SCALE(VMW_USER_W),
		VMW_SCALE(VMW_USER_H));

	vmapp.hwnd.userWait = vmSpinnerCreate(vmapp.hwnd.userList, VMW_SCALE(VMW_USER_W) / 2, VMW_SCALE(VMW_USER_H) / 2, icw, icw);

	vmapp.uHover = NULL;
	vmapp.uheight = VMW_SCALE(VMW_USER_PAD_T + VMW_USER_PAD_B);
	vmapp.uscroll = 0;
}

int vmSize(HWND hWnd) {
	if (IsIconic(hWnd)) return 0;
	RECT rect;
	GetClientRect(hWnd, &rect);
	int w = rect.right - rect.left - VMW_SCALE(VMW_PAD_L + VMW_PAD_R);
	int h = rect.bottom - rect.top - VMW_SCALE(VMW_PAD_T + VMW_PAD_B);
	int uw = w;
	vmapp.uwidth = uw;
	int bw = VMW_SCALE(VMW_BTN_W);
	if (bw > uw) bw = uw;
	MoveWindow(vmapp.hwnd.btn,
		(uw - bw) / 2 + VMW_SCALE(VMW_PAD_L),
		VMW_SCALE(VMW_PAD_T + VMW_BTN_Y),
		bw,
		VMW_SCALE(VMW_BTN_H),
		0);
	MoveWindow(vmapp.hwnd.userSt,
		VMW_SCALE(VMW_PAD_L + VMW_USER_ST_X),
		VMW_SCALE(VMW_PAD_T + VMW_USER_ST_Y),
		uw - VMW_SCALE(VMW_USER_ST_X + VMW_USER_ST_R),
		VMW_SCALE(VMW_USER_ST_H),
		0);
	int uh = h - VMW_SCALE(VMW_USER_ST_Y + VMW_USER_ST_H);
	MoveWindow(vmapp.hwnd.userList,
		VMW_SCALE(VMW_PAD_L),
		VMW_SCALE(VMW_PAD_T + VMW_USER_ST_Y + VMW_USER_ST_H),
		uw,
		uh,
		0);
	if (vmapp.uscroll + uh > vmapp.uheight) vmapp.uscroll = vmapp.uheight > uh ? vmapp.uheight - uh : 0;
	struct vmapp_st::user *vu;
	int uidx = 0;
	int icw = VMW_SCALE(8);
	for (vu = vmapp.users; vu; vu = vu->chain, uidx++) {
		int y = VMW_SCALE(VMW_USER_PAD_T + VMW_USER_LOGIN_H * uidx) - vmapp.uscroll;
		MoveWindow(vu->hwnd.stat,
			VMW_SCALE(VMW_USER_STAT_X),
			y + VMW_SCALE(VMW_USER_STAT_Y),
			icw,
			icw,
			0);
		MoveWindow(vu->hwnd.login,
			VMW_SCALE(VMW_USER_LOGIN_X),
			y,
			uw - VMW_SCALE(VMW_USER_LOGIN_X + VMW_USER_LOGIN_R),
			VMW_SCALE(VMW_USER_LOGIN_H),
			0);
		MoveWindow(vu->hwnd.profile,
			uw + VMW_SCALE(VMW_USER_PROF_DX),
			y + VMW_SCALE(VMW_USER_PROF_Y),
			icw,
			icw,
			0);
	}
	if (vmapp.hwnd.userWait) vmSpinnerMove(vmapp.hwnd.userWait, uw / 2, uh / 2, VMW_SCALE(8), VMW_SCALE(8));
	return 1;
}

int vmSetBullet(HWND hWnd, int st, int blink) {
	int b;
	int rs = 0;
	if (st >= 0 && !(st & 0x0020)) {
		if ((st & 0x00cc) != 0) {
			if (blink) b = 2;
			else b = 1;
			rs = blink;
		} else if (st & 0x0010) {
			b = 2;
		} else if (st & 0x0001) {
			b = 1;
		} else if (st & 0x0002) {
			b = 3;
		} else {
			b = 0;
		}
	} else b = 3;
	HICON ic = vmapp.st_icons[b];
	if (((HICON) SendMessageA(hWnd, STM_GETICON, 0, 0)) != ic)
		SendMessageA(hWnd, STM_SETICON, (WPARAM) ic, NULL);
	return rs;
}

VESmail_daemon *vmChkError(VESmail_daemon *d) {
	const char *e = NULL;
	VESmail_daemon **pd;
	int i = 0;
	if (d) for (pd = VESmail_local_daemons; *pd; pd++, i++) if (*pd == d) {
		if (!(VESmail_local_getstat(i) & 0x0002)) d = NULL;
		break;
	}
	if (d != vmapp.daemon.error) {
		vmapp.daemon.error = d;
		if (d) {
			WCHAR fmt[128];
			VMW_LOADSTW(fmt, IDS_FAIL);
			wsprintfW(vmapp.txt.fail, fmt, VESmail_local_getport(d));
			SendMessageW(vmapp.hwnd.userSt, WM_SETTEXT, NULL, (LPARAM) vmapp.txt.fail);
		}
		else {
			SendMessageW(vmapp.hwnd.userSt, WM_SETTEXT, NULL, (LPARAM) vmapp.txt.user_st);
		}
	}
	return d;
}

int vmShowStat(HWND hWnd, int blink) {
	int rs = 0;
	if (blink) {
		VESmail_daemon **dp;
		for (dp = vmapp.daemon.all; ; dp++) {
			if (vmChkError(*dp) || !*dp) break;
		}
	}
	const char *usr = NULL;
	struct vmapp_st::user **pvu = &vmapp.users;
	int uidx = 0;
	while (1) {
		int st;
		VESmail_local_getuser(&usr, (blink ? &st : NULL));
		if (!usr) break;
		struct vmapp_st::user *vu = *pvu;
		if (!vu) {
			if (!vmapp.users) {
				if (!vmapp.daemon.error) SendMessageW(vmapp.hwnd.userSt, WM_SETTEXT, NULL, (LPARAM)vmapp.txt.user_st);
				if (vmapp.hwnd.userWait) {
					vmSpinnerDestroy(vmapp.hwnd.userWait);
					vmapp.hwnd.userWait = NULL;
				}
			}
			*pvu = vu = (struct vmapp_st::user *) malloc(sizeof(*vu));
			vu->user = usr;
			vu->chain = NULL;
			int y = VMW_SCALE(VMW_USER_PAD_T + VMW_USER_LOGIN_H * uidx) - vmapp.uscroll;
			PWCHAR buf = (PWCHAR) malloc(sizeof(*buf) * (strlen(usr) + 1));
			vu->hwnd.login = CreateWindow(L"STATIC",
				vm_2wchar(buf, usr),
				WS_VISIBLE | WS_CHILD | SS_ENDELLIPSIS,
				VMW_SCALE(VMW_USER_LOGIN_X),
				y,
				vmapp.uwidth - VMW_SCALE(VMW_USER_LOGIN_R + VMW_USER_LOGIN_X),
				VMW_SCALE(VMW_USER_LOGIN_H),
				vmapp.hwnd.userList, NULL, vmInst, NULL);
			SendMessageA(vu->hwnd.login, WM_SETFONT, (WPARAM)vmapp.hfont.value, NULL);
			free(buf);
			int icw = VMW_SCALE(8);
			vu->hwnd.stat = CreateWindow(L"STATIC",
				NULL,
				SS_ICON | WS_VISIBLE | WS_CHILD,
				VMW_SCALE(VMW_USER_STAT_X),
				y + VMW_SCALE(VMW_USER_STAT_Y),
				icw,
				icw,
				vmapp.hwnd.userList, NULL, vmInst, NULL);
			vu->hwnd.profile = CreateWindow(L"STATIC",
				NULL,
				SS_ICON | SS_NOTIFY | WS_TABSTOP | WS_VISIBLE | WS_CHILD,
				vmapp.uwidth + VMW_SCALE(VMW_USER_PROF_DX),
				y + VMW_SCALE(VMW_USER_PROF_Y),
				icw,
				icw,
				vmapp.hwnd.userList, (HMENU)(VMW_ID_BTN_UPROF + uidx), vmInst, NULL);
			SendMessageA(vu->hwnd.profile, STM_SETICON, (WPARAM)vmapp.pr_icon.n, NULL);
			vu->stat = vu->error = 0;
			vmapp.uheight += VMW_SCALE(VMW_USER_LOGIN_H);
		}
		if (blink) vu->stat = st;
		if (vmSetBullet(vu->hwnd.stat, vu->stat, blink)) rs = 1;
		int e = vu->stat & 0x0002 ? VESmail_local_getusererror(usr, NULL) : 0;
		if (e != vu->error) {
			vu->error = e;
			if (vu != vmapp.uHover) SendMessage(vu->hwnd.profile, STM_SETICON, (WPARAM)(e ? vmapp.pr_icon.e : vmapp.pr_icon.n) , NULL);
		}
		pvu = &vu->chain;
		uidx++;
	}
	vmapp.blink &= ~(VMWF_BLINK | VMWF_SKIP);
	return rs;
}

void vmWatch(HWND hWnd) {
	int r = VESmail_local_watch();
	if (r <= 0) {
		PostQuitMessage(0);
		return;
	}
	int blink = vmShowStat(hWnd, 1);
	if (vmapp.blink & VMWF_ACTIVE) {
		vmapp.blink |= (blink ? VMWF_BLINK : VMWF_SKIP);
	}
}

void vmSetWatch(HWND hWnd, int active) {
	if (active) {
		if (!(vmapp.blink & VMWF_ACTIVE)) {
			if (SetTimer(hWnd, VMW_ID_TIMER, 125, NULL)) {
				vmapp.blink |= VMWF_ACTIVE;
			}
		}
	}
	else {
		vmapp.blink &= ~VMWF_ACTIVE;
		SetTimer(hWnd, VMW_ID_TIMER, 2500, NULL);
	}
}

void vmOpenProfile(int uidx, HWND hWnd) {
	const char *prf = NULL;
	if (uidx >= 0) {
		struct vmapp_st::user *vu = vmapp.users;
		int i;
		for (i = 0; i < uidx && vu; i++) vu = vu->chain;
		if (vu) prf = vu->user;
	}
	const char *pu = VESmail_local_getuserprofileurl(prf);
	if (!pu || strncmp(pu, "https://", 8)) {
		pu = "https://my.vesmail.email/profile";
	}
	int l = strlen(pu);
	char *buf = (char *) malloc(l + (prf ? 3 * strlen(prf) + 64 : 128));
	char *d = buf;
	memcpy(buf, pu, l);
	d += l;
	*d++ = strchr(pu, '?') ? '&' : '?';
	memcpy(d, "local=1", 7);
	d += 7;
	if (prf || vmapp.users) {
		*d++ = '&';
		*d++ = 'p';
		*d++ = '=';
		if (prf) {
			const char hex[] = "0123456789ABCDEF";
			const char *s = prf;
			unsigned char c;
			while ((c = *s++)) {
				if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '-' || c == '.' || c == '@') {
					*d++ = c;
				} else {
					*d++ = '%';
					*d++ = hex[c >> 4];
					*d++ = hex[c & 0x0f];
				}
			}
		}
	}
	*d = VESmail_local_getusererror(prf, d + 1) ? '#' : 0;
	ShellExecuteA(hWnd, "open", buf, NULL, NULL, SW_SHOWNORMAL);
	free(buf);
}


BOOL vmShellIcon(HWND hwnd)
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
	nid.guidItem = __uuidof(ICON_SMALL);
	nid.uCallbackMessage = VMW_MSG_NOTIFY;
	LoadIconMetric(vmInst, MAKEINTRESOURCEW(IDI_ICON1), LIM_SMALL, &nid.hIcon);
	LoadString(vmInst, IDS_TRAY, nid.szTip, ARRAYSIZE(nid.szTip));
	Shell_NotifyIcon(NIM_ADD, &nid);
	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

LRESULT CALLBACK vmWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int r;
	void *dp;
	
    switch (message)
    {
	case VMW_MSG_NOTIFY:
		switch (lParam) {
		case NIN_SELECT: case NIN_KEYSELECT:
			ShowWindow(hWnd, SW_RESTORE);
			SetForegroundWindow(hWnd);
		default:
			break;
		}
		break;
	case VMW_MSG_SIGNAL:
		VESmail_daemon_SIG = lParam;
		break;
	case WM_SIZE:
		vmSetWatch(hWnd, vmSize(hWnd));
		break;
	case WM_SHOWWINDOW:
		vmSetWatch(hWnd, wParam);
		break;
	case WM_TIMER:
		if (vmapp.hwnd.userWait) vmSpinnerStep(vmapp.hwnd.userWait);
		if (vmapp.blink & VMWF_BLINK) {
			vmShowStat(hWnd, 0);
		}
		else if (vmapp.blink & VMWF_SKIP) {
			vmapp.blink &= ~VMWF_SKIP;
		} else {
			vmWatch(hWnd);
		}
		break;
	case VMW_MSG_USCROLL: {
		int y0 = vmapp.uscroll;
		vmapp.uscroll += (signed)wParam;
		if (vmapp.uscroll < 0) vmapp.uscroll = 0;
		vmSize(hWnd);
		if (vmapp.uscroll != y0) InvalidateRect(vmapp.hwnd.userList, NULL, 1);
		break;
	}
	case VMW_MSG_UHOVER: {
		HWND uh = (HWND)wParam;
		struct vmapp_st::user *vu;
		if (uh) for (vu = vmapp.users; vu ; vu = vu->chain) {
			if (uh == vu->hwnd.profile) break;
		}
		else vu = NULL;
		if (vmapp.uHover) {
			InvalidateRect(vmapp.uHover->hwnd.login, NULL, TRUE);
			SendMessage(vmapp.uHover->hwnd.profile, STM_SETICON, (WPARAM)(vmapp.uHover->error ? vmapp.pr_icon.e : vmapp.pr_icon.n), NULL);
		}
		if (vu) {
			InvalidateRect(vu->hwnd.login, NULL, TRUE);
			SendMessage(uh, STM_SETICON, (WPARAM)vmapp.pr_icon.h, NULL);
		}
		vmapp.uHover = vu;
		if (vu) return SetCursor(vmapp.hcursor.hand), TRUE;
		break;
	}
	case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			if (wmId >= VMW_ID_BTN_PROF) vmOpenProfile(wmId - VMW_ID_BTN_UPROF, hWnd);
        }
        break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_CTLCOLORSTATIC:
		if (vmapp.daemon.error && (HWND)lParam == vmapp.hwnd.userSt) {
			SetTextColor((HDC)wParam, RGB(192, 0, 0));
			SetBkColor((HDC)wParam, GetSysColor(COLOR_MENU));
			return (COLOR_MENU + 1);
		}
		else if (vmapp.uHover && vmapp.uHover->hwnd.login == (HWND)lParam) {
			SetTextColor((HDC)wParam, RGB(0, 0, 192));
			SetBkColor((HDC)wParam, GetSysColor(COLOR_MENU));
			return (COLOR_MENU + 1);
		}
	default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ATOM vmRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = vmWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = vmapp.hcursor.arrow = LoadCursor(nullptr, IDC_ARROW);
	vmapp.hcursor.hand = LoadCursor(nullptr, IDC_HAND);
	wcex.hbrBackground = (HBRUSH)(COLOR_MENU + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = VMW_CLASS;
	return RegisterClassExW(&wcex);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	char cmd[MAX_PATH];
	GetModuleFileNameA(NULL, cmd, MAX_PATH);

	HWND hwCur = FindWindowW(VMW_CLASS, NULL);

	if (lpCmdLine) {
		if (!lstrcmpW(lpCmdLine, L"/Install")) {
			int r = WinExec(cmd, nCmdShow);
			return r > 31 ? 0 : r;
		}
		else if (!lstrcmpW(lpCmdLine, L"/Uninstall") || !lstrcmpW(lpCmdLine, L"/Kill")) {
			if (hwCur) SendMessage(hwCur, VMW_MSG_SIGNAL, 0, 15);
			return 0;
		}
		else if (!lstrcmpW(lpCmdLine, L"/Shutdown")) {
			if (hwCur) SendMessage(hwCur, VMW_MSG_SIGNAL, 0, 1);
			return 0;
		}
		else if (!lstrcmpW(lpCmdLine, L"/Hidden")) {
			nCmdShow = SW_HIDE;
		}
	}

	if (hwCur) {
		SendMessage(hwCur, VMW_MSG_NOTIFY, 0, NIN_SELECT);
		return 0;
	}

	VESmail_local_init();
	vmInst = hInstance;

	VMW_LOADSTW(vmTitle, IDS_APP_TITLE);
	VMW_LOADSTW(vmapp.txt.btn, IDS_BTN);
	VMW_LOADSTW(vmapp.txt.user_st, IDS_ST_USERS);
	VMW_LOADSTW(vmapp.txt.user_st0, IDS_ST_USERS0);

	vmRegisterClass(hInstance);

	char *p = cmd;
	char *e = p;
	char c;
	while ((c = *p)) switch (c) {
	case '/': case '\\':
		e = p;
	default:
		p++;
	}
	*e = 0;
	SetCurrentDirectoryA(cmd);

	VESmail_tls_caBundle = (char *) "curl-ca-bundle.crt";

	vmapp.scale = GetDpiForSystem() / 48;
	if (vmapp.scale < 2) vmapp.scale = 2;

	RECT rect;
	rect.left = rect.top = 0;
	rect.right = VMW_SCALE(VMW_PAD_L + VMW_USER_W + VMW_PAD_R);
	rect.bottom = VMW_SCALE(VMW_PAD_T + VMW_USER_ST_Y + VMW_USER_ST_H + VMW_USER_H + VMW_PAD_B);
	DWORD flgs = WS_OVERLAPPEDWINDOW;
	AdjustWindowRect(&rect, flgs, 0);

	HWND hWnd = CreateWindowW(VMW_CLASS, vmTitle, flgs,
		CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr, hInstance, nullptr);


	if (!hWnd) return FALSE;

	VESmail_local_start();

	vmInit(hWnd);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	vmShellIcon(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VESMAILWIN));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}
