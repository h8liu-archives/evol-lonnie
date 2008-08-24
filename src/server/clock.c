#include "server.h"

char * timertitle = "Timer";
HWND hwndClock;
HINSTANCE hInstClock;
pthread_t thClock;
unsigned int curClock;
unsigned int elapse = DEFAULT_CLOCK_ELAPSE;
HANDLE hClockInitDone;

#define CLOCK_TIMER_IDT 14	// Just an identifier

#define CLOCK_SET(hwndClock) \
	SetTimer(hwndClock, CLOCK_TIMER_IDT, elapse * CLOCK_UNIT, NULL)

#define CLOCK_KILL(hwndClock) \
	KillTimer(hwndClock, CLOCK_TIMER_IDT)

LRESULT CALLBACK 
TimerWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_CREATE:
			curClock = 0;
			CLOCK_SET(hwnd);
			break;
		case WM_TIMER:
			curClock += elapse;

			if (curClock > CLOCK_TIME_END)
				event_post(ES_TIMEEND);
			else	
				event_post(EVENT_CLOCKNEW(curClock));
			break;
		case WM_DESTROY:
			CLOCK_KILL(hwnd);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

void * 
clock_listen()
{
	MSG msg;

	hwndClock = CreateWindow(timertitle,
			timertitle,
			WS_DISABLED, // style
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,
			NULL,
			hInstClock,
			NULL
			);

	SetEvent(hClockInitDone);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	pthread_exit(NULL);
	
	return 0;
}

int 
clock_start(HINSTANCE instance)
{
	WNDCLASS wc;
	
	wc.style = 0;
	wc.lpfnWndProc = TimerWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = timertitle;

	if (!RegisterClass(&wc))
	{
		dprintf("ERROR: Register window class for clock.\n");
		return -E_PANIC;
	}

	hClockInitDone = CreateEvent(NULL, FALSE, FALSE, NULL);

	pthread_create(&thClock, NULL, clock_listen, NULL);

	WaitForSingleObject(hClockInitDone, INFINITE);
	CloseHandle(hClockInitDone);

	return SUCC;
}

int
clock_pause()
{
	CLOCK_KILL(hwndClock);
	return SUCC;
}


int
clock_resume()
{
	CLOCK_SET(hwndClock);
	return SUCC;
}

int
clock_stop()
{
	PostMessage(hwndClock, WM_CLOSE, 0, 0);
	pthread_join(thClock, NULL);

	return SUCC;
}

void
clock_setElapse(unsigned int e)
{
	elapse = e;
	if (elapse > MAX_CLOCK_ELAPSE)
	{
		dprintf("WARNING: too large elapse, set to max %d.\n", 
				MAX_CLOCK_ELAPSE);
		elapse = MAX_CLOCK_ELAPSE;
	}
	if (elapse < MIN_CLOCK_ELAPSE)
	{
		dprintf("WARNING: too small elapse, set to min %d.\n", 
				MIN_CLOCK_ELAPSE);
		elapse = MIN_CLOCK_ELAPSE;
	}
}

unsigned int
clock_getElapse()
{
	return elapse;
}
