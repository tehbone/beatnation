
/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Copied newer physics w/ collisions over - GS

#include <stdio.h>
#include "utils.h"
#include "physics.h"
#include "t_utils.cpp"



//---------------------------------------------------------------------------------
Force3f::Force3f() {}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
Force3f::Force3f(Vector3f& pos, Vector3f& components, float decayTime)
//---------------------------------------------------------------------------------
{
	this->pos = pos;
	this->components = components;
	this->decayTime = decayTime;
}

//---------------------------------------------------------------------------------
Force3f::~Force3f()
//---------------------------------------------------------------------------------
{
	return;
}


//---------------------------------------------------------------------------------
ResolvedForce3f::ResolvedForce3f() {}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
ResolvedForce3f::ResolvedForce3f(Vector3f& translation, Vector3f& rotation)
//---------------------------------------------------------------------------------
{
	this->translation = translation;
	this->rotation = rotation;
}

//---------------------------------------------------------------------------------
ResolvedForce3f::~ResolvedForce3f()
//---------------------------------------------------------------------------------
{
	return;
}


//---------------------------------------------------------------------------------
void PhysicsEntity::init(Vector3f& position, float mass)
//---------------------------------------------------------------------------------
{
	this->mass = mass;
	this->position = position;
	this->springK = PHYSICSENTITY_DEFAULT_SPRINGK;
	this->mu = PHYSICSENTITY_DEFAULT_FRICTION_MU;
	this->dissipation = PHYSICSENTITY_DEFAULT_DISSIPATION;
	this->flags = PHYSICSENTITY_DEFAULT_FLAGS;
	this->updatesSinceLastMotion = 0;

	this->forceList = NULL;
	this->lastPhysicsCollisions = NULL;
	this->spaceEntity.physicsEntity = this;
	this->engine = NULL;	
}

//---------------------------------------------------------------------------------
void PhysicsEntity::init()
//---------------------------------------------------------------------------------
{
	Vector3f position;
	init(position, PHYSICSENTITY_DEFAULT_MASS);
}


//---------------------------------------------------------------------------------
void PhysicsEntity::cleanUp()
//---------------------------------------------------------------------------------
{
	
	OctTree::RemoveObject(&spaceEntity);

	DLinkedList_DeleteTailObjects<CollideSpace*>(lastPhysicsCollisions);
	lastPhysicsCollisions = NULL;

	DLinkedList_DeleteTailObjects<Force3f*>(forceList);
	forceList = NULL;
	
	//DLinkedList_DeleteTailObjects<Joint*>(jointList);
	//jointList = NULL;
}

//---------------------------------------------------------------------------------
void PhysicsEntity::AddForce(Force3f* force)
//---------------------------------------------------------------------------------
{
	forceList = DLinkedList_AddHeadEntry<Force3f*>(forceList, force);
	engine->updateMap.Put(this, true);
}

//---------------------------------------------------------------------------------
void PhysicsEntity::UpdatePhysics(DLinkedList<OctTree*>* physicsSpaces, float timeStep)
//---------------------------------------------------------------------------------
{
	if(!engine || !physicsSpaces) return;
	engine->UpdateObject(physicsSpaces, this, timeStep);
}

//---------------------------------------------------------------------------------
ConstrainedJoint::ConstrainedJoint(PhysicsEntity* entityA, PhysicsEntity* entityB)
//---------------------------------------------------------------------------------
{
	this->physEntities[0] = entityA;
	this->physEntities[1] = entityB;
}

//---------------------------------------------------------------------------------
ConstrainedJoint::~ConstrainedJoint()
//---------------------------------------------------------------------------------
{}

//---------------------------------------------------------------------------------
bool ConstrainedJoint::TransferForces(PhysicsEntity* entity, ResolvedForce3f* force)
//---------------------------------------------------------------------------------
{
	return true;
}

//---------------------------------------------------------------------------------
SpringPhysicsEngine::SpringPhysicsEngine()
//---------------------------------------------------------------------------------
{ PhysicsEngine(); }

//---------------------------------------------------------------------------------
SpringPhysicsEngine::~SpringPhysicsEngine()
//---------------------------------------------------------------------------------
{}


//---------------------------------------------------------------------------------
PhysicsEngine::PhysicsEngine()
//---------------------------------------------------------------------------------
{}

//---------------------------------------------------------------------------------
PhysicsEngine::~PhysicsEngine()
//---------------------------------------------------------------------------------
{}

//---------------------------------------------------------------------------------
void PhysicsEngine::Update(DLinkedList<OctTree*>* physicsSpaces, float timeStep)
//---------------------------------------------------------------------------------
{
	int numEntities = updateMap.GetNumEntries();
	PhysicsEntity** entities = updateMap.GetKeys();

	for(int i = 0; i < numEntities; i++){
		PhysicsEntity* entity = entities[i];
		if(entity->engine == this)
			UpdateObject(physicsSpaces, entities[i], timeStep);
	}
}

//---------------------------------------------------------------------------------

void SpringPhysicsEngine::UpdateObject(DLinkedList<OctTree*>* physicsSpaces, PhysicsEntity* entity, float timeStep)
//---------------------------------------------------------------------------------
{
	//if(timeStep <= 0) return;

	//if(timeStep > 0.1) timeStep = 0.1;

	CollideObject* collideObject = &entity->spaceEntity;

	ResolvedForce3f resolvedForce;
	
	ResolveForces(entity, timeStep, &resolvedForce);
	

	if(resolvedForce.translation.magnitude() > 0){
	//	printf("Resolved Force: (%f, %f, %f)\n", 
	//	resolvedForce.translation.x, resolvedForce.translation.y, resolvedForce.translation.z);
	}
	
	Vector3f velocity = entity->velocity;
	velocity = velocity + (resolvedForce.translation / entity->mass) * timeStep;

	Vector3f bounds = collideObject->boundBox.extents;

	/*
	float moveFraction = velocity.magnitude() * timeStep / bounds.magnitude();
	if(moveFraction > SPRINGENGINE_MAX_MOVE_RATIO){
		int numSteps = (int) (moveFraction / SPRINGENGINE_MAX_MOVE_RATIO) + 1;
		float smallStep = timeStep / numSteps;
		for(int i = 0; i < numSteps; i++){
			UpdateObject(physicsSpaces, entity, smallStep);
		}
		return;
	}
	*/
	

	entity->velocity = velocity;

	// TODO:  Possibly add notion of joints, to make sliding on smooth surfaces more efficient?
	//ResolveCurrJoints(entity, &resolvedForce);
	
	Vector3f oldPosition = collideObject->boundBox.pos;
	Vector3f ballPosition(0.0f, 0.0f, 0.0f);
	CalcBallisticPos(entity, resolvedForce, timeStep, &ballPosition);
	
	// TODO:  Need to divide up ballistic trajectory so that really fast objects don't just
	// pass through things, but for now assume everything is slow.

	entity->position = ballPosition;
	collideObject->oldBoundBox.pos = oldPosition;
	collideObject->boundBox.pos = ballPosition;
	
	DLinkedList<CollideSpace*>* fullCollideSpaceList = NULL;

	for(DLinkedList<OctTree*>* currPhysicsSpace = physicsSpaces;
	    currPhysicsSpace != NULL;
		currPhysicsSpace = currPhysicsSpace->nextList)
	{
		currPhysicsSpace->object->CheckObjectHere(collideObject);
		if(collideObject->collideSpaceList != NULL){
			
			if(*(collideObject->collideSpaceList->listSize) > 0){
				collideObject = collideObject;
			}

			DLinkedList<CollideSpace*>* tail = 
				DLinkedList_GetTail<CollideSpace*>(collideObject->collideSpaceList);
			fullCollideSpaceList =
				DLinkedList_AddAfter(collideObject->collideSpaceList, tail, fullCollideSpaceList);
			collideObject->collideSpaceList = NULL;
		}
	}

	if(fullCollideSpaceList != NULL && *(fullCollideSpaceList->listSize) > 2){
		fullCollideSpaceList = fullCollideSpaceList;	
	}


	Vector3f sumNormalForce(0.0f, 0.0f, 0.0f);
	bool collisionsOccur = (fullCollideSpaceList != NULL);

	Vector3f position = entity->position;
	Vector3f positionChange;
	Vector3f velocityChange;

	bool testBool = false;

	float springKA = entity->springK;
	float mass = entity->mass;
	float muA = entity->mu;
	float dissipationA = entity->dissipation;
	int numCollisions = 0;

	for(DLinkedList<CollideSpace*>* collideEntry = fullCollideSpaceList;
	    collideEntry != NULL;
		collideEntry = collideEntry->nextList)
	{
		CollideSpace* collideSpace = collideEntry->object;
		PhysicsEntity* otherEntity = collideSpace->objectB->physicsEntity;
		bool fixedObject = (otherEntity == NULL ||
							((otherEntity->flags & PHYSICSENTITY_FLAG_STATIONARY) != 0));
		Vector3f normal = collideSpace->info.normal;

		BoundBox collideBox = collideSpace->info.box;
		float totalPenetration = OctTree::absf(2 * collideBox.extents.dot(normal));
		Vector3f moveBack = normal * totalPenetration;

		
		Vector3f moveNormal = positionChange;
		if(moveNormal.magnitude() != 0){
			moveNormal.normalize();
		}

		Vector3f movedBackAlready = normal * moveBack.dot(moveNormal);
		moveBack = moveBack - movedBackAlready;

		if(collideBox.extents.x > 3.0f){
			normal = normal;
		}

		//printf("Normal Vector: (%f, %f, %f)\n", normal.x, normal.y, normal.z);

		float springK = springKA;
		float mu = muA;
		float dissipation = dissipationA;
		
		// Combine physical properties if colliding with another physical object
		if(otherEntity != NULL){
			springK = 
				1.0f / ((1.0f / springKA) + (1.0f / otherEntity->springK));
			mu = (muA + otherEntity->mu) / 2.0f;
			dissipation = (dissipationA + otherEntity->dissipation) / 2;
		}

		float dissipationCoeff = (float) sqrt(1.0f - dissipation);
		
		// Max penetration when 1/2mv^2 = 1/2kx^2 (energy balance)
		// Solved for x.

		if(moveBack.y > 3.0f){
			moveBack = moveBack;
		}


		float vPerp = velocity.dot(normal);
		if(vPerp > 0) vPerp = 0.0f;
		float w = (float) sqrt((double) (springK / mass));
		float A = OctTree::absf(vPerp / w);
				
		// Calculating moving back amount: 
		// Find projection of moveBackVector on normal Vector, then use that to find 
		// number of units to move back along normal vector, which is same as number of
		// units to move back along moveBackVector
		Vector3f velocityNormal = normal * vPerp;
		Vector3f velocityParallel = velocity - velocityNormal;

		
		// Calculating Frictional Stuff
		// 
		float vPara = velocityParallel.magnitude();

		float vConstant = (mu * springK * A) / (w * mass);
		
		bool objectStops = (OctTree::absf(1.0f - vPara / vConstant) <= 1.0f);
		float tStop = 0;

		if(objectStops) tStop = acosf(1 - vPara / vConstant) / w;
		else tStop = PIf / w;

		//NOTE:  ONLY ELASTIC COLLISIONS ARE MODELLED NOW

		float deltaVA = 0;		
		if(!fixedObject){

			float otherMass = otherEntity->mass;
			float otherVPerp = otherEntity->velocity.dot(normal);

			if(OctTree::absf(vPerp) < OctTree::absf(otherVPerp)) continue;

			float vRelative = (vPerp - otherVPerp);

			float massRatio = mass / otherMass;

			float dissCoeff = 0.8f;

			float a = mass * massRatio * massRatio + otherMass;
			float b = 2 * mass * massRatio * (vRelative);
			float c = mass * vRelative * vRelative * (1.0f - dissCoeff);

			// InElastic
			float test = b*b - 4.0f * a * c;
			float deltaVB = 0;
			if(test <= 0){
				deltaVB = vRelative / (1 + massRatio);
			}
			else{
				deltaVB = -((-b + sqrt(test)) / (2.0f * a));
			}
			deltaVA = (vRelative - massRatio * deltaVB);
			
			// Elastic
			//float deltaVB = (2 * mass * vRelative) / (mass + otherMass);
			//deltaVA = -(deltaVB - vPerp);
			
			
			deltaVB = (deltaVB + otherVPerp);// * dissipationCoeff;
			deltaVA = (deltaVA + otherVPerp);// * dissipationCoeff;
		
			if(deltaVA > 10.0f){
				deltaVA = deltaVA;
			}

			
			// Change the velocity of the other object
			//Vector3f pos;
			//Vector3f components = normal * (mass * vPerp * 2 / tStop);
			//otherEntity->AddForce(new Force3f(pos, components, tStop));
			otherEntity->velocity = otherEntity->velocity + normal * deltaVB;
			otherEntity->engine->updateMap.Put(otherEntity, true);

			vPerp = vRelative;
		}
		else{
			deltaVA = vPerp * dissipationCoeff;
		}

		float xStop = (vPara - vConstant) * tStop + vConstant * sinf(w * tStop) / w;

		if(objectStops) vPara = 0;
		else vPara = vPara - 2 * vConstant;
		
		Vector3f unitVelocityParallel = velocityParallel;
		if(velocityParallel.magnitude() != 0)
			 unitVelocityParallel.normalize();
		
		//Vector3f moveUnitVector = ballPosition - oldPosition; moveUnitVector.normalize();
		//float backUnitSize = moveUnitVector.dot(normal);
		//float backUnits = totalPenetration / backUnitSize;
		//Vector3f moveBack = moveUnitVector * backUnits;

		Vector3f positionParaChange = velocityParallel * xStop;
		positionChange = positionChange + moveBack;/* moveBack + positionParaChange; */
		velocityChange = velocityChange + (unitVelocityParallel * vPara - velocityParallel);
		velocity = velocity - (normal * (vPerp + deltaVA));

		vPerp = vPerp;
		if(deltaVA / vPerp > 0.9){	
			vPara = vPara;
		}
		// TODO:  Figure out some way to route the colliding spring constant with this one
		// i.e. need to link together CollideObjects and PhysicsEntities (probably will be done
		// soon, just not now

		// Normal force in spring engine = springK * x;
		//sumNormalForce = sumNormalForce + 
						//((normal * (boxExtents.dot(normal))) * entity->springK);
		numCollisions++;
	}

	entity->position = ballPosition + positionChange;
	entity->velocity = velocity + velocityChange;

	// Stop evaluating physics on an object if the object is moving too slowly
	float moveDist = (oldPosition - entity->position).magnitude();
	if(moveDist < PHYSICSENGINE_MOTION_CUTOFF){
		entity->updatesSinceLastMotion++;
		if(entity->updatesSinceLastMotion > PHYSICSENGINE_UPDATE_CUTOFF){
			updateMap.Remove(entity);
			entity->updatesSinceLastMotion = 0;
		}
	}
	else entity->updatesSinceLastMotion = 0;

	collideObject->boundBox.pos = entity->position;
	collideObject->oldBoundBox.pos = oldPosition;
	entity->lastPhysicsCollisions = fullCollideSpaceList;

	// here for now, so I don't forget
	OctTree::UpdateObject(&entity->spaceEntity, false);

	// Create normal force from vector
/*
	Vector3f pos(0.0f, 0.0f, 0.0f);
	Force3f* normalForce = new Force3f(pos, sumNormalForce);
	
	if(collisionsOccur)
		entity->forceList = DLinkedList_AddHeadEntry<Force3f*>(entity->forceList, normalForce);
*/
	//entity->pos = ballPosition;
	//entity->velocity = entity->velocity + resolvedForce.translation / entity->mass;


}

