/**
 * @file camera.hpp
 * Class defines and methods for the camera system.
 */
#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "utils.hpp"

class Camera {
public: /* TODO: Really should be private */
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

public:
	void initialize();
	void initialize(Vector3f up, Vector3f origin, Vector3f target);

	void pivot(bool right, float t);

	/**
	 * Moves the camera based on the type.
	 * @param v
	 * @param dir
	 */
	void move(Vector3f v,Vector3f dir);

	/**
	 * Switches to the new view specified by view.
	 * @param t the new position vector
	 * @param object
	 */
	void switchtoView(int view, Vector3f t, Object *object);

	void view(float t);
	void setView(Vector3f f,Vector3f pos);

	void setTarget(Vector3f target);
	void setOrigin(Vector3f origin);
	void setUp(Vector3f up);
	Vector3f getTarget();
	Vector3f getOrigin();
	Vector3f getUp();
	Vector3f getDirection();
	Vector3f getRightVector();
};

#endif /* CAMERA_HPP_ */

