//***************************
//  TARGA.H
//  LOADS TARGA IMAGES
//***************************

#include <stdio.h>
#include <fstream>

#ifndef TARGA_H
#define TARGA_H



typedef struct
{
	int extensionOffset;
	int directorOffset;
	char signature[16];
	char period;
	char end;
} TARGA_FOOTER;

typedef struct
{
	char firstEntry;
	unsigned short colorMapLength;
	char colorMapEntrySize;
} COLOR_MAP_SPEC;

typedef struct
{
	short x;
	short y;
	short width;
	short height;
	char depth;
	char descriptor;
} IMAGE_MAP_SPEC;

typedef struct
{
	char IDlength;
	char colorMapType;
	char imageType;
	COLOR_MAP_SPEC colorMapSpec;
	
	IMAGE_MAP_SPEC imageSpec;
} TARGA_HEADER;

typedef struct
{
	char* data;
	int width;
	int height;
	int bpp;
} targaData;

targaData *LoadTarga(char* filename);





#endif