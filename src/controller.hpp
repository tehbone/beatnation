#ifndef CONTROLLER_HPP_
#define CONTROLLER_HPP_

class Object;
class Camera;

class Controller {
public:
	// FIXME: Should be private
	Object *controlledObject;

public:
	Controller() {}
	virtual ~Controller() {}
	virtual bool ProcessKeys(bool *keys, float time) { return false; } 
};

class PlayerController : public Controller {
public:
	// FIXME: Should be private
	Camera *controlsCam;
public:
	PlayerController() {}
	virtual ~PlayerController() {}

	virtual bool ProcessKeys(bool* keys, float time);
};

#endif /* CONTROLLER_HPP_ */

