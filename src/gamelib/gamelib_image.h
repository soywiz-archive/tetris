#ifndef __GAMELIB_IMAGE_H
#define __GAMELIB_IMAGE_H

// Declaramos el tipo "Image"
typedef struct _Image _Image;
typedef struct _Image *Image;

// Variable utilizada para medir la cantidad de mem�ria usada en las texturas
extern int textureMemory;

// Estructura de la Imagen
struct _Image {
	// Imagen padre para subim�genes
	Image father;

	// Coordenadas de la subim�gen en la imagen real
	int x, y, w, h;

	// Centro de la imagen
	int cx, cy;

	// Ancho y altos reales de la imagen
	int rw, rh;

	// Textura OpenGL
	GLuint gltex;

	// CallList para renderizaci�n r�pida
	GLuint callList;

	// Lista de puntos en la im�gen
	struct { float x, y; } texp[2];
};

// Crea una imagen vac�a (para pintar con ImageXXXDrawing
Image ImageCreate(int w, int h);

// Crea una im�gen a partir de un recorte de otra
Image ImageCreateFromSubImage(Image vpi, int x, int y, int w, int h);

// Carga de im�genes de diferentes medios
Image ImageLoadFromStreamEx(Stream s, int freesrc);
Image ImageLoadFromStream(Stream s);
Image ImageLoadFromMemory(void *ptr, int length);
Image ImageLoadFromFile(char *filename);
#define ImageLoad ImageLoadFromFile

// Liberaci�n de im�genes
void ImageFree(Image i);

// Duplicar im�gen
Image ImageDuplicate(Image i);

// Colocaci�n de hotspot
void ImageSetCXY(Image i, int cx, int cy);

// Renderizado de im�genes
void DrawImageEx(Image i, int x, int y, float size, float angle, float alpha);
void DrawImage(Image i, int x, int y);

// Renderizado sobre texturas
int  ImageStartDrawing(Image i);
void ImageStopDrawing();

// Renderizar Primitivas
void DrawLine(int x1, int y1, int x2, int y2);
void DrawRectangle(int x1, int y1, int x2, int y2);
void DrawRectangleFill(int x1, int y1, int x2, int y2);

// Borrar la pantalla
void DrawClear();


#endif
