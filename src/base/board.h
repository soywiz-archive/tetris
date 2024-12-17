#ifndef __BOARD_H
#define __BOARD_H

	#include "piece.h"

	// Ancho y alto del tablero
	#define BOARD_WIDTH  10
	//#define BOARD_HEIGHT 18
	#define BOARD_HEIGHT 20

	struct Board;

	// Tablero
	typedef struct Board {
		RealPiece next;
		RealPiece current;

		char   data[BOARD_WIDTH][BOARD_HEIGHT];
		char   fline[BOARD_HEIGHT], flines;

		int    playing;
		int    frozen;

		int    level;

		int    lines;
		int    llines;

		double points;
	} Board;

	void initBoard          (Board *board, int piece1, int piece2);
	void setNextPiece       (Board *board, int piece);
	int  rotatePieceLeft    (Board *board);
	int  rotatePieceRight   (Board *board);
	int  movePieceLeft      (Board *board);
	int  movePieceRight     (Board *board);
	int  movePieceDown      (Board *board);
	int  putPiece           (Board *board);
	void showBoard          (Board *board);
	int  checkLines         (Board *board);
	int  deleteLines        (Board *board);

	//int checkRealPieceInBoard(Board *board, struct RealPiece *rpiece);

#endif
