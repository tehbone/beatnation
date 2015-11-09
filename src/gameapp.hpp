/**
 * @file Declarations for everything dealing with the main game interface.
 */
#ifndef GAMEAPP_HPP_
#define GAMEAPP_HPP_

#include <windows.h>

#define GAME_STARTUP 0
#define GAME_MAIN_MENU 1
#define GAME_PLAYER_SELECT 2
#define GAME_LEVEL_SELECT 3
#define GAME_MAIN 4
#define GAME_LOADING 5

class Level;

class GameApp {
public:
	//FIXME: Should be private
	static StringMap *animationResource; //FIXME: Why is this static?
	static StringMap *modelResource; //FIXME: Why is this static?
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

public:
	GameApp(int w, int h, bool *keys);
	~GameApp();
	
	/**
	 * Initializes the GL Window
	 * @return true on success.
	 */
	bool InitGL();

	/**
	 * Resizes the GL window.
	 * @param width the new width
	 * @param height the new height
	 */ 
	void ReSizeGLScene(int width, int height);
	void kill();
	void RenderScene();
	void MainGame(float time);
	void Startup(float time);
	void CharSelect(float time);
	void LevelSelect(float time);
	void MainMenu(float time);

	//FIXME: Should these really be static?
	static unsigned int LoadTexture(char* filename);
	static void print(const char* fmt,...);
};

#endif /* GAMEAPP_HPP_ */

