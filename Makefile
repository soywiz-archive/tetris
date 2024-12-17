tetris:  src/tetris_menu.c src/net/net.c src/net/server.c src/util/mt19938ar.c src/base/piece.c src/base/board.c src/media/keyboard.c src/media/image.c src/media/font.c src/tetris.c
	gcc src/tetris_menu.c src/net/net.c src/net/server.c src/util/mt19938ar.c src/base/piece.c src/base/board.c src/media/keyboard.c src/media/image.c src/media/font.c src/tetris.c -o tetris  -Iinclude -Llib -lSDL -lSDL_image -lSDL_mixer -lSDL_net -lSDL_ttf -lglut  -lGL  -Wall

