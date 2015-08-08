/****************************************
*										*
*				objects.h				*
*	class defines for objects			*
*										*
****************************************/

/************************************
*	Versioning Information			*
************************************/
//last update: 3/22/04
// 4/12/2004
// See Objects.cpp
// Rerouted all the object constructors through init() functions.
// Fixed bug with unset Object::controller variable.  - Love, GS


#ifndef OBJECTS_H
#define OBJECTS_H

#include "physics.h"
#include "utils.h"
#include "controller.h"
//#include <cg/cg.h>
//#include <cg/cggl.h>
#include "model.h"
#include <stdio.h>
#include <fstream>


#define DONOTHING 0
#define REMOVE_OBJECT 1

//************PLAYER*STATES************//
//movement states
#define DEAD 0
#define IDLE 1
#define STANDING 2
#define WALKING 3
#define RUNNING 4
#define JUMPING 5
#define DOUBLE_JUMP 6
#define CROUCHING 7
#define FALLING 7
#define DYING 8
#define PICKUP 9
#define THROWING 10
#define ATTACK1 11
#define ATTACK2 12
#define ATTACK3 13
#define ATTACK4 14
#define ATTACK5 15
#define ATTACK6 16
#define ATTACK7 17
#define ATTACK8 18
#define ATTACK9 19
#define ATTACK10 20
#define RESPAWN 21

//status states

//animation states
#define NORM 0
#define HIT 1
#define HURT 2

#define INC_FUNK 4
#define INC_HEALTH 5

//************ITEM*STATES*************//
#define DESTROY 0
//#define IDLE 1
#define THROWN 2
#define CARRIED 3


//************PROTOTYPES**************//
#define OBJECT_TYPE_ITEM 1
class Item;

#define OBJECT_TYPE_NULL 0
class Object;

#define OBJECT_TYPE_BULLET 2
class Bullet;

#define OBJECT_TYPE_PLAYER 3
class Player;


//***********OBJECT FLAGS**********//
#define OBJECT_FLAG_MOVABLE 0x01



//**********************************************************
class Object : public PhysicsEntity
//**********************************************************
{
	public:
		Object(){ init(); }
		virtual void init();

		virtual ~Object(){ cleanUp(); }
		virtual void cleanUp();

		int type;
		
		virtual void draw(CGparameter color, const Light &l,float t){ return; };

		void setState(int state);
		void setFlag(unsigned flag);
		void unsetFlag(unsigned flag);

		int state;
		unsigned objectFlags;

		float health, funk;
		
		float drawtime;
        Model **model;
		int numModels;
		float frametime;
		Controller* controller;
		float clock;
		Vector3f modelPosition;
		// Depreciated
		Vector3f up;
		float speed, yVel;
		
};
	
//**********************************************************
class Bullet : public Object
//**********************************************************
{
public:
	//constructors
	Bullet() 
	{
		Object::init();
		type = OBJECT_TYPE_BULLET;
		model = NULL;
		time = 0.0f;
		speed = 3.0f;
	}

	Bullet(Vector3f p, Vector3f v, float s, Player* o)
	{
		Object::init();
		type = OBJECT_TYPE_BULLET;

		position = p;
		velocity = v;
		time = 0.0f;

		spaceEntity.boundBox.pos = p;
		spaceEntity.boundBox.extents.x = spaceEntity.boundBox.extents.y = spaceEntity.boundBox.extents.z = 1.5f;

		owner = o;
		health = -25;
		funk = 0;
	}

	~Bullet()
	{
		cleanUp();
	}

	// Methods
	void draw(CGparameter color, float t);
	int update(float t,OctTree* space,OctTree* playerSpace,OctTree* bulletSpace);
	
private:
	float time;
	Player* owner;
};

