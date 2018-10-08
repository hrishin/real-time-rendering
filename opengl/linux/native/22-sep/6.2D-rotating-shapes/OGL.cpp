#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

using namespace std;

bool gbFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColorMap;
Window gWindow;
int giWindowWidth = 800;
int giWindowHeight = 600;

GLXContext gGLXContext;

GLfloat angleTri = 0.0f;
GLfloat angleRect = 360.0f;

// entry point function
int main(void)
{
    // function prototypes
    void CreateWindow(void);
    void ToggleFullscreen(void);
	void Initialize(void);
	void Render(void);
	void Resize(int, int); 
    void Uninitialize(void);

    // varible declearation
    int winWidth = giWindowWidth;
    int winHeight = giWindowHeight;

    bool bDone = false;

    CreateWindow();

	Initialize();
	
	// Game loop
    XEvent event;
    KeySym keysym;

    while(bDone == false)
    {
        while(XPending(gpDisplay))
		{
		
			XNextEvent(gpDisplay, &event);
        	switch(event.type)
        	{
            	case MapNotify:
                	break;

            	case KeyPress:
                	keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                	switch(keysym)
                	{
                    	case XK_Escape:
                        	bDone = true;
							break;

                    	case XK_F:
                    	case XK_f:
                        	if(gbFullScreen == false)
                        	{
                            	ToggleFullscreen();
                            	gbFullScreen = true;
                        	}
                        	else
                        	{
                            	ToggleFullscreen();
                            	gbFullScreen = false;
                        	}
                        	break;

                    	default:
                        	break;
                	}
                	break;

            	case ButtonPress:
                	switch(event.xbutton.button)
                	{
                    	case 1:
                        	break;
  
                    	case 2:
                        	break;
                    
  		    			case 3:
                        	break;
                    
	            		default:
                        	break;

                	}
                	break;

            	case MotionNotify:
                	break;

            	case ConfigureNotify:
                	winWidth = event.xconfigure.width;
                	winHeight = event.xconfigure.height;
					Resize(winWidth, winHeight);
                	break;

            	case Expose:
                	break;

	            case DestroyNotify:
   		            break;

            	case 33:
					bDone = true;
					break;

            	default:
                	break;
        	}
		}

		Render();
    }

	Uninitialize();
    return (EXIT_SUCCESS);
}

void CreateWindow(void)
{
    void Uninitialize(void);

    //varible declarations
    XSetWindowAttributes winAttribs;
    int defaultScreen;
    int styleMask;

	static int frameBufferAttributes[]=
	{
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_ALPHA_SIZE, 1,
		GLX_DOUBLEBUFFER, True,
		None
	};

    gpDisplay = XOpenDisplay(NULL);
    if(gpDisplay == NULL)
    {
        printf("ERROR : Unable to Open X Display. \nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    defaultScreen = XDefaultScreen(gpDisplay);

    gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
    if(gpXVisualInfo == NULL)
    {
        printf("ERROR: Unable to get a Visual Info.\nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    winAttribs.border_pixel = 0;
    winAttribs.background_pixmap = 0;
    winAttribs.colormap = XCreateColormap(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo -> screen), gpXVisualInfo ->visual, AllocNone);
    gColorMap = winAttribs.colormap;
    winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
    winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask | StructureNotifyMask;

	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

    gWindow  = XCreateWindow(gpDisplay,
                            RootWindow(gpDisplay, gpXVisualInfo -> screen),
                            0,
                            0,
                            giWindowWidth,
                            giWindowHeight,
                            0,
                            gpXVisualInfo -> depth,
                            InputOutput,
                            gpXVisualInfo -> visual,
                            styleMask,
                            &winAttribs);

    if(!gWindow)
    {
        printf("ERROR : Failed to creat main window.\nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    XStoreName(gpDisplay, gWindow, "2D shape double buffer");

    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELTE_WINDOW", True);
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);

    XMapWindow(gpDisplay, gWindow);
}

void ToggleFullscreen(void)
{
    Atom wm_state;
    Atom fullscreen;
    XEvent xev={0};

    wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    memset(&xev, 0, sizeof(xev));

    xev.type = ClientMessage;
    xev.xclient.window = gWindow;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = gbFullScreen ? 0 : 1;

    fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[1] = fullscreen;

    XSendEvent(gpDisplay,
            RootWindow(gpDisplay, gpXVisualInfo -> screen),
	    	False,
            StructureNotifyMask,
            &xev);
}

void Initialize(void)
{
	void Resize(int, int);

	gGLXContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);

	glXMakeCurrent(gpDisplay, gWindow, gGLXContext);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	Resize(giWindowWidth, giWindowHeight);
}

void Render(void)
{
	void renderSquare(void);
   	void renderTriangle(void);
    void update(void);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1.5f, 0.0f, -6.0f); //model transformation
    glRotatef(angleTri, 0.0f, 1.0f, 0.0f);
    renderTriangle();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(1.5f, 0.0f, -6.0f); 
    glRotatef(angleRect, 1.0f, 0.0f, 0.0f);
   	renderSquare();

  	/*Animation comes here*/
  	update();

	glXSwapBuffers(gpDisplay, gWindow); 
}

void renderTriangle(void)
{
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 1.0f, 0.5f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glColor3f(0.2f, 1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
        glEnd();
}

void renderSquare(void)
{
        glBegin(GL_QUADS);
        glColor3f(0.3f, 0.5f, 0.9f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glEnd();
}

void update(void)
{
        angleTri = angleTri + 0.1f;
        if (angleTri >= 360)
        {
                angleTri = 0;
        }

        angleRect = angleTri - 0.4f;
        if (angleRect <=  0)
        {
                angleRect = 360.0f;
        }
}

void Resize(int width, int height)
{
	if(height == 0)
	{
		height = 1;
	}
	
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void Uninitialize(void)
{
	GLXContext currentGLXContext;

	currentGLXContext = glXGetCurrentContext();

	if(currentGLXContext != NULL && currentGLXContext != gGLXContext)
	{	
		glXMakeCurrent(gpDisplay, 0, 0);
	}
    
	if(gGLXContext)
	{	
		glXDestroyContext(gpDisplay, gGLXContext);
	}
	
	if(gWindow)
    {
        XDestroyWindow(gpDisplay, gWindow);
    }

    if(gColorMap)
    {
        XFreeColormap(gpDisplay, gColorMap);
    }

    if(gpXVisualInfo)
    {
        free(gpXVisualInfo);
        gpXVisualInfo = NULL;
    }

    if(gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }
}
