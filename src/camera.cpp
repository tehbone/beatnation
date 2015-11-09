/**
 * @file Camera.cpp
 * Definitions for camera related functions.
 */
#include <gl/gl.h>
#include "camera.hpp"

#define STATIC_CAMERA 1
#define DYNAMIC_CAMERA 2
#define FREE_CAMERA 3
#define FLYBY_LENGTH 1.0f

void 
Camera::setView(Vector3f f,Vector3f pos)
{
	target = pos;
	origin = target - f* maxdistance;
	origin = origin + up*camheight;
	facingDir = f;
	oldFacing = f;
	targetDir = f;
}

void
Camera::setOrigin(Vector3f origin)
{
	this->origin = origin;
}

void
Camera::setUp(Vector3f up)
{
	this->up = up;
	this->up.normalize();
}

void
Camera::setTarget(Vector3f target)
{
	this->target=target;
}

Vector3f
Camera::getUp()
{
	return up;
}

Vector3f
Camera::getOrigin()
{
	return origin;
}

Vector3f
Camera::getTarget()
{
	return target;
}

Vector3f
Camera::getDirection()
{
	Vector3f temp = target-origin;
	temp.normalize();
	return temp;
}

Vector3f
Camera::getRightVector()
{
	Vector3f temp(getDirection());
	temp = temp.cross(up);
	return temp;
}

void
Camera::view(float t)
{
	Vector3f temp,temp2;
	if (flyby) {
		switch (type) {
		case STATIC_CAMERA:
		case DYNAMIC_CAMERA:
			origin =  newpos * time + oldpos*(1.0f-time);
		}

		time+= t/FLYBY_LENGTH;
		if (time > 1.0) {
			flyby = false;
			origin = newpos;
			oldpos = newpos;
		}
	}

	if (type == DYNAMIC_CAMERA && time < 0.5f) {
		float angle = acosf(oldFacing.dot(targetDir));
		float s = oldFacing.cross(targetDir).dot(up);
		s = (s > 0.0f) ? 1.0f : -1.0f;
		time+=t;
		if(time > 0.5f) {
			facingDir = targetDir;
		} else {
			facingDir.x = oldFacing.x*cosf(time*s*angle/0.5f) +oldFacing.z*sinf(time*s*angle/0.5f);
			facingDir.z = oldFacing.z*cosf(time*s*angle/0.5f) -oldFacing.x*sinf(time*s*angle/0.5f);
		}

		facingDir.normalize();		
		origin = target - facingDir*maxdistance;
		origin = origin + up*camheight;
	}
		
	gluLookAt(origin.x,origin.y,origin.z,target.x,target.y,target.z,up.x,up.y,up.z);
}

void
Camera::switchtoView(int view, Vector3f t, Object *object)
{
	Vector3f temp;
	type = view;
	switch (view) {
	case STATIC_CAMERA:
		target = object->position;
		newpos = t;
		oldpos = origin;
		break;
	case DYNAMIC_CAMERA:
		target = object->position;
		temp = getDirection() * -1.0f * maxdistance;
		newpos = temp + target;
		oldpos = origin;
		oldFacing = getDirection();
		facingDir = ((Player*)object)->facing;
		break;
	case FREE_CAMERA:
		target = object->position;
	}

	flyby = true;
	time = 0.0f;
}

void
Camera::move(Vector3f v, Vector3f dir)
{
	Vector3f temp;
	
	switch (type) {
	case STATIC_CAMERA:
		target = v;
		break;
	case DYNAMIC_CAMERA:
		target = v;
		if(facingDir.dot(dir) != 1.0f ) {
			oldFacing = facingDir;
			targetDir = dir;
			time = 0.0f;
			time = time; /* TODO: What is this for? */
		} else {
			origin = dir*(-maxdistance)+ target;
			origin = origin + up*camheight;
		}
		break;
	case FREE_CAMERA:
		temp = v-target;
		target = v;
		origin = origin + temp;
	}
}

void
Camera::initialize()
{
	up.set(0.0f,1.0f,0.0f);
	origin.set(0.0f,0.0f,10.0f);
	target.set(0.0f,0.0f,0.0f);
	type = DYNAMIC_CAMERA;//STATIC_CAMERA;
	flyby = false;
	maxdistance = 200.0f;
	camheight = 300.0f;
}
		
void
Camera::initialize(Vector3f up, Vector3f origin, Vector3f target) 
{
	this->up = up;
	this->origin = origin;
	this->target = target;
	type = FREE_CAMERA; //STATIC_CAMERA;
	flyby = false;
	maxdistance = 300.0f *sqrtf(3);
	camheight = 100.0f;
}

void
Camera::pivot(bool right,float t)
{
	float angle = right ? t*3.14859f*1.5f:t*-3.14859f*1.5f;
	Vector3f temp = facingDir;
	facingDir.x = temp.x*cosf(angle) +temp.z*sinf(angle);
	facingDir.z = temp.z*cosf(angle) -temp.x*sinf(angle);
	origin = target - facingDir*maxdistance;
	origin = origin + up*camheight;
}

