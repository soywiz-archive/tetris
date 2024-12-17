#ifndef __TETRIS_H
#define __TETRIS_H

	#include <math.h>
	#include <stdio.h>
	#include <string.h>
	#include <memory.h>
	#include <stdlib.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <SDL/SDL.h>
	#include <SDL/SDL_thread.h>
	#include <SDL/SDL_net.h>
	#include <SDL/SDL_mixer.h>
	#include <SDL/SDL_image.h>
	#include <SDL/SDL_ttf.h>

	#include "base/piece.h"
	#include "base/board.h"

	#include "base/player.h"

	#include "gamelib/gamelib.h"
	#include "gamelib/gamelib_rand.h"
	#include "gamelib/gamelib_font.h"
	#include "gamelib/gamelib_image.h"

	#include "tetris_menu.h"
	#include "tetris_common.h"
	#include "tetris_config.h"
	#include "tetris_score.h"

	#define PI 3.14159265f

	void wait_fps();
	void Quit(int r);

	extern int nplayers;
	extern Player players[4];
	extern int player1;

	extern Image image_single_player;
	extern Image image_multi_player;
	extern Image image_exit;
	extern Image image_create_game;
	extern Image image_join_game;
	extern Image image_ball;

	extern Image background, border, ilocal, iremote, iescape;
	extern Font font1, font2;

	extern SDL_Surface *screen;
	extern Mix_Music *bgmusic, *bgmusic2;
	extern Mix_Chunk *sndrotate, *sndlines, *sndlines4, *snddown, *sndgameover;

#endif
