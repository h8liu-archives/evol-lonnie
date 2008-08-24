#include "sample.h"

extern HINSTANCE hCoreInst;

int WINAPI 
WinMain(HINSTANCE hInstance, 
		HINSTANCE hPrevInstance, 
		LPSTR lpCmdLine,
		int nCmdShow)
{
	hCoreInst = hInstance;

	clock_setElapse(5);
	game_init();

	core_setInitFunc(sample_init);
	core_setClockFunc(sample_clock);
	core_setActionFunc(sample_action);
	core_setCleanFunc(sample_clean);

	// add players here
	player_internal_start(0, sample_ipRandom);
	player_local_start(1, "simple.exe");

	sample_gameinit();
	sample_wininit(nCmdShow);

	game_start(hInstance);

	game_wait();
	game_clean();
	sample_gameclean();

	return 0;
}

