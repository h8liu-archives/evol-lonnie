#include "player.h"

#include <windows.h>
#include <stdio.h>

#define EXPIRE_DELAY 100

void * view;
void * serverView;
DWORD dwServer;
unsigned int viewClock;
unsigned int lastClock;
unsigned int actionLeft;
int outflag;
HANDLE hEvent;

void view_sync(unsigned int offset, unsigned int size)
{
	memcpy(view + offset, serverView + offset, size);
}

void player_event(EVENT e)
{
	switch (e & ET_MASK)
	{
		case ET_VIEW:
			viewClock = lastClock;
			actionLeft = MAX_ACTION_PER_CLOCK;
			view_sync(EVENT_VIEW_OFF(e), EVENT_VIEW_SIZE(e));
			SetEvent(hEvent);
			ai_callback(viewClock, view,
					EVENT_VIEW_OFF(e),
					EVENT_VIEW_SIZE(e));
			break;
		case ET_CLOCK:
			lastClock = EVENT_CLOCK(e);
			break;
		case ET_SYSTEM:
			if (ES_ISEND(e))
			{
				outflag = TRUE;
				break;
			}
		default:
			// this should not happen
			break;
	}
}

int ai_action(ACTION a)
{
	if (actionLeft == 0)
		return 0;

	actionLeft--;
	PostThreadMessage(dwServer, WM_LOCAL_PLAYER,
			EVENT_ACTIONNEW(0, a, (viewClock + EXPIRE_DELAY)),
			WM_MAGIC_LPARAM);

	return actionLeft;
}

void player_init()
{
	lastClock = 0;
	viewClock = 0;
	actionLeft = 0;
	outflag = FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, 
		HINSTANCE hPrevInstance, 
		LPSTR lpCmdLine,
		int nCmdShow)
{
	HANDLE hServerView;
	MSG msg;
	EVENT e;
	int hvCache, pidCache, heCache;

	view = malloc(VIEW_SIZE);

	if (sscanf(lpCmdLine, "run view=%d init=%d server=%d", 
			&hvCache, 
			&heCache, 
			&pidCache) < 3)
	{
		ai_intro();

		return 0;
	}

	hServerView = (HANDLE)(hvCache);
	hEvent = (HANDLE)(heCache);
	dwServer = pidCache;

	serverView = MapViewOfFile(
			hServerView,
			FILE_MAP_READ,
			0, 0, 0);

	if (view == NULL)
	{
		printf("ERROR: view empty.\n");
		goto out;
	}
	memcpy(view, serverView, VIEW_SIZE);
	
	PeekMessage(&msg, (HWND)(-1), WM_LOCAL_PLAYER, 
			WM_LOCAL_PLAYER, PM_NOREMOVE);

	SetEvent(hEvent);

	player_init();
	while (GetMessage(&msg, (HWND)(-1), 
		WM_LOCAL_PLAYER, WM_LOCAL_PLAYER))
	{
		if (msg.lParam == WM_MAGIC_LPARAM)
		{
			e = msg.wParam;
			player_event(e);
		}
		if (outflag)
			break;
	}

out:
	free(view);

	return 0;
}
