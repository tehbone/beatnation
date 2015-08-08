/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS

#ifndef CONTROLLER_HQ
#define CONTROLLER_HQ

class Object;
class Camera;

class Controller {

public:
	Controller(){ return; }

	Object* controlledObject;

	virtual bool ProcessKeys(bool* keys, float time){ return false; } 

};

class PlayerController : public Controller {

public:
	PlayerController(){ Controller(); }

	Camera* controlsCam;

	virtual bool ProcessKeys(bool* keys, float time);

};

#include "camera.hpp"
#include "objects.hpp"

#endif
