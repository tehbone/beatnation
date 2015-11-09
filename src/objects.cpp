#include <GL/gl.h>
#include <cstdlib>
#include "objects.hpp"
#include "collision.hpp"

void Object::init(){
	PhysicsEntity::init();
	
	this->type = OBJECT_TYPE_NULL;
	this->objectFlags = 0;
	this->setFlag(OBJECT_FLAG_MOVABLE);
	this->controller = NULL;
	frametime = 0.0f;
}

void Object::cleanUp(){
	PhysicsEntity::cleanUp();
	
	if(controller)
	{
		delete controller;
		controller = NULL;
	
		if(model)
	{
		for(int i=0; i < numModels;i++)
		{
			if(model[i])
			{
				model[i]->Cleanup();
				delete model[i];
				model[i] = NULL;
			}
		}
		delete [] model;
	}
	}
	
	
}

void Bullet::draw(CGparameter color, float t)
{
	//cgGLSetParameter4f(color, 1.0f,0.0f,0.0f,1.0f);
	glColor3f(0.0f,1.0f,0.0f);
	glBegin(GL_QUADS);
		glVertex3f(position.x - 1.5f, position.y + 1.5f, position.z + 1.5f);
		glVertex3f(position.x - 1.5f, position.y - 1.5f, position.z + 1.5f);
		glVertex3f(position.x + 1.5f, position.y - 1.5f, position.z + 1.5f);
		glVertex3f(position.x + 1.5f, position.y + 1.5f, position.z + 1.5f);

		glVertex3f(position.x + 1.5f, position.y + 1.5f, position.z - 1.5f);
		glVertex3f(position.x + 1.5f, position.y - 1.5f, position.z - 1.5f);
		glVertex3f(position.x - 1.5f, position.y - 1.5f, position.z - 1.5f);
		glVertex3f(position.x - 1.5f, position.y + 1.5f, position.z - 1.5f);

		glVertex3f(position.x - 1.5f, position.y + 1.5f, position.z - 1.5f);
		glVertex3f(position.x - 1.5f, position.y + 1.5f, position.z + 1.5f);
		glVertex3f(position.x + 1.5f, position.y + 1.5f, position.z - 1.5f);
		glVertex3f(position.x + 1.5f, position.y + 1.5f, position.z + 1.5f);

		glVertex3f(position.x - 1.5f, position.y - 1.5f, position.z - 1.5f);
		glVertex3f(position.x + 1.5f, position.y - 1.5f, position.z - 1.5f);
		glVertex3f(position.x + 1.5f, position.y - 1.5f, position.z + 1.5f);
		glVertex3f(position.x - 1.5f, position.y - 1.5f, position.z + 1.5f);

		glVertex3f(position.x - 1.5f, position.y + 1.5f, position.z - 1.5f);
		glVertex3f(position.x - 1.5f, position.y - 1.5f, position.z - 1.5f);
		glVertex3f(position.x - 1.5f, position.y - 1.5f, position.z + 1.5f);
		glVertex3f(position.x - 1.5f, position.y + 1.5f, position.z + 1.5f);

		glVertex3f(position.x + 1.5f, position.y + 1.5f, position.z + 1.5f);
		glVertex3f(position.x + 1.5f, position.y - 1.5f, position.z + 1.5f);
		glVertex3f(position.x + 1.5f, position.y - 1.5f, position.z - 1.5f);
		glVertex3f(position.x + 1.5f, position.y + 1.5f, position.z - 1.5f);
	glEnd();
}

int Bullet::update(float t,OctTree* space, OctTree* playerSpace, OctTree* projectileSpace)
{
	position = position + velocity * speed*t;
	
	spaceEntity.boundBox.pos = position;
	
	time+=t;
	space->CheckObjectHere(&spaceEntity);
	DLinkedList<CollideSpace*>* collideSpaceList = spaceEntity.collideSpaceList;
	if(time >= 1.0f || collideSpaceList != NULL)
		state = DESTROY;
	if(state != DESTROY)
	{
		Player* target = NULL;
		playerSpace->CheckObjectHere(&spaceEntity);
		for(DLinkedList<CollideSpace*>* tempList = collideSpaceList; tempList != NULL; tempList = tempList->nextList)
		{
			target = (Player*)(((CollideSpace*)(tempList->object))->objectB);
			if(target != owner)
			{
				target->health += health;
				target->funk += funk;
				state = DESTROY;
			}
		}
	}
	return 0;
}

