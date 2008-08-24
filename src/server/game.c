#include "server.h"

#include "stdarg.h"

FILE * flog = NULL;
int inited = FALSE;
int started = FALSE;

int
game_init()
{
	int r;

	if (inited)
	{
		dprintf("FAIL: game_init() too many initializing.\n");
		return -E_PANIC;
	}

	// create new log file
	flog = fopen("game.log", "w");

	ASSERT_POS(event_init());

	// initialize player info
	ASSERT_POS(player_init());

	inited = TRUE;

	return SUCC;
}

int
game_clean()
{
	int r;

	if (!inited)
		return -E_PANIC;

	ASSERT_POS(player_clean());

	ASSERT_POS(event_clean());

	fclose(flog);
	flog = NULL;

	inited = FALSE;

	return SUCC;
}

int 
game_start(HINSTANCE instance)
{
	int r;

	if (!inited)
		return -E_PANIC;

	// initialize event queue
	ASSERT_POS(event_start(instance));

	// start the clock
	if (clock_start(instance) < 0)
	{
		event_post(ES_TERMINATE);
		event_stop();
		return -E_PANIC;
	}

	started = TRUE;
	return SUCC;
}

int
game_stop()
{
	if (!started)
	{
		dprintf("FAIL: game_stop() not started yet.\n");
		return -E_PANIC;
	}
	clock_stop();

	event_post(ES_TERMINATE);
	event_stop();
	
	started = FALSE;

	return SUCC;
}

int
game_wait()
{
	if (!started)
	{
//		dprintf("FAIL: game_wait() not started yet.\n");
		return SUCC;
	}

	event_wait();
	clock_stop();

	started = FALSE;

	return SUCC;
}

int
game_pause()
{
	if (!started)
	{
		dprintf("FAIL: game_pause() not started yet.\n");
		return -E_PANIC;
	}

	clock_pause();
	event_post(ES_PAUSE);

	return SUCC;
}

int
game_resume()
{
	if (!started)
	{
		dprintf("FAIL: game_pause() not started yet.\n");
		return -E_PANIC;
	}


	clock_resume();
	event_post(ES_RESUME);
	
	return SUCC;
}

void
dprintf(const char * format, ...)
{
	if (flog == NULL)
		return;

	va_list arg;
	va_start(arg, format);
	vfprintf(flog, format, arg);
	va_end(arg);
	fflush(flog);
}
