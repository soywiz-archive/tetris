#include "tetris.h"

//#define SOUND_MUTE 1

#ifdef WIN32
	#include <windows.h>
#endif

// Jugadores
int nplayers;
Player players[4];

// Nº de jugador local
int player1 = -1, player2 = -1;

// Estado de conexión con el servidor
int server_connected_status = 0;

// Imagenes utilizadas en el juego
Image image_single_player;
Image image_multi_player;
Image image_exit;
Image image_ranking;
Image image_ball;

Image background, border, ilocal, iremote, iescape;

// Fuentes
Font font1, font2;

// Socket para conexión al servidor
TCPsocket server;

// Indica si el cliente debe parar
int client_stopping = 0;

// SDL_Surface con la ventana
SDL_Surface *screen;

// Musicas de fondo utilizadas en el juego
Mix_Music *bgmusic, *bgmusic2, *bgmusic3;

// Sonidos utilizados en el juego
Mix_Chunk *sndrotate, *sndlines, *sndlines4, *snddown, *sndgameover;

// Dirección del servidor
char serverip[50];

// DisplayLists
GLuint glbox, glboardboard;
int done = 0;

Mix_Chunk *LoadSound(const char* name) {
	Mix_Chunk *r;
	fprintf(stderr, "Cargando sonido '%s'...", name);
	if ((r = Mix_LoadWAV(name)) == NULL) {
		fprintf(stderr, "ERROR\n");
		exit(-1);
	}
	fprintf(stderr, "Ok\n");
	return r;
}

Mix_Music *LoadMusic(const char* name) {
	Mix_Music *r;
	printf("Cargando musica '%s'...", name);
	if ((r = Mix_LoadMUS(name)) == NULL) {
		printf("ERROR\n");
		exit(-1);
	}
	printf("Ok\n");
	return r;
}

// Colores utilizados para los bloques
float colors[][3] = {
	{0.0f, 0.0f, 1.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{1.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 1.0f},
	{1.0f, 0.0f, 1.0f},
	{0.5f, 0.0f, 1.0f},
	{1.0f, 0.5f, 0.0f},
	{1.0f, 0.0f, 1.0f},
};

int levels[10] = { 60, 50, 45, 40, 28, 23, 15, 11, 8, 5 };
int linestolevel[10] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 0 };

// Finalizamos el sistema
void Quit(int r) {
	TTF_Quit();
    SDL_Quit();
    exit(r);
}

// Esperamos un Frame
void wait_fps() {
	static unsigned int currentrender = 0, lastrender = 0, nextrender = 0;

	currentrender = SDL_GetTicks();

	if (currentrender >= lastrender && currentrender < nextrender && nextrender - currentrender < FPS) {
		SDL_Delay(nextrender - currentrender);
	}

	lastrender = SDL_GetTicks();
	nextrender = lastrender + 1000 / FPS;
}

// Iniciamos ciertos aspectos de OpenGL
int init_gl() {
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST); glClearDepth(1.0f);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
	glDepthRange(20.0, -20.0);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    return 1;
}

void load_resources_draw_text(float alpha) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawFontSimpleShadow(font1, "Cargando recursos...", SCREEN_WIDTH / 2 - FontWidth(font1, "Cargando recursos...") / 2 - 16, SCREEN_HEIGHT / 2 - font1->h / 2, 1, 0, 0, alpha);
	SDL_GL_SwapBuffers();
}

void load_resources_show_loading(float alpha) {
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0); glTranslatef(0, 1, 0);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	load_resources_draw_text(alpha);
}

void load_resources_fadeout() {
	float alpha = 1;

	while (alpha > 0) {
		load_resources_draw_text(alpha);
		alpha -= 0.1; SDL_Delay(1000 / 60);
	}
}

