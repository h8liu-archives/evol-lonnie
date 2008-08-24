#include "server.h"

#define LOCAL_CLIENT_TIMEOUT 1000

void * 
player_local_proc(void * para)
{
	EVENT e;
	PLAYER * p = (PLAYER *) para;
	MSG msg;

	PeekMessage(&msg, (HWND)(-1), WM_LOCAL_PLAYER, WM_LOCAL_PLAYER,
			PM_NOREMOVE);
	p->dwLocalListener = GetCurrentThreadId();
	SetEvent(p->hEvent);

	while (GetMessage(&msg, (HWND)(-1), 
		WM_LOCAL_PLAYER, WM_LOCAL_PLAYER))
	{
		if (msg.lParam == WM_MAGIC_LPARAM)
		{
			e = msg.wParam;
			if ((e & ET_MASK) == ET_ACTION)
			{
				player_action(p->playerID,
						EVENT_ACTIONID(e),
						EVENT_ACTION_EXPIRE_TIME(e));
			}
			else if (ES_ISEND(e))
			{
				break;
			}
		}
	}

	pthread_exit(NULL);
	return 0;
}

int 
player_local_start(PID i, const char * command)
{
	SECURITY_ATTRIBUTES attr;
	HANDLE hViewTemp;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char cmd[1024];
	int ret;
	
	PLAYER * p = player_get(i);

	// parameter checking
	if (p->type != PT_NONE)
	{
		dprintf("ERROR: attempt to start (local) player %d\n", i);
		return -E_PANIC;
	}
	if (command == NULL)
	{
		dprintf("ERROR: null command line for local player\n");
		return -E_PANIC;
	}

	// shared event
	attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	attr.lpSecurityDescriptor = NULL;
	attr.bInheritHandle = TRUE;
	p->hEvent = CreateEvent(&attr, FALSE, FALSE, NULL);

	// view
	attr.bInheritHandle = FALSE;
	p->hView = CreateFileMapping(INVALID_HANDLE_VALUE, &attr, PAGE_READWRITE,
			0, VIEW_SIZE, NULL);
	p->view = MapViewOfFile(p->hView, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	// listener
	ResetEvent(p->hEvent);
	pthread_create(&p->thLocalListener, NULL, player_local_proc, (void *)p);
	WaitForSingleObject(p->hEvent, INFINITE); // p->dwLocalListener will be set

	// shared view
	DuplicateHandle(GetCurrentProcess(), p->hView, 
			GetCurrentProcess(), &hViewTemp,
			0, TRUE, DUPLICATE_SAME_ACCESS);
	
	// process
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.lpTitle = "";
	si.dwFlags = 0;

	sprintf(cmd, "client run view=%d init=%d server=%d",
			(int)hViewTemp, (int)(p->hEvent), (int)(p->dwLocalListener));
	ResetEvent(p->hEvent);
	CreateProcess(command, cmd, NULL, NULL, TRUE,
			CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	// close shared view after inherited, no use in server now.
	CloseHandle(hViewTemp);

	// wait for message queue built
	ret = WaitForSingleObject(p->hEvent, LOCAL_CLIENT_TIMEOUT);
	if (ret == WAIT_TIMEOUT)
		goto timeout;

	// setup local player info
	p->dwThread = pi.dwThreadId;
	p->hProcess = pi.hProcess;
	p->hThread = pi.hThread;

	p->type = PT_LOCAL;

	// active player to active list and return
	return player_active(i);

timeout:
	dprintf("ERROR: local player %d init time out", i);

	TerminateProcess(pi.hProcess, 0);
	
	PostThreadMessage(p->dwLocalListener, WM_LOCAL_PLAYER,
			ES_TERMINATE, WM_MAGIC_LPARAM);
	pthread_join(p->thLocalListener, NULL);
	
	UnmapViewOfFile(p->view);
	CloseHandle(p->hView);

	CloseHandle(p->hEvent);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	
	return -E_PANIC;
}

void 
player_local_send(PLAYER * p, EVENT e)
{
	if ((e & ET_MASK) == ET_VIEW)
		ResetEvent(p->hEvent);
	PostThreadMessage(p->dwThread, WM_LOCAL_PLAYER, e, WM_MAGIC_LPARAM);
	if ((e & ET_MASK) == ET_VIEW)
		WaitForSingleObject(p->hEvent, LOCAL_CLIENT_TIMEOUT);
}

void 
player_local_end(PLAYER * p)
{
	DWORD ret;
	ret = WaitForSingleObject(p->hProcess, LOCAL_CLIENT_TIMEOUT);
	if (ret != WAIT_OBJECT_0)
	{
		dprintf("WARNING: local player %d is dead, terminate forcedly.\n", p->playerID);
		TerminateProcess(p->hProcess, 0);
	}

	PostThreadMessage(p->dwLocalListener, WM_LOCAL_PLAYER,
			ES_TERMINATE, WM_MAGIC_LPARAM);
	pthread_join(p->thLocalListener, NULL);

	UnmapViewOfFile(p->view);
	CloseHandle(p->hView);

	CloseHandle(p->hEvent);
	CloseHandle(p->hThread);
	CloseHandle(p->hProcess);
}
