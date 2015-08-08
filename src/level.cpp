/****************************************
*										*
*				level.cpp				*
*	Defines for the level class			*
*										*
****************************************/

/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS

#include <fstream>
#include "level.hpp"
#include "controller.hpp"
#include "t_utils.cpp"
#include "gameapp.hpp"

void Level::AddItem(Item* item)
{
	DLinkedList_AddHeadEntry<Object*>(itemList, item);
	OctTree::AddObject(itemSpace, &item->spaceEntity);
}

void Level::AddChar(Player *character)
{
	numChars++;
	characterList = DLinkedList_AddHeadEntry<Object*>(characterList, character);
	character->spaceEntity.remainingSpaces = 1;
	OctTree::AddObject(playerSpace, &character->spaceEntity,false);
	character->engine = &engine;
	engine.updateMap.Put(character, true);
	respawn(character,false);
}

void Level::initialize(char *filename,int width, int height)
{
	podium = NULL;
	minorstate = 0;
	physicsSpaces = NULL;
	projectileList = NULL;
	itemList = NULL;
	itemZones = NULL;
	itemSpawnPos = NULL;
	itemModels = NULL;
	spawnPos = NULL;
	spawnZones = NULL;
	characterList = NULL;
	state = LVL_STARTING;
	this->width = width;
	this->height = height;
	beatoutline = LoadTGATexture("../Textures/beatoutline.tga");
	pause = LoadTGATexture("../Textures/pause.tga");
	lifebar[0] = LoadTGATexture("../Textures/lifebar.tga");
	lifebar[1] = LoadTGATexture("../Textures/lifebarfill.tga");
	funkbar[0] = LoadTGATexture("../Textures/funkbar.tga");
	funkbar[1] = LoadTGATexture("../Textures/funkbarfill.tga");
	ready = LoadTGATexture("../Textures/ready.tga");
	go = LoadTGATexture("../Textures/go.tga");
	dodec = new Model("../models/dodec.MD3");
	outline = new Model("../models/dodecoutline.MD3");
	//REMEMBER! ADD PLAYER TO SPACE
	float lightpos[4] = {0,800.0,0.0f,1.0f};
	float lightcolor[4] = {1.0f,1.0f,1.0f,1.0f};
	float ambiant[4] = {0.1f,0.1f,0.1f,1.0f};
	glLightfv(GL_LIGHT0,GL_POSITION,lightpos);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,lightcolor);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambiant);
	float f;
	Matrix4f m;
	int tempnum,i;
	float garbagefloat; //used to toss the nonimplemented parameters
	float garbageint; //used to toss the nonimplemented parameters
	char pathname[256];
	Vector3f up(0.0f,1.0f,0.0f);
	Vector3f camera;
	
	
	Vector3f tempVector;
	std::ifstream levelfile(filename);
	//BEGIN PARSING
	//MAXCHARACTERS
	levelfile >> MAXCHARS;
	//CAMERA stuff
	levelfile >> camera.x;
	levelfile >> camera.y;
	levelfile >> camera.z;
	cam = new Camera();
	cam->initialize(up,camera,camera);
	//LIGHTING
	//ambient color
	levelfile >> garbagefloat;
	levelfile >> garbagefloat;
	levelfile >> garbagefloat;
	//start light parsing
	levelfile >> garbageint; //number of lights
	//end light parsing
	//PHYSICS PROPERTIES
	levelfile >> garbageint;
	levelfile >> garbagefloat; //gravity
	levelfile >> garbagefloat; //wind
	levelfile >> garbagefloat; //surface friction
	levelfile >> falldistance;
	//SCALEFACTOR
	levelfile >> tempnum;
	//TEXTURES
	levelfile >> pathname;

	levelfile >> pathname;
    background = LoadTGATexture(pathname);
	levelfile >> pathname;
	garbageint = LoadTGATexture(pathname);
	//LEVEL INFORMATION
	levelfile >> pathname;
	model = new Model(pathname);
	m = m.scale(tempnum,tempnum,tempnum);
	model->meshes[0].TransformPermanent(m);
	model->texture = garbageint;
	levelfile >> pathname;
	levelCollide = new Model(pathname);
	levelCollide->meshes[0].TransformPermanent(m);
	levelfile >> numSpawnPos;
	spawnPos = new CollideObject[numSpawnPos];
	levelfile >> pathname;
	Model* spawnArea = new Model(pathname);
	spawnArea->meshes[0].TransformPermanent(m);
	Vector3f spawnExtents(1.0f,1.0f,1.0f);
	spawnZones = new Player*[numSpawnPos];
	for(i = 0; i < numSpawnPos; i++)
	{
		if(i == 0){
			spawnPos[i].boundBox.pos.set(0.0f, 30.0f, 0.0f);	
		}
		else{
			spawnPos[i].boundBox.pos = spawnArea->meshes[0].verts[0][i];
		}
		spawnPos[i].boundBox.extents = spawnExtents;
		spawnPos[i].oldBoundBox.pos = spawnArea->meshes[0].verts[0][i];
		spawnZones[i] = NULL;
	}
	spawnArea->Cleanup();
	
	delete spawnArea;
	//END PARSING
	tempVector.set(0.0f,100.0f,0.0f);
		
	Vector3f temp(0,0,0);
	
	levelObject.collisionTypes = ENTITY_COLLISION_TYPE_MESH;
	levelObject.model = levelCollide;
	levelObject.remainingSpaces = 1024;

	Vector3f levelExtents(levelObject.model->modelMatrix.GetData(0,0)*(levelObject.model->meshes->maxPoint.x - levelObject.model->meshes->minPoint.x / 2.0f),
						  levelObject.model->modelMatrix.GetData(1,1)*(levelObject.model->meshes->maxPoint.x - levelObject.model->meshes->minPoint.x / 2.0f),
						  levelObject.model->modelMatrix.GetData(2,2)*(levelObject.model->meshes->maxPoint.x - levelObject.model->meshes->minPoint.x / 2.0f));

	collideSpace = new OctTree(temp,levelExtents.x);
	itemSpace = new OctTree(temp,10000.0f);
	playerSpace = new OctTree(temp,10000.0f);

	physicsSpaces = DLinkedList_AddHeadEntry<OctTree*>(physicsSpaces, collideSpace);
	physicsSpaces = DLinkedList_AddHeadEntry<OctTree*>(physicsSpaces, itemSpace);
	physicsSpaces = DLinkedList_AddHeadEntry<OctTree*>(physicsSpaces, playerSpace);

	levelObject.boundBox.extents = levelExtents;
	levelObject.boundBox.pos = temp;

	OctTree::AddObject(collideSpace, &levelObject);





	
	
	up.set(0,50.0f,0.0f);
	camera.set(1.0f,1.0f,1.0f);
	light = new Light(up,camera);

	
	/*Item* item = NULL;
	int itemGridSize = 8;
	int gridHeight = 1;
	for(i = 0; i < itemGridSize; i++){
		for(int j = 0; j < itemGridSize; j++){
			for(int k = 0; k < gridHeight; k++){
				item = new Item(tempVector.x + i * 50.0f, tempVector.y + k * 50.0f + 200.0f, tempVector.z * -1.0f + 200.0f + j * 50.0f);
				item->spaceEntity.remainingSpaces = 1;
				OctTree::AddObject(itemSpace, &item->spaceEntity);
				itemList = DLinkedList_AddHeadEntry<Object*>(itemList, item);
				item->engine = &engine;
				item->mass = 1.0f;
				engine.updateMap.Put(item, true);
			}
		}
	}*/

	
	numItems = 0;
	timer  = 3.0f;
}

