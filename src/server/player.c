#include "server.h"

PLAYER players[PLAYER_MAX];
PLAYER * activePlayers[PLAYER_MAX];
unsigned int activePlayerCount;

// implementation

PLAYER *
player_get(PID i)
{
	return &(players[i]);
}

int
player_init()
{
	unsigned int i;
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		dprintf("ERROR: WSAStartup() failed.\n");
		return -E_PANIC;
	}

	for (i = 0; i < PLAYER_MAX; i++)
	{
		players[i].playerID = i;
		players[i].type = PT_NONE;
		players[i].actionPoint = 0;
	}
	
	activePlayerCount = 0;

	return SUCC;
}

int
player_active(PID playerID)
{
	activePlayers[activePlayerCount++] = &(players[playerID]);
	return SUCC;
}

int
player_clean()
{
	// clean all connections
	unsigned int i;

	for (i = 0; i < PLAYER_MAX; i++)
	{
		PLAYER * p = &(players[i]);
		switch (p->type)
		{
			case PT_REMOTE: 
				player_remote_end(p);
				break;
			case PT_LOCAL:	
				player_local_end(p);
				break;
			case PT_INTERNAL: 
				player_internal_end(p);
				break;
		}
		p->type = PT_NONE;
	}

	WSACleanup();

	activePlayerCount = 0;

	return SUCC;
}

void
player_send(PLAYER * p, EVENT event)
{
	switch (p->type)
	{
		case PT_LOCAL:
			player_local_send(p, event);
			break;
		case PT_REMOTE:
			player_remote_send(p, event);
			break;
		case PT_INTERNAL:
			player_internal_send(p, event);
			break;
	}
}

void 
player_event(EVENT event)
{
	// If player is local, send windows message
	// If player is remote:
	// 	Send message through tcp connection
	// 	If event is ET_VIEW, send view change.
	// DO NOT call event_post() in this function
	
	unsigned int i;

	if ((event & ET_MASK) == ET_VIEW)
		player_send(&players[EVENT_PLAYERID(event)], event);
	else // broadcast to every active player
	{
		for (i = 0; i < activePlayerCount; i++)
			if (activePlayers[i]->type != PT_NONE)
				player_send(activePlayers[i], event);
	}
}

void
player_action(PID pid, ACTION action, EXPIRETIME expire)
{
	PLAYER * p = &(players[pid]);
	event_post(EVENT_ACTIONNEW(p->playerID,
			action,
			(expire)));
}