//---------------------------------------------------------------------------------
//void SpringPhysicsEngine(PhysicsEntity* entity, 
//---------------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------------
void ResolveCurrJoints(PhysicsEntity* entity, ResolvedForce3f* resolvedForce)
//---------------------------------------------------------------------------------
{

	DLinkedList<Joint*>* jointList = entity->jointList;

	for(DLinkedList<Joint*>* jointEntry = entity->jointList;
		jointEntry != NULL;
		jointEntry = jointEntry->nextList)
	{
		Joint* currJoint = jointEntry->object;
		bool breakJoint = currJoint->TransferForces(entity, resolvedForce);

		if(breakJoint){
			jointEntry = DLinkedList_DeleteEntry<Joint*>(jointEntry, currJoint);
			if(jointEntry == NULL) break;
		}	
	}
}
*/



//---------------------------------------------------------------------------------
void SpringPhysicsEngine::ResolveForces(PhysicsEntity* entity, float timeStep, ResolvedForce3f* resolvedForce)
//---------------------------------------------------------------------------------
{
	// Add gravity to the resolved forces
	Vector3f gravity(0.0, (-9.8f * entity->mass), 0.0f);
	resolvedForce->translation = resolvedForce->translation + gravity;

	// For now, only calculate forces assuming they act on center of gravity
	
	DLinkedList<Force3f*>* forceEntry = entity->forceList;
	while(forceEntry != NULL)
	{	
		float timeLeft = forceEntry->object->decayTime;
		float timeRemaining = timeLeft - timeStep;
		if(timeRemaining > 0){
			resolvedForce->translation = resolvedForce->translation + forceEntry->object->components;
			forceEntry->object->decayTime = timeRemaining;
			
			forceEntry = forceEntry->nextList;
		}
		else{
			resolvedForce->translation = resolvedForce->translation + forceEntry->object->components * (timeLeft / timeStep);
		
			DLinkedList<Force3f*>* currEntry = forceEntry;
			forceEntry = forceEntry->nextList;
			entity->forceList = DLinkedList_Remove<Force3f*>(entity->forceList, currEntry);
			DLinkedList_DeleteTailObjects(currEntry); currEntry = NULL;						
		}
	}

	resolvedForce->rotation.set(0.0f, 0.0f, 0.0f);

}


//---------------------------------------------------------------------------------
void SpringPhysicsEngine::CalcBallisticPos(PhysicsEntity* entity,
					  ResolvedForce3f resolvedForce,
					  float timeStep,
					  Vector3f* newPos)
//---------------------------------------------------------------------------------
{	
	// Get acceleration
	ResolvedForce3f acceleration;
	acceleration.translation = resolvedForce.translation / entity->mass;

	*newPos =/* Position */
			 entity->position +
			 /* Velocity */
			 (entity->velocity * timeStep);// + 
			 /* Acceleration */
			 //(acceleration.translation * timeStep * timeStep * 0.5f);

}

//---------------------------------------------------------------------------------
void CreateCollideJoints(PhysicsEntity* entity,
						 DLinkedList<CollideSpace*>* collideList)
//---------------------------------------------------------------------------------
{
	
	
}