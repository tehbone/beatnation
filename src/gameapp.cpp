/**
 * @file gameapp.cpp
 * Defines for everything dealing with the main game app.
 */
#include <gl/gl.h>
#include <gl/glu.h>
#include "gameapp.hpp"
#include "level.hpp"
#include "light.hpp"
#include "targa.hpp"

StringMap *GameApp::animationResource;
StringMap *GameApp::modelResource;
unsigned int GameApp::font;

GameApp::GameApp(int w, int h, bool *keys) : width(w), height(h)
{
	numChars = 10;
	characters = NULL;
	this->keys = keys;
	InitGL();
	ReSizeGLScene(width,height);
	state = GAME_STARTUP;
	level = new Level();
	level->initialize("downtown.lvl",width,height);
	SetCursorPos(width/2,height/2);
	textures[GAME_STARTUP] = LoadTexture("../Textures/logo.bmp");
	textures[GAME_MAIN_MENU] = LoadTexture("../Textures/beatnation7.bmp");
	textures[GAME_LOADING] = LoadTexture("../Textures/loadingicon.bmp");
	textures[GAME_PLAYER_SELECT] = LoadTGATexture("../Textures/chrselbkgd.tga");
	cursor = LoadTGATexture("../Textures/cursor.tga");
	startgame = LoadTGATexture("../Textures/play.tga");
	options = LoadTGATexture("../Textures/options.tga");
	quit = LoadTGATexture("../Textures/exit.tga");
	back = LoadTGATexture("../Textures/back.tga");
	OK = LoadTGATexture("../Textures/OK.tga");
	HFONT f = CreateFont(-12,0,0,0,FW_BOLD,false,false,false,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,FF_DONTCARE|DEFAULT_PITCH,"BeatNat!on");
	font = glGenLists(256);

	HFONT oldfont = (HFONT)SelectObject(hDC,f);
	//wglUseFontBitmaps(hDC,32,96,font);
	if (!wglUseFontOutlines(hDC,0,255,font,0.0f,0.2f,WGL_FONT_POLYGONS,gmf))
		gmf[0] = gmf[0];
	//SelectObject(hDC,oldfont);
	//DeleteObject(f);
}

GameApp::~GameApp()
{
	if(level) {
		level->cleanup();
		delete level;
	}

	level = NULL;
	if (characters) {
		for (int i = 0; i < numChars; i++) {
			delete characters[i];
			characters[i] = NULL;
		}
		delete [] characters;
		characters = NULL;
	}
	glDeleteLists(font,256);
	kill();
}

bool
GameApp::InitGL()
{
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(5.0f);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	//glPolygonMode(GL_BACK,GL_LINE);
#ifdef USE_CG
	initCG();
#endif
	return true;
}

void
GameApp::kill()							// Properly Kill The Window
{
	if(modelResource)
	{
		DeleteStringMap(modelResource);
	}
	if(animationResource)
	{
		DeleteStringMap(animationResource);
	}
#ifdef USE_CG
	cgDestroyContext(context);
#endif
}

/**
 * HOLY FUNCTION LENGTH, BATMAN!
 */