//**********************************************************
class Player : public Object
//**********************************************************
{
public:
	//constructors
	
	Player(char* filename)
	{
		score = 0;
		int i;
		int tempint;
		lastHit = NULL;
		meleeActive[0] = false;
		meleeActive[1] = false;
		meleeActive[2] = false;
		meleeActive[3] = false;
		Object::init();
		type = OBJECT_TYPE_PLAYER;
		
		
		state = IDLE;
		facing.set(0.0f,0.0f,-1.0f);
		carriedItem = NULL;
		weapon = NULL;
		drawtime = 0;
		controller = (Controller*) new PlayerController();
		((PlayerController*)controller)->controlledObject = this;
		spaceEntity.collisionTypes = ENTITY_COLLISION_TYPE_BOUNDING_BOX;
		std::ifstream file(filename);
		char tempchar[256];
		file >> name;
		file >> maxHealth;
		health = maxHealth;
		file >> maxFunk;
		funk = maxFunk;
		file >> numModels;
		model = new Model*[numModels];
		for(i = 0; i < numModels; i++)
		{
			model[i] = NULL;
		}
		for(i = 0; i < numModels; i++)
		{
			file >> tempchar;
			model[i] = new Model(tempchar);
		}
		file >> tempchar;
		statusface[0] = LoadTGATexture(tempchar);
		file >> numAnimations;
		animationSets = new Animation[numAnimations];
		for(i = 0; i < numAnimations; i++)
		{
			file >> animationSets[i].name;
			file >> animationSets[i].begin;
			file >> animationSets[i].end;
			file >> animationSets[i].beginloop;
			file >> animationSets[i].state;
			file >> tempchar;
			animationSets[i].rh = tempchar[3] == 'y';
			animationSets[i].rf = tempchar[2] == 'y';
			animationSets[i].lh = tempchar[1] == 'y';
			animationSets[i].lf = tempchar[0] == 'y';

		}
		file.close();
		float maxY=model[0]->frames[0].max_bounds.y;
		float minY=model[0]->frames[0].min_bounds.y;
		for(i=1; i <numModels; i++)
		{
			if(model[i]->frames[0].max_bounds.y > maxY)
				maxY = model[i]->frames[0].max_bounds.y;
			if(model[i]->frames[0].min_bounds.y < minY)
				minY = model[i]->frames[0].min_bounds.y;
		}
		Vector3f maxV(model[0]->frames[0].max_bounds.x,model[0]->frames[0].max_bounds.y,model[0]->frames[0].max_bounds.z);
		Vector3f minV(model[0]->frames[0].min_bounds.x,model[0]->frames[0].min_bounds.y,model[0]->frames[0].min_bounds.z);
		maxV.y = maxY;
		minV.y = minY;
		spaceEntity.boundBox.pos = position;
		spaceEntity.boundBox.extents = (maxV-minV)/2.0f;
		
		statusState = NORM;
		recoverytime=1.0f;
		defaultWeapons[0].boundBox.extents.set(5.0f,5.0f,5.0f);
		defaultWeapons[1].boundBox.extents.set(5.0f,5.0f,5.0f);
		defaultWeapons[2].boundBox.extents.set(5.0f,5.0f,5.0f);
		defaultWeapons[3].boundBox.extents.set(5.0f,5.0f,5.0f);
		switchState(IDLE);
	}

	bool physics;

	~Player()
	{
		cleanUp();
		if(animationSets)
		{
			delete [] animationSets;
		}
		
	}

	//methods
	void spawn(Vector3f v, bool death);
	void dumbAI();
	bool hurt(int damage);
	void switchState(int newstate);
//************************************************
//  void draw(CGparameter)
//  draws a representation of the player.
//  CGparameter only matters when the system is using
//  the CG library.  Usually set to the color param
//  in GameApp.
//************************************************
	void draw(CGparameter color, Light *l, float t);

//************************************************
//  int update(float,Vector3f,Octree*)
//  int update(float)
//  these functions update the position and such based
//  on the change of time.  the 1 argument version
//  assumes no movement.
//  the 3 argument version takes in the change in
//  time, acceleration vector, and current space
//  the object is in.
//************************************************
	// Use this update for physics
	int update(float t, Vector3f v, DLinkedList<OctTree*>* physicsSpaces, OctTree* itemSpace, OctTree* playerSpace);
	int update(float t) {return 0;}

//************************************************
//  bool isDead(void)
//  returns whether or not the character is dead
//************************************************
	bool isDead();


//************************************************
//  void throwItem(void)
//  Throws the currently held item
//************************************************
	void throwItem();

//************************************************
//  Getters and setters
	Vector3f getPosition();
	Vector3f getVelocity();
	void setFacing(Vector3f v);
	bool isCarrying();

	// (Deprecated)
	//void setYVel(float v);
//************************************************


	//fields

protected:
	//(Deprecated, now in Controller class)
	//float maxSpeed; //maximum speed the character can move
	//float jumpSpeed;
public:
	Vector3f facing; //current direction character is facing
	int maxHealth;
	int maxFunk;
	unsigned int statusface[2];
    int statusState;
	float recoverytime;
	float oldhealth;
	int numAnimations;
	CollideObject defaultWeapons[4];
	bool meleeActive[4];
	Animation* animationSets;
	Vector3f lastWeaponPos[4];
	Player* lastHit;
	Item *carriedItem;
	Animation* curAnimation;
	int score;
	float deathtimer;
protected:
	
	int power;
	
	
	Item *weapon;
	
	char name[256];
	
};

