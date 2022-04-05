#include "stdafx.h"
#include "VESmail-win.h"
#include <shellapi.h>
#include <CommCtrl.h>
#include "vmSpinner.h"

struct {
	HWND dots[8];
	HICON icons[2];
	HWND prompt;
	int curr;
} vmSpinner;


HWND vmSpinnerDot(HWND spn, int x, int y, int w, int h, int st) {
	HWND dot = CreateWindow(L"STATIC", NULL, SS_ICON | WS_VISIBLE | WS_CHILD, x - w / 2, y - h / 2, w, h, spn, NULL, vmInst, NULL);
	SendMessage(dot, STM_SETICON, (WPARAM)vmSpinner.icons[st], NULL);
	return dot;
}

void vmSpinnerStep(HWND spn) {
	SendMessage(vmSpinner.dots[vmSpinner.curr++], STM_SETICON, (WPARAM)vmSpinner.icons[0], NULL);
	if (vmSpinner.curr >= sizeof(vmSpinner.dots) / sizeof(*vmSpinner.dots)) vmSpinner.curr = 0;
	SendMessage(vmSpinner.dots[vmSpinner.curr], STM_SETICON, (WPARAM)vmSpinner.icons[1], NULL);
}

HWND vmSpinnerCreate(HWND hWnd, int x, int y, int w, int h) {
	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCEW(IDI_LED_C), w, h, vmSpinner.icons);
	LoadIconWithScaleDown(vmInst, MAKEINTRESOURCEW(IDI_LED_A), w, h, vmSpinner.icons + 1);

	HWND spn = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD, x - 2 * w, y - 2 * h, 4 * w, 4 * h, hWnd, NULL, vmInst, NULL);
	vmSpinner.dots[0] = vmSpinnerDot(spn, w / 2, h * 2, w, h, 1);
	vmSpinner.dots[1] = vmSpinnerDot(spn, w * 15 / 16, h * 15 / 16, w, h, 0);
	vmSpinner.dots[2] = vmSpinnerDot(spn, w * 2, h / 2, w, h, 0);
	vmSpinner.dots[3] = vmSpinnerDot(spn, w * 49 / 16, h * 15 / 16, w, h, 0);
	vmSpinner.dots[4] = vmSpinnerDot(spn, w * 7 / 2, h * 2, w, h, 0);
	vmSpinner.dots[5] = vmSpinnerDot(spn, w * 49 / 16, h * 49 / 16, w, h, 0);
	vmSpinner.dots[6] = vmSpinnerDot(spn, w * 2, h * 7 / 2, w, h, 0);
	vmSpinner.dots[7] = vmSpinnerDot(spn, w * 15 / 16, h * 49 / 16, w, h, 0);
	vmSpinner.curr = 0;
	return spn;
}

int vmSpinnerMove(HWND spn, int x, int y, int w, int h) {
	return MoveWindow(spn, x - 2 * w, y - 2 * h, 4 * w, 4 * h, 0);
}

void vmSpinnerDestroy(HWND spn) {
	int i;
	for (i = 0; i < sizeof(vmSpinner.dots) / sizeof(*vmSpinner.dots); i++) {
		SendMessage(vmSpinner.dots[i], STM_SETICON, NULL, NULL);
	}
}
