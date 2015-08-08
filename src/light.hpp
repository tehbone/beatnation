/****************************************
*										*
*				Light.h					*
*		Class defines and whatnot for	*
*				lights					*
*										*
****************************************/

/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS


#ifndef LIGHT_H
#define LIGHT_H

#include "utils.hpp"


//Light Types
#define LT_POINT 0
#define LT_SPOT 1
//--------------------------------------
class Light
//--------------------------------------
{
public:
	Light(Vector3f p,Vector3f color,int t = LT_POINT,float angle = 0.0f)
	{
		type = t;
		this->color = color;
		spotangle = angle;
		brightness = 1.0f;
		start = end = 0.0f;
		pos = p;
		on = true;
	}
	//*****************************************
	//  void turnOn(void)
	//  turns the light on
	//*****************************************
	void turnOn();

	//*****************************************
	//  void turnOff(void)
	//  turns the light off
	//*****************************************
	void turnOff();

	//*****************************************
	//  void dim(float)
	//  brightens the light by + offset
	//*****************************************
	void dim(float offset);

	//*****************************************
	//  Vector3f computeColor(Vector3f,Vector3f,Vector3f)
	//  computest the color of a vertex based on 
	//  the normals and the vertex's color.
	//  to be used either when Vertex processing
	//  is not enabled, or when there are 10+
	//  lights in the scene.
	//*****************************************
	void computeColor(Vector3f norm, Vector3f c, Vector3f ambient, Vector3f p, float* out);

	Vector3f pos;
private:
	float start; //attenuation start
	float end;  //light end
	Vector3f color;  //lights color(normalized)
	int type;  //type of light
	float brightness;  //how bright the light is
	bool on; //am I on?
	float spotangle;  //Angle for the spotlight
	

};
	
	
#endif