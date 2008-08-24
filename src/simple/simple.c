#include <evol/player.h>
#include <evol/sample/base.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void ai_intro()
{
	printf("Hello, this is my first AI.\n");
}

void ai_callback(unsigned int clock, const void * view,
		unsigned int offset, unsigned int size)
{
	static int first = 1;
	static int counter = 0;
	ACTION a;

	if (first)
	{
		srand(time(0));
		first = 0;
	}

	// printf("CLOCK %d: call back here.\n", clock);

	counter++;
	if (counter < 1000)
		a = rand() % 4;
	else
		a = GSA_QUIT;

	// printf("ACTION %d\n", a);

	ai_action(a);
}
