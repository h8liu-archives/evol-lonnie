#ifndef EVOL_H
#define EVOL_H

#define EVENT		unsigned int
#define PID		unsigned char
#define ACTION		unsigned char
#define EXPIRETIME	unsigned short

#define NET_IP		unsigned int
#define NET_PORT	unsigned short

#define PLAYER_MAX	64

// view size
#define VIEW_SIZE 4096

// event type
#define ET_MASK		0x00000003

#define	ET_CLOCK	0x1
#define ET_ACTION	0x3
#define ET_VIEW		0x2
#define ET_SYSTEM	0x0

#define ES_START	0x000
#define ES_END		0x510
#define ES_TERMINATE	0x520
#define ES_TIMEEND 	0x530
#define ES_PAUSE	0x100
#define ES_RESUME	0x110

#define ES_ISEND(event)\
	(((event & ET_MASK) == ET_SYSTEM) && ((event & 0xF00) == 0x500))

#define EVENT_FOR_PLAYER(event)\
	((event & ET_MASK) ^ ET_ACTION) // action is not for player

#define EVENT_FOR_CORE(event)\
	((event & ET_MASK) ^ ET_VIEW) // view is not for core

// player id
#define EP_MASK		0x000000FC
#define EP_OFFSET	2
#define EVENT_PLAYERID(event)\
	((event & EP_MASK) >> EP_OFFSET)

// action id
#define EA_ID_MASK	0x0000FF00
#define EA_ID_OFFSET	8
#define EVENT_ACTIONID(event)\
	((event & EA_ID_MASK) >> EA_ID_OFFSET)

// action expire time
#define EA_EXP_MASK	0xFFFF0000
#define EA_EXP_OFFSET	16
#define EVENT_ACTION_EXPIRE_TIME(event)\
	((event & EA_EXP_MASK) >> EA_EXP_OFFSET)
#define CLOCK_EXPIRE_MASK 	0xFFFF

#define EVENT_ACTIONNEW(pid, action, exp)\
	(((pid << EP_OFFSET) & EP_MASK)\
	 | ((action << EA_ID_OFFSET) & EA_ID_MASK)\
	 | ((exp << EA_EXP_OFFSET) & EA_EXP_MASK)\
	 | ET_ACTION)

// clock 
#define EC_MASK		0xFFFFFF00
#define EC_OFFSET	8
#define EVENT_CLOCK(event)\
	((event & EC_MASK) >> EC_OFFSET)

#define EVENT_CLOCKNEW(clock)\
	(((clock << EC_OFFSET) & EC_MASK) | ET_CLOCK)

// view update offset
#define EV_OFF_MASK	0xFFF00000
#define EV_OFF_OFFSET	20

#define EVENT_VIEW_OFF(event)\
	((event & EV_OFF_MASK) >> EV_OFF_OFFSET)

// view update size
#define EV_SIZE_MASK	0x000FFF00
#define EV_SIZE_OFFSET	8
#define EV_SIZE_UNIT	4

#define EVENT_VIEW_SIZE(event)\
	(((event & EV_SIZE_MASK) >> EV_SIZE_OFFSET) / EV_SIZE_UNIT)

#define EVENT_VIEWNEW(pid, offset, size)\
	(((offset << EV_OFF_OFFSET) & EV_OFF_MASK)\
	 | (((size / EV_SIZE_UNIT) << EV_SIZE_OFFSET) & EV_SIZE_MASK)\
	 | ((pid << EP_OFFSET) & EP_MASK)\
	 | ET_VIEW )

#define EXPIRE_RANGE (5 * 60 * 100 ) // 5 minutes

// Windows Message type for local communication
#define WM_LOCAL_PLAYER (WM_USER + 14) 
#define WM_MAGIC_LPARAM 1104

#define MAX_ACTION_PER_CLOCK 10

#endif // EVOL_H
