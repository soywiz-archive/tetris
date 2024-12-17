#include "gamelib.h"
#include "gamelib_internal.h"
#include <GL/glext.h>

int textureMemory = 0;

//
// http://www.opengl.org/registry/specs/EXT/framebuffer_object.txt
// http://www.codesampler.com/oglsrc/oglsrc_14.htm
// http://www.gamedev.net/reference/articles/article2331.asp
// http://www.gamedev.net/reference/articles/article2333.asp
//
// Note: The EXT_framebuffer_object extension is an excellent replacement for
//       the WGL_ARB_pbuffer and WGL_ARB_render_texture combo which is normally
//       used to create dynamic textures. An example of this older technique
//       can be found here:
//
//       http://www.codesampler.com/oglsrc/oglsrc_7.htm#ogl_offscreen_rendering
//

int extframebuffer = 0;
GLuint fbo = -1;

PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC        glGenFramebuffersEXT        = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC   glFramebufferTexture2DEXT   = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC     glDeleteFramebuffersEXT     = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC        glBindFramebufferEXT        = NULL;

// Eliminamos el FrameBufferObject
void RemoveFrameBufferObject() {
	if (!fbo) return;
	glDeleteFramebuffersEXT(1, &fbo);
	fbo = 0;
}

void InitializeFrameBufferObject() {
	// Comprobamos si ya hemos inicializado el FrameBufferObject
	if (fbo != -1) return;

	// Comprobamos que exista la extensión del framebuffer_object
	if (strstr(glGetString(GL_EXTENSIONS), "EXT_framebuffer_object") == NULL) {
		SDL_SetError("EXT_framebuffer_object extension was not found");
		GamePrintError(); extframebuffer = 0;
		return;
	}

	// Obtenemos la dirección de los procedimientos
	glGenFramebuffersEXT        = (PFNGLGENFRAMEBUFFERSEXTPROC)       SDL_GL_GetProcAddress("glGenFramebuffersEXT");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
	glFramebufferTexture2DEXT   = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)  SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
	glDeleteFramebuffersEXT     = (PFNGLDELETEFRAMEBUFFERSEXTPROC)    SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
	glBindFramebufferEXT        = (PFNGLBINDFRAMEBUFFEREXTPROC)       SDL_GL_GetProcAddress("glBindFramebufferEXT");

	// Comprobamos que se han obtenido correctamente los procedimientos
	if(!glCheckFramebufferStatusEXT || !glGenFramebuffersEXT || !glFramebufferTexture2DEXT || !glBindFramebufferEXT || !glDeleteFramebuffersEXT) {
		SDL_SetError("One or more EXT_framebuffer_object functions were not found");
		GamePrintError(); extframebuffer = 0;
		return;
	}

	// Generamos el FrameBufferObject
	glGenFramebuffersEXT(1, &fbo);

	// Comprobamos su estado
	switch (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)) {
		// Si se ha completado el proceso satisfactoriamente seguimos
		case GL_FRAMEBUFFER_COMPLETE_EXT: break;
		// Si se ha producido un error, mostramos un error
		default: case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			RemoveFrameBufferObject();
			SDL_SetError("Can't initialize framebuffer object");
			GamePrintError(); extframebuffer = 0;
			return;
		break;
	}

	// Finalmente ya podemos concluir que la extensión ha sido cargada satisfactoriamente
	// y puede ser usada.
	extframebuffer = 1;
}

// TODO: Implementar usando pbuffer o extbufs si no está disponible la extensión framebuffer.
int ImageStartDrawing(Image i) {
	if (i == NULL) {
		SDL_SetError("ImageStartDrawing - without image");
		GamePrintError();
		return -1;
	}

	if (fbo == -1) InitializeFrameBufferObject();

	if (!extframebuffer) {
		SDL_SetError("Can't draw on images because the lack of framebuffer object");
		GamePrintError();
		return -1;
	}

	glFlush();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, i->gltex, 0);

	glViewport(0, 0, i->w, i->h);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, i->w, 0, i->h, -1.0, 1.0);
	glTranslatef(0, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	return 0;
}

