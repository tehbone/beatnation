#include <stdio.h>
#include "physics.hpp"
#include <cstdlib>
#include "gfx.hpp"
#include "camera.hpp"
#include "objects.hpp"
#include "level.hpp"
#include "gameapp.hpp"
#include "t_utils.cpp"
#include "utils.hpp"

#include <iostream>
#include <SDL.h>
#include <SDL_video.h>





#define MAXPOS 90.0f





GameApp*        game;
bool	keys[256];								    // Array Used For The Keyboard Routine
bool	active=true;								// Window Active Flag Set To TRUE By Default
bool lbuttondown = false;
//Prototypes
#if 0

// Called when window is closed

GLvoid KillGLWindow(GLvoid)							// Properly Kill The Window
{
	ChangeDisplaySettings(NULL,0);
	if (hRC)										// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))				// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		
		if (!wglDeleteContext(hRC))					// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;									// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))				// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;									// Set DC To NULL
	}

		if (hWnd && !DestroyWindow(hWnd))			// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;									// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))				// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;								// Set hInstance To NULL
	}
	
}



// Creates The GL Window (Fullscreen support: supported)

BOOL CreateGLWindow(char* title, int width, int height, int bits)
{
	
	
	// Ready data types
	GLuint		PixelFormat;					// Holds The Results After Searching For A Match
	WNDCLASS	wc;								// Windows Class Structure
	DWORD		dwExStyle;						// Window Extended Style
	DWORD		dwStyle;						// Window Style
	
	// Set up window settings
	RECT WindowRect;							// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;					// Set Left Value To 0
	WindowRect.right=(long)width;				// Set Right Value To Requested Width
	WindowRect.top=(long)0;						// Set Top Value To 0
	WindowRect.bottom=(long)height;				// Set Bottom Value To Requested Height
	
	// Initialize Window Class
	hInstance			= GetModuleHandle(NULL);					// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;		// Redraw On Move, And Own DC For Window
	wc.lpfnWndProc		= (WNDPROC) WndProc;						// WndProc Handles Messages
	wc.cbClsExtra		= 0;										// No Extra Window Data
	wc.cbWndExtra		= 0;										// No Extra Window Data
	wc.hInstance		= hInstance;								// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);				// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);				// Load The Arrow Pointer
	wc.hbrBackground	= NULL;										// No Background Required For GL
	wc.lpszMenuName		= NULL;										// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";									// Set The Class Name


	if (!RegisterClass(&wc))										// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;												// Exit And Return FALSE
	}

	DEVMODE dmScreenSettings;									// Device Mode
	memset(&dmScreenSettings,0,sizeof(dmScreenSettings));		// Makes Sure Memory's Cleared
	dmScreenSettings.dmSize=sizeof(dmScreenSettings);			// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth	= width;					// Selected Screen Width
	dmScreenSettings.dmPelsHeight	= height;					// Selected Screen Height
	dmScreenSettings.dmBitsPerPel	= bits;						// Selected Bits Per Pixel
	dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
	
	// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
	if(ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
	{
		char* blah = "testing";
	}
	dwExStyle=WS_EX_APPWINDOW;				// Window Extended Style
	dwStyle=WS_POPUP;						// Windows Style
	ShowCursor(FALSE);						// Hide Mouse Pointer
		
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size
	
	if (!(hWnd=CreateWindowEx(	dwExStyle,				// Extended Style For The Window
					"OpenGL",							// Class Name
					title,								// Window Title
					WS_CLIPSIBLINGS |					// Required Window Style
					WS_CLIPCHILDREN |					// Required Window Style
					dwStyle,							// Selected Window Style
					0, 0,								// Window Position
					WindowRect.right-WindowRect.left,	// Calculate Adjusted Window Width
					WindowRect.bottom-WindowRect.top,	// Calculate Adjusted Window Height
					NULL,								// No Parent Window
					NULL,								// No Menu
					hInstance,							// Instance
					NULL)))								// Don't Pass Anything To WM_CREATE
	{
		KillGLWindow();									// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									// Return FALSE
	}
	
	static	PIXELFORMATDESCRIPTOR pfd=					// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		bits,											// Select Our Color Depth
		0, 0, 0, 0, 0, 0,								// Color Bits Ignored
		0,												// No Alpha Buffer
		0,												// Shift Bit Ignored
		0,												// No Accumulation Buffer
		0, 0, 0, 0,										// Accumulation Bits Ignored
		16,												// 16Bit Z-Buffer (Depth Buffer)
		0,												// Stencil Buffer Enabled!!
		0,												// No Auxiliary Buffer
		PFD_MAIN_PLANE,									// Main Drawing Layer
		0,												// Reserved
		0, 0, 0											// Layer Masks Ignored
	};

	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
	
	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))				// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}


	if(!SetPixelFormat(hDC,PixelFormat,&pfd))				// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}


	if (!(hRC=wglCreateContext(hDC)))					// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}


	if(!wglMakeCurrent(hDC,hRC))						// Try To Activate The Rendering Context
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}


	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);								// Sets Keyboard Focus To The Window
	
	return TRUE;								// Success
}


