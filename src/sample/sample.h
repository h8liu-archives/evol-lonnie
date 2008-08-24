#include <evol/server.h>
#include <evol/sample/base.h>

void sample_init();
void sample_clean();
void sample_clock(unsigned int clock);
void sample_action(PID playerID, ACTION action);

void sample_ipRandom(PID playerID, unsigned int clock, const void * view,
		unsigned int offset, unsigned int size);

void sample_wininit(int nCmdShow);

void sample_gameinit();
void sample_gameclean();

int WINAPI WinMain(HINSTANCE hInstance, 
		HINSTANCE hPrevInstance, 
		LPSTR lpCmdLine,
		int nCmdShow);
