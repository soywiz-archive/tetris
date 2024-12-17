#ifndef __GAMELIB_H
#define __GAMELIB_H

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_net.h>

#ifdef WINDOWS
#include <windows.h>
#endif

#define bool int
#define false 0
#define true (!false)
#define null NULL

#undef main
#define main gamelib_main

typedef SDL_RWops* Stream;

#include "gamelib_image.h"
#include "gamelib_font.h"

extern SDL_Surface *_screen;
extern char key[SDLK_LAST];
extern int keyv[SDLK_LAST];
extern char RequestExit;
extern int  AutoExitAtRequest;
extern int  FPS;

extern int screenWidth, screenHeight;
extern int screenWidthReal, screenHeightReal;

typedef struct { int x, y, b, pb, rb; } MouseStatus;

extern MouseStatus mouse;

// Funciones de inicialización y finalización
void GameInit();
void GameQuit();

// Funciones relativas al teclado
void KeyboardUpdate();
void KeyboardSetDelay(int delay, int interval);

// Funciones relativas al modo gráfico
void VideoModeSetTitle(char *title);
void VideoModeSetEx(int widthScreen, int heightScreen, int widthDraw, int heightDraw, int windowed);
void VideoModeSet(int width, int height, int windowed);
void VideoModeFrame();
void VideoModeEnable2D();
void VideoModeDisable2D();
void VideoModeSetFPS(int fps);
int  VideoModeGetFPS();

void DrawClear();

void ColorSet(float r, float g, float b, float a);

#endif