void ImageStopDrawing() {
	if (!extframebuffer) {
		SDL_SetError("Can't draw on images because the lack of framebuffer object");
		GamePrintError();
		return;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glViewport(0, 0, screenWidthReal, screenHeightReal);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE   );
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
}

void __ImageUpdateCallList(Image i) {
	if (i->callList) glDeleteLists(i->callList, 1);

	i->callList = glGenLists(1);
	glNewList(i->callList, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, i->gltex);
		glTexParameterf(GL_TEXTURE_2D, 0x84FF, 16);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glBegin(GL_POLYGON);
			glTexCoord2f(i->texp[0].x, i->texp[0].y); glVertex2f(0    - i->cx, 0    - i->cy);
			glTexCoord2f(i->texp[1].x, i->texp[0].y); glVertex2f(i->w - i->cx, 0    - i->cy);
			glTexCoord2f(i->texp[1].x, i->texp[1].y); glVertex2f(i->w - i->cx, i->h - i->cy);
			glTexCoord2f(i->texp[0].x, i->texp[1].y); glVertex2f(0    - i->cx, i->h - i->cy);
		glEnd();
	glEndList();
}

void __ImageUpdateTexPoints(Image i) {
	float fx1, fx2, fy1, fy2;

	textureMemory += i->rw * i->rh * 4;

	fx1 = (float)i->x / (float)i->rw;
	fy1 = (float)i->y / (float)i->rh;

	fx2 = (float)(i->w + i->x) / (float)i->rw;
	fy2 = (float)(i->h + i->y) / (float)i->rh;

	i->texp[0].x = fx1; i->texp[0].y = fy1;
	i->texp[1].x = fx2; i->texp[1].y = fy2;

	__ImageUpdateCallList(i);
}

void __ImageInit(Image i) {
	i->callList = 0;
}

void __ImagePrepareNew(Image i, int w, int h, int rw, int rh) {
	i->father = NULL;
	i->y = i->x = 0;
	i->w = w; i->h = h;
	i->rw = rw; i->rh = rh;
	i->cy = i->cx = 0;

	__ImageUpdateTexPoints(i);
}

Image ImageCreate(int w, int h) {
	int rw = __NextPowerOfTwo(w), rh = __NextPowerOfTwo(h);
	Image i;
	if ((i = malloc(sizeof(_Image))) == NULL) return NULL;
	__ImageInit(i);

	__ImagePrepareNew(i, w, h, rw, rh);

	glGenTextures(1, &i->gltex);
	glBindTexture(GL_TEXTURE_2D, i->gltex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, rw, rh, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return i;
}

Image ImageCreateFromSubImage(Image vpi, int x, int y, int w, int h) {
	Image i;

	if (vpi == NULL) return NULL;

	if ((i = malloc(sizeof(_Image))) == NULL) return NULL;
	__ImageInit(i);

	i->gltex = vpi->gltex;

	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > vpi->x + vpi->w) { w = vpi->x + vpi->w - x; }
	if (y + h > vpi->y + vpi->h) { h = vpi->y + vpi->h - y; }
	if (w < 0) w = 0;
	if (h < 0) h = 0;

	i->x = x; i->y = y;
	i->w = w; i->h = h;
	i->rw = vpi->rw; i->rh = vpi->rh;
	i->cy = i->cx = 0;

	i->father = (vpi->father == NULL) ? vpi : vpi->father;

	__ImageUpdateTexPoints(i);

	return i;
}

