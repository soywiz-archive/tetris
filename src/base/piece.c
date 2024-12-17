#include "piece.h"
#include <stdio.h>
#include <stdlib.h>

#define pldat pieceList[pieceListCount].data
#define plrot pieceList[pieceListCount].nrotations

// Carga las piezas desde un fichero
int loadPiecesFromFile(char *path) {
	char line[800]; int cline, n;
	FILE *f; if (!(f = fopen(path, "rb"))) return -1;

	pieceListCount = 0;
	plrot = 0;
	cline = 0;

	while (!feof(f)) {
		fgets(line, sizeof(line) - 1, f);
		if (line[0] == 0 || line[0] == '\n' || line[0] == '\r') {
			if (cline > 0) {
				cline = 0;
				plrot++;
			}
			continue;
		}

		if (line[0] == '-') {
			if (plrot > 0) {
				pieceListCount++;
				plrot = 0;
			}
			cline = 0;
			continue;
		}

		for (n = 0; n < 4; n++) {
			pldat[plrot][n][cline] = (char)((line[n] == '*') ? (pieceListCount + 1) : 0);
		}
		cline++;
	}

	if (plrot > 0) pieceListCount++;

	fclose(f);

	return 0;
}

// DEBUG

// Muestra todas las piezas
void showPieces() {
	int n, m, l, x, y;
	for (n = 0; n < pieceListCount; n++) {
		l = pieceList[n].nrotations;
		printf("PIEZA: %d\n", n);
		for (m = 0; m < l; m++) {
			printf("ROTACIÓN: %d\n", m);
			for (y = 0; y < 4; y++) {
				for (x = 0; x < 4; x++) {
					printf("%c", pieceList[n].data[m][x][y] ? '*' : '.');
				}
				printf("\n");
			}
			printf("\n");
		}
		printf("\n");
	}
}

// Muestra una RealPiece
void showRealPiece(RealPiece *r) {
	Piece *p = &pieceList[r->npiece];
	int x, y;
	printf("(%d, %d) : %d\n", r->x, r->y, r->rotation);
	for (y = 0; y < 4; y++) {
		for (x = 0; x < 4; x++) {
			printf("%c", p->data[r->rotation][x][y] ? '*' : '.');
		}
		printf("\n");
	}
	printf("\n");
}
