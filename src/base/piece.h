#ifndef __PIECE_H
#define __PIECE_H

	struct RealPiece;

	// Estructura que almacenará una pieza en concreto
	typedef struct RealPiece {
		int x, y;
		int rotation;
		//struct Piece *piece;
		int npiece;
	} RealPiece;

	struct Piece;

	// Estructura que almacenará una pieza cualquiera
	typedef struct Piece {
		int nrotations;
		char data[4][4][4];
	} Piece;

	#define MAX_PIECE_COUNT 10

	// Piezas generales globales para todos los tableros
	int pieceListCount;
	Piece pieceList[MAX_PIECE_COUNT];

	int loadPiecesFromFile(char *path);
	void showPieces();
	void showRealPiece(RealPiece *r);

#endif
