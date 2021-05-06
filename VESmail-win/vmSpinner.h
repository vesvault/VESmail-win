#pragma once


HWND vmSpinnerCreate(HWND parent, int x, int y, int w, int h);
void vmSpinnerStep(HWND spn);
int vmSpinnerMove(HWND spn, int x, int y, int w, int h);
void vmSpinnerDestroy(HWND spn);
