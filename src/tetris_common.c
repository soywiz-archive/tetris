#include "tetris.h"

void showFadeBalls(float alpha) {
	int x, y, ix, iy;
	float ralpha = 1 - alpha;
	if (ralpha <= 0) return;
	ix = image_ball->w / 2; iy = image_ball->h / 2;
	for (y = 0; y < SCREEN_HEIGHT; y += iy) {
		for (x = 0; x < SCREEN_WIDTH; x += ix) {
			DrawImageEx(image_ball, x, y, ralpha, 0, 0.7 + (ralpha * 0.3));
		}
	}
}
