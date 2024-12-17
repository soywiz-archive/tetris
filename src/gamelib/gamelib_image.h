#ifndef __GAMELIB_IMAGE_H
#define __GAMELIB_IMAGE_H

// Declaramos el tipo "Image"
typedef struct _Image _Image;
typedef struct _Image *Image;

// Variable utilizada para medir la cantidad de memória usada en las texturas
extern int textureMemory;

// Estructura de la Imagen
struct _Image {
	// Imagen padre para subimágenes
	Image father;

	// Coordenadas de la subimágen en la imagen real
	int x, y, w, h;

	// Centro de la imagen
	int cx, cy;

	// Ancho y altos reales de la imagen
	int rw, rh;

	// Textura OpenGL
	GLuint gltex;

	// CallList para renderización rápida
	GLuint callList;

	// Lista de puntos en la imágen
	struct { float x, y; } texp[2];
};

// Crea una imagen vacía (para pintar con ImageXXXDrawing
Image ImageCreate(int w, int h);

// Crea una imágen a partir de un recorte de otra
Image ImageCreateFromSubImage(Image vpi, int x, int y, int w, int h);

// Carga de imágenes de diferentes medios
Image ImageLoadFromStreamEx(Stream s, int freesrc);
Image ImageLoadFromStream(Stream s);
Image ImageLoadFromMemory(void *ptr, int length);
Image ImageLoadFromFile(char *filename);
#define ImageLoad ImageLoadFromFile

// Liberación de imágenes
void ImageFree(Image i);

// Duplicar imágen
Image ImageDuplicate(Image i);

// Colocación de hotspot
void ImageSetCXY(Image i, int cx, int cy);

// Renderizado de imágenes
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
