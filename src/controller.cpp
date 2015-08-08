/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS

#include "controller.h"
#include "physics.h"
#include "t_utils.cpp"


//-----------------------------------------------------------------------------------
bool PlayerController::ProcessKeys(bool* keys, float time)
//-----------------------------------------------------------------------------------
{

	if(Controller::controlledObject == NULL) return false;
	
	// Acc in units / sec
	float moveAcc = 100.0f; 
	float jumpAcc = 700.0f;

	// Slowdown of object on surface directions.
	float velocitySlowdown = 0.9f;

	bool hasMoved = false;

	// Holds direction of motion from user
	Vector3f direction;

	// Throwaway position vector, may be needed if forces ever act other than 
	// on center of mass.  Used to create forces.
	Vector3f pos;
	
	Vector3f forward = controlsCam->getDirection();
	forward.set(forward.x, 0.0f, forward.z);
	forward.normalize();
	
	Vector3f right = controlsCam->getRightVector();
	right.set(right.x, 0.0f, right.z);
	right.normalize();

	/*
	 * Finding basis vectors for the planes in which a player can apply force.
	 * For example, if a player stands on a surface, he/she can only walk along
	 * the plane of that surface.  This is generalized to all surfaces here.
	 */
	
	// Holds the surface orthogonal basis vectors
	DLinkedList<Vector3f>* basisVectors = NULL;
	// Holds the sum of all the normal vectors, useful for jumping
	Vector3f normalSum;

	// Find all surface basis vectors
	for(DLinkedList<CollideSpace*>* currCollideSpaceList = controlledObject->lastPhysicsCollisions;
	    currCollideSpaceList != NULL;
		currCollideSpaceList = currCollideSpaceList->nextList)
	{
		Vector3f normalVector = currCollideSpaceList->object->info.normal;
		normalSum = normalSum + normalVector;

		if(normalVector.magnitude() == 0) break;

		/*
		 * There are an infinite number of choices for orthogonal basis vectors, I'm 
		 * choosing one that's easy to calculate.  Need to check a lot of divide 
		 * by zeros though.  The other basis vector is then just the cross product of 
		 * the normal vector and the first basis vector.
		 */
		Vector3f perpVector;
		if(normalVector.y != 0)
			perpVector.set(1.0f, - (normalVector.x + normalVector.z) / normalVector.y, 1.0f);
		else if(normalVector.z != 0)
			perpVector.set(1.0f, 1.0f, - (normalVector.x + normalVector.y) / normalVector.z);
		else if(normalVector.x != 0)
			perpVector.set(- (normalVector.z + normalVector.y) / normalVector.x, 1.0f, 1.0f);

		basisVectors = DLinkedList_AddHeadEntry<Vector3f>(basisVectors, perpVector);
		basisVectors = DLinkedList_AddHeadEntry<Vector3f>(basisVectors, perpVector.cross(normalVector));
		
	}
	if(normalSum.magnitude() > 0) normalSum.normalize();

	//Move up/Forward
	if(keys['W'])
	{
		direction = direction + forward;
	}

	//Move Down/Backward
	if(keys['S'])
	{
		direction = direction + forward * -1.0f;
	}

	//Move Right/Sidestep Right
	if(keys['D'])
	{
		direction = direction + right;
	}

	//Move Left/Sidestep Left
	if(keys['A'])
	{
		direction = direction + right * -1.0f;
	}	

	//Pick up stuff
	if(keys['F'])
	{
		((Object*) controlledObject)->setState(PICKUP);
		keys['F'] = false;
	}
	int state = ((Player*)controlledObject)->state;
	if(state == IDLE || state == WALKING)
	{
		if(keys[VK_NUMPAD4])
		{	
			((Player*)controlledObject)->switchState(ATTACK1);
			
			keys[VK_NUMPAD4] = false;
		}
		if(keys[VK_NUMPAD6])
		{	
			((Player*)controlledObject)->switchState(ATTACK2);
			
			keys[VK_NUMPAD6] = false;
		}
		if(keys[VK_NUMPAD8])
		{	
			((Player*)controlledObject)->switchState(ATTACK3);
			
			keys[VK_NUMPAD8] = false;
		}
		if(keys[VK_NUMPAD5])
		{	
			((Player*)controlledObject)->switchState(ATTACK4);
			
			keys[VK_NUMPAD5] = false;
		}
	}


    //Jump normal to the current touched surface.  If not touching a surface, normalSum = 0
	// and therefore there is no jump.
	if(keys[' '])
	{
		Vector3f components = normalSum * ((jumpAcc /* time*/) * controlledObject->mass);
		controlledObject->AddForce(new Force3f(pos, components, 0.05));
		keys[' '] = false;
		hasMoved = true;
	}

	if(direction.magnitude() != 0) 
	{	
		
		direction.normalize();
		((Player*)controlledObject)->setFacing(direction);
		switch(((Player*)controlledObject)->state)
		{
		case IDLE:
			((Player*)controlledObject)->switchState(WALKING);
		}
	}
	else
	{
		switch(((Player*)controlledObject)->state)
		{
		case WALKING:
			((Player*)controlledObject)->switchState(IDLE);
		}
	}
	
	// Force components
	
	Vector3f components = direction * ((moveAcc /* time*/) * controlledObject->mass);

	// Holds the total force applied to the object
	Vector3f totalForce;	
	// Holds the velocity slowdown in directions along the surface basis vectors
	Vector3f slowV;

	for(DLinkedList<Vector3f>* currBasisVector = basisVectors;
		currBasisVector != NULL;
		currBasisVector = currBasisVector->nextList)
	{
		Vector3f basisVector = currBasisVector->object;
		basisVector.normalize();
		totalForce = totalForce + 
					 basisVector * components.dot(basisVector);
		slowV = slowV + (basisVector) * (controlledObject->velocity).dot(basisVector);
	}

	// Slow down the object
	controlledObject->velocity = (controlledObject->velocity - slowV) + slowV * velocitySlowdown;
	
	// If the object needs to be moved, add the force in the correct direction.
	if(direction.magnitude() != 0){
		hasMoved = true;
		
		Vector3f pos;
		controlledObject->AddForce(new Force3f(pos, totalForce, 0.10f));
	}
	
	// Clean up the basis vector list
	DLinkedList_DeleteTail<Vector3f>(basisVectors); basisVectors = NULL;

	return hasMoved;

}