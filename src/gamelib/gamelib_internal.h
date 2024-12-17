#ifndef __GAMELIB_INTERNAL_H
#define __GAMELIB_INTERNAL_H

void __checkTexRectangle();
void __checkPow2();
SDL_Surface *__SDL_CreateRGBSurfaceForOpenGL(int w, int h, int *rw, int *rh);
__inline int __NextPowerOfTwo(int v);
void GamePrintError();
void GamePrintFatalError();

#endif
