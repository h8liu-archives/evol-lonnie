#include "server.h"

int player_remote_start(PID playerID, NET_IP ip, NET_PORT port)
{

	return SUCC;
}

void player_remote_send(PLAYER * p, EVENT e)
{
	// send TCP message

	if ((e & ET_MASK) == ET_VIEW)
	{
		// send updated info
	}
}

void player_remote_end(PLAYER * p)
{
	
}