void
GameApp::RenderScene()
{
	float xslide;
	Vector3f pos(0.0f,0.0f,80.0f);
	Vector3f c(1.0f,1.0f,1.0f);
	Light light(pos,c);
	static bool up = false;
	static long lastTickCount = GetTickCount();
	static float charfloat = 0.0f;
	int i;
	long thisTickCount = GetTickCount();
	float time = (float)(thisTickCount-lastTickCount)/1000.0f;
	float xratio = (float)width/1024.0f;
	float yratio = (float)height/768.0f;
	
	lastTickCount = thisTickCount;
	CGparameter color;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// Clear The Screen And The Depth Buffer
	POINT p;
	static float angle = 0.0f;
	
	
	static float brightness=0.0f;
	static int startupstate = 0;
	static float splashtime = 0;
	float blah = charfloat;
	GetCursorPos(&p);
	switch(state) {
	case GAME_STARTUP:
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0,(double)width,-(double)height,0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();	
		switch(startupstate) {
		case 0:
			brightness+=time;
			if (brightness > 1.0f) {
				brightness = 1.0f;
				startupstate = 1;
			}
			break;
		case 1:
			splashtime+=time;
			if (splashtime > 5.0f) {
				startupstate = 2;
			}
			break;
		case 2:
			brightness-=time;
			if (brightness < 0.0f) {
				brightness = 0.0f;
				SetCursorPos(width/2,height/2);
				state = GAME_MAIN_MENU;
				brightness = 0.0f;
				startupstate = 0;
			}
			break;
		}

		glColor3f(brightness,brightness,brightness);
		glBindTexture(GL_TEXTURE_2D,textures[GAME_STARTUP]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f); glVertex2i(0,-height);
		glTexCoord2f(1.0f,0.0f); glVertex2i(width,-height);
		glTexCoord2f(1.0f,1.0f); glVertex2i(width,0);
		glTexCoord2f(0.0f,1.0f); glVertex2i(0,0);
		glEnd();
		break;
	case GAME_MAIN_MENU:
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0,(double)width,-(double)height,0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		switch(startupstate) {
		case 0:
			brightness+=time;
			if (brightness >= 1.0f) {
				brightness = 1.0f;				
			}

			if (lbuttondown && (float)p.x > 100.0f*xratio && (float)p.x < 300.0f*xratio && (float)p.y < 480.0f*yratio && (float)p.y > 400.0f*yratio) {
				startupstate = 1;
			}
			if (lbuttondown && (float)p.x > 100.0f*xratio && (float)p.x < 300.0f*xratio && (float)p.y < 640.0f*yratio && (float)p.y > 560.0f*yratio) {
				PostQuitMessage(0);
			}
			break;
		case 1:
			brightness-=time*2.0f;
			if (brightness <= 0.0f) {
				brightness <= 0.0f;
				startupstate = 2;
			}
			break;
		case 2:
			state = GAME_PLAYER_SELECT;
			Vector3f temp(0.0f,0.0f,0.0f);
			chosenChar = 0;
			Player *tempdata[50];
			WIN32_FIND_DATA findData;
			HANDLE tempHand = FindFirstFile("../characters/*.chr",&findData);
			numChars = 1;
			char filename[1024];
			sprintf(filename,"../characters/%s",findData.cFileName);
			tempdata[0] = new Player(filename);
			while (FindNextFile(tempHand,&findData) && numChars < 50) {
				numChars++;
				sprintf(filename,"../characters/%s",findData.cFileName);
				tempdata[numChars-1] = new Player(filename);
			}
		
			characters = new Player*[numChars];
			for (i = 0; i < numChars; i++) {
				characters[i] = tempdata[i];
			}
			break;
		}

		glColor3f(brightness,brightness,brightness);
		glBindTexture(GL_TEXTURE_2D,textures[GAME_MAIN_MENU]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f); glVertex2i(0,-height);
		glTexCoord2f(1.0f,0.0f); glVertex2i(width,-height);
		glTexCoord2f(1.0f,1.0f); glVertex2i(width,0);
		glTexCoord2f(0.0f,1.0f); glVertex2i(0,0);
		glEnd();
		glBindTexture(GL_TEXTURE_2D,startgame);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex2f(100.0f*xratio,-400*yratio);
		glTexCoord2f(0.0f,0.0f); glVertex2f(100.0f*xratio,-480*yratio);
		glTexCoord2f(1.0f,0.0f); glVertex2f(300.0f*xratio,-480*yratio);
		glTexCoord2f(1.0f,1.0f); glVertex2f(300.0f*xratio,-400*yratio);
		glEnd();
		glBindTexture(GL_TEXTURE_2D,options);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex2f(100.0f*xratio,-480*yratio);
		glTexCoord2f(0.0f,0.0f); glVertex2f(100.0f*xratio,-560*yratio);
		glTexCoord2f(1.0f,0.0f); glVertex2f(300.0f*xratio,-560*yratio);
		glTexCoord2f(1.0f,1.0f); glVertex2f(300.0f*xratio,-480*yratio);
		glEnd();
		glBindTexture(GL_TEXTURE_2D,quit);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex2f(100.0f*xratio,-560*yratio);
		glTexCoord2f(0.0f,0.0f); glVertex2f(100.0f*xratio,-640*yratio);
		glTexCoord2f(1.0f,0.0f); glVertex2f(300.0f*xratio,-640*yratio);
		glTexCoord2f(1.0f,1.0f); glVertex2f(300.0f*xratio,-560*yratio);
		glEnd();
		glTranslatef(p.x,-p.y,0.0f); 
		glBindTexture(GL_TEXTURE_2D,cursor);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex2f(0,0);
		glTexCoord2f(0.0f,0.0f); glVertex2f(0,-30.0f*yratio);
		glTexCoord2f(1.0f,0.0f); glVertex2f(30*xratio,-30.0f*yratio);
		glTexCoord2f(1.0f,1.0f); glVertex2f(30*xratio,0);
		glEnd();
		break;
	case GAME_PLAYER_SELECT:
		glColor3f(0.75f,0.75f,0.75f);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,(double)width,-(double)height,0,500.0f,-500.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,textures[GAME_PLAYER_SELECT]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,3.0f); glVertex2i(0,0);
		glTexCoord2f(0.0f,0.0f); glVertex2i(0,-height);
		glTexCoord2f(4.0f,0.0f); glVertex2i(width,-height);
		glTexCoord2f(4.0f,3.0f); glVertex2i(width,0);
		glEnd();
		glColor3f(1.0f,1.0f,1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		blah = blah; /* FIXME: I think this was for breakpoints? */
		/* FIXME: HOLY IF CHAIN */
		if (charfloat != (float)chosenChar) {
			if(up) {
				if ((float)chosenChar < charfloat) {
					charfloat += 0.01f;
					if (charfloat > (float)(numChars+chosenChar))
						charfloat = (float)chosenChar;
				} else {
					charfloat += 0.01f;
					if ((float)chosenChar < charfloat)
						charfloat = (float)chosenChar;
				}
			} else {
				if((float)chosenChar > charfloat) {
					charfloat -= 0.01f;
					if(charfloat+(float)numChars < (float)chosenChar)
						charfloat = (float)chosenChar;
				} else {
					charfloat -= 0.01f;
					if((float)chosenChar > charfloat)
						charfloat = (float)chosenChar;
				}
			}
		} else {
			glDisable(GL_TEXTURE_2D);
			glTranslatef(100.0f,-600.0f,0.0f);
			glScalef(3.0f,3.0f,3.0f);
			//glRotatef(-90.0f,0.0f,1.0f,0.0f);
			CGparameter color;
			characters[chosenChar]->draw(color,&light,0);
			glLoadIdentity();
			glEnable(GL_TEXTURE_2D);
			glClear(GL_DEPTH_BUFFER_BIT);
		}

		glColor3f(1.0f,1.0f,1.0f);
		if (keys['W']) {
			chosenChar++;
			if(chosenChar >= numChars)
				chosenChar = 0;
			up = true;
			keys['W'] = false;
		}
		if (keys['S']) {
			chosenChar--;
			if(chosenChar < 0)
				chosenChar = numChars-1;
			up = false;
			keys['S'] = false;
		}
		
		for( i = 0; i < numChars; i++) {
			float x = cosf(((float)i-charfloat)*2.0f*3.14859f/(float)numChars)*300.0f+100.0f;
			float y = sinf(((float)i-charfloat)*2.0f*3.14859f/(float)numChars)*300.0f-height/2;
			
			glBindTexture(GL_TEXTURE_2D,characters[i]->statusface[0]);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f,1.0f); glVertex2f(x-100.0f,y+100.0f);
				glTexCoord2f(0.0f,0.0f); glVertex2f(x-100.0f,y-100.0f);
				glTexCoord2f(1.0f,0.0f); glVertex2f(x+100.0f,y-100.0f);
				glTexCoord2f(1.0f,1.0f); glVertex2f(x+100.0f,y+100.0f);
			glEnd();	
			if(lbuttondown && 
				(float)p.x > (x-100.0f)*xratio && 
				(float)p.x < (x+100.0f)*xratio && 
				(float)p.y < -(y-100.0f)*yratio && 
				(float)p.y > -(y+100.0f)*yratio)
			{
				chosenChar = i;
				up = (y > -height/2);
				lbuttondown = false;
			}
					
		}

		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f,0.0f,0.0f);
		xslide = sinf(angle)*50.0f+525.0f;
		glBegin(GL_TRIANGLES);
		glVertex2f(xslide+20.0f*sqrtf(3),-height/2+20.0f);
		glVertex2f(xslide,-height/2);
		glVertex2f(xslide+20.0f*sqrtf(3),-height/2-20.0f);
		glEnd();
		glEnable(GL_TEXTURE_2D);
		angle+=time*3.14859f/0.5f;
		if (angle > 3.14859f) {
			angle = 0.0f;
		}

		glColor3f(1.0f,1.0f,1.0f);
		glTranslatef((float)p.x,-(float)p.y,0.0f);
		glBindTexture(GL_TEXTURE_2D,cursor);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex2f(0,0);
		glTexCoord2f(0.0f,0.0f); glVertex2f(0,-30.0f*yratio);
		glTexCoord2f(1.0f,0.0f); glVertex2f(30*xratio,-30.0f*yratio);
		glTexCoord2f(1.0f,1.0f); glVertex2f(30*xratio,0);
		glEnd();

		if (keys[VK_RETURN]) {
			state = GAME_MAIN;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();									    
			gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,100.0f,10000.0f);
			glMatrixMode(GL_MODELVIEW);								
			glLoadIdentity();
			level->setPlayer(characters[chosenChar]);
			
			for (i = 0; i < 1/*level->MAXCHARS*/; i++) {
				Player* tempplayer = new Player("../characters/hihat.chr");
				level->AddChar(tempplayer);//new Player(*(characters[rand() %numChars])));
			}

			for (i = 0; i < numChars; i++) {
				if (i != chosenChar) {
					delete characters[i];
					characters[i] = NULL;
				}
			}
			delete [] characters;
			characters = NULL;
		}
		break;
	case GAME_MAIN:
		glLoadIdentity();
		level->Update(keys);
		level->draw(color);
		break;
	}
}