// WndProc - Handles Window's events
LRESULT CALLBACK WndProc(	HWND	hWnd,					// Handle For This Window
				UINT	uMsg,					// Message For This Window
				WPARAM	wParam,					// Additional Message Information
				LPARAM	lParam)					// Additional Message Information
{

		
	switch (uMsg)								// Check For Windows Messages
	{
	
		case WM_ACTIVATE:						// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;					// Program Is Active
			}
			else
			{
				active=FALSE;					// Program Is No Longer Active
			}

			return 0;						// Return To The Message Loop
		}

		case WM_SYSCOMMAND:						// Intercept System Commands
		{
			switch (wParam)						// Check System Calls
			{
				case SC_SCREENSAVE:				// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;					// Prevent From Happening
			}
			break;							// Exit
		}

		case WM_CLOSE:							// Did We Receive A Close Message?
		{
			PostQuitMessage(0);					// Send A Quit Message
			return 0;						// Jump Back
		}

		case WM_KEYDOWN:						// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;						// Jump Back
		}

		case WM_KEYUP:							// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;						// Jump Back
		}

		case WM_SIZE:							// Resize The OpenGL Window
		{
			//ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));		// LoWord=Width, HiWord=Height
			return 0;						// Jump Back
		}
		case WM_LBUTTONDOWN:
			lbuttondown = true;
			break;
		case WM_LBUTTONUP:
			lbuttondown = false;
			break;

	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


// WinMain - Main Windows Function (Loop)

int WINAPI WinMain(	HINSTANCE	hInstance,				// Instance
			HINSTANCE	hPrevInstance,				// Previous Instance
			LPSTR		lpCmdLine,				// Command Line Parameters
			int		nCmdShow)				// Window Show State
{

	
	MSG	msg;								// Windows Message Structure
	BOOL	done=FALSE;							// Bool Variable To Exit Loop
	
	// Ask The User Which Screen Mode They Prefer
		
	// Create Our OpenGL Window
	if (!CreateGLWindow("NeHe's OpenGL Framework",1024,768,32))
	{
		return 0;							// Quit If Window Was Not Created
	}

	//Some initialization stuff
	ShowCursor(false);
	game = new GameApp(1024,768,keys);
	// Main Loop
	while(!done)								// Loop That Runs Until done=TRUE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))			// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;					// If So done=TRUE
			}
			else							// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else								// If There Are No Messages
		{
			if (active)						// Program Active?
			{

			}
		
			if (keys[VK_ESCAPE])				// Was ESC Pressed?
			{
					done=TRUE;				// ESC Signalled A Quit
			}
			else						// Not Time To Quit, Update Screen
			{
					if(game)
					{
						SwapBuffers(hDC);			// Swap Buffers (Double Buffering)
						game->RenderScene();				// Draw The Scene

					}
			}
		}
	}
	// Shutdown
	if(game)
		delete game;
	ShowCursor(true);
	KillGLWindow();								// Kill The Window
	return (msg.wParam);							// Exit The Program
}
#endif

int
main (int argc, char **argv)
{
	//FIXME: Perhaps wrap this in a class
	SDL_Window *window;
	SDL_GLContext glcontext;
	bool done = false;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	window = SDL_CreateWindow("BeatNat!on", SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_OPENGL);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0); //FIXME: Use a more recent version.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			    SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); //FIXME: Core
	glcontext = SDL_GL_CreateContext(window);
	if (!glcontext) {
		std::cout << "Something Happened" << std::endl;
	}

	SDL_ShowCursor(SDL_DISABLE);
	game = new GameApp(1024,768,keys);
	while (!done) {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				done = true;
				break;
			case SDL_KEYDOWN:
				keys[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				keys[event.key.keysym.sym] = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
					lbuttondown = true;
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
					lbuttondown = false;
				break;
			}
		} else {
			if (active) {

			}
		
			if (keys[SDLK_ESCAPE]) {
				done = true;
			} else if (game) {
				SDL_GL_SwapWindow(window);
				game->RenderScene();
			}
		}
	}

	if(game)
		delete game;

	SDL_ShowCursor(SDL_ENABLE);
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

