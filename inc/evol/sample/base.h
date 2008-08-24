#ifndef _EVOL_GAME_SAMPLE
#define _EVOL_GAME_SAMPLE

enum GSA_ACTION
{
	GSA_UP = 0,
	GSA_DOWN,
	GSA_LEFT,
	GSA_RIGHT,
	GSA_IDLE,
	GSA_QUIT,
};

#define SW 10 	// space width
#define SH 10 	// space height

#define B_BLANK		0xF0
#define B_WALL		0xF1
#define B_INVISIBLE	0xF2

typedef struct _GSA_VIEW
{
	unsigned char space[SH][SW];
} GSA_VIEW;

#endif // _EVOL_GAME_SAMPLE
