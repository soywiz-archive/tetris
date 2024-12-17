#ifndef __GAMELIB_FONT_H
#define __GAMELIB_FONT_H

typedef struct {
	int    h;
	int    w[0x100];
	GLuint textures[0x100];
	GLuint listBase;
} *Font, _Font;

Font FontLoadFromStream(Stream s, int height);
Font FontLoadFromMemory(void *ptr, int length, int height);
Font FontLoadFromFile(char *filename, int height);
#define FontLoad FontLoadFromFile

int FontWidth(Font f, char *text);
void FontFree(Font f);

void DrawFontText(Font font, int x, int y, char *text);
void DrawFontTextf(Font font, int x, int y, char *format, ...);
void DrawFontSimpleShadow(Font font, char *text, float x, float y, float r, float g, float b, float a);

#endif