void
GameApp::ReSizeGLScene(int width, int height)
{
	this->width = width;
	this->height = height;
	if (height==0) {
		height=1;
	}

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,10.0f,10000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

unsigned int
GameApp::LoadTexture(char *filename)
{
	unsigned int texID;
	FILE* file = NULL;
	return -1;
	//file = fopen(filename,"r");
	
	// TODO: anything.
	//AUX_RGBImageRec *tex;
	//tex = auxDIBImageLoad(filename);
	/* STUB CODE HERE SINCE I DON'T REMEMBER WHERE THE ORIGINAL CODE CAME FROM */
	/* read header */
#if 0
	glGenTextures(1,&texID);
	glBindTexture(GL_TEXTURE_2D,texID);
	glTexImage2D(GL_TEXTURE_2D,0,3,tex->sizeX,tex->sizeY,0,GL_RGB, GL_UNSIGNED_BYTE,tex->data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	if(tex->data)
	{
		free(tex->data);
		tex->data = NULL;
	}
	free(tex);
	tex = NULL;
	return texID;
#endif
}

void
GameApp::Startup(float time)
{
	float splashtime = 0.0f;
	int startupstate = 0;
	float brightness = 0.0f;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,(double)width,-(double)height,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	switch(startupstate) {
	case 0:
		brightness+=time;
		if (brightness > 1.0f) {
			brightness = 1.0f;
			startupstate = 1;
		}
		break;
	case 1:
		splashtime+=time;
		if (splashtime > 5.0f) {
			startupstate = 2;
		}
		break;
	case 2:
		brightness-=time;
		if (brightness < 0.0f) {
			brightness = 0.0f;
			SetCursorPos(width/2,height/2);
			state = GAME_MAIN_MENU;
			brightness = 0.0f;
			startupstate = 0;
		}
		break;
	}

	glColor3f(brightness,brightness,brightness);
	glBindTexture(GL_TEXTURE_2D,textures[GAME_STARTUP]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,0.0f); glVertex2i(0,-height);
	glTexCoord2f(1.0f,0.0f); glVertex2i(width,-height);
	glTexCoord2f(1.0f,1.0f); glVertex2i(width,0);
	glTexCoord2f(0.0f,1.0f); glVertex2i(0,0);
	glEnd();
}		

void
GameApp::MainMenu(float time)
{
	float brightness = 0.0f;
	int startupstate = 0;
	int i;
	POINT p;
	GetCursorPos(&p);
	float xratio = (float)width/1024.0f;
	float yratio = (float)height/1024.0f;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,(double)width,-(double)height,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	switch(startupstate) {
	case 0:
		brightness+=time;
		if (brightness >= 1.0f) {
			brightness = 1.0f;				
		}
		if (lbuttondown && (float)p.x > 100.0f*xratio && (float)p.x < 300.0f*xratio && (float)p.y < 480.0f*yratio && (float)p.y > 400.0f*yratio) {
			startupstate = 1;
		}
		if (lbuttondown && (float)p.x > 100.0f*xratio && (float)p.x < 300.0f*xratio && (float)p.y < 640.0f*yratio && (float)p.y > 560.0f*yratio) {
			PostQuitMessage(0);
		}
		break;
	case 1:
		brightness-=time*2.0f;
		if (brightness <= 0.0f) {
			brightness <= 0.0f;
			startupstate = 2;
		}
		break;
	case 2:
		state = GAME_PLAYER_SELECT;
		Vector3f temp(0.0f,0.0f,0.0f);
		chosenChar = 0;
		Player *tempdata[50];
		WIN32_FIND_DATA findData;
		HANDLE tempHand = FindFirstFile("../characters/*.chr",&findData);
		numChars = 1;
		tempdata[0] = new Player(findData.cFileName);
		while (FindNextFile(tempHand,&findData) && numChars < 50) {
			numChars++;
			tempdata[numChars-1] = new Player(findData.cFileName);
		}
		characters = new Player*[numChars];
		for (i = 0; i < numChars; i++) {
			characters[i] = tempdata[i];
		}
		break;
	}

	glColor3f(brightness,brightness,brightness);
	glBindTexture(GL_TEXTURE_2D,textures[GAME_MAIN_MENU]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,0.0f); glVertex2i(0,-height);
	glTexCoord2f(1.0f,0.0f); glVertex2i(width,-height);
	glTexCoord2f(1.0f,1.0f); glVertex2i(width,0);
	glTexCoord2f(0.0f,1.0f); glVertex2i(0,0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D,startgame);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,1.0f); glVertex2f(100.0f*xratio,-400*yratio);
	glTexCoord2f(0.0f,0.0f); glVertex2f(100.0f*xratio,-480*yratio);
	glTexCoord2f(1.0f,0.0f); glVertex2f(300.0f*xratio,-480*yratio);
	glTexCoord2f(1.0f,1.0f); glVertex2f(300.0f*xratio,-400*yratio);
	glEnd();
	glBindTexture(GL_TEXTURE_2D,options);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,1.0f); glVertex2f(100.0f*xratio,-480*yratio);
	glTexCoord2f(0.0f,0.0f); glVertex2f(100.0f*xratio,-560*yratio);
	glTexCoord2f(1.0f,0.0f); glVertex2f(300.0f*xratio,-560*yratio);
	glTexCoord2f(1.0f,1.0f); glVertex2f(300.0f*xratio,-480*yratio);
	glEnd();
	glBindTexture(GL_TEXTURE_2D,quit);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,1.0f); glVertex2f(100.0f*xratio,-560*yratio);
	glTexCoord2f(0.0f,0.0f); glVertex2f(100.0f*xratio,-640*yratio);
	glTexCoord2f(1.0f,0.0f); glVertex2f(300.0f*xratio,-640*yratio);
	glTexCoord2f(1.0f,1.0f); glVertex2f(300.0f*xratio,-560*yratio);
	glEnd();
	glTranslatef(p.x,-p.y,0.0f); 
	glBindTexture(GL_TEXTURE_2D,cursor);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,1.0f); glVertex2f(0,0);
	glTexCoord2f(0.0f,0.0f); glVertex2f(0,-30.0f*yratio);
	glTexCoord2f(1.0f,0.0f); glVertex2f(30*xratio,-30.0f*yratio);
	glTexCoord2f(1.0f,1.0f); glVertex2f(30*xratio,0);
	glEnd();
}

void
GameApp::print(const char* fmt, ...)
{
	int length = 0;
	char string[256];
	va_list args;

	if(fmt == NULL)
		return;
	va_start(args,fmt);
		vsprintf(string,fmt,args);
	va_end(args);
	
	glCullFace(GL_BACK);
	glEnable(GL_COLOR_MATERIAL);
	glPushAttrib(GL_LIST_BIT);
	glListBase(font);
	glCallLists(strlen(string),GL_UNSIGNED_BYTE,string);
	glPopAttrib();
	glCullFace(GL_FRONT);
	glColor3f(1.0f,1.0f,1.0f);
	glDisable(GL_COLOR_MATERIAL);
}

