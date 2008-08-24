#include "display.h"

#include <pthread.h>

KEYDOWN_CALLBACK _kd = NULL;
KEYUP_CALLBACK _ku = NULL;

HANDLE hInit;
HANDLE hwnd;
pthread_t thWin;
char * title = "SampleGame";
pthread_mutex_t mView;
GSA_VIEW displayView;

void draw(HDC bufferDC)
{
	LOGBRUSH lb;
	HPEN hPen, hPenOld;
	unsigned int i, j;

	pthread_mutex_lock(&mView);

	lb.lbStyle = BS_SOLID;
	lb.lbHatch = 0;
	for (i=0; i<SH; i++)
	{
		for (j=0; j<SW; j++)
		{
			if (internalView.space[i][j] == B_WALL)
				lb.lbColor = RGB(255,255,255);
			else if (internalView.space[i][j] == B_BLANK)
				lb.lbColor = RGB(90, 90, 90);
			else if (internalView.space[i][j] == 0)
				lb.lbColor = RGB(100, 255, 100);
			else if (internalView.space[i][j] == 1)
				lb.lbColor = RGB(255, 100, 100);
			else if (internalView.space[i][j] == B_INVISIBLE)
				lb.lbColor = RGB(50, 50, 50);

			hPen = ExtCreatePen(PS_COSMETIC | PS_SOLID,
					1, &lb, 0, NULL);
			hPenOld = (HPEN) SelectObject(bufferDC,
					hPen);

			MoveToEx(bufferDC, j*20 + 2, i*20 + 2, NULL);
			LineTo(bufferDC, j*20 + 18, i*20 + 2);
			LineTo(bufferDC, j*20 + 18, i*20 + 18);
			LineTo(bufferDC, j*20 + 2, i*20 + 18);
			LineTo(bufferDC, j*20 + 2, i*20 + 2);
			SelectObject(bufferDC, hPenOld);

			DeleteObject(hPen);
		}
	}

	pthread_mutex_unlock(&mView);
}

LRESULT CALLBACK
CoreWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// PAINTSTRUCT ps;
	HDC hdc, bufferDC;
	HBITMAP memBM;
	RECT rc;
	UINT width, height;

	switch (msg)
	{
		case WM_CREATE:
			break;
		case WM_CLOSE:
			game_stop();
			// return DefWindowProc(hwnd, msg, wparam, lparam);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			hdc = GetDC(hwnd);
			GetClientRect(hwnd, &rc);
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;

			bufferDC = CreateCompatibleDC(hdc);
			memBM = CreateCompatibleBitmap(hdc, width, height);
			SelectObject(bufferDC, memBM);
			
			draw(bufferDC);

			BitBlt(hdc, 0, 0, width, height, bufferDC, 0, 0, SRCCOPY);

			DeleteObject(memBM);
			DeleteDC(bufferDC);
			ReleaseDC(hwnd, hdc);

			break;
		case WM_KEYDOWN:
			if (_kd)
			{
				(*_kd)(wparam);
			}
			break;
		case WM_KEYUP:
			if (_ku)
			{
				(*_ku)(wparam);
			}
			break;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

void *
display_listen()
{
	MSG msg;

	hwnd = CreateWindow(title,
		title,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WIN_WIDTH, WIN_HEIGHT,
		NULL,
		NULL,
		hCoreInst,
		NULL);

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	SetEvent(hInit);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

void
display_start(KEYDOWN_CALLBACK kdCallback, KEYUP_CALLBACK kuCallback)
{
	WNDCLASS wc;

	_kd = kdCallback;
	_ku = kuCallback;

	wc.style = 0;
	wc.lpfnWndProc = DisplayWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hCoreInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = title;

	if (!RegisterClass(&wc))
	{
		dprintf("ERROR: Register window class for display.\n");
		return;
	}
	
	pthread_mutex_init(&mView, NULL);

	hInit = CreateEvent(NULL, FALSE, FALSE, NULL);
	pthread_create(&thWin, NULL, display_listen, NULL);
	WaitForSingleObject(hInit, INFINITE);
	CloseHandle(hInit);
}

void display_clean()
{
	PostMessage(hwnd, WM_DESTROY, 0, 0);
	pthread_mutex_destroy(&mView);
}

void display_view(GSA_VIEW * view)
{
	pthread_mutex_lock(&mView);
	memcpy(&displayView, view, sizeof(GSA_VIEW));
	pthread_mutex_unlock(&mView);
}
