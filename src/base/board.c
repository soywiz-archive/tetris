#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "../gamelib/gamelib_rand.h"

// Muestra el tablero
void showBoard(Board *board) {
	int x, y;

	printf("PIEZA ACTUAL:\n");
	showRealPiece(&board->current);

	printf("PIEZA SIGUIENTE:\n");
	showRealPiece(&board->next);

	for (y = BOARD_HEIGHT - 1; y >= 0; y--) {
		for (x = 0; x < BOARD_WIDTH; x++) {
			printf("%c", board->data[x][y] ? '*' : '.');
		}
		printf("\n");
	}
	printf("\n");
}

// Elige una nueva pieza
void setNextPiece(Board *board, int piece) {
	if (piece == -1) piece = genrand_int32() % pieceListCount;

	board->current = board->next;
	board->next.x = (BOARD_WIDTH / 2) - (4 / 2);
	board->next.y = BOARD_HEIGHT + 0;
	board->next.rotation = 0;
	board->next.npiece = piece;
}

// Inicializa el tablero
void initBoard(Board *board, int piece1, int piece2) {
	int x, y; board->flines = 0;
	for (y = 0; y < BOARD_HEIGHT; y++) {
		board->fline[y] = 0;
		for (x = 0; x < BOARD_WIDTH; x++) board->data[x][y] = 0;
	}
	board->level = board->points = board->llines = board->lines = 0;
	setNextPiece(board, piece1); setNextPiece(board, piece2);
	board->playing = 0;
	board->frozen = 0;
}

// private
int checkRealPieceInBoard(Board *board, RealPiece *rpiece) {
	Piece *piece = &pieceList[rpiece->npiece];
	int x, y, px, py;

	if (!board || !rpiece) return 1;

	px = rpiece->x; py = rpiece->y;

	for (y = 0; y < 4; y++) {
		//if (py + y < 0) continue;
		for (x = 0; x < 4; x++) {
			if (piece->data[rpiece->rotation][x][y] != 0) {
				// La pieza está fuera de los límites
				if ((px + x < 0 || px + x >= BOARD_WIDTH)) return 0;
				if (py + y < 0) return 0;
				if (py + y >= BOARD_HEIGHT) continue;

				// La pieza está colisionando con el tablero
				if (board->data[px + x][py + y] != 0) return 0;
			}
		}
	}

	return 1;
}

#define BEGIN_TRY_PIECE RealPiece checkPiece = board->current;
#define END_TRY_PIECE return 0;
#define TRY_VALIDATE_PIECE if (checkRealPieceInBoard(board, &checkPiece)) { board->current = checkPiece; return 1; }

#define TRY_VALIDATE_ROTATE_COMMON \
	TRY_VALIDATE_PIECE \
	checkPiece.x += 1; TRY_VALIDATE_PIECE \
	checkPiece.x -= 2; TRY_VALIDATE_PIECE \
	checkPiece.x += 1; checkPiece.y += 1; TRY_VALIDATE_PIECE \

// Trata de rotar la pieza actual del tablero indicado a la izquierda
int rotatePieceLeft(Board *board) {
	BEGIN_TRY_PIECE {
		// Rotamos la pieza de prueba a la izquierda
		if (--checkPiece.rotation < 0) checkPiece.rotation = pieceList[checkPiece.npiece].nrotations - 1;
		TRY_VALIDATE_ROTATE_COMMON;
	} END_TRY_PIECE;
}

// Trata de rotar la pieza actual del tablero indicado a la derecha
int rotatePieceRight(Board *board) {
	BEGIN_TRY_PIECE {
		// Rotamos la pieza de prueba a la derecha
		checkPiece.rotation = (checkPiece.rotation + 1) % pieceList[checkPiece.npiece].nrotations;
		TRY_VALIDATE_ROTATE_COMMON;
	} END_TRY_PIECE;
}

// Trata de mover la pieza actual del tablero indicado a la izquierda
int movePieceLeft(Board *board) {
	BEGIN_TRY_PIECE {
		// Movemos la pieza de prueba a la izquierda
		checkPiece.x--;
		TRY_VALIDATE_PIECE;
	} END_TRY_PIECE;
}

// Trata de mover la pieza actual del tablero indicado a la derecha
int movePieceRight(Board *board) {
	BEGIN_TRY_PIECE {
		// Movemos la pieza de prueba a la derecha
		checkPiece.x++;
		TRY_VALIDATE_PIECE;
	} END_TRY_PIECE;
}

// Trata de mover la pieza actual del tablero indicado hacia abajo
int movePieceDown(Board *board) {
	BEGIN_TRY_PIECE {
		// Movemos la pieza de prueba a la derecha
		checkPiece.y--;
		TRY_VALIDATE_PIECE;
	} END_TRY_PIECE;
}

// Coloca la pieza en la posición actual
int putPiece(Board *board) {
	RealPiece *r = &board->current;
	char c;
	int x, y;

	for (y = 0; y < 4; y++) {
		for (x = 0; x < 4; x++) {
			if ((c = pieceList[r->npiece].data[r->rotation][x][y]) != 0) {
				if (r->y + y < 0 || r->y + y >= BOARD_HEIGHT) return 0;
				board->data[r->x + x][r->y + y] = c;
			}
		}
	}

	r->y = -10;

	return 1;
}

// Comprueba las líneas que van a ser eliminadas
int checkLines(Board *board) {
	int x, y, c;

	for (board->flines = 0, y = 0; y < BOARD_HEIGHT; y++) {
		c = 0; for (x = 0; x < BOARD_WIDTH; x++) if (board->data[x][y] != 0) c++;
		if (c == BOARD_WIDTH) { board->fline[y] = 1; board->flines++; }
	}

	return board->flines;
}

// Elimina las lineas que van a ser eliminadas y han sido comprobadas por checkLines
int deleteLines(Board *board) {
	int x, y, y2;

	for (y2 = y = 0; y < BOARD_HEIGHT; y++, y2++) {
		if (y2 != y) {
			for (x = 0; x < BOARD_WIDTH; x++) {
				board->data[x][y2] = board->data[x][y];
				board->data[x][y] = 0;
			}
		}

		if (board->fline[y]) y2--;
		board->fline[y] = 0;
	}
	board->flines = 0;
	y--; y2--;
	if (y2 != y) for (x = 0; x < BOARD_WIDTH; x++) board->data[x][y] = 0;
	return 0;
}
