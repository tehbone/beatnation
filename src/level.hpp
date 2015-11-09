/**
 * @file Declarations for everything that deals with levels.
 */
#ifndef LEVEL_HPP_
#define LEVEL_HPP_

#include <windows.h>
#include <gl/gl.h>
#include "collision.hpp"
#include "objects.hpp"
#include "utils.hpp"
#include "camera.hpp"
#include <stdio.h>
#include "light.hpp"
#include "model.hpp"

//level state defines
#define LVL_ACTIVE 0
#define LVL_PAUSED 1
#define LVL_STARTING 2
#define LVL_ENDING 3
#define LVL_SUDDENDEATH 4
#define LVL_SCORESCREEN 5

#define CHARS 4

struct Poly {
	Vector3f verts[4];
	Vector3f norm[4];
};
	
class Level {
	int state;
	int width, height;
	Camera* cam;
	Camera controlsCam;
	float falldistance;
	
	int numChars;
	int numItems;
	int numSpawnPos;
	int numItemSpawn;
	int numItemModels;
	int numLights;
	int texture;
	bool hastexture;
	unsigned int pause;
	CollideObject *itemSpawnPos;
	CollideObject *spawnPos;
	Player **spawnZones;
	Item **itemZones;
	Model **itemModels;
	Model *model;
	Model *levelCollide;
	Model *dodec;
	Model *outline;
	Light *light;
	CollideObject levelObject;
	float timer;
	float gametimer;
	int minorstate;
	unsigned int beatoutline;
	unsigned int background;
	unsigned int lifebar[2];
	unsigned int funkbar[2];
	unsigned int ready;
	unsigned int go;
	Model *podium;
	Player* winner;

public:
	//FIXME: Should be private
	OctTree* collideSpace;
	OctTree* playerSpace;
	OctTree* itemSpace;
	DLinkedList<OctTree*>* physicsSpaces;
	DLinkedList<Object*>* characterList;
	DLinkedList<Object*>* projectileList;
	DLinkedList<Object*>* itemList;
	CollideObject *boxList;
	SpringPhysicsEngine engine;
	Object* player;
	int MAXCHARS;

public:
	void setPlayer(Player* p);
	void respawn(Player* p,bool death = true);
	void initialize(char* filename,int width, int height);
	void AddChar(Player *p);
	void AddItem(Item *i);
	void draw(CGparameter color);
	bool Update(bool *keys);
	void cleanup();
};
#endif /* LEVEL_HPP_ */

