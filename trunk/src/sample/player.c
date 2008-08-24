#include "sample.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

GSA_VIEW internalView;

// Random move player
void 
sample_ipRandom(PID playerID, unsigned int clock, const void * view, 
		unsigned int offset, unsigned int size)
{
	memcpy(&internalView, view, sizeof(GSA_VIEW));
}

