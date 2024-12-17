#include "gamelib.h"
#include "gamelib_internal.h"

int __FontCreateGlyph(TTF_Font *font, Font ttf, int c) {
	int minx, maxx, miny, maxy, advance;
	int rw = 1, rh = 1; SDL_Rect dest; SDL_Color color = {0xff, 0xff, 0xff};
	SDL_Surface *temp, *glyph = TTF_RenderGlyph_Blended(font, c, color);
	if (glyph == 0) return -1;

	temp = __SDL_CreateRGBSurfaceForOpenGL(glyph->w, glyph->h, &rw, &rh);

	dest.x = dest.y = 0; dest.w = rw; dest.h = rh;
	SDL_SetAlpha(glyph, 0, 0);
	SDL_BlitSurface(glyph, 0, temp, &dest);
	glBindTexture(GL_TEXTURE_2D, ttf->textures[c]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, rw, rh, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp->pixels);
	SDL_FreeSurface(temp); TTF_GlyphMetrics(font, c, &minx, &maxx, &miny, &maxy, &advance);
	ttf->w[c] = advance;

	glNewList (ttf->listBase + c, GL_COMPILE);
		glPushMatrix();

		glTranslatef(minx, -maxy + TTF_FontAscent(font) + TTF_FontDescent(font), 0);

		float x = (float)glyph->w / (float)rw, y = (float)glyph->h / (float)rh;

		glBindTexture(GL_TEXTURE_2D, ttf->textures[c]);
		glBegin(GL_POLYGON);
			glTexCoord2f(0, 0); glVertex2f(0, 0);
			glTexCoord2f(x, 0); glVertex2f(glyph->w, 0);
			glTexCoord2f(x, y); glVertex2f(glyph->w, glyph->h);
			glTexCoord2f(0, y); glVertex2f(0, glyph->h);
		glEnd();

		glPopMatrix();
		glTranslatef(advance, 0, 0);
	glEndList();

	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(glyph);

	return 0;
}

__inline Font FontLoadFromStreamEx(Stream s, int freesrc, int height) {
	Font f; int n;
	TTF_Font *ttf = TTF_OpenFontRW(s, freesrc, height);
	f = (Font)malloc(sizeof(_Font));
	if (!ttf || !f) return NULL;
	f->listBase = glGenLists(0x100);
	f->h = height;
	glGenTextures(0x100, f->textures);
	for (n = 32; n < 0x100; n++) __FontCreateGlyph(ttf, f, n);
	TTF_CloseFont(ttf);

	return f;
}

Font FontLoadFromStream(Stream s, int height) {
	return FontLoadFromStreamEx(s, 0, height);
}

Font FontLoadFromMemory(void *ptr, int length, int height) {
	return FontLoadFromStreamEx(SDL_RWFromMem(ptr, length), 1, height);
}

Font FontLoadFromFile(char *filename, int height) {
	Font f = FontLoadFromStreamEx(SDL_RWFromFile(filename, "rb"), 1, height);

	if (!f) {
		SDL_SetError("Can't open ttf font '%s'", filename);
		GamePrintError();
	}

	return f;
}

int FontWidth(Font f, char *text) {
	int r = 0, n, l;
	for (n = 0, l = strlen(text); n < l; n++) r += f->w[(int)text[n]];
	return r;
}

void FontFree(Font f) {
	if (f == NULL) return;

	glDeleteTextures(0x100, f->textures);
	glDeleteLists(f->listBase, 0x100);

	free(f);
}

int fromHex(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

void __DrawFontTextColored(char *text, int length) {
	int n, s = 0;

	for (n = 0; n <= length; n++) {
		char c = text[n];
		if (c == '\b' || n == length) {
			glCallLists(n - s, GL_UNSIGNED_BYTE, text + s);

			if (c == '\b') {
				int r = (fromHex(text[n + 1]) << 4) | fromHex(text[n + 2]);
				int g = (fromHex(text[n + 3]) << 4) | fromHex(text[n + 4]);
				int b = (fromHex(text[n + 5]) << 4) | fromHex(text[n + 6]);

				glColor3f(
					(float)r / 255,
					(float)g / 255,
				  	(float)b / 255
				);

				//printf("%d, %d, %d\n", r, g, b);

				s = n + 7;
				n += 6;
			}
		}
	}
	//printf("\n");
}

void DrawFontText(Font font, int x, int y, char *text) {
	char *stext;
	int line = 0;
	//int limit = 5;
	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
		glLoadIdentity();

		glDisable(GL_LIGHTING); glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST); glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glListBase(font->listBase);

		while (*text != '\0') {
			stext = text;

			while (*text != '\n' && *text != '\0') text++;
			if (*text == '\n') text++;

			glPushMatrix();
				glTranslatef((float)x, (float)y + line * (font->h + 3), 0.0f);
				glScalef(1.0f, 1.0f, 1.0f);
				__DrawFontTextColored(stext, text - stext);
			glPopMatrix();

			line++;
		}

	glPopAttrib();

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void DrawFontTextf(Font font, int x, int y, char *format, ...) {
	char temp[1024];
	va_list lptr;
	va_start(lptr, format);
	vsprintf(temp, format, lptr);
	va_end(lptr);

	//vsprintf(temp, format, (&format + sizeof(format)));

	DrawFontText(font, x, y, temp);
}

void DrawFontSimpleShadow(Font font, char *text, float x, float y, float r, float g, float b, float a) {
	glColor4f(0, 0, 0, a * 0.7);
	DrawFontText(font, x + 2, y + 2, text);
	glColor4f(r, g, b, a);
	DrawFontText(font, x, y, text);
}
