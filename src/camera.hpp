/****************************************
*										*
*				Camera.hpp				*
*	Class defines and methods for the	*
*	Camera system						*
*										*
****************************************/

/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS


#ifndef CAMERA_H
#define CAMERA_H

#include "utils.hpp"
#include "objects.hpp"
#include <gl/gl.h>
//#include <gl/glaux.h>


#define STATIC_CAMERA 1
#define DYNAMIC_CAMERA 2
#define FREE_CAMERA 3
#define FLYBY_LENGTH 1.0f;


class Camera
{
public:
	//constructors

	//Initializers
	void initialize();
	void initialize(Vector3f up, Vector3f origin, Vector3f target);

	
    void pivot(bool right,float t);
//*************************************************************
// void move(Vector3f)
// moves the camera based on what type it is
// STATIC_CAMERA: stationary camera. works fine
// DYNAMIC_CAMERA: stationary camera within a certain distance
//        otherwise, follows player (buggy)
// EYE_CAMERA: First Person camera. Not implemented
// argument is the new poistion vector of the target.
//*************************************************************
	void move(Vector3f v,Vector3f dir);

//*************************************************************
// void switchtoView(int, Vector3f, Object*)
// switches to the new view specified by view
// t is the new position vector
//*************************************************************
	void switchtoView(int view, Vector3f t, Object *object);

//*************************************************************
// void view(float)
// sets up the OpenGL modelview matrix correctly.  Also handles
// transition from one view to another.
// transition is done linearly.
//*************************************************************
	void view(float t);
	void setView(Vector3f f,Vector3f pos);
//*************************************************************
//  Getters and Setters
	void setTarget(Vector3f target);
	void setOrigin(Vector3f origin);
	void setUp(Vector3f up);
	Vector3f getTarget();
	Vector3f getOrigin();
	Vector3f getUp();
	Vector3f getDirection();
	Vector3f getRightVector();
//*************************************************************
	
public:
	Matrix4f camMatrix; //not used currently
	Vector3f up, origin, target; //used to create a basis for the camera
	Vector3f oldpos, newpos; //used for camera transistions.
	int type;  //camera type
	float maxdistance;  //maximum distance for DYNAMIC_CAMERA
	float time;  //necessary for camera transistions.
	bool flyby;  //determines whether or not we need a camera transition
	Vector3f facingDir;
	Vector3f oldFacing;
	Vector3f targetDir;
	float camheight;
};

#endif
