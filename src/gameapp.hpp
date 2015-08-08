/********************************************
*											*
*				  GameApp.h					*
*  Declarations for everything dealing with	*
*		the main game interface				*
*											*
********************************************/


/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS

#ifndef GAME_APP_H
#define GAME_APP_H

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "level.hpp"
#include "light.hpp"

#define GAME_STARTUP 0
#define GAME_MAIN_MENU 1
#define GAME_PLAYER_SELECT 2
#define GAME_LEVEL_SELECT 3
#define GAME_MAIN 4
#define GAME_LOADING 5

class GameApp
{
public:
	GameApp(int w, int h, bool *keys);

	~GameApp()
	{
		if(level)
		{
			level->cleanup();
			delete level;
		}
		level = NULL;
		if(characters)
		{
			for(int i = 0; i < numChars; i++)
			{
				delete characters[i];
				characters[i] = NULL;
			}
			delete [] characters;
			characters = NULL;
		}
		glDeleteLists(font,256);
		kill();
	}

//*******************************************************
//  bool InitGL(void)
//  Initializes the GL window
//*******************************************************
	bool InitGL();

//*******************************************************
//  void ReSizeGLScene(int,int)
//  ReSizes the GL window
//*******************************************************
	void ReSizeGLScene(int width, int height);

//*******************************************************
//  void kill(void)
//  sets up the window to be destroyed
//*******************************************************
	void kill();

//*******************************************************
//  void RenderScene(void)
//  Renders the current scene (be it level select or whatnot)
//*******************************************************
	void RenderScene();

//*******************************************************
//  static int LoadTexture(char* filename)
//  loads a texture for use
//*******************************************************
	static unsigned int LoadTexture(char* filename);

//*******************************************************
//  void MainGame();
//  void Startup();
//  void CharSelect();
//  void LevelSelect();
//  void MainMenu();
//  different states of the GameApp
//*******************************************************
	void MainGame(float time);
	void Startup(float time);
	void CharSelect(float time);
	void LevelSelect(float time);
	void MainMenu(float time);
	static void print(const char* fmt,...);
//****************FIELDS*******************************
public:
	static StringMap *animationResource;
	static StringMap *modelResource;
	bool *keys;
	bool fullscreen;
	int width;
	int height;
	int state;
	Level* level;
	unsigned int textures[4];
	unsigned int cursor;
	unsigned int startgame;
	unsigned int options;
	unsigned int quit;
	unsigned int OK;
	unsigned int back;
	static unsigned int font;
	Player** characters;
	int chosenChar;
	int numChars;
	GLYPHMETRICSFLOAT gmf[256];
	
	
};

#endif