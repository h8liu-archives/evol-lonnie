#include "sample.h"

#include <pthread.h>
#include <stdio.h>

#define MAX_TIME 10000

#define WIN_WIDTH 210
#define WIN_HEIGHT 230

// The Game core

HWND hwndCore;
char * coreTitle = "SampleGame";
HINSTANCE hCoreInst;
HANDLE hMainWinInit;
pthread_t thMainWin;

enum SPACE_BLOCKS
{
	S_BLANK = 0,
	S_WALL,
	S_OCCUPIED1,
	S_OCCUPIED2,
};

pthread_mutex_t mSampleCore;
int gameStarted = FALSE;
unsigned char space[SH][SW];

typedef struct _PState
{
	int top, left;
	ACTION lastAction;
} PState;

PState pstate[2]; // for two players

int paused = FALSE;

extern GSA_VIEW internalView;

void draw(HDC bufferDC)
{
	LOGBRUSH lb;
	HPEN hPen, hPenOld;
	unsigned int i, j;

	pthread_mutex_lock(&mSampleCore);
	if (gameStarted)
	{
		// printf("I am drawing!.\n");
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
	}
	pthread_mutex_unlock(&mSampleCore);
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
			switch (wparam)
			{
				case VK_LEFT:
					player_internal_action(0, GSA_LEFT);
					break;
				case VK_RIGHT:
					player_internal_action(0, GSA_RIGHT);
					break;
				case VK_UP:
					player_internal_action(0, GSA_UP);
					break;
				case VK_DOWN:
					player_internal_action(0, GSA_DOWN);
					break;
				case '\r':
					player_internal_action(0, GSA_IDLE);
					break;
				case VK_ESCAPE:
					player_internal_action(0, GSA_QUIT);
					break;
				case ' ':
					if (!paused)
						game_pause();
					else
						game_resume();
					paused = !paused;
					break;
			}
			break;
		case WM_KEYUP:
			player_internal_action(0, GSA_IDLE);
			break;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

void *
sample_winlisten()
{
	MSG msg;

	hwndCore = CreateWindow(coreTitle,
		coreTitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WIN_WIDTH, WIN_HEIGHT,
		NULL,
		NULL,
		hCoreInst,
		NULL);

	ShowWindow(hwndCore, SW_SHOWNORMAL);
	UpdateWindow(hwndCore);

	SetEvent(hMainWinInit);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	// pthread_exit(NULL);
	return 0;
}

void
sample_wininit(int nCmdShow)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = CoreWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hCoreInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = coreTitle;

	if (!RegisterClass(&wc))
	{
		dprintf("ERROR: Register window class for core.\n");
		return;
	}
	
	hMainWinInit = CreateEvent(NULL, FALSE, FALSE, NULL);
	pthread_create(&thMainWin, NULL, sample_winlisten, NULL);
	WaitForSingleObject(hMainWinInit, INFINITE);
	CloseHandle(hMainWinInit);
}


void 
sample_init()
{
	unsigned int i, j;

	pthread_mutex_lock(&mSampleCore);

	gameStarted = TRUE;
	
	// clear space
	for (i = 0; i < SH; i++)
	{
		for (j = 0; j < SW; j++)
		{
			space[i][j] = B_BLANK;
		}
	}

	// build walls
	for (i = 0; i < SW; i++)
	{
		space[0][i] = B_WALL;
		space[SH-1][i] = B_WALL;
	}
	for (i = 0; i < SH; i++)
	{
		space[i][0] = B_WALL;
		space[i][SW-1] = B_WALL;
	}

	// set initial positions
	i = 3; j = 3;
	space[i][j] = 0;
	
	pstate[0].top = i;
	pstate[0].left = j;
	pstate[0].lastAction = GSA_IDLE;

	i = 7; j = 7;
	space[i][j] = 1;
	pstate[1].top = i;
	pstate[1].left = j;
	pstate[1].lastAction = GSA_IDLE;

	pthread_mutex_unlock(&mSampleCore);
}

void 
sample_clean()
{
 	PostMessage(hwndCore, WM_DESTROY, 0, 0);
}

void
sample_gameinit()
{
	pthread_mutex_init(&mSampleCore, NULL);
}

void
sample_gameclean()
{
	pthread_mutex_destroy(&mSampleCore);
}

void 
sample_clock(unsigned int c)
{
	unsigned int i, h, w;
	unsigned int left, top;
	GSA_VIEW view;
	
	/* if (c > MAX_TIME)
	{
		core_exit();
		return;
	} */

	pthread_mutex_lock(&mSampleCore);
	for (i = 0; i < 2; i++)
	{
		left = pstate[i].left;
		top = pstate[i].top;

		if (pstate[i].lastAction != GSA_IDLE)
		{
			switch (pstate[i].lastAction)
			{
				case GSA_UP:
					top--;
					break;
				case GSA_DOWN:
					top++;
					break;
				case GSA_LEFT:
					left--;
					break;
				case GSA_RIGHT:
					left++;
					break;
			}

			if (space[top][left] == B_BLANK)
			{
				space[pstate[i].top][pstate[i].left] = B_BLANK;
				pstate[i].left = left;
				pstate[i].top = top;
				space[top][left] = (i==0)?0:1;
				// pstate[i].lastAction = GSA_IDLE;
			}
		}

		for (h = 0; h < SH; h++)
		{
			for (w = 0; w < SW; w++)
			{
				if ((top > 2 + h) || (h > 2 + top) ||
						(left > 2 + w) || (w > 2 + left))
					view.space[h][w] = B_INVISIBLE;
				else
					view.space[h][w] = space[h][w];

				if (space[h][w] == B_WALL)
					view.space[h][w] = space[h][w];
				// view.space[h][w] = B_INVISIBLE;
			}
		}

		core_viewWrite(i, 0, &view, sizeof(GSA_VIEW));
	}
	pthread_mutex_unlock(&mSampleCore);

	dprintf("CLOCK %8d\n", c);
	PostMessage(hwndCore, WM_PAINT, 0, 0);
}

void 
sample_action(PID playerID, ACTION action)
{
	dprintf("PLAYER %2d  ACTION %02x\n", playerID, action);
	if (action == GSA_QUIT)
		core_exit();
	else
	{
		pthread_mutex_lock(&mSampleCore);
		if (playerID == 0)
		{
			pstate[0].lastAction = action;
		}
		else
		{
			pstate[1].lastAction = action;
		}
		pthread_mutex_unlock(&mSampleCore);
	}
}

