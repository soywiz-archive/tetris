#include "gamelib.h"
#include "gamelib_internal.h"
#include <math.h>
#include <SDL_syswm.h>

SDL_Surface *_screen;
char key[SDLK_LAST];
int  keyv[SDLK_LAST];
char keyset[120];
char RequestExit = 0;
int  AutoExitAtRequest = 1;
int  FPS = 60;
MouseStatus mouse;

int screenWidth = 0, screenHeight = 0;
int screenWidthReal = 0, screenHeightReal = 0;

void KeyboardInitialize() {
	int n = 1, v = 0;

	SDL_EnableUNICODE(1);

	memset(keyset, 0, sizeof(keyset));
	memset(key   , 0, sizeof(key   ));
	memset(keyv  , 0, sizeof(keyv  ));

	keyset[0] = 1;
	do {
		v = (int)(pow(n++, 0.46f) * 13);
		if (v < 120) keyset[v] = 1;
	} while (v < 120);
}

HICON icon;

void GameQuitEx(int rv) {
	DestroyIcon(icon);
	SDL_Quit();
	exit(rv);
}

void GameQuit() {
	GameQuitEx(0);
}

void GameInit() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) GamePrintFatalError();

	SDL_putenv("SDL_VIDEO_WINDOW_POS=center");
	SDL_putenv("SDL_VIDEO_CENTERED=1");

	if (TTF_Init() != 0) GamePrintFatalError();
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) GamePrintFatalError();
	if (SDLNet_Init() == -1) GamePrintFatalError();

	KeyboardInitialize();
}

#define keydownm 50
int keydown[keydownm + 1] = {0}, keydownp = 0;

void KeyboardUpdate() {
	SDL_Event event;
	int n;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN: keyv[event.key.keysym.sym]++; break;
			case SDL_KEYUP: key [event.key.keysym.sym] = keyv[event.key.keysym.sym] = 0; break;
			case SDL_QUIT:
				RequestExit = 1;
				if (AutoExitAtRequest) GameQuit();
			break;
		}
	}

	for (n = 0; n < SDLK_LAST; n++) {
		if (keyv[n] > 0 && keyv[n] < 120) {
			key[n] = (keyset[keyv[n] - 1]) ? 1 : 0;
		} else if (keyv[n] >= 120) {
			key[n] = 1;
		}

		if (keyv[n] > 0) keyv[n]++;
	}
}

void KeyboardSetDelay(int delay, int interval) {
	SDL_EnableKeyRepeat(delay, interval);
}

void VideoModeSetTitle(char *title) {
	SDL_WM_SetCaption(title, title);
}

void VideoModeSetEx(int widthScreen, int heightScreen, int widthDraw, int heightDraw, int windowed) {
	#ifdef WIN32
		HWND hwnd;
		struct zWMcursor { void* curs; };
		SDL_Cursor *cursor = SDL_GetCursor();

		HINSTANCE handle = GetModuleHandle(NULL);
		//((struct zWMcursor *)cursor->wm_cursor)->curs = (void *)LoadCursorA(NULL, IDC_ARROW);
		((struct zWMcursor *)cursor->wm_cursor)->curs = (void *)LoadCursorA(NULL, IDC_ARROW);
		SDL_SetCursor(cursor);

		icon = LoadIcon(handle, (char *)101);
		printf("ICON: %p\n", icon);
		SDL_SysWMinfo wminfo; SDL_GetWMInfo(&wminfo);
		hwnd = (HANDLE)wminfo.window;
		SetClassLong(hwnd, GCL_HICON, (LONG)icon);
	#endif

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 6);

	if (!(_screen = SDL_SetVideoMode(
		widthScreen,
		heightScreen,
		32,
		SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL | (windowed ? 0 : SDL_FULLSCREEN)
	))) {
    	GamePrintFatalError();
    }

    #ifdef WIN32
    	printf("ICON: %p <- %p\n", hwnd, icon);
    	SetClassLong(hwnd, GCL_HICON, (LONG)icon);
    #endif

	//glSetInteger(GL_LINE_SMOOTH, 1)
	glEnable(GL_LINE_SMOOTH);

    screenWidthReal = widthDraw; screenHeightReal = heightDraw;
    screenWidth = widthScreen; screenHeight = heightScreen;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	SDL_GL_SwapBuffers();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

void VideoModeSet(int width, int height, int windowed) {
	VideoModeSetEx(width, height, width, height, windowed);
}

void VideoModeClear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void VideoModeFrame() {
	KeyboardUpdate();
	SDL_Delay(1000 / FPS);

	/*
	while (1) {
		int dtime = abs(lasttime - SDL_GetTicks());
		if (dtime >= 1000 / FPS) break;
	}
	*/

	//if (lasttime)
	SDL_GL_SwapBuffers();
	VideoModeClear();
	//lasttime = SDL_GetTicks();
}

// TODO
void VideoModeEnable2D() {
	glViewport(0, 0, screenWidth, screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_TEXTURE   ); glLoadIdentity();

	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(0, screenWidthReal, screenHeightReal, 0, -1.0, 1.0);
	glTranslatef(0, 1, 0);

	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	glShadeModel(GL_SMOOTH);

	glEnable(GL_SCISSOR_TEST);

	glDisable(GL_TEXTURE_2D);
}

// TODO
void VideoModeDisable2D() {
}

// TODO
void VideoModeSetFPS(int fps) {
	FPS = fps;
}

// TODO
int VideoModeGetFPS() {
	return FPS;
}

void MouseShow() { SDL_ShowCursor(1); }
void MouseHide() { SDL_ShowCursor(0); }

__inline void ColorSet(float r, float g, float b, float a) {
	glColor4f(r, g, b, a);
}

Image ImageDuplicate(Image i) {
	Image r = malloc(sizeof(_Image));
	memcpy(r, i, sizeof(_Image));
	return r;
}

#undef main

int gamelib_main(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	GameInit();
	GameQuitEx(gamelib_main(argc, argv));
	return -1;
}