// Cargamos los recursos utilizados en el juego
int load_resources() {
	printf("Cargando fuentes...\n");

	// Carga de fuentes
	font1 = FontLoadFromFile("data/fonts/font1.ttf", 58);

	load_resources_show_loading(1.0f);

	font2 = FontLoadFromFile("data/fonts/font2.ttf", 36);

#if SOUND_MUTE
	bgmusic3 = bgmusic2 = bgmusic = NULL;
	sndlines4 = sndrotate = snddown = sndgameover = sndlines = NULL;
#else
	// Carga las canciones de fondo
	printf("Cargando musicas de fondo...\n");
	bgmusic  = LoadMusic("data/bgm/music1.mod");
	bgmusic2 = LoadMusic("data/bgm/music2.mod");
	bgmusic3 = LoadMusic("data/bgm/music3.mod");

	// Carga de sonidos
	printf("Cargando efectos de sonido...\n");
	sndlines4   = LoadSound("data/se/lines4.wav");
	sndrotate   = LoadSound("data/se/rotate.wav");
	snddown     = LoadSound("data/se/down.wav");
	sndlines    = LoadSound("data/se/lines.wav");
	sndgameover = LoadSound("data/se/gameover.wav");
#endif

	// Carga de imágenes
	printf("Cargando imagenes...\n");
	background  = ImageLoadFromFile("data/background.jpg");
	border      = ImageLoadFromFile("data/border.png");
	ilocal      = ImageLoadFromFile("data/local.png");
	iremote     = ImageLoadFromFile("data/remote.png");
	iescape     = ImageLoadFromFile("data/escape.png");

	// Carga de
	printf("Cargando opciones...\n");
	image_single_player = ImageLoadFromFile("data/options/1p.png");
	image_multi_player  = ImageLoadFromFile("data/options/2p.png");
	image_ranking       = ImageLoadFromFile("data/options/ranking.png");
	image_exit          = ImageLoadFromFile("data/options/exit.png");

	image_ball = ImageLoadFromFile("data/ball.png");
	image_ball->cx = image_ball->w / 2;
	image_ball->cy = image_ball->h / 2;

	printf("Recursos cargados satisfactoriamente...\n");
	load_resources_fadeout();

	return 0;
}

// Compilamos instrucciones opengl para acceder a ellas posteriormente
// de una forma mucho mas eficiente.
void prepareRenderObjects() {
	int x, y;
	float a, b;

	// Preparamos una display list para renderizar un cubo
	glbox = glGenLists(1);
	glNewList(glbox, GL_COMPILE);

		glBindTexture(GL_TEXTURE_2D, 0);

		glBegin(GL_QUADS);
			glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); glVertex3f(1, 1, 0); glVertex3f(0, 1, 0);
			glVertex3f(0, 0, 1); glVertex3f(1, 0, 1); glVertex3f(1, 1, 1); glVertex3f(0, 1, 1);
			glVertex3f(0, 0, 0); glVertex3f(0, 1, 0); glVertex3f(0, 1, 1); glVertex3f(0, 0, 1);
			glVertex3f(1, 0, 0); glVertex3f(1, 1, 0); glVertex3f(1, 1, 1); glVertex3f(1, 0, 1);
			glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); glVertex3f(1, 0, 1); glVertex3f(0, 0, 1);
			glVertex3f(0, 1, 0); glVertex3f(1, 1, 0); glVertex3f(1, 1, 1); glVertex3f(0, 1, 1);
		glEnd();

		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

		glBegin(GL_LINES);
			for (a = 0; a <= 1; a++) for (b = 0; b <= 1; b++) {
				glVertex3i(a, b, 0); glVertex3i(a, b, 1);
				glVertex3i(0, a, b); glVertex3i(1, a, b);
				glVertex3i(a, 0, b); glVertex3i(a, 1, b);
			}
		glEnd();

	glEndList();

	// Preparamos una display list para renderizar un tablero
	glboardboard = glGenLists(1);
	glNewList(glboardboard, GL_COMPILE);

		glBindTexture(GL_TEXTURE_2D, 0);

		glColor4f(0.0f, 0.0f, 0.0f, 0.72f);
		glBegin(GL_QUADS);
			glVertex3i(0, 0, 0);
			glVertex3i(BOARD_WIDTH, 0, 0);
			glVertex3i(BOARD_WIDTH, BOARD_HEIGHT, 0);
			glVertex3i(0, BOARD_HEIGHT, 0);
		glEnd();

		glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
		glBegin(GL_QUADS);
			glVertex3i(0, 0, 0);
			glVertex3i(0, 0, 1);
			glVertex3i(0, BOARD_HEIGHT, 1);
			glVertex3i(0, BOARD_HEIGHT, 0);

			glVertex3i(BOARD_WIDTH, 0, 0);
			glVertex3i(BOARD_WIDTH, 0, 1);
			glVertex3i(BOARD_WIDTH, BOARD_HEIGHT, 1);
			glVertex3i(BOARD_WIDTH, BOARD_HEIGHT, 0);

			glVertex3i(0, 0, 0);
			glVertex3i(BOARD_WIDTH, 0, 0);
			glVertex3i(BOARD_WIDTH, 0, 1);
			glVertex3i(0, 0, 1);
		glEnd();

		glBegin(GL_LINES);
			glColor4f(1.0f, 1.0f, 1.0f, 0.2f);

			for (y = 0; y <= BOARD_HEIGHT; y++) {
				glVertex3i(0, y, 0);
				glVertex3i(BOARD_WIDTH, y, 0);

				glVertex3i(0, y, 0);
				glVertex3i(0, y, 1);
				glVertex3i(BOARD_WIDTH, y, 0);
				glVertex3i(BOARD_WIDTH, y, 1);
			}

			glVertex3i(0, 0, 1);
			glVertex3i(0, BOARD_HEIGHT, 1);

			glVertex3i(BOARD_WIDTH, 0, 1);
			glVertex3i(BOARD_WIDTH, BOARD_HEIGHT, 1);

			glVertex3i(0, 0, 1);
			glVertex3i(BOARD_WIDTH, 0, 1);

			for (x = 0; x <= BOARD_WIDTH; x++) {
				glVertex3i(x, 0, 0);
				glVertex3i(x, BOARD_HEIGHT, 0);
				glVertex3i(x, 0, 0);
				glVertex3i(x, 0, 1);
			}

    	glEnd();

	glEndList();
}

