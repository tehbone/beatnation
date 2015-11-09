/**
 * @file class defines and whatnot for lights.
 */

#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "utils.hpp"

//Light Types
//FIXME: enum
#define LT_POINT 0
#define LT_SPOT 1

class Light {
	float start; //attenuation start
	float end;  //light end
	Vector3f color;  //lights color(normalized)
	int type;  //type of light
	float brightness;  //how bright the light is
	bool on; //am I on?
	float spotangle;  //Angle for the spotlight

public:
	//FIXME: Should be private.
	Vector3f pos;

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

//	void turnOn();
//	void turnOff();
//	void dim(float offset);

	/**
	 * Computes the color of a vertex based on the normals and the vertex's color.
	 * This is used when either vertex processing is not enabled or when there are
	 * 10 or more lights in the scene.
	 * @param norm the normal vector of the polygon.
	 * @param c the color
	 * @param ambient the ambient color
	 * @param p the position of the point
	 * @param out Array which stores the resulting color.
	 */
	void computeColor(Vector3f norm, Vector3f c, Vector3f ambient, Vector3f p, float* out);
};
#endif /* LIGHT_HPP_ */