void Level::draw(CGparameter color)
{
	float xratio = (float)width/1024.0f;
	float yratio = (float)height/768.0f;
	static float drawtimer = 0;
	
	int i;
//*******TIME*BASED*CODE*********************//
	static long lastTickCount = 0;
	long thisTickCount = GetTickCount();
	float time = (thisTickCount - lastTickCount)/1000.0f;
	lastTickCount = thisTickCount;
	if(state != LVL_SCORESCREEN)
	{
		glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,width,-height,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D,background);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex2i(0,0);
		glTexCoord2f(0.0f,0.0f); glVertex2i(0,-height);
		glTexCoord2f(1.0f,0.0f); glVertex2i(width,-height);
		glTexCoord2f(1.0f,1.0f); glVertex2i(width,0);
	glEnd();
	glClear(GL_DEPTH_BUFFER_BIT);
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,10.0f,10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(state != LVL_SCORESCREEN)
	{
//*******RENDER*CAMERA*SCENE*****************//
		if(cam)
		{
			cam[0].view(time);
		}
		float ack[4] = {player->position.x,player->position.y,player->position.z,1.0f};
		glLightfv(GL_LIGHT0,GL_POSITION,ack);

//*******DRAW*LEVEL**************************//
	//Bounding Boxes
	//Level
		glColor3f(0.0f,0.5f,0.0f);
		Vector3f c(1.0f,1.0f,1.0f);
		Vector3f ambient(0.1f,0.1f,0.1f);
		float col[3];
		model->draw(light);
	//#define SHOWWORLD
	#ifdef SHOWWORLD
		glPolygonMode(GL_FRONT,GL_LINE);
	//	glDisable(GL_LIGHTING);
		glColor3f(1.0f,0.0f,0.0f);
		levelCollide->draw(light);
	//	glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT,GL_FILL);
	#endif
	}
		
	DLinkedList<Object*>* it = NULL;
	int seconds;
	int millisecs;
	switch(state)
	{
	case LVL_STARTING:
	case LVL_ACTIVE:
	
	//**********DRAW*NPCS**************************************//
		for(it = characterList; it != NULL; it = it->nextList)
		{
			((Player*)(it->object))->draw(color,light,time);
			
		}
	//**********DRAW*ITEMS*************************************//
		for(it = itemList; it != NULL; it = it->nextList)
		{
			((Item*)(it->object))->draw(color,time);
		}
		break;
	case LVL_PAUSED:
		for(it = characterList; it != NULL; it = it->nextList)
		{
			((Player*)(it->object))->draw(color,light,0);
			
		}
		for(it = itemList; it != NULL; it = it->nextList)
		{
			((Item*)(it->object))->draw(color,0);
		}
		break;
	case LVL_SCORESCREEN:
		glTranslatef(0.0f,-600.0f,-2000.0f);
		podium->draw(NULL);
		glTranslatef(0.0f,600.0f,0.0f);
		winner->draw(color,light,time);

	}
	
//**********OVERLAY*MODE***********************************//
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,width,-height,0,500.0f,-500.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH);
	glColor3f(1.0f,1.0f,1.0f);
	switch(state)
	{
	case LVL_STARTING:
		
		if(timer > 0.0f)
		{
			drawtimer+=time*3.14859f*3.0f;
			if(drawtimer >= 3.14859f*3.0f)
			{
				drawtimer = 0.0f;
			}
			glColor3f(OctTree::absf(cosf(drawtimer)),0.0f,OctTree::absf(sinf(drawtimer)));
			glBindTexture(GL_TEXTURE_2D,ready);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f); glVertex2f(275.0f,-height/2.0f + 90.0f);
				glTexCoord2f(0.0f,0.0f); glVertex2f(275.0f,-height/2.0f -15.0f);
				glTexCoord2f(1.0f,0.0f); glVertex2f((float)width/2.0f-5.0f,-height/2.0f-15.0f);
				glTexCoord2f(1.0f,1.0f); glVertex2f((float)width/2.0f-5.0f,-height/2.0f + 90.0f);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			seconds = (int)timer;
			millisecs = ((int)(timer*100.0f)) % 100;
			glTranslatef(width/2.0f,-height/2.0f,0);
			
			glScalef(50.0f,50.0f,50.0f);
			GameApp::print("%1.2f",timer);
			glEnable(GL_TEXTURE_2D);
			glColor3f(1.0f,1.0f,1.0f);
		}
		else
		
		{
			state = LVL_ACTIVE;
			minorstate = 0;
			gametimer=300.0f;//300.0f;
		}
		
		break;
	case LVL_PAUSED:
		glBindTexture(GL_TEXTURE_2D, pause);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,1.0f); glVertex2f(0.0f,0.0f);
			glTexCoord2f(0.0f,0.0f); glVertex2f(0.0f,-(float)height);
			glTexCoord2f(1.0f,0.0f); glVertex2f((float)width,-(float)height);
			glTexCoord2f(1.0f,1.0f); glVertex2f((float)width,0.0f);
		glEnd();
		break;
	case LVL_ACTIVE:
		if(player)
		{
			gametimer -=time;
			if(minorstate == 0)
			{
				glColor3f(1.0f,0.0f,0.0f);
				glBindTexture(GL_TEXTURE_2D,go);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f,1.0f); glVertex2f((float)width/2.0f-150.0f,-height/2.0f + 125.0f);
					glTexCoord2f(0.0f,0.0f); glVertex2f((float)width/2.0f-150.0f,-height/2.0f -125.0f);
					glTexCoord2f(1.0f,0.0f); glVertex2f((float)width/2.0f+150.0f,-height/2.0f-125.0f);
					glTexCoord2f(1.0f,1.0f); glVertex2f((float)width/2.0f+150.0f,-height/2.0f + 125.0f);
				glEnd();
				timer-=time;
				if(timer < -1.0f)
				{
					minorstate++;
				}
				
			}
			float healthratio = (float)(((Object*)player)->health)/(float)(((Player*)player)->maxHealth);
			float funkratio = (float)(((Object*)player)->funk)/(float)(((Player*)player)->maxFunk);
			
			glColor3f(0.0f,0.0f,1.0f);
			glBindTexture(GL_TEXTURE_2D, lifebar[0]);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f); glVertex2f(15.0f*xratio,-170.0f*yratio);
				glTexCoord2f(0.0f,0.0f); glVertex2f(15.0f*xratio,-620.0f*yratio);
				glTexCoord2f(1.0f,0.0f); glVertex2f(115.0f*xratio,-620.0f*yratio);
				glTexCoord2f(1.0f,1.0f); glVertex2f(115.0f*xratio,-170.0f*yratio);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, lifebar[1]);
			if(((Player*)player)->oldhealth > ((Player*)player)->health)
			{
				glColor3f(1.0f,0.0f,0.0f);
				float oldratio = ((Player*)player)->oldhealth/(float)((Player*)player)->maxHealth;
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f,oldratio); glVertex2f(25.0f*xratio,(-600.0f+oldratio*450.0f)*yratio);
					glTexCoord2f(0.0f,healthratio); glVertex2f(25.0f*xratio,(-600.0f+healthratio*450.0f)*yratio);
					glTexCoord2f(1.0f,healthratio); glVertex2f(125.0f*xratio,(-600.0f+healthratio*450.0f)*yratio);
					glTexCoord2f(1.0f,oldratio); glVertex2f(125.0f*xratio,(-600.0f+oldratio*450.0f)*yratio);
				glEnd();
				if(((Player*)player)->statusState != HURT)
				{
					oldratio-=time;//1.0f;
					((Player*)player)->oldhealth = (float)((Player*)player)->maxHealth*oldratio;
					((Player*)player)->oldhealth = ((Player*)player)->oldhealth;
				}
			}

			glColor3f(0.0f,0.0f,1.0f);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f,healthratio); glVertex2f(25.0f*xratio,(-600.0f+healthratio*450.0f)*yratio);
				glTexCoord2f(0.0f,0.0f); glVertex2f(25.0f*xratio,-600.0f*yratio);
				glTexCoord2f(1.0f,0.0f); glVertex2f(125.0f*xratio,-600.0f*yratio);
				glTexCoord2f(1.0f,healthratio); glVertex2f(125.0f*xratio,(-600.0f+healthratio*450.0f)*yratio);
			glEnd();
			
			
			glColor3f(0.0f,1.0f,0.0f);
			glBindTexture(GL_TEXTURE_2D, lifebar[0]);
			glBegin(GL_QUADS);
				glTexCoord2f(1.0f,0.0f); glVertex2f(155.0f*xratio,-660.0f*yratio);
				glTexCoord2f(0.0f,0.0f); glVertex2f(155.0f*xratio,-760.0f*yratio);
				glTexCoord2f(0.0f,1.0f); glVertex2f(605.0f*xratio,-760.0f*yratio);
				glTexCoord2f(1.0f,1.0f); glVertex2f(605.0f*xratio,-660.0f*yratio);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, lifebar[1]);
			glBegin(GL_QUADS);
				glTexCoord2f(1.0f,0.0f); glVertex2f(175.0f*xratio,-650.0f*yratio);
				glTexCoord2f(0.0f,0.0f); glVertex2f(175.0f*xratio,-750.0f*yratio);
				glTexCoord2f(0.0f,funkratio); glVertex2f((175.0f+funkratio*450.0f)*xratio,-750.0f*yratio);
				glTexCoord2f(1.0f,funkratio); glVertex2f((175.0f+funkratio*450.0f)*xratio,-650.0f*yratio);
			glEnd();
			
			
			glColor3f(1.0f,1.0f,1.0f);
			glBindTexture(GL_TEXTURE_2D, ((Player*)player)->statusface[0]);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f); glVertex2f(30.0f*xratio,-590.0f*yratio);
				glTexCoord2f(0.0f,0.0f); glVertex2f(30.0f*xratio,-745.0f*yratio);
				glTexCoord2f(1.0f,0.0f); glVertex2f(185.0f*xratio,-745.0f*yratio);
				glTexCoord2f(1.0f,1.0f); glVertex2f(185.0f*xratio,-590.0f*yratio);
			glEnd();
			glLoadIdentity();
			int minutes = (int)(gametimer /60);
			int seconds = (int)gametimer %60;
			int millisecs = (int)(gametimer * 100.0f) %100;
			glTranslatef(width/2.0f -100.0f,-100.0f,0.0f);
			glScalef(30.0f,30.0f,1.0f);
			glDisable(GL_TEXTURE_2D);
			GameApp::print("%d;%d.%2.0d",minutes,seconds,millisecs);
			glEnable(GL_TEXTURE_2D);
			glLoadIdentity();
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_DEPTH);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDisable(GL_COLOR_MATERIAL);
			glTranslatef(860.0f,-100.0f,0.0f);
			glScalef(0.7f,0.5f,0.7f);
			glColor3f(0.75f,0.75f,0.75f);
			outline->draw(NULL);
			glColor3f(1.0f,1.0f,1.0f);
			dodec->draw(NULL);
			glLoadIdentity();
			glTranslatef(940.0f,-100.0f,0.0f);
			glScalef(0.7f,0.5f,0.7f);
			glColor3f(0.75f,0.75f,0.75f);
			outline->draw(NULL);
			glColor3f(1.0f,1.0f,1.0f);
			dodec->draw(NULL);
			glLoadIdentity();
			glTranslatef(900.0f,-75.0f,0.0f);
			glScalef(0.7f,0.5f,0.7f);
			glColor3f(0.75f,0.75f,0.75f);
			outline->draw(NULL);
			glColor3f(1.0f,1.0f,1.0f);
			dodec->draw(NULL);
			glLoadIdentity();
			glTranslatef(900.0f,-125.0f,0.0f);
			glScalef(0.7f,0.5f,0.7f);
			glColor3f(0.75f,0.75f,0.75f);
			outline->draw(NULL);
			glColor3f(1.0f,1.0f,1.0f);
			dodec->draw(NULL);
			glLoadIdentity();
			glDisable(GL_LIGHTING);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glTranslatef(900.0f,-100.0f,0.0f);
			glBindTexture(GL_TEXTURE_2D,beatoutline);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f); glVertex2f(-67.0f,52.0f);
				glTexCoord2f(0.0f,0.0f); glVertex2f(-67.0f,-52.0f);
				glTexCoord2f(1.0f,0.0f); glVertex2f(67.0f,-52.0f);
				glTexCoord2f(1.0f,1.0f); glVertex2f(67.0f,52.0f);
			glEnd();
		}
		break;
	case LVL_SCORESCREEN:
		break;

	}
	glEnable(GL_DEPTH);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

