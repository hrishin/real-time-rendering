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

#define CLOSE_FILE(x) if(x != NULL) { fclose(x); }

using namespace std;

bool gbFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColorMap;
Window gWindow;
int giWindowWidth = 800;
int giWindowHeight = 600;

GLXContext gGLXContext;

FILE *gpLogFile = NULL;

GLfloat anglePyramid = 0.0f;
GLfloat angleCube = 360.0f;

int shoulder = 0;
int elbow = 0;
GLUquadric *qudric = NULL;

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
	gpLogFile = fopen("trace.log", "w+");
	{	
		printf("error in creating log file\n");
	}	
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
					XLookupString(&event.xkey, NULL, 0, &keysym, NULL);
                	switch(keysym)
                	{
						
                    	case XK_Escape:
                        	bDone = true;
							break;

						case XK_S:
							shoulder = (shoulder + 3) % 360;
							break;

						case XK_s:
							shoulder = (shoulder - 3) % 360;
							break;

                    	case XK_E:
							elbow = (elbow + 3) % 360;
							break;
						
						case XK_e:
							elbow = (elbow - 3) % 360;
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
	fprintf(gpLogFile, "entering CreateWindow\n");
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
		GLX_DEPTH_SIZE, 24,	
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

    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);

    XMapWindow(gpDisplay, gWindow);
	fprintf(gpLogFile, "exiting CreateWindow\n");
}

void ToggleFullscreen(void)
{
	fprintf(gpLogFile, "entering ToggleFullScreen\n");

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
	fprintf(gpLogFile, "exiting ToggleFullScreen\n");
}

void Initialize(void)
{
	fprintf(gpLogFile, "entering Initialization\n");

	void Resize(int, int);

	gGLXContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);

	glXMakeCurrent(gpDisplay, gWindow, gGLXContext);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	Resize(giWindowWidth, giWindowHeight);
	fprintf(gpLogFile, "exiting Initialization\n");
}

void Render(void)
{
	fprintf(gpLogFile, "entering render\n");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// view transformation
	glTranslatef(0.0f, 0.0f, -12.0f);

	// model transformation
	glPushMatrix();

	glRotatef((GLfloat)shoulder, 0.0f, 0.0f, 1.0f);
	glTranslatef(1.0f, 0.0f, 0.0f);
	glPushMatrix();
	glScalef(2.0f, 0.5f, 1.0f);
	qudric = gluNewQuadric();
	glColor3f(0.5f, 0.35f, 0.05f);
	gluSphere(qudric, 0.5, 10, 10);
	glPopMatrix();

	glTranslatef(1.0f, 0.0f, 0.0f);
	glRotatef((GLfloat)elbow, 0.0f, 0.0f, 1.0f);
	glTranslatef(1.0f, 0.0f, 0.0f);
	glPushMatrix();
	glScalef(2.0f, 0.05f, 1.0f);
	qudric = gluNewQuadric();
	glColor3f(0.5f, 0.35f, 0.05f);
	gluSphere(qudric, 0.5f, 10, 10);
	glPopMatrix();

	glPopMatrix();

	glXSwapBuffers(gpDisplay, gWindow); 
	fprintf(gpLogFile, "exiting render\n");
}

void update(void)
{
	fprintf(gpLogFile, "entering Update\n");
	anglePyramid = anglePyramid + 0.1f;
    if (anglePyramid >= 360)
    {
    	anglePyramid = 0;
    }

  	angleCube = anglePyramid - 0.4f;
  	if (angleCube <=  0)
  	{
    	angleCube = 360.0f;
  	}
	fprintf(gpLogFile, "exiting Update\n");
}

void Resize(int width, int height)
{
	fprintf(gpLogFile, "entering Resize\n");
	if(height == 0)
	{
		height = 1;
	}
	
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	fprintf(gpLogFile, "exiting Resize\n");
}

void Uninitialize(void)
{
	fprintf(gpLogFile, "entering Uninitialize\n");
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
	fprintf(gpLogFile, "exiting Uninitialize\n");
	CLOSE_FILE(gpLogFile);
}