//************************************************
class Item : public Object
//************************************************
{
public:
	

	Item(float x, float y, float z)
	{
		drawArea = false;
		Object::init();
		type = OBJECT_TYPE_PLAYER;
		model = NULL;
		position.x = x;
		position.y = y;
		position.z = z;

		Vector3f extents(10.0f, 10.0f, 10.0f);
		Vector3f extentsModifier(15.0f,15.0f,15.0f);
		spaceEntity.boundBox.extents = extents;
		spaceEntity.boundBox.pos = position;
		pickUpArea.boundBox.extents = extents + extentsModifier;

		mu = 0.1f;

		state = IDLE;

		blink = 0.0f;
		health = 30;
		funk = 0;
		carryable = true;
		carrier = NULL;
	}

	~Item()
	{
		cleanUp();
	}

	void draw(CGparameter color, float t)
	{
		BoundBox boundBox = spaceEntity.boundBox;
		blink+=t;
		if(blink > 1.0f)
			blink = 0.0f;
		/*if(state == CARRIED)
		{
			position = carrier->getPosition();
			boundBox.pos = position;
		}*/
		if(drawArea)
		{
			glColor3f(0.25f,0.25f,1.0f);
			pickUpArea.boundBox.draw();
		}
		glColor3f(0.5f+0.5f*sinf(2*3.1485*blink/2.0f),0.0f,0.0f);
		glBegin(GL_QUADS);
			glVertex3f(position.x - boundBox.extents.x, position.y + boundBox.extents.y, position.z + boundBox.extents.z);
			glVertex3f(position.x - boundBox.extents.x, position.y - boundBox.extents.y, position.z + boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y - boundBox.extents.y, position.z + boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y + boundBox.extents.y, position.z + boundBox.extents.z);

			glVertex3f(position.x + boundBox.extents.x, position.y + boundBox.extents.y, position.z - boundBox.extents.z);	
			glVertex3f(position.x + boundBox.extents.x, position.y - boundBox.extents.y, position.z - boundBox.extents.z);
			glVertex3f(position.x - boundBox.extents.x, position.y - boundBox.extents.y, position.z - boundBox.extents.z);
			glVertex3f(position.x - boundBox.extents.x, position.y + boundBox.extents.y, position.z - boundBox.extents.z);

			glVertex3f(position.x - boundBox.extents.x, position.y + boundBox.extents.y, position.z - boundBox.extents.z);	
			glVertex3f(position.x - boundBox.extents.x, position.y + boundBox.extents.y, position.z + boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y + boundBox.extents.y, position.z - boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y + boundBox.extents.y, position.z + boundBox.extents.z);

			glVertex3f(position.x - boundBox.extents.x, position.y - boundBox.extents.y, position.z - boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y - boundBox.extents.y, position.z - boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y - boundBox.extents.y, position.z + boundBox.extents.z);
			glVertex3f(position.x - boundBox.extents.x, position.y - boundBox.extents.y, position.z + boundBox.extents.z);

			glVertex3f(position.x - boundBox.extents.x, position.y + boundBox.extents.y, position.z - boundBox.extents.z);
			glVertex3f(position.x - boundBox.extents.x, position.y - boundBox.extents.y, position.z - boundBox.extents.z);
			glVertex3f(position.x - boundBox.extents.x, position.y - boundBox.extents.y, position.z + boundBox.extents.z);
			glVertex3f(position.x - boundBox.extents.x, position.y + boundBox.extents.y, position.z + boundBox.extents.z);

			glVertex3f(position.x + boundBox.extents.x, position.y + boundBox.extents.y, position.z + boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y - boundBox.extents.y, position.z + boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y - boundBox.extents.y, position.z - boundBox.extents.z);
			glVertex3f(position.x + boundBox.extents.x, position.y + boundBox.extents.y, position.z - boundBox.extents.z);
		glEnd();
	}

	int update(float t,OctTree* space,OctTree* itemSpace, OctTree *playerSpace);
    bool isDead();
	void toss(Vector3f v, float strength);
	
public:
	float blink;
	bool carryable;
	Player *carrier;
	CollideObject pickUpArea;

	//DEBUG VARIABLES
	bool drawArea;

	
};

#endif