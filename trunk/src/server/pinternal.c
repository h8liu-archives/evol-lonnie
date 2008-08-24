#include "server.h"

#define EXPIRE_DELAY 100

void * 
player_internal_proc(void * para)
{
	int outflag = FALSE;
	EVENT e;
	PLAYER * p = (PLAYER *) para;

	while (!outflag)
	{
		pthread_mutex_lock(&(p->mlstEvent));

		while (LIST_EMPTY(*(p->plstEvent)))
			pthread_cond_wait(&(p->condListCome), &(p->mlstEvent));
		
		e = LIST_DEQUEUE(*(p->plstEvent));
		pthread_cond_signal(&(p->condListFree));
	
		if (!LIST_EMPTY(*(p->plstEvent)))
			pthread_cond_signal(&p->condListCome);

		pthread_mutex_unlock(&(p->mlstEvent));

		switch (e & ET_MASK)
		{
			case ET_VIEW:
				p->viewClock = p->lastClock;
				p->actionLeft = MAX_ACTION_PER_CLOCK;
				(*(p->funcCallback))(p->playerID,
						p->viewClock,
						p->view, 
						EVENT_VIEW_OFF(e), 
						EVENT_VIEW_SIZE(e));
				
				break;

			case ET_CLOCK:
				p->lastClock = EVENT_CLOCK(e);
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

	pthread_exit(NULL);
	return NULL;
}

int 
player_internal_start(PID i, PLAYER_CALLBACK cb)
{
	PLAYER * p = player_get(i);

	if (p->type != PT_NONE)
	{
		dprintf("ERROR: attempt to start (internal) player %d\n", i);
		return -E_PANIC;
	}
	if (cb == NULL)
	{
		dprintf("ERROR: null call back for internal player\n");
		return -E_PANIC;
	}

	pthread_mutex_init(&(p->mlstEvent), NULL);
	pthread_cond_init(&(p->condListCome), NULL);
	pthread_cond_init(&(p->condListFree), NULL);

	p->plstEvent = malloc(sizeof(EVENT_LIST));
	LIST_INIT((*(p->plstEvent)));

	p->view = malloc(VIEW_SIZE);

	p->funcCallback = cb;

	pthread_create(&(p->thProcessor), NULL, player_internal_proc, (void *)p);

	p->type = PT_INTERNAL;

	return player_active(i);
}

void
player_internal_end(PLAYER * p)
{
	pthread_join(p->thProcessor, NULL);

	free(p->view);
	free(p->plstEvent);

	pthread_cond_destroy(&(p->condListCome));
	pthread_cond_destroy(&(p->condListFree));
	pthread_mutex_destroy(&(p->mlstEvent));
}

void
player_internal_send(PLAYER * p, EVENT event)
{
	pthread_mutex_lock(&(p->mlstEvent));

	while (LIST_FULL(*(p->plstEvent)))
		pthread_cond_wait(&(p->condListFree), 
				&(p->mlstEvent));
	LIST_ENQUEUE((*(p->plstEvent)), event);
	pthread_cond_signal(&(p->condListCome));

	pthread_mutex_unlock(&(p->mlstEvent));
}

int
player_internal_action(PID pid, ACTION action)
{
	PLAYER * p;
	if (pid > PLAYER_MAX)
	{
		dprintf("FAIL: player_internal_action() invalid pid.\n");
		return -E_PANIC;
	}
	
	p = player_get(pid);
	if (p->type != PT_INTERNAL)
	{
		dprintf("FAIL: player_internal_action() not internal player.\n");
		return -E_PANIC;
	}

	if (p->actionLeft == 0)
		dprintf("WARNING: internal player %d too many actions each clock.\n",
				pid);
	else
		p->actionLeft--;
	
	player_action(pid, action, p->viewClock + EXPIRE_DELAY);

	return SUCC;
}

