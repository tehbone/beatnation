#ifndef MAIN_HPP_
#define MAIN_HPP_

// Windows Defines
#define WIN32_LEAN_AND_MEAN	

// Include Files
#include <windows.h>								// Header File For Windows
#include <gl\gl.h>									// Header File For The OpenGL32 Library
#include <gl\glu.h>									// Header File For The GLu32 Library
#include <gl\glaux.h>								// Header File For The GLaux Library
//other stuff
#include <math.h>
#include <mmsystem.h>

//#include "CVector3.h"

// Game Defines
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define SCREEN_BIT_DEPTH 32

extern HGLRC           hRC;							// Permanent Rendering Context
extern HDC             hDC;							// Private GDI Device Context
extern HWND            hWnd;						// Holds Our Window Handle
extern HINSTANCE       hInstance;					// Holds The Instance Of The Application


// This is our basic 3D point/vector class
class CVector3 {
public:
	
	// A default constructor
	CVector3() {}

	// This is our constructor that allows us to initialize our data upon creating an instance
	CVector3(float X, float Y, float Z) 
	{ 
		x = X; y = Y; z = Z;
	}

	// Here we overload the + operator so we can add vectors together 
	CVector3 operator+(CVector3 vVector)
	{
		// Return the added vectors result.
		return CVector3(vVector.x + x, vVector.y + y, vVector.z + z);
	}

	// Here we overload the - operator so we can subtract vectors 
	CVector3 operator-(CVector3 vVector)
	{
		// Return the subtracted vectors result
		return CVector3(x - vVector.x, y - vVector.y, z - vVector.z);
	}
	
	// Here we overload the * operator so we can multiply by scalars
	CVector3 operator*(float num)
	{
		// Return the scaled vector
		return CVector3(x * num, y * num, z * num);
	}

	// Here we overload the / operator so we can divide by a scalar
	CVector3 operator/(float num)
	{
		// Return the scale vector
		return CVector3(x / num, y / num, z / num);
	}

	float Magnitude()
	{
		return sqrtf(x*x + y*y + z*z);
	}

	void normalize()
	{
		float temp = Magnitude();
		x/=temp;
		y/=temp;
		z/=temp;
	}

	CVector3 cross(CVector3 v)
	{
		CVector3 temp;
		temp.x = y*v.z-z*v.y;
		temp.y = z*v.x-x*v.z;
		temp.z = x*v.y-y*v.x;
		return temp;
	}

	float dot(CVector3 v)
	{
		return v.x*x + v.y*y + v.z*z;
	}

	float x, y, z;						
};

#endif /* MAIN_HPP_ */

