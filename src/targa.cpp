//targa.cpp
#include <cstring>
#include "targa.hpp"

targaData *LoadTarga(char* filename)
{
#if 0
	targaData* data;
	FILE* file;
	file = fopen(filename,"rb");
	char IDlength;
	char colorMapType;
	char ImageMapType;
	unsigned short firstEntry;
	unsigned short colorLength;
	char numColors;
	short x;
	short y;
	short width;
	short height;
	char depth;
	char descriptor;
	TARGA_FOOTER footer;

	char bytedata[3];
	int i;
	char* ID = NULL;
	
	if(!file)
	{
		return NULL;
	}
	fseek(file,-26,SEEK_END);
	fread(&footer,sizeof(TARGA_FOOTER),1,file);
	fseek(file,0,SEEK_SET);
	fread(&IDlength,sizeof(char),1,file);
	fread(&colorMapType,sizeof(char),1,file);
	fread(&ImageMapType,sizeof(char),1,file);
	fread(&firstEntry,sizeof(unsigned short),1,file);
	fread(&colorLength,sizeof(unsigned short),1,file);
	fread(&numColors,sizeof(char),1,file);
	fread(&x,sizeof(short),1,file);
	fread(&y,sizeof(short),1,file);
	fread(&width,sizeof(short),1,file);
	fread(&height,sizeof(short),1,file);
	fread(&depth,sizeof(char),1,file);
	fread(&descriptor,sizeof(char),1,file);
	if(IDlength > 0)
	{
		ID = new char[IDlength];
		fread(ID,sizeof(char),IDlength,file);
	}
	fseek(file,firstEntry,SEEK_CUR);
	for(i = 0; i <colorLength-firstEntry; i++)
	{
		fread(bytedata,sizeof(char),3,file);
	}
	data = new targaData;
	
	//RUNLENGTH
	i = 0;
	int depthbytes = depth/8;
	int totalpixels = width*height*depthbytes;
	data->data = new char[totalpixels];
	char packetdata;
	char *color = new char[depthbytes];
	int marker = 0;
	fread(data->data,sizeof(char),totalpixels,file);
	char temp;
	for(i = 0; i < totalpixels; i+=depthbytes)
	{
		temp = data->data[i];
		data->data[i] = data->data[i+2];
		data->data[i+2] = temp;
	}
	/*
	while(i < totalpixels/depthbytes)
	{
		fread(&packetdata,sizeof(char),1,file);
		for(int j = 0; j <= (packetdata & 0x7F)*depthbytes; j+=depthbytes)
		{
			fread(color,sizeof(char),depthbytes,file);
			for(int k = 0; k < depthbytes; k++)
			{
				data->data[j+i+k] = color[k];
			}
		}
		i+=((packetdata & 0x7F) +1);
		i = i;
	}*/
	fclose(file);
	delete [] color;
	if(ID)
	{
		delete [] ID;
		ID = NULL;
	}
	data->width = width;
	data->height = height;
	return data;
#endif
	targaData *data = new targaData;
	data->data = new char[4*4];
	std::memset(data->data, 0xFF, (sizeof *data->data) * 16);
	data->bpp = 32;
	data->width = data->height = 2;
}
