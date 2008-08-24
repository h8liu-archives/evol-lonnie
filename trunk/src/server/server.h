#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <evol/server.h>

// Some APIs for internal use

int view_update(PID playerID, unsigned int offset, 
		const void * src, unsigned int size);

int event_post(EVENT event);

void core_event(EVENT event);

void player_event(EVENT event);
void * player_view(PID playerID);

// Other includings

#include <pthread.h>
#include <stdio.h>
#include <winsock.h>

// Player

enum PLAYER_TYPE
{
	PT_NONE = 0,
	PT_LOCAL,
	PT_REMOTE,
	PT_INTERNAL,
};

typedef struct _PLAYER
{
	PID playerID;

	int type;
	void * view;
	unsigned int actionPoint;
	
	// for internal only
	pthread_mutex_t mlstEvent;
	pthread_cond_t condListFree;
	pthread_cond_t condListCome;
	EVENT_LIST * plstEvent;

	pthread_t thProcessor;
	PLAYER_CALLBACK funcCallback;

	int lastClock;
	int viewClock;
	unsigned int actionLeft;

	// for local only
	HANDLE hProcess;
	HANDLE hThread;
	HANDLE hEvent;
	HANDLE hView;
	DWORD dwThread;
	pthread_t thLocalListener;
	DWORD dwLocalListener;

	// for remote only
	NET_IP ip;
	NET_PORT port;
	SOCKET socket;

} PLAYER;



// useful macro

#define ASSERT_POS(call)\
	if ((r = call) < 0) return r

// Module API

int event_start();
int event_stop();
int event_post(EVENT event);
int event_wait();
int event_init();
int event_clean();

void player_event(EVENT event);
int player_init();
int player_active(PID playerID);
int player_clean();
void player_action(PID playerID, ACTION action, EXPIRETIME expire);
PLAYER * player_get(PID playerID);

int clock_start(HINSTANCE instance);
int clock_stop();
int clock_pause();
int clock_resume();

void player_internal_end(PLAYER * p);
void player_internal_send(PLAYER * p, EVENT event);

void player_remote_end(PLAYER * p);
void player_remote_send(PLAYER * p, EVENT event);

void player_local_end(PLAYER * P);
void player_local_send(PLAYER * p, EVENT event);

#endif // SERVER_SERVER_H