// Pintamos un cubo en las coordenadas correspondientes
void drawCubeAt(int x, int y, int c, int a, char mark) {
	glPushMatrix();
		glTranslatef((float)x, (float)y, 0.0f);
		if (mark) {
			glColor4f(1.0f, 1.0f, 1.0f, 0.15f);
			//glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
		} else {
			glColor4f(colors[c][0], colors[c][1], colors[c][2], a ? 0.62f : 0.32f);
			//glColor4f(colors[c][0], colors[c][1], colors[c][2], a ? 1.0f : 0.8f);
		}
		glCallList(glbox);
    glPopMatrix();
}

// Renderizamos el tablero de un jugador
int drawBoardScene(Player *p, int n) {
	char temp[1000];
	RealPiece *rp;
    int x, y; char mark, c;
    Board *b = &p->board;

	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0); glTranslatef(0, 1, 0);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	DrawImageEx(border, 0, 0, 1, 0, 1);

    glPushMatrix();
    	glPushAttrib(GL_TRANSFORM_BIT);
			glMatrixMode(GL_PROJECTION); glLoadIdentity();
			gluPerspective(76.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.0f, 1.0f);
		glPopAttrib();

		glLoadIdentity();

		glTranslatef(-BOARD_WIDTH / 2, -10, -14.0f);

		glCallList(glboardboard);

		//if (b->playing) {
			//glDisable(GL_BLEND);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			for (y = 0; y < BOARD_HEIGHT / 2; y++) {
				mark = ((p->tick % 20 <= 10)) && (b->fline[y] && ((n % 2) == 0));
				for (x = 0; x < BOARD_WIDTH / 2; x++) if ((c = b->data[x][y]) != 0) drawCubeAt(x, y, c, 1, mark);
				for (x = BOARD_WIDTH - 1; x >= BOARD_WIDTH / 2; x--) if ((c = b->data[x][y]) != 0) drawCubeAt(x, y, c, 1, mark);
			}

			for (y = BOARD_HEIGHT - 1; y >= BOARD_HEIGHT / 2; y--) {
				mark = ((p->tick % 20 <= 10)) && (b->fline[y] && ((n % 2) == 0));
				for (x = 0; x < BOARD_WIDTH / 2; x++) if ((c = b->data[x][y]) != 0) drawCubeAt(x, y, c, 1, mark);
				for (x = BOARD_WIDTH - 1; x >= BOARD_WIDTH / 2; x--) if ((c = b->data[x][y]) != 0) drawCubeAt(x, y, c, 1, mark);
			}

			if (b->playing) {
				rp = &b->current;
				if (rp->y > -10) {
					for (y = 0; y < 4; y++) for (x = 0; x < 4; x++) if ((c = pieceList[rp->npiece].data[rp->rotation][x][y]) != 0) drawCubeAt(rp->x + x, rp->y + y, c, 0, 0);
				}

				rp = &b->next;
				for (y = 0; y < 4; y++) for (x = 0; x < 4; x++) if ((c = pieceList[rp->npiece].data[rp->rotation][x][y]) != 0) drawCubeAt(BOARD_WIDTH + x + 2, y + BOARD_HEIGHT - 5, c, 0, 0);
			}
			//glEnable(GL_BLEND);
		//}

		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0); glTranslatef(0, 1, 0);
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();

		if (!b->playing) {
			if (p->tick % 70 <= 40) {
				char *text = "Game Over";
				DrawFontSimpleShadow(font1, text, SCREEN_WIDTH / 2 - FontWidth(font1, text) / 2, SCREEN_HEIGHT / 2 - font2->h / 2 - 16, 1, 1, 0.6, 1);
			}
		}

		sprintf(temp, "Jugador %d", p->n + 1); DrawFontSimpleShadow(font2, temp, 32, 32, 1, 1, 1, 1);
		DrawFontSimpleShadow(font2, p->name, 32, 64, 1, 1, 1, 1);

		DrawFontSimpleShadow(font2, "Nivel", 48, 128 + 24, 1, 1, 0.6, 1);
		sprintf(temp, "%d", b->level); DrawFontSimpleShadow(font1, temp, 48 + 16, 164 + 24, 1, 1, 1, 1);

		DrawFontSimpleShadow(font2, "Puntos", 48, 228 + 24, 0.6, 1, 0.6, 1);
		sprintf(temp, "%d", (int)b->points); DrawFontSimpleShadow(font1, temp, 48 + 16, 264 + 24, 1, 1, 1, 1);

		DrawFontSimpleShadow(font2, "Lineas", 48, 328 + 24, 1, 0.6, 1, 1);
		sprintf(temp, "%d", b->lines); DrawFontSimpleShadow(font1, temp, 48 + 16, 364 + 24, 1, 1, 1, 1);

		if (b->frozen && p->tick >= 10 * 6) b->frozen = 0;

		if (p->used) {
			float alpha = 0.65 + fabs(cos((float)(SDL_GetTicks() % 1300) * PI / 1300)) * 0.35;

			if (player1 == p->n) {
				DrawImageEx(ilocal, SCREEN_WIDTH - ilocal->w, SCREEN_HEIGHT - ilocal->h, 1, 0, alpha);
			} else {
				DrawImageEx(iremote, SCREEN_WIDTH - iremote->w, SCREEN_HEIGHT - iremote->h, 1, 0, alpha);
			}
		}

	glPopMatrix();

	p->tick++;

	return 0;
}