__inline Image ImageLoadFromStreamEx(Stream s, int freesrc) {
	int rw, rh;
	SDL_Surface *surface;
	SDL_Surface *surfaceogl;
	Image i;

	if (!(surface = IMG_Load_RW(s, freesrc))) return NULL;
	if (!(surfaceogl = __SDL_CreateRGBSurfaceForOpenGL(surface->w, surface->h, &rw, &rh))) return NULL;

	SDL_SetAlpha(surface, 0, SDL_ALPHA_OPAQUE);
	SDL_BlitSurface(surface, 0, surfaceogl, 0);

	if ((i = malloc(sizeof(_Image))) == NULL) return NULL;
	__ImageInit(i);

	__ImagePrepareNew(i, surface->w, surface->h, rw, rh);

	glGenTextures(1, &i->gltex);
	glBindTexture(GL_TEXTURE_2D, i->gltex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, rw, rh, 0, GL_RGBA, GL_UNSIGNED_BYTE, surfaceogl->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);
	SDL_FreeSurface(surfaceogl);

	return i;
}

Image ImageLoadFromStream(Stream s) {
	return ImageLoadFromStreamEx(s, 0);
}

Image ImageLoadFromMemory(void *ptr, int length) {
	return ImageLoadFromStreamEx(SDL_RWFromMem(ptr, length), 1);
}

Image ImageLoadFromFile(char *filename) {
	Image i = ImageLoadFromStreamEx(SDL_RWFromFile(filename, "rb"), 1);

	if (!i) {
		SDL_SetError("Can't open image '%s'", filename);
		GamePrintError();
	}

	return i;
}

void ImageFree(Image i) {
	if (i == NULL) return;

	// Textura final, liberar
	if (i->father == NULL) glDeleteTextures(1, &i->gltex);

	free(i);
}

void ImageSetCXY(Image i, int cx, int cy) {
	if (i == NULL) return;
	i->cx = cx; i->cy = cy;
	__ImageUpdateCallList(i);
}

void DrawImageEx(Image i, int x, int y, float size, float angle, float alpha) {
	float r = 1.0, g = 1.0, b = 1.0;

	if (i == NULL) return;

	if (alpha < 0) alpha = 0; else if (alpha > 1) alpha = 1;

	x -= i->cx; y -= i->cy + 1;

	glColor4f(r, g, b, alpha);
	glEnable(GL_TEXTURE_2D);


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef((float)x + i->cx, (float)y + i->cy, 0.0f);

	glRotatef(angle, 0, 0, 1);
	glScalef(size, size, size);

	glBindTexture(GL_TEXTURE_2D, i->gltex);
	glTexParameterf(GL_TEXTURE_2D, 0x84FF, 16);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glBegin(GL_POLYGON);
		glTexCoord2f(i->texp[0].x, i->texp[0].y); glVertex2f(0    - i->cx, 0    - i->cy);
		glTexCoord2f(i->texp[1].x, i->texp[0].y); glVertex2f(i->w - i->cx, 0    - i->cy);
		glTexCoord2f(i->texp[1].x, i->texp[1].y); glVertex2f(i->w - i->cx, i->h - i->cy);
		glTexCoord2f(i->texp[0].x, i->texp[1].y); glVertex2f(0    - i->cx, i->h - i->cy);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
}

void DrawImage(Image i, int x, int y) {
	DrawImageEx(i, x, y, 1.0f, 0.0f, 1.0f);
}

void DrawLine(int x1, int y1, int x2, int y2) {
	glBegin(GL_LINES);
		glVertex2i(x1, y1); glVertex2i(x2, y2);
	glEnd();
}

void DrawRectangle(int x1, int y1, int x2, int y2) {
	glBegin(GL_LINES);
		glVertex2i(x1, y1); glVertex2i(x2 - 1, y1);
		glVertex2i(x2, y1); glVertex2i(x2, y2 - 1);
		glVertex2i(x2, y2); glVertex2i(x1 + 1, y2);
		glVertex2i(x1, y2); glVertex2i(x1, y1 + 1);
	glEnd();
}

void DrawRectangleFill(int x1, int y1, int x2, int y2) {
	glBegin(GL_QUADS);
		glVertex2i(x1, y1);
		glVertex2i(x2, y1);
		glVertex2i(x2, y2);
		glVertex2i(x1, y2);
	glEnd();
}

void DrawClear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