void Player::draw(CGparameter color, Light *l, float t)
{
	float temp;
    glColor3f(0.0f,0.75f,0.0f);
	spaceEntity.boundBox.draw();
	if(model)
	{
		glPushMatrix();
		if(animationSets)
		{
			int f1,f2;
			float offset;
			frametime += t*15.0f;
			if(frametime > curAnimation->end/* || frametime < curAnimation->begin*/)
			{
				frametime = (float)curAnimation->beginloop;
			}
			f1 = (int)frametime;
			offset = frametime-f1;
			if( f1 +1 > curAnimation->end)
			{
				f2 = curAnimation->beginloop;
			}
			else
			{
				f2 = f1+1;
			}
			switch(state)
			{
			case ATTACK1:
			case ATTACK2:
			case ATTACK3:
			case ATTACK4:
				if(f1 == curAnimation->end)
					switchState(IDLE);
				break;
			case DYING:
				if(f1 == curAnimation->end)
				{
					state = DEAD;
					deathtimer=1.0f;
				}
				break;
			case DEAD:
				deathtimer-=t;
				if(deathtimer < 0.0f)
					state = RESPAWN;
				break;
			default:
				break;
			}
			
			modelPosition = position;
			modelPosition.y-=spaceEntity.boundBox.extents.y;
			model[0]->UpdateTags(f1,f2,offset);
			glColor3f(0.0f,0.0f,1.0f);
			lastWeaponPos[0] = defaultWeapons[0].boundBox.pos;
			lastWeaponPos[1] = defaultWeapons[1].boundBox.pos;
			lastWeaponPos[2] = defaultWeapons[2].boundBox.pos;
			lastWeaponPos[3] = defaultWeapons[3].boundBox.pos;
			int i;
			defaultWeapons[0].boundBox.pos = modelPosition+model[0]->tagPos[model[0]->FindTag("tag_lf")];
			defaultWeapons[1].boundBox.pos = modelPosition+model[0]->tagPos[model[0]->FindTag("tag_lh")];
			defaultWeapons[2].boundBox.pos = modelPosition+model[0]->tagPos[model[0]->FindTag("tag_rf")];
			defaultWeapons[3].boundBox.pos = modelPosition+model[0]->tagPos[model[0]->FindTag("tag_rh")];

			for(i = 0; i < 4; i++)
			{
				defaultWeapons[i].oldBoundBox.pos = lastWeaponPos[i];
				if(meleeActive[i])
				{
					defaultWeapons[i].boundBox.draw();
				}
			}
			glTranslatef(modelPosition.x,modelPosition.y,modelPosition.z);
			for(i=0; i < numModels;i++)
			{
				model[i]->draw(l,modelPosition,f1,f2,offset);
			}
		}
		else
			for(int j=0; j < numModels;j++)
			{
				model[j]->draw(l);
			}
		glPopMatrix();
		
	}

	
	//***************DEBUG*MODE************************//
	//draw direction character is facing
	//glBegin(GL_LINES);
	//	glVertex3f(position.x,position.y,position.z);
	//	glVertex3f(position.x + 10*facing.x,position.y + 10*facing.y,position.z + 10*facing.z);
    //glEnd();
	//***************DEBUG*MODE************************//
		
}


int Player::update(float t, Vector3f v, 
				   DLinkedList<OctTree*>* physicsSpaces,
				   OctTree* itemspace, OctTree* playerSpace)
{


	switch(statusState)
	{
	case HURT:
		clock-=t;
		if(clock <0.0f)
			statusState = NORM;
	}
	if(health <=0 && state != DYING && state != DEAD)
	{
		switchState(DYING);
	}
		
	itemspace->CheckObjectHere(&spaceEntity);

	DLinkedList<CollideSpace*>* collideSpaceList = spaceEntity.collideSpaceList;

	for(DLinkedList<CollideSpace*> *tlist = collideSpaceList; tlist != NULL; tlist = tlist->nextList)
	{
		Item *item = (Item*)(((CollideSpace*)(tlist->object))->objectB);
		/*if(state == PICKUP)
		{
			if(item->carryable && carriedItem == NULL)
			{
				itemspace->RemoveObject(&(item->spaceEntity));
				DLinkedList_DeleteTailObjects<CollideSpace*>(item->spaceEntity.collideSpaceList);
				item->spaceEntity.collideSpaceList = NULL;
				item->setState(CARRIED);
				carriedItem = item;
				item->carrier = this;
			}else
			if(!item->carryable)
			{
				itemspace->RemoveObject(&item->spaceEntity);
				health += item->health;
				funk += item->funk;
				item->setState(DESTROY);
			}
		}*/
	}
	for(int i = 0; i < 4; i++)
	{
		if(meleeActive[i])
		{
			MultiKey<CollideObject*,CollideObject*> key;
			key.keyA = defaultWeapons+i;
			key.keyB = &spaceEntity;
			OctTree::excludeMap.Put(key,true);
			playerSpace->CheckObjectHere(defaultWeapons+i);
			
			DLinkedList<CollideSpace*>* c;
			Vector3f forceVelocity = (defaultWeapons[i].boundBox.pos-lastWeaponPos[i])/(t ==0.0f? 0.05f:t);
			Vector3f pos;
			for(c = defaultWeapons[i].collideSpaceList; c != NULL; c = c->nextList)
			{
				Force3f* force = new Force3f(pos,forceVelocity,0.3f);
				forceVelocity = forceVelocity;
				((Player*)(c->object->objectB->physicsEntity))->hurt(20.0f);
				(c->object)->objectB->physicsEntity->AddForce(force);
				((Player*)(c->object->objectB->physicsEntity))->lastHit = this;
				//meleeActive[i] = false;
			}
			itemspace->CheckObjectHere(defaultWeapons+i);
			for(c = defaultWeapons[i].collideSpaceList; c != NULL; c = c->nextList)
			{
				Force3f* force = new Force3f(position,forceVelocity,0.3f);
				forceVelocity = forceVelocity;
				//((Player*)(c->object->objectB))->hurt(20.0f);
				(c->object)->objectB->physicsEntity->AddForce(force);
				//meleeActive[i] = false;
			}
		}
	}
	if(state == PICKUP)
	{
		//state = IDLE;
	}
	if(state == THROWING)
	{
	}

   	//model[0]->modelMatrix.SetColumn(3,position);
	return 0;
	
}

