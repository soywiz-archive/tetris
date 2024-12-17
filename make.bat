@echo off
pushd src
@windres.exe tetris.rc tetris_rc.o
popd
@gcc.exe src\tetris_rc.o src\tetris.c src\tetris_common.c src\tetris_score.c src\tetris_menu.c src\gamelib\gamelib.c src\gamelib\gamelib_font.c src\gamelib\gamelib_image.c src\gamelib\gamelib_internal.c src\gamelib\gamelib_rand.c src\base\piece.c src\base\board.c -o "tetris.exe" -Dmain=SDL_main -Iinclude -Llib -lmingw32 -lSDLMain -lSDL -lSDL_Image -lSDL_mixer -lSDL_net -lSDL_ttf -lglut32 -lglu32 -lopengl32 -lwinmm -lgdi32 -Wall -mconsole
strip.exe tetris.exe > NUL 2> NUL
