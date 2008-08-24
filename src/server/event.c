#include "server.h"

#include <string.h>

pthread_mutex_t mEvent;

extern PLAYER players[PLAYER_MAX];
extern PLAYER * activePlayers[PLAYER_MAX];
extern unsigned int activePlayerCount;

int isRunning;
pthread_cond_t condExit;
unsigned int coreClock;

CORE_INIT coreFuncInit = NULL;
CORE_CLOCK coreFuncClock = NULL;
CORE_ACTION coreFuncAction = NULL;
CORE_CLEAN coreFuncClean = NULL;


void core_setInitFunc(CORE_INIT init)
{
	coreFuncInit = init;
}

void core_setClockFunc(CORE_CLOCK clock)
{
	coreFuncClock = clock;
}

void core_setActionFunc(CORE_ACTION action)
{
	coreFuncAction = action;
}

void core_setCleanFunc(CORE_CLEAN clean)
{
	coreFuncClean = clean;
}

void 
core_init(void)
{
	if (coreFuncInit != NULL)
		(*coreFuncInit)();
}

void 
core_clock(unsigned int clock)
{
	if (coreFuncClock != NULL)
		(*coreFuncClock)(clock);
}

void 
core_action(PID playerID, ACTION action)
{
	if (coreFuncAction != NULL)
		(*coreFuncAction)(playerID, action);
}

void 
core_clean(void)
{
	if (coreFuncClean != NULL)
		(*coreFuncClean)();
}


int
core_viewWrite(PID playerID, unsigned int offset, 
		void * src, unsigned int size)
{
	if (players[playerID].type == PT_NONE)
	{
		dprintf("FAILED: core_viewWrite() of empty player %d\n", playerID);
		return -E_PANIC;
	}
	if (offset + size > VIEW_SIZE || offset > VIEW_SIZE)
	{
		dprintf("FAILED: core_viewWrite() out of range.\n");
		return -E_PANIC;
	}
	
	memcpy(players[playerID].view + offset, src, size);
	return SUCC;
}

int
core_viewRead(PID playerID, unsigned int offset,
		void * dest, unsigned int size)
{
	if (players[playerID].type == PT_NONE)
	{
		dprintf("FAILED: core_viewRead() of empty player %d", playerID);
		return -E_PANIC;
	}

	if (offset + size > VIEW_SIZE || offset > VIEW_SIZE);
	{
		dprintf("FAILED: core_viewRead() out of range.");
		return -E_PANIC;
	}

	memcpy(dest, players[playerID].view + offset, size);
	return SUCC;
}

int
core_exit()
{
	core_event(ES_END);
	player_event(ES_END);
	return SUCC;
}

int
event_wait()
{
	pthread_mutex_lock(&mEvent);
	while (isRunning)
		pthread_cond_wait(&condExit, &mEvent);
	pthread_mutex_unlock(&mEvent);
	return SUCC;
}

void 
core_event(EVENT event)
{
	// enqueue the event to core event queue, and notify
	// DO NOT call event_post() in this function.
	
	unsigned int i;

	// process event here
	// don't forget quit for system event
	switch (event & ET_MASK)
	{
		case ET_CLOCK:
			// process the game

			coreClock = EVENT_CLOCK(event);
			core_clock(coreClock * CLOCK_UNIT);

			// send out view update for every player
			for (i = 0; i < activePlayerCount; i++)
			{
				if (activePlayers[i]->type == PT_NONE)
					continue;
				player_event(
					EVENT_VIEWNEW(
					(activePlayers[i]->playerID), 0,
					 VIEW_SIZE));
			}
			break;
		case ET_ACTION:
			// apply action
			core_action(EVENT_PLAYERID(event),
					EVENT_ACTIONID(event));
			break;
		case ET_SYSTEM:
			if (ES_ISEND(event))
			{
				core_clean();
				isRunning = FALSE;
				pthread_cond_signal(&condExit);
				switch (event)
				{
					case ES_TERMINATE:
						dprintf("User terminated.\n");
						break;
					case ES_END:
						dprintf("End gracefully.\n");
						break;
					case ES_TIMEEND:
						dprintf("Time end.\n");
						break;
				}
			}
			else if (event == ES_START)
			{
				core_init();
			}
			break;
		case ET_VIEW:
		default:
			// This should never happen.
			dprintf("WARNING: unexpected event received in core.\n");
			break;
	}
}

int
event_start()
{


	coreClock = 0;
	
	isRunning = TRUE;

	event_post(ES_START);

	return SUCC;
}

int
event_stop()
{


	return SUCC;
}

int
event_init()
{	
	pthread_mutex_init(&mEvent, NULL);
	pthread_cond_init(&condExit, NULL);

	return SUCC;
}

int
event_clean()
{	
	pthread_mutex_destroy(&mEvent);
	pthread_cond_destroy(&condExit);

	return SUCC;
}

int
event_post(EVENT event)
{
	unsigned int i;
	EXPIRETIME exp;
	PLAYER * p;

	pthread_mutex_lock(&mEvent);

//	dprintf("EVENT %08x\n", event);

	if (isRunning)
	{
		if ((event & ET_MASK) == ET_ACTION)
		{
			exp = coreClock & CLOCK_EXPIRE_MASK;
			exp -= EVENT_ACTION_EXPIRE_TIME(event);
			if (exp < EXPIRE_RANGE)
				goto out;

			p = &players[EVENT_PLAYERID(event)];
			if (p->actionPoint == 0)
				goto out;
			p->actionPoint--;
		}
		else if ((event & ET_MASK) == ET_CLOCK)
		{
			for (i = 0; i < activePlayerCount; i++)
			{
				p = activePlayers[i];
				if (p->type != PT_NONE)
					p->actionPoint = MAX_ACTION_PER_CLOCK;
			}
		}

		if (EVENT_FOR_PLAYER(event))
			player_event(event);

		if (EVENT_FOR_CORE(event))
			core_event(event);
	}
out:
	pthread_mutex_unlock(&mEvent);

	return SUCC;
}

