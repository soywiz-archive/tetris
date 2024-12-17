#include "tetris.h"

extern Image iescape, background;

int noptions = 0;

struct SOption;

typedef struct SOption {
	int n;
	float alpha;
	float size;
	float x, y;
	Image image;
} SOption;

struct Option {
	int value;
	Image image;
	float angle;
} options[100];

int action_clear_options() {
	noptions = 0;
	return 0;
}

int action_add_option(int value, Image image) {
	options[noptions].value = value;
	options[noptions].image = image;
	noptions++;
	return 0;
}

int sortsoptions(const void *a, const void *b) {
	float c1 = ((SOption *)a)->size, c2 = ((SOption *)b)->size;
	if (c1 == c2) return 0;
	return (c1 < c2) ? -1 : 1;
}

void action_select_option_draw(SOption *soptions, float alpha) {
	int n;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0); glTranslatef(0, 1, 0);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	DrawImageEx(background, 0, 0, 1, 0, 1);

	// Mostramos las imágenes de las opciones
	for (n = 0; n < noptions; n++) DrawImageEx(soptions[n].image, soptions[n].x, soptions[n].y, soptions[n].size, 0, alpha * soptions[n].alpha);

	DrawImageEx(iescape, SCREEN_WIDTH - iescape->w, SCREEN_HEIGHT - iescape->h, 1, 0, 0.3);

	showFadeBalls(alpha);

	SDL_GL_SwapBuffers();

	wait_fps();
}

int action_input_text(char *title, char *text, int mlength) {
	int clen = strlen(text);
	sprintf(text, "%s_", text);

	while (1) {
		KeyboardUpdate();

		if (key[SDLK_ESCAPE]) {
			text[clen] = 0;
			return -1;
		}
		if (key[SDLK_RETURN]) {
			text[clen] = 0;
			return 1;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0); glTranslatef(0, 1, 0);
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();

		DrawImageEx(background, 0, 0, 1, 0, 1);

		DrawFontSimpleShadow(font2, title, 16, 16, 1, 1, 1, 1);

		DrawFontSimpleShadow(font1, text, (SCREEN_WIDTH / 2) - (FontWidth(font1, text) / 2), SCREEN_HEIGHT / 2 - font1->h, 1, 1, 1, 1);

		if (clen < mlength) {
			int n;
			for (n = 32; n <= 255; n++) {
				if (key[n]) {
					text[clen] = n;
					text[clen + 1] = '_';
					text[clen + 2] = 0;
					clen++;
				}
			}
		}

		if (clen > 0 && key[SDLK_BACKSPACE]) {
			text[clen - 1] = '_';
			text[clen] = 0;
			clen--;
		}

		SDL_GL_SwapBuffers();

		wait_fps();
	}
}

int action_select_option(int currentoption, int escape) {
	SOption soptions[100];

	//int currentoption = 0;
	float backangle = 0;
	float nextangle = 0;
	int steps = 60 / 4;
	int cstep = steps;
	float cpos;

	float anglestep;
	float incangle = 0;
	int n;
	float rx = SCREEN_WIDTH / 4;
	//float ry = SCREEN_HEIGHT / 32;
	float size = 1.4;
	float alpha = 0;

	if (noptions < 1) return -1;

	anglestep = (float)(2.0f * PI) / (float)(noptions);

	for (n = 0; n < noptions; n++) {
		options[n].angle = anglestep * n;
	}

	nextangle = backangle = incangle = anglestep * currentoption;

	while (1) {
		KeyboardUpdate();

		if (escape && key[SDLK_ESCAPE]) return -1;

		for (n = 0; n < noptions; n++) {
			float angle = -options[n].angle + PI / 2 + incangle;
			size = soptions[n].size = 1 + sin(angle) / 4;
			soptions[n].x = (float)(SCREEN_WIDTH / 2) - (float)(image_single_player->w * size / 2) + (float)cos(angle) * (float)rx;
			soptions[n].y = (float)(SCREEN_HEIGHT / 2) - (float)(image_single_player->h * size / 2);
			soptions[n].image = options[n].image;
			soptions[n].n = n;
			soptions[n].alpha = (sin(angle) * 0.4) + 0.6;
		}

		// Ordenamos las imágenes de las opciones
		qsort(soptions, noptions, sizeof(soptions[0]), sortsoptions);

		action_select_option_draw(soptions, alpha);

		if (alpha < 1) alpha += 0.1;

		if (key[P1_LEFT] || key[P1_RIGHT] || key[P2_LEFT] || key[P2_RIGHT]) {
			if (key[P1_LEFT] || key[P2_LEFT]) {
				currentoption--; if (currentoption < 0) currentoption = noptions - 1;

				if (incangle < 0) incangle = PI * 2 + incangle;
				if (incangle > PI * 2) incangle = fmod(incangle, PI * 2);

				backangle = incangle;
				nextangle = anglestep * currentoption;

				if (abs(nextangle - backangle) >= PI) {
					nextangle = -(anglestep * (noptions - currentoption));
				}

				Mix_PlayChannel(-1, sndrotate, 0);
			}

			if (key[P1_RIGHT] || key[P2_RIGHT]) {
				currentoption++; if (currentoption >= noptions) currentoption = 0;

				if (incangle < 0) incangle = PI * 2 + incangle;
				if (incangle > PI * 2) incangle = fmod(incangle, PI * 2);

				backangle = incangle;
				nextangle = anglestep * currentoption;

				if (abs(nextangle - backangle) > PI) {
					nextangle = PI * 2 + currentoption * anglestep;
				}

				Mix_PlayChannel(-1, sndrotate, 0);
			}

			cstep = 0;
		}

		if (key[P1_OK] || key[P2_OK]) {
			Mix_PlayChannel(-1, sndrotate, 0);

			while (alpha > 0) {
				action_select_option_draw(soptions, alpha);
				alpha -= 0.1;
			}

			return options[currentoption].value;
		}

		if (cstep >= 0) {
			cpos = (-cos((PI * (float)cstep) / (float)steps) + 1) / 2;
			incangle = (nextangle - backangle) * cpos + backangle;
			cstep++;

			if (cstep > steps) { backangle = nextangle; cstep = -1; }
		}
	}

	return 0;
}
