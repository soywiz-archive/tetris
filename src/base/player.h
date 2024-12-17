#ifndef __PLAYER_H
#define __PLAYER_H

	#include "board.h"

	#define MAX_PLAYERS 4

	struct Player;

	typedef struct Player {
		Board board;
		int n;
		char name[0x40];
		int tick;
		int used;
	} Player;

#endif
