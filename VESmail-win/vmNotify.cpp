
#include "stdafx.h"
#include "VESmail-win.h"
#include <shellapi.h>
#include <CommCtrl.h>
#include <strsafe.h>
#include "vmNotify.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

struct vmNotify_st {
	const char *err;
	WORD msg1;
	WORD msg2;
	int flags;
} vmNotify[] = {
	{"XVES-3.7", IDS_XVES_LOCAL, IDS_XVES_3_7, NIIF_ERROR},
	{"XVES-3.4", IDS_XVES_LOCAL, IDS_XVES_3_4, NIIF_ERROR},
	{"XVES-7", IDS_XVES_REMOTE, IDS_XVES_7, NIIF_WARNING},
	{"XVES-16", IDS_XVES_REMOTE, IDS_XVES_16, NIIF_WARNING},
	{"XVES-17", IDS_XVES_REMOTE, IDS_XVES_17, NIIF_WARNING},
	{"XVES-18", IDS_XVES_REMOTE, IDS_XVES_18, NIIF_WARNING},
	{"XVES-19", IDS_XVES_REMOTE, IDS_XVES_19, NIIF_WARNING},
	{"XVES-20", IDS_XVES_REMOTE, IDS_XVES_20, NIIF_WARNING},
	{"XVES-22", IDS_XVES_REMOTE, IDS_XVES_22, NIIF_WARNING},
	{"XVES-31", IDS_XVES_REMOTE, IDS_XVES_31, NIIF_WARNING},
	{NULL, IDS_XVES_REMOTE, IDS_XVES_UNKNOWN, NIIF_WARNING}
};

BOOL vmErrorIcon(HWND hwnd, int uidx, const char *user, const char *err) {
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_INFO;
	nid.uID = VMW_ID_BTN_UPROF + uidx;
	nid.uCallbackMessage = VMW_MSG_NOTIFY;
	if (err) {
		char proto[16];
		const char *pr = NULL;
		const char *e = err;
		char *d = proto;
		while (1) {
			char c = *e++;
			if (c == '.') {
				*d = 0;
				pr = proto;
				break;
			}
			else if (!c || d >= proto + sizeof(proto) - 1) {
				e = err;
				break;
			}
			else if (c >= 'a' && c <= 'z') {
				c &= 0xdf;
			}
			*d++ = c;
		}
		struct vmNotify_st *ntfy = vmNotify;
		while (ntfy->err) {
			const char *s1 = ntfy->err;
			const char *s2 = e;
			char c;
			while ((c = *s1++)) {
				if (*s2++ != c) break;
			}
			if (!c && (!*s2 || *s2 == '.')) break;
			ntfy++;
		}

		WCHAR fmt[128];
		WCHAR msg[256];
		LoadString(vmInst, ntfy->msg2, fmt, ARRAYSIZE(fmt));
		LoadString(vmInst, ntfy->msg1, msg, ARRAYSIZE(msg));
		StringCchCopyW(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), msg);
		PWCHAR msg2 = msg + lstrlenW(msg);
		*msg2++ = ':';
		*msg2++ = ' ';
		wsprintfW(msg2, fmt, pr, user);
		StringCchCopyW(nid.szInfo, ARRAYSIZE(nid.szInfo), msg2);
		StringCchCopyW(nid.szTip, ARRAYSIZE(nid.szTip), msg);
		LoadIconMetric(vmInst, MAKEINTRESOURCEW(IDI_ICON1_E), LIM_SMALL, &nid.hIcon);
		nid.dwInfoFlags = ntfy->flags;
		Shell_NotifyIcon(NIM_ADD, &nid);
		nid.uVersion = NOTIFYICON_VERSION_4;
		return Shell_NotifyIcon(NIM_SETVERSION, &nid);
	}
	else {
		return Shell_NotifyIcon(NIM_DELETE, &nid);
	}
}

BOOL vmSnifAuthIcon(HWND hwnd, int show) {
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_INFO;
	nid.uID = VMW_MSG_SNIFAUTH;
	nid.uCallbackMessage = VMW_MSG_NOTIFY;
	if (show) {
		WCHAR msg[256];
		LoadString(vmInst, IDS_SNIF_AUTH_TITLE, msg, ARRAYSIZE(msg));
		StringCchCopyW(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), msg);
		LoadString(vmInst, IDS_SNIF_AUTH_TEXT, msg, ARRAYSIZE(msg));
		StringCchCopyW(nid.szInfo, ARRAYSIZE(nid.szInfo), msg);
		StringCchCopyW(nid.szTip, ARRAYSIZE(nid.szTip), msg);
		LoadIconMetric(vmInst, MAKEINTRESOURCEW(IDI_ICON1), LIM_SMALL, &nid.hIcon);
		nid.dwInfoFlags = NIIF_WARNING;
		Shell_NotifyIcon(NIM_ADD, &nid);
		nid.uVersion = NOTIFYICON_VERSION_4;
		return Shell_NotifyIcon(NIM_SETVERSION, &nid);
	}
	else {
		return Shell_NotifyIcon(NIM_DELETE, &nid);
	}
}

BOOL vmShellIcon(HWND hwnd)
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
	nid.uID = VMW_MSG_NOTIFY;
	nid.uCallbackMessage = VMW_MSG_NOTIFY;
	LoadIconMetric(vmInst, MAKEINTRESOURCEW(IDI_ICON1), LIM_SMALL, &nid.hIcon);
	LoadString(vmInst, IDS_TRAY, nid.szTip, ARRAYSIZE(nid.szTip));
	Shell_NotifyIcon(NIM_ADD, &nid);
	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}
