/**
 * @file Main program code file.
 * Game Design Project 2004
 */
// Remember to add libraries to settings->options: OpenGL32.lib GLu32.lib GLaux.lib (windows)
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#define MAXPOS 90.0f

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

HGLRC hRC;
HDC hDC;
HWND hWnd;
HINSTANCE hInstance
GameApp* game;

bool keys[256];
bool active=true;
bool lbuttondown;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

GLvoid
KillGLWindow(GLvoid)
{
	ChangeDisplaySettings(NULL,0);
	if (hRC) {
		if (!wglMakeCurrent(NULL,NULL)) {
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		
		if (!wglDeleteContext(hRC)) {
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;
	}

	if (hDC && !ReleaseDC(hWnd,hDC)) {
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;
	}

	if (hWnd && !DestroyWindow(hWnd)) {
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;
	}

	if (!UnregisterClass("OpenGL",hInstance)) {
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits)
{
	GLuint PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;
	WindowRect.left=0l;
	WindowRect.right=(long)width;
	WindowRect.top=0l;
	WindowRect.bottom=(long)height;
	
	// Initialize Window Class
	hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC) WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OpenGL";

	if (!RegisterClass(&wc)) {
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
	dmScreenSettings.dmSize=sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth = width;
	dmScreenSettings.dmPelsHeight = height;
	dmScreenSettings.dmBitsPerPel = bits;
	dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
	
	// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
	if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
		char* blah = "testing";
	}
	dwExStyle = WS_EX_APPWINDOW;
	dwStyle = WS_POPUP;
	ShowCursor(FALSE);
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);
	if (!(hWnd=CreateWindowEx(dwExStyle,
				  "OpenGL",
				  title,
				  WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
				  0, 0,
				  WindowRect.right-WindowRect.left,
				  WindowRect.bottom-WindowRect.top,
				  NULL,
				  NULL,
				  hInstance,
				  NULL))) {
		KillGLWindow();
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	static	PIXELFORMATDESCRIPTOR pfd = {
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

	if (!(hDC=GetDC(hWnd))) {
		KillGLWindow();
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	{
		KillGLWindow();
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!SetPixelFormat(hDC,PixelFormat,&pfd)) {
		KillGLWindow();
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(hRC=wglCreateContext(hDC))) {
		KillGLWindow();
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!wglMakeCurrent(hDC,hRC)) {
		KillGLWindow();
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	ShowWindow(hWnd,SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	
	return TRUE;
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
/*void initCG()
{
	context = cgCreateContext();
	profile = cgGLGetLatestProfile(CG_GL_VERTEX);
	

	cgGLSetOptimalOptions(profile);
	program = cgCreateProgramFromFile(context,CG_SOURCE, "VertexProgram.cg", profile,"main",0);
	cgGLLoadProgram(program);
	color = cgGetNamedParameter(program,"color");
	position = cgGetNamedParameter(program,"position");
	modelViewProjMatrix = cgGetNamedParameter(program,"model");
}*/