bool Level::Update(bool *keys)
{
	
	
	static float lightAngle=0.0f;
	light[0].pos.z = cosf(lightAngle) * 200.0f;
	light[0].pos.x = sinf(lightAngle) * 200.0f;
	static float camAngle = 0.0f;
	static long lastTickCount = 0;
	
	long TickCount = GetTickCount();
	float time;
	if(lastTickCount == 0)
		lastTickCount = TickCount-5;
	time = (float)(TickCount-lastTickCount)/1000.0f;
	
	lastTickCount=TickCount;
	
	lightAngle+=time*3.14856f/4.0f;
	if(lightAngle > 3.14856f*2)
		lightAngle = 0.0f;
	Vector3f temp;
	Vector3f direction;
	DLinkedList<Object*> *it;
	Vector3f c;
	

	//*******************DEBUG*MODE*ONLY*******************//
	//chose Static Camera
	if(keys[VK_F4])
	{
		temp.set(0,100.0f,500.0f);	
		cam[0].switchtoView(STATIC_CAMERA,temp,(Object*)player);
		keys[VK_F4] = false;
	}
	//chose Dynamic Camera  (still being worked on)
	if(keys[VK_F3])
	{
		cam[0].switchtoView(DYNAMIC_CAMERA,temp,(Object*)player);
		keys[VK_F3] = false;
	}
	//Automatically Respawn Player
	if(keys[VK_F5])
	{
		respawn((Player*)player);
		keys[VK_F5] = false;
	}

	if(keys['P'])
	{
		if(state == LVL_PAUSED)
		{
			state = LVL_ACTIVE;
		}
		else state = LVL_PAUSED;
		keys['P'] = false;
	}
	if(keys['H'])
	{
		((Player*)player)->hurt(100);
		keys['H'] = false;

	}
	if(keys['J'])
	{
		((Object*)player)->health+=15;
		keys['J'] = false;
	}
	if(keys['K'])
	{
		((Object*)player)->funk-=20;
		keys['K'] = false;
	}
	if(keys['L'])
	{
		((Object*)player)->funk+=15;
		keys['L']=false;
	}


	if(keys[VK_NUMPAD7])
	{
		cam[0].pivot(false,time);
	}
	if(keys[VK_NUMPAD9])
	{
		cam[0].pivot(true,time);
	}
	float adjustedTime = time * 6.0f;
	
	switch(state)
	{
	case LVL_STARTING:
		timer -= time;
		if(minorstate == 0 && timer < 3.0f)
		{
			minorstate++;
			cam[0].pivot(true,(float)(rand() % 314)/100.0f);
		}
		if(minorstate == 1 && timer < 2.0f)
		{
			minorstate++;
			cam[0].pivot(false,(float)(rand() % 314)/100.0f);

		}
		if(minorstate == 2 && timer < 1.0f)
		{
			minorstate++;
			cam[0].pivot(true,(float)(rand() % 314)/100.0f);
		}
		if(minorstate == 3 && timer < 0.0f)
		{
			minorstate++;
			cam[0].setView(((Player*)player)->facing,player->position);
		}
		engine.Update(physicsSpaces, time * 6.0f);
		break;
	case LVL_ACTIVE:
		//**************PLAYER*MOVEMENT************************//
		if(player)
		{
			
			
			bool hasmoved = false;
			if(!((Player*)player)->isDead())
				hasmoved= player->controller->ProcessKeys(keys, adjustedTime);
						

		//*******************END*DEBUG*MODE*******************//
			
			if(!hasmoved)
			{
				controlsCam = cam[0];
			}
			else
			{
				if(cam[0].type = FREE_CAMERA)
					controlsCam = cam[0];
			}
	
				
			
		}
		for(it = characterList; it != NULL; it = it->nextList)
		{
			if(!((Player*)it->object)->isDead())
			{
				((Player*)it->object)->update(adjustedTime, direction, physicsSpaces, itemSpace,playerSpace);
				if((it->object) != player)
					((Player*)it->object)->dumbAI();
			}
			
			if(((Player*)it->object)->state == RESPAWN || ((Player*)it->object)->getPosition().y < falldistance)
				respawn(((Player*)it->object));
			
		}
		for(it = itemList; it!= NULL; it = it->nextList)
		{
			((Item*)it->object)->update(adjustedTime,NULL,itemSpace,playerSpace);
		}
		engine.Update(physicsSpaces, time * 6.0f);
		
		if(gametimer < 0.0f)
		{
			it = characterList;
			int max = ((Player*)it->object)->score;
			winner = ((Player*)it->object);
			int tempscore;
			for(it = it->nextList; it != NULL; it= it->nextList)
			{
				tempscore = ((Player*)it->object)->score;
				if(tempscore > max)
				{
					max = tempscore;
					winner = ((Player*)it->object);
				}
			}
			state = LVL_SCORESCREEN;
			podium = new Model("../models/winnerstand.MD3");
		}
		break;
	case LVL_PAUSED:
		break;
	case LVL_SCORESCREEN:
		break;

	}
	
	if(player)
	{
		cam[0].move(((Player*)player)->getPosition(),((Player*)player)->facing);
	}
	return false;
}

