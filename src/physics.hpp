/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Copied newer physics w/ collisions over - GS

#include "collision.hpp"
#include "t_utils.hpp"
#include "utils.hpp"

#ifndef PHYSICS_H
#define PHYSICS_H

class Force3f{
	
public:
	Force3f();
	Force3f(Vector3f& pos, Vector3f& components, float decayTime);
	virtual ~Force3f();

	Vector3f pos;
	Vector3f components;

	float decayTime;
};

class ResolvedForce3f{
	
public:
	ResolvedForce3f();
	ResolvedForce3f(Vector3f& translation , Vector3f& rotation);
	virtual ~ResolvedForce3f();

	Vector3f translation;
	Vector3f rotation;
};

typedef ResolvedForce3f Constraints3f;

class PhysicsEngine;
class Joint;

#define PHYSICSENTITY_DEFAULT_SPRINGK 5000.0f
#define PHYSICSENTITY_DEFAULT_FRICTION_MU 0.5f
#define PHYSICSENTITY_DEFAULT_DISSIPATION 1.0f
#define PHYSICSENTITY_DEFAULT_MASS 1.0f

#define PHYSICSENTITY_DEFAULT_FLAGS 0x00
#define PHYSICSENTITY_FLAG_STATIONARY 0x01

#define PIf 3.14159f

class PhysicsEntity {
	
public:
	PhysicsEntity(){ init(); }
	virtual void init();

	PhysicsEntity(Vector3f& position, float mass) { init(position, mass); }
	virtual void init(Vector3f& position, float mass);

	virtual ~PhysicsEntity(){ cleanUp(); }
	virtual void cleanUp();

	void AddForce(Force3f* force);

	Vector3f position;
	Vector3f velocity;

	// Mass of the entity
	float mass;
	// Hardness of the entity
	float springK;
	// Friction along the entity
	float mu;
	// Elasticity of the entity
	float dissipation;

	int flags;

	CollideObject spaceEntity;
	DLinkedList<Force3f*>* forceList;
	DLinkedList<CollideSpace*>* lastPhysicsCollisions;

	//DLinkedList<Joint*>* jointList;

	// Engine to use when evaluating new entity position
	PhysicsEngine* engine;
	
	// Update an entity's fields using the physics engine
	void UpdatePhysics(DLinkedList<OctTree*>* physicsSpaces, float timeStep);
	
	int updatesSinceLastMotion;

	// From CollideObject, temp
	int up;
	float time;
	int maxtime;
};


class Joint {

public:
	Joint() {}
	virtual ~Joint() {}
	virtual bool TransferForces(PhysicsEntity* entity, ResolvedForce3f* forces)
		{ return true; }

	PhysicsEntity* physEntities[2];
};

class ConstrainedJoint : public Joint {

public:
	ConstrainedJoint(PhysicsEntity* entityA, PhysicsEntity* entityB);
	virtual ~ConstrainedJoint();
	
	virtual bool TransferForces(PhysicsEntity* entity, ResolvedForce3f* forces);

	Constraints3f breakLimits;
	Constraints3f moveLimits;
};


#define PHYSICSENGINE_MOTION_CUTOFF 0.01f
#define PHYSICSENGINE_UPDATE_CUTOFF 10

class PhysicsEngine {

public:
	PhysicsEngine();
	virtual ~PhysicsEngine();
	
	void Update(DLinkedList<OctTree*>* physicsSpaces, float timeStep);
	virtual void UpdateObject(DLinkedList<OctTree*>* physicsSpaces, PhysicsEntity* entity, float timeStep){ return; }

	HashMap<PhysicsEntity*,bool> updateMap;
	CollideMap<PhysicsEntity*, PhysicsEntity*, bool> collisionMap;
};

#define SPRINGENGINE_MAX_MOVE_RATIO 1000.5f

#define SPRINGPHYSICSENGINE_SPEED_CUTOFF 0.01f

class SpringPhysicsEngine : public PhysicsEngine {

public:
	SpringPhysicsEngine();
	virtual ~SpringPhysicsEngine();
	
	virtual void UpdateObject(DLinkedList<OctTree*>* physicsSpaces, PhysicsEntity* entity, float timeStep);
	
	void ResolveForces(PhysicsEntity* entity, float timeStep, ResolvedForce3f* resolvedForce);
	
	void CalcBallisticPos(PhysicsEntity* entity,
						  ResolvedForce3f resolvedForce,
						  float timeStep,
						  Vector3f* newPos);

};

#endif
