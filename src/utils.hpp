/*****************************************
*										 *
*				Utils.h					 *
*	Various utilities for the game.		 *
*										 *
*****************************************/

/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS

#ifndef UTILS_H
#define UTILS_H

#include "math.h"
#include <GL/gl.h>
//#include <gl/glaux.h>
#include <GL/glu.h>
#include "targa.hpp"

extern bool lbuttondown;

unsigned int LoadTGATexture(char* filename);
//primatives
class Vector3f;
class Vector4f;
class Matrix4f;

//****************************************
class StringMap
//****************************************
{
public:
	StringMap()
	{
		object = NULL;
		nextMap = NULL;
	}
	StringMap(char* k,void *o)
	{
		object = o;
		sprintf(key,"%s",k);
	}
	void *object;
	StringMap *nextMap;
	char key[1024];
	
};

StringMap* AddEntry(StringMap *s,char* key,void *o);
void DeleteStringMap(StringMap *s);
void* FindEntry(StringMap *s, char* key);
	
//****************************************
class Vector3f
//****************************************
{
public:
	//constructors
	Vector3f() : x(0), y(0), z(0) {}
	Vector3f(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	//methods
	void normalize();
	float magnitude();
	float dot(const Vector3f v);
	Vector3f cross(const Vector3f v);
	void set(float x, float y, float z);

	//operation overrides
	Vector3f operator+(Vector3f v);
	Vector3f operator-(Vector3f v);
	Vector3f operator*(float f);
	Vector3f operator/(float f);
	Vector3f & operator=(Vector3f v);	

	//data members
	float x;
	float y;
	float z;
};

//****************************************
class Vector4f
//****************************************
{
public:
	//constructors
	Vector4f() : x(0), y(0), z(0), w(1)
	{
	}
	Vector4f(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	Vector4f(Vector3f v) : x(v.x), y(v.y), z(v.z), w(1)
	{
	}

	//methods
	void normalize();
	float magnitude();
	float dot(const Vector4f v);
	float dot(const Vector3f v);
	Vector3f getVector();
	void set(float x, float y, float z, float w);
	void setVector(Vector3f v,float f);

	//operation overrides
	Vector4f operator+(Vector4f v);
	Vector4f operator-(Vector4f v);
	Vector4f operator*(float f);
	Vector4f operator/(float f);
	Vector4f & operator=(Vector4f v);	
	

	//data members
	float x;
	float y;
	float z;
	float w;
};

//********************************************
class Matrix4f
//********************************************
{
public:
	//constructors
	Matrix4f()
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				data[i][j] = (i==j ? 1.0f: 0.0f);
			}
		}
	}			
	
	//methods
	void SetColumn(int c, Vector4f v);
	void SetColumn(int c, Vector3f v);
	void SetRow(int r, Vector4f v);
	
	Vector4f GetColumn(int c);
	Vector4f GetRow(int r);
	void SetData(int r, int c, float d);
	float GetData(int r, int c);
	void SetData(float* f);
	Matrix4f transpose();
	Matrix4f invert();
	Matrix4f scale(float x,float y, float z);
	float det();
	float cofactor(int i, int j);
	Matrix4f translate(Vector3f v);
	
	//operation overrides
	Matrix4f operator+(Matrix4f m);
	Matrix4f operator-(Matrix4f m);
	Matrix4f operator*(float f);
	Matrix4f operator*(Matrix4f m);
	Vector3f operator*(Vector3f v);
	Vector4f operator*(Vector4f v);
	Matrix4f operator/(float f);

private:
	float data[4][4];
};

//********************************************************
class Quaternion
//********************************************************
{
public:
	Quaternion();
	Quaternion(float f,Vector3f vector);

	Quaternion operator*(Quaternion q);
	Quaternion operator*(float f);
	Quaternion operator+(Quaternion q);
	Quaternion operator-(Quaternion q);
	Quaternion operator/(float f);
	Quaternion conj();
	float magnitude();
	Quaternion rotate(float angle);
	
	Vector3f v;
	float s;
	
};
#endif