void Level::setPlayer(Player *p)
{
	player = p;
	((PlayerController*) player->controller)->controlsCam = &controlsCam;
	AddChar(p);
	cam[0].setView(p->facing,p->position);
	
}

void Level::respawn(Player *p,bool death)
{
	int i;
	bool entered = false;
	for(i = 0; i < numSpawnPos && !entered; i++)
	{
		playerSpace->CheckObjectHere(spawnPos+i);
		if(spawnPos[i].collideSpaceList == NULL)
		{
			p->spawn(spawnPos[i].boundBox.pos,death);
			entered = true;
		}
	}
}

void Level::cleanup()
{
	DLinkedList <Object*>* it;
	for(it = characterList; it != NULL; it = it->nextList)
	{
		((Player*)it->object)->cleanUp();
	}
	DLinkedList_DeleteTailObjects(characterList); characterList = NULL;
	DLinkedList_DeleteTailObjects(projectileList); projectileList = NULL;
	DLinkedList_DeleteTailObjects(itemList); itemList = NULL;
			
	if(model)
	{
		model->Cleanup();
		delete model;
		model = NULL;
		levelCollide->Cleanup();
		delete levelCollide;
		levelCollide = NULL;
	}
	if(dodec)
	{
		dodec->Cleanup();
		delete dodec;
	}
	if(outline)
	{
		outline->Cleanup();
		delete outline;
	}
	if(itemModels)
	{
		for(int i =0; i < numItemModels; i++)
		{
			itemModels[i]->Cleanup();
			delete itemModels[i];
			itemModels[i] = NULL;
		}
		delete [] itemModels;
	}
	if(spawnZones)
		{
		delete [] spawnZones;
		spawnZones = NULL;
	}
	if(itemZones)
	{
		delete [] itemZones;
		itemZones = NULL;
	}
	if(itemSpawnPos)
	{
		delete [] itemSpawnPos;
		itemSpawnPos = NULL;
	}
	if(spawnPos)
	{
		delete [] spawnPos;
		spawnPos = NULL;
	}
		
	delete light;
	light = NULL;
	if(playerSpace)
	{
		delete playerSpace;
		playerSpace = NULL;
	}
	if(itemSpace)
	{
		itemSpace = NULL;
		delete itemSpace;
	}
	if(collideSpace)
	{
		collideSpace->RemoveObject((CollideObject*)(&levelObject));
		delete collideSpace;
		collideSpace = NULL;
	}
	if(podium)
	{
		delete podium;
		podium = NULL;
	}
}
	

