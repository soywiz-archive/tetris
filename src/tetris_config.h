#ifndef __TETRIS_CONFIG_H
#define __TETRIS_CONFIG_H

	// Configuración de la pantalla
	#define FPS            60
	#define SCREEN_BPP     32
	#define SCREEN_WIDTH  800
	#define SCREEN_HEIGHT 600

	#define P1_LEFT   SDLK_LEFT
	#define P1_RIGHT  SDLK_RIGHT
	#define P1_TURN   SDLK_UP
	#define P1_UP     SDLK_UP
	#define P1_DOWN   SDLK_DOWN
	#define P1_OK     SDLK_RETURN
	#define P1_CANCEL SDLK_ESCAPE

	#define P2_LEFT   SDLK_a
	#define P2_RIGHT  SDLK_d
	#define P2_TURN   SDLK_w
	#define P2_UP     SDLK_w
	#define P2_DOWN   SDLK_s
	#define P2_OK     SDLK_e
	#define P2_CANCEL SDLK_q

#endif