// Renderizamos el layout para un jugador
int drawBoard1(int n) {
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	drawBoardScene(&players[0], n);

	return 0;
}

// Renderizamos el layout para un jugador
int drawBoard2(int n) {
	glColor4f(0, 0, 0, 1.0f);
	glBegin(GL_LINES);
		glVertex2i(SCREEN_WIDTH / 2, 0);
		glVertex2i(SCREEN_WIDTH / 2, SCREEN_HEIGHT);
	glEnd();

	glViewport(0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT);
	drawBoardScene(&players[0], n);

	glViewport(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT);
	drawBoardScene(&players[1], n);

	glViewport(0 , 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	return 0;
}

// Renderizamos el layout para hasta cuatro jugadores
int drawBoard4(int n) {
	glColor4f(0, 0, 0, 1.0f);
	glBegin(GL_LINES);
		glVertex2i(0, SCREEN_HEIGHT / 2);
		glVertex2i(SCREEN_WIDTH, SCREEN_HEIGHT / 2);
		glVertex2i(SCREEN_WIDTH / 2, 0);
		glVertex2i(SCREEN_WIDTH / 2, SCREEN_HEIGHT);
	glEnd();

	glViewport(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	drawBoardScene(&players[0], n);

	glViewport(0 + SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	drawBoardScene(&players[1], n);

	glViewport(0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	drawBoardScene(&players[2], n);

	glViewport(0 + SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	drawBoardScene(&players[3], n);

	glViewport(0 , 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	return 0;
}

// Renderizamos la escena
int drawScene(int n, float alpha) {
	// Toda la pantalla
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_TEXTURE   ); glLoadIdentity();

	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
	glTranslatef(0, 1, 0);

	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_TEXTURE_2D);

	DrawImageEx(background, 0, 0, 1, 0, 1);

	switch (nplayers) {
		case 2:default:
			drawBoard4(n);
		break;
		//case 2:
		//	drawBoard2(n);
		break;
		case 1:
			drawBoard1(n);
		break;
	}

	showFadeBalls(alpha);

    SDL_GL_SwapBuffers();

    return 1;
}

// Movemos la ficha hacia abajo
int movePieceDownReal(Player *p, int npiece) {
	Board *b = &p->board;
	p->tick = 0;

	if (!movePieceDown(b)) {
		Mix_PlayChannel(-1, snddown, 0);

		int clines;

		if (p->board.current.y > -10) {
			if (!putPiece(b)) {
				Mix_FadeOutMusic(2000);
				Mix_PlayChannel(-1, sndgameover, 0);
				b->playing = 0;
				return -2;
			}
		}

		if ((clines = checkLines(b)) != 0) {
			Mix_PlayChannel(-1, (clines >= 4) ? sndlines4 : sndlines, 0);
			b->frozen = 1;
		} else {
			setNextPiece(&p->board, npiece);
		}

		return -1;
	}

	return 0;
}

// Hacemos la parte de gestión de la parte principal del juego
int moveScenePlayer(Player *p) {
	int npiece, moveDown = 0, moveDownType = 0;
	Board  *b = &p->board;

	if (b->frozen  ) return -1;

	if (!b->playing) {
		int pos, zpos = p->tick * 2 - 2;
		for (pos = zpos; pos <= zpos + 1; pos++) {
			//printf("%d\n", p->tick);
			if (pos < BOARD_WIDTH * BOARD_HEIGHT) {
				int x, y;
				x = pos % BOARD_WIDTH;
				y = pos / BOARD_WIDTH;
				if (y % 2 == 0) x = BOARD_WIDTH - x - 1;
				b->data[x][y] = 1;
			}
		}

		if (zpos >= BOARD_WIDTH * BOARD_HEIGHT * 2.3) {
			return -3;
		}

		return -2;
	}

	if (player1 < 0) return -3;

	if (b->flines > 0) {
		b->points += 10 * pow(2, b->flines) * sqrt(b->level + 1);
		b->lines  += b->flines;
		b->llines += b->flines;

		if (b->llines >= linestolevel[b->level]) {
			b->level++;
			if (b->level > 9) b->level = 9;
			b->llines = 0;
		}

		deleteLines(b);

		npiece = genrand_int32() % pieceListCount;
		setNextPiece(b, npiece);

		if (keyv[SDLK_DOWN] > 0) keyv[SDLK_DOWN] >>= 2;
	}

	if (p->n == player1) {
		if (key[P1_LEFT] ) movePieceLeft(b);
		if (key[P1_RIGHT]) movePieceRight(b);

		if (key[P1_TURN]) {
			Mix_PlayChannel(-1, sndrotate, 0);
			rotatePieceRight(b);
		}

		if (key[P1_DOWN]) {
			moveDown = 1;
			b->points += 0.03 * sqrt(b->level + 1);
			moveDownType = 1;
		}
	} else {
		if (key[P2_LEFT] ) movePieceLeft(b);
		if (key[P2_RIGHT]) movePieceRight(b);

		if (key[P2_TURN]) {
			Mix_PlayChannel(-1, sndrotate, 0);
			rotatePieceRight(b);
		}

		if (key[P2_DOWN]) {
			moveDown = 1;
			b->points += 0.03 * sqrt(b->level + 1);
			moveDownType = 1;
		}
	}

	if (p->tick % levels[b->level] == 0) {
		moveDown = 1;
		moveDownType = 2;
	}

	if (moveDown) {
		int npiece = genrand_int32() % pieceListCount;

		if (movePieceDownReal(p, npiece)) {
			if (keyv[SDLK_DOWN] > 0) keyv[SDLK_DOWN] >>= 1;
		}
		p->tick = 0;
	}

	return 0;
}

int moveScene() {
	int a = -3, b = -3;
	if (player1 != -1) {
		a = moveScenePlayer(&players[player1]);
	}
	if (player2 != -1) {
		b = moveScenePlayer(&players[player2]);
	}
	return ((a == b) && (a == -3)) ? -3 : 0;
}

float minf(float a, float b) { return (a < b) ? a : b; }

// MENÚ: Pantalla de ranking
#define SPLIT_BG_IMAGE 20
int action_ranking() {
	Image background2;
	Image option;
	Image bgs[SPLIT_BG_IMAGE];
	Font font;

	float alpha = 0;
	int tick = 0, tack = 0;
	int n, ysub, xsub;
	int status = 0;

	printf("action_ranking()\n");

	Mix_FadeInMusic(bgmusic3, -1, 3000);

	background2 = ImageLoadFromFile("data/backgroundranking.jpg");
	option = ImageLoadFromFile("data/highoption.png");
	font = FontLoadFromFile("data/fonts/font3.ttf", 44);
	option->cx = option->w / 2;
	option->cy = option->h / 2;

	ysub = (background2->h / SPLIT_BG_IMAGE);
	xsub = background2->w;

	for (n = 0; n < SPLIT_BG_IMAGE; n++) {
		bgs[n] = ImageCreateFromSubImage(background2, 0, ysub * n, background2->w, ysub);
	}

	glMatrixMode(GL_TEXTURE   ); glLoadIdentity();

	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
	glTranslatef(0, 1, 0);

	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_TEXTURE_2D);

    while (1) {
		KeyboardUpdate();

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (key[P1_CANCEL]) return -1;

		if (tick >= 60 * 23.62) break;

		switch (status) {
			case 0: {
				int ok = 1;
				for (n = 0; n < SPLIT_BG_IMAGE; n++) {
					float angle, alpha;
					int disp;
					angle = ((float)tick * (M_PI / 2)) / (60 * 2.1) - (float)n * 0.159;
					if (angle < 0) angle = 0;
					if (angle >= (M_PI / 2)) angle = (M_PI / 2);
					disp = sin(angle) * (xsub + 1);
					alpha = minf(1, sin(angle) * 1.3);
					if (n % 2) {
						DrawImageEx(bgs[n], -xsub + disp, ysub * n, 1, 0, alpha);
					} else {
						DrawImageEx(bgs[n], xsub - disp, ysub * n, 1, 0, alpha);
					}
					if (disp < xsub) ok = 0;
				}
				if (ok) status = 1;
			} break;
			case 1: {
				int n;

				DrawImageEx(background2, 0, 0, 1, 0, 1);

				for (n = 0; n < 6; n++) {
					int w, h;
					float maxalpha;
					float alpha, angle;
					float maxsize;
					char cpoints[128];
					char *name;
					int y, yp;
					yp = (SCREEN_WIDTH / 2 - (6 * option->h) / 2) + n * option->h - 44;

					if (n == 0) maxsize = 1.2; else if (n < 3) maxsize = 1.0; else maxsize = 0.9;
					if (n < 3) maxalpha = 1; else maxalpha = 0.6;

					sprintf(cpoints, "%d", score_points[n]);
					name = score_names[n];

					w = option->w * maxsize;
					h = option->h * maxsize;

					if (n >= 3) {
						angle = (((float)tack - (pow(1.4, (float)(6 - n + 2)) / 2) * 14) * (M_PI / 2)) / 64;
					} else {
						angle = (((float)tack - 132 - (pow(1.43, (float)(6 - n + 2)) / 2) * 14) * (M_PI / 2)) / 64;
					}
					//angle = ((float)tack * (M_PI / 2)) / 100;

					//printf("%f", angle);

					if (angle < 0) angle = 0;
					if (angle > (M_PI / 2)) angle = (M_PI / 2);

					alpha = sin(angle);
					y = yp + 50 - sin(angle) * 50;

					alpha *= maxalpha;


					DrawImageEx(option, 400, y, maxsize, 0, alpha);

					DrawFontSimpleShadow(font1, cpoints, 400 - w / 2 + w * 0.2, y + h / 2 - font1->h - h * 0.1, 1, 0.7, 0.7, 0.5 * alpha);
					DrawFontSimpleShadow(font, name, 400 + w / 2 - w * 0.2 - FontWidth(font, name), y - font->h / 2, 1, 1, 0.7, 0.7 * alpha);
				}

				tack++;
			} break;
		}

		showFadeBalls(alpha);

	    SDL_GL_SwapBuffers();

		if (alpha < 1) alpha += 0.1; else alpha = 1;

		wait_fps();
		tick++;
	}

	while (alpha >= 0) {
		KeyboardUpdate();

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawImageEx(background2, 0, 0, 1, 0, 1);
		showFadeBalls(alpha);
		SDL_GL_SwapBuffers();

		alpha -= 0.03;
	}

	ImageFree(background2);
	ImageFree(option);
	FontFree(font);

	return 0;
}

int action_submit_score(Player *p) {
	char charset[] = "ABDEFGHIJKLMNOPQRSTUVWXYZ ";
	int tick = 0;
	float alpha = 0;
	char title[0x100];
	char name[0x10], rname[0x10];
	Font font;
	char temp[10];
	int cchar = 0;
	Image background2;
	rname[0] = rname[1] = rname[2] = 0;
	//if (!score_better(p->board.points)) return 0;

	background2 = ImageLoadFromFile("data/backgroundranking.jpg");
	font = FontLoadFromFile("data/fonts/font3.ttf", 60);

	sprintf(title, "El jugador '%s' ha batido un nuevo record:", p->name);

    while (1) {
		KeyboardUpdate();

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawImage(background2, 0, 0);

		if (p->n == player2) {
			if (key[P2_LEFT] && cchar > 0) { cchar--; tick = 0; }
			if (key[P2_RIGHT] && cchar < 2) { cchar++; tick = 0; }
			if (key[P2_DOWN]) {
				if (rname[cchar] == 0) rname[cchar] = strlen(charset) - 2; else rname[cchar]--;
			}
			if (key[P2_UP]) {
				if (rname[cchar] >= strlen(charset) - 2) rname[cchar] = 0; else rname[cchar]++;
			}
			if (key[P2_CANCEL]) break;
			if (key[P2_OK]) break;
		} else {
			if (key[P1_LEFT] && cchar > 0) { cchar--; tick = 0; }
			if (key[P1_RIGHT] && cchar < 2) { cchar++; tick = 0; }
			if (key[P1_DOWN]) {
				if (rname[cchar] == 0) rname[cchar] = strlen(charset) - 2; else rname[cchar]--;
			}
			if (key[P1_UP]) {
				if (rname[cchar] >= strlen(charset) - 2) rname[cchar] = 0; else rname[cchar]++;
			}
			if (key[P1_CANCEL]) break;
			if (key[P1_OK]) break;
		}

		memcpy(name, rname, 3);
		if (tick / 8 % 6 == 0) name[cchar] = strlen(charset) - 1;

		DrawFontSimpleShadow(font2, title, SCREEN_WIDTH / 2 - FontWidth(font2, title) / 2, 40, 1, 1, 1, 1);
		sprintf(temp, "%c", charset[(int)name[0]]); DrawFontSimpleShadow(font, temp, SCREEN_WIDTH / 2 - 40 * 3 / 2 + 70 * 0 - FontWidth(font, temp) / 2, SCREEN_HEIGHT / 2 - font->h / 2 + 12, 1, 1, 1, 1);
		sprintf(temp, "%c", charset[(int)name[1]]); DrawFontSimpleShadow(font, temp, SCREEN_WIDTH / 2 - 40 * 3 / 2 + 70 * 1 - FontWidth(font, temp) / 2, SCREEN_HEIGHT / 2 - font->h / 2 + 12, 1, 1, 1, 1);
		sprintf(temp, "%c", charset[(int)name[2]]); DrawFontSimpleShadow(font, temp, SCREEN_WIDTH / 2 - 40 * 3 / 2 + 70 * 2 - FontWidth(font, temp) / 2, SCREEN_HEIGHT / 2 - font->h / 2 + 12, 1, 1, 1, 1);

		showFadeBalls(alpha);

	    SDL_GL_SwapBuffers();

		if (alpha < 1) alpha += 0.1; else alpha = 1;

		wait_fps();
		tick++;
	}

	temp[0] = charset[(int)name[0]];
	temp[1] = charset[(int)name[1]];
	temp[2] = charset[(int)name[2]];
	temp[3] = 0;

	score_submit(temp, p->board.points);

	ImageFree(background2);
	FontFree(font);

	return 0;
}

// JUEGO: 1 Jugador
int action_game_single() {
	float alpha = 0;
	player1 = 0;
	nplayers = 1;

	printf("action_game_single()\n");

	initBoard(&players[0].board, -1, -1);
	sprintf(players[player1].name, "Jugador");
	players[player1].n  = player1;
	players[player1].tick = 0;
	players[player1].used = 1;
	players[player1].board.playing = 1;
	players[player1].board.frozen = 0;

	Mix_FadeInMusic(bgmusic, -1, 3000);

    while (1) {
		KeyboardUpdate();

		if (key[P1_CANCEL]) return -1;

		if (moveScene() == -3) break;
		drawScene(0, alpha);
		if (alpha < 1) alpha += 0.1; else alpha = 1;

		wait_fps();
	}

	if (score_better(players[player1].board.points)) {
		action_submit_score(&players[player1]);
	}

	action_ranking();

	return 0;
}

// JUEGO: 2 Jugador
int action_game_double() {
	float alpha = 0;
	player1 = 0;
	player2 = 1;
	nplayers = 2;

	printf("action_game_double()\n");

	initBoard(&players[0].board, -1, -1);
	sprintf(players[player1].name, "Jugador");
	players[player1].n  = player1;
	players[player1].tick = 0;
	players[player1].used = 1;
	players[player1].board.playing = 1;
	players[player1].board.frozen = 0;

	initBoard(&players[1].board, -1, -1);
	sprintf(players[player2].name, "Jugador");
	players[player2].n = player2;
	players[player2].tick = 0;
	players[player2].used = 1;
	players[player2].board.playing = 1;
	players[player2].board.frozen = 0;

	Mix_FadeInMusic(bgmusic, -1, 3000);

	printf("%d, %d\n", players[player1].board.playing, players[player2].board.playing);

    while (1) {
		KeyboardUpdate();

		if (key[P1_CANCEL]) return -1;

		if (moveScene() == -3) break;
		drawScene(0, alpha);
		if (alpha < 1) alpha += 0.1; else alpha = 1;

		wait_fps();
	}

	if (score_better(players[player1].board.points)) {
		action_submit_score(&players[player1]);
	}

	if (score_better(players[player2].board.points)) {
		action_submit_score(&players[player2]);
	}

	action_ranking();

	return 0;
}


// MENÚ: Pantalla principal
int action_main() {
	int changedmusic = 1;
	int option = 0;

	printf("action_main()...\n");

	//action_submit_score(&players[player1]);

	while (1) {
		if (changedmusic) {
			Mix_FadeOutMusic(300);
			Mix_PlayMusic(bgmusic2, -1);
			changedmusic = 0;
		}

		action_clear_options();
			action_add_option(0, image_single_player);
			action_add_option(1, image_multi_player);
			action_add_option(2, image_ranking);
			action_add_option(3, image_exit);
		option = action_select_option(option, 1);

		switch (option) {
			case 0: // 1 Player
				Mix_FadeOutMusic(400);
				action_game_single();
				changedmusic = 1;
			break;
			case 1: // 2 Players
				Mix_FadeOutMusic(400);
				action_game_double();
				changedmusic = 1;
			break;
			case 2: // Ranking
				Mix_FadeOutMusic(400);
				action_ranking();
				changedmusic = 1;
			break;
			default: case -1: case 3: // ESC/Exit
				Quit(-999);
			break;
		}
	}
	return 0;
}

// Inicio del programa
int main(int argc, char **argv) {
	// Parche para windows para abrir la consola
	#ifdef WIN32
		freopen("CON", "w", stdout);
		freopen("CON", "w", stderr);
	#endif

	printf("Inicializando sistema...\n");
	GameInit();

	printf("Inicializando modo gráfico...\n");
	VideoModeSetTitle("Tetris");
    VideoModeSet(SCREEN_WIDTH, SCREEN_HEIGHT, 1);

	printf("Inicializando GL...\n");
    init_gl();

	printf("Cargando recursos...\n");
    load_resources();

	printf("Preparando objetos...\n");
    prepareRenderObjects();

	printf("Inicializando generador de numeros aleatorios...\n");
	genrand_init();

	printf("Cargando piezas...\n");
	loadPiecesFromFile("data/pieces.txt");

	score_load();

	action_main();

    Quit(0);

    return 0;
}
