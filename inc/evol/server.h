#ifndef EVOL_SERVER_H
#define EVOL_SERVER_H

#include <evol/base.h>
#include <windows.h>

// Event list

#define EVENT_LIST_CAPACITY 1024
#define EVENT_LIST_MASK (EVENT_LIST_CAPACITY - 1)

typedef struct _EVENT_LIST
{
	EVENT events[EVENT_LIST_CAPACITY];
	int head, tail;
} EVENT_LIST;


#define LIST_INIT(lst) \
	(lst).head = (lst).tail = 0

#define LIST_ENQUEUE(lst, e) \
	(lst).events[((lst).tail++) & EVENT_LIST_MASK] = e;

#define LIST_DEQUEUE(lst) \
	(lst).events[((lst).head++) & EVENT_LIST_MASK];

#define LIST_EMPTY(lst) \
	((((lst).head ^ (lst).tail) & EVENT_LIST_MASK) == 0)

#define LIST_FULL(lst) \
	(((((lst).tail + 1) ^ (lst).tail) & EVENT_LIST_MASK) == 0)

// Internal callback

typedef void (*PLAYER_CALLBACK) (
		PID playerID,
		unsigned int clock,
		const void * view, 
		unsigned int offset,
		unsigned int size
		);


// Global variables

#define E_PANIC 1
#define SUCC 0

#define DEFAULT_CLOCK_ELAPSE 10		// 10 CLOCK_UNIT
#define MAX_CLOCK_ELAPSE 500
#define MIN_CLOCK_ELAPSE 3

#define CLOCK_TIME_END (1000 * 60 * 60 * 4) // 4 hours

#define CLOCK_UNIT 10		// 10 ms


// APIs

int game_init();
int game_clean();

int game_start(HINSTANCE instance);
int game_pause();
int game_resume();
int game_stop();
int game_wait();

int player_local_start(PID playerID, const char * commandLine);

int player_remote_start(PID playerID, NET_IP ip, NET_PORT port);

int player_internal_start(PID playerID, PLAYER_CALLBACK callback);
int player_internal_action(PID playerID, ACTION action);

int core_viewRead(PID playerID, unsigned int offset,
		void * dest, unsigned int size);
int core_viewWrite(PID playerID, unsigned int offset, 
		void * src, unsigned int size);
int core_exit();


typedef void (*CORE_INIT) (void);
typedef void (*CORE_CLOCK) (unsigned int clock);
typedef void (*CORE_ACTION) (PID playerID, ACTION action);
typedef void (*CORE_CLEAN) (void);

void core_setInitFunc(CORE_INIT init);
void core_setClockFunc(CORE_CLOCK clock);
void core_setActionFunc(CORE_ACTION action);
void core_setCleanFunc(CORE_CLEAN clean);

// DO NOT set elapse when the game is running
void clock_setElapse(unsigned int times10ms);
unsigned int clock_getElapse();

void dprintf(const char * format, ...);

#endif // EVOL_SERVER_H