Vector3f Player::getPosition()
{
	return position;
}

Vector3f Player::getVelocity()
{
	return velocity;
}

bool Player::hurt(int damage)
{
	statusState = HURT;
	clock = 2.0f;
	if(oldhealth < health)
		oldhealth=health;
	health-=damage;
	if(health <=0.0f)
	{
		health = 0.0f;
		switchState(DYING);
	}
	
	return isDead();
}

void Player::dumbAI()
{
	if(state == IDLE)
	{
		switchState(11+(std::rand()%4));
		curAnimation = curAnimation;
	}
}

void Player::switchState(int newstate)
{
	state = newstate;
	curAnimation = FindAnimation(newstate,animationSets,numAnimations);
	if(!curAnimation)
	{
		curAnimation = FindAnimation(IDLE,animationSets,numAnimations);
		state = IDLE;
	}
	meleeActive[0] = curAnimation->lf;
	meleeActive[1] = curAnimation->lh;
	meleeActive[2] = curAnimation->rf;
	meleeActive[3] = curAnimation->rh;
	frametime = curAnimation->begin;
	
}

bool Player::isDead()
{
	return (state == DEAD || state == DYING || state == RESPAWN);
}

void Player::spawn(Vector3f v,bool death = true)
{
	if(death)
	{
		if(lastHit)
		{
			lastHit->score++;
		}
		else
			score--;
	}
	position = v;
	modelPosition = v;
	position.y += spaceEntity.boundBox.extents.y;
	spaceEntity.boundBox.pos = v;
	spaceEntity.boundBox.pos.y += spaceEntity.boundBox.extents.y;
	health = maxHealth;
	funk = maxFunk;
	switchState(IDLE);
	

}

void Object::setState(int state)
{
	this->state = state;
}

void Object::setFlag(unsigned flag)
{
	objectFlags = objectFlags | flag;
}

void Object::unsetFlag(unsigned flag)
{
	objectFlags = objectFlags & (~flag);
}

int Item::update(float t,OctTree *space,OctTree *itemSpace, OctTree *playerSpace)
{	
	if(state == CARRIED)
		spaceEntity.boundBox.pos = carrier->defaultWeapons[0].boundBox.pos;
	pickUpArea.oldBoundBox.pos = pickUpArea.boundBox.pos;
	pickUpArea.boundBox.pos = spaceEntity.boundBox.pos;
	
	playerSpace->CheckObjectHere(&pickUpArea);
	if(pickUpArea.collideSpaceList != NULL)
		drawArea = true;
	else drawArea = false;
	DLinkedList <CollideSpace*>* it;
	for(it = pickUpArea.collideSpaceList; it!= NULL && state == IDLE && carryable; it = it->nextList)
	{
		if(((Player*)(it->object->objectB->physicsEntity))->state == PICKUP)
		{
			carrier = (Player*)(it->object->objectB->physicsEntity);
			((Player*)(it->object->objectB->physicsEntity))->carriedItem = this;
			state = CARRIED;
		}
	}
	
	
	
	
	
	return 0;
}

bool Item::isDead()
{
	return state == DEAD;
}

void Player::setFacing(Vector3f v)
{
	//float angle = v.dot(facing);
	
	facing = v;
	Vector3f up = model[0]->modelMatrix.GetColumn(1).getVector();
	Vector3f x;
	Vector3f z;
	Vector4f temp;
	z = facing;
	x = up.cross(z);
	temp.setVector(z*-1.0f,0.0f);
	model[0]->modelMatrix.SetColumn(2,temp);
	temp.setVector(x*-1.0f,0.0f);
	model[0]->modelMatrix.SetColumn(0,temp);
	for(int i = 1; i < numModels; i++)
	{
		model[i]->modelMatrix.SetColumn(2,model[0]->modelMatrix.GetColumn(2));
		model[i]->modelMatrix.SetColumn(0,model[0]->modelMatrix.GetColumn(0));
	}

	
}

bool Player::isCarrying()
{
	return carriedItem != NULL;
}

void Item::toss(Vector3f v, float initSpeed)
{
	velocity = v; 
	speed = initSpeed + carrier->speed;
	state = THROWN;
	yVel = carrier->yVel;
}

void Player::throwItem()
{
	carriedItem->toss(facing,500.0f);
	carriedItem = NULL;
}
