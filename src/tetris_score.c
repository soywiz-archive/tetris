#include "tetris.h"

int  score_points[MAX_SCORES];
char score_names[16][MAX_SCORES];

int n;
void score_load() {
	FILE *f = fopen("data\\scores.dat", "rb");

	printf("Cargando puntuaciones...\n");

	if (!f) {
		printf("El fichero de puntuaciones data/scores.dat no existe, creando tabla por defecto...\n");

		for (n = 0; n < MAX_SCORES; n++) {
			sprintf((char *)&score_names[n], "DEMO");
			score_points[n] = 1000 - ((n * n) / 7 + n) * 113;
		}

		return;
	}

	fread(score_points, MAX_SCORES, sizeof(score_points[0]), f);
	fread(score_names, MAX_SCORES, sizeof(score_names[0]), f);

	fclose(f);
}

void score_save() {
	FILE *f = fopen("data\\scores.dat", "wb");
	printf("Guardando puntuaciones...\n");

	if (!f) return;

	fwrite(score_points, MAX_SCORES, sizeof(score_points[0]), f);
	fwrite(score_names, MAX_SCORES, sizeof(score_names[0]), f);

	fclose(f);
}

void score_submit(char *name, int points) {
	int n, m;
	for (n = 0; n < MAX_SCORES; n++) {
		if (points > score_points[n]) {
			for (m = MAX_SCORES - 1; m > n; m--) {
				sprintf(score_names[m], "%s", score_names[m - 1]);
				score_points[m] = score_points[m - 1];
			}
			score_points[n] = points;
			sprintf(score_names[n], "%s", name);
			break;
		}
	}

	score_save();
}

int score_better(int v) {
	int n;
	for (n = 0; n < MAX_SCORES; n++) {
		if (v > score_points[n]) return 1;
	}
	return 0;
}
