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

using namespace std;

bool gbFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColorMap;
Window gWindow;
typedef GLXContext (*glXCreateContextAttribsARBProc) (Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig gGLXFBConfig;
GLXContext gGLXContext;

int giWindowWidth = 800;
int giWindowHeight = 600;

FILE *gpFile = NULL;


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
	
	gpFile = fopen("Log.txt", "w");
	if(gpFile == NULL)
	{
		printf("Log File can be created. Exiting now \n");
		exit(0);
	}
	else 
	{
		fprintf(gpFile, "Log file is created successfully \n");
	}

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
	GLXFBConfig *pGLXFBConfigs = NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo = NULL;
	int iNumFBConfigs = 0;
	int styleMask;
	int i;

	static int frameBufferAttributes[]=
	{
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_DOUBLEBUFFER, 1,
		None
	};

    gpDisplay = XOpenDisplay(NULL);
    if(gpDisplay == NULL)
    {
        printf("ERROR : Unable to Open X Display. \nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

	// get all mathcing FB configs	
	pGLXFBConfigs = glXChooseFBConfig(gpDisplay, DefaultScreen(gpDisplay), frameBufferAttributes, &iNumFBConfigs);
	
	if(pGLXFBConfigs == NULL)
	{
		printf("Failed to get vlida frame buffer config\n");
		Uninitialize();
		exit(0);
	}

	printf("%d  mathcing FB configs found", iNumFBConfigs);
	
	// Pick the FB config with most with most samples per pixel
	int bestFBConfig = 1;
	int worstFBConfig = 1;
	int bestNumberOfSamples = -1;
	int worstNumberOfSamples = 999;

	for(i=0; i<iNumFBConfigs; i++)
	{
		pTempXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, pGLXFBConfigs[i]);

		if(pTempXVisualInfo)
		{
			int sampleBuffer, samples;
			
			glXGetFBConfigAttrib(gpDisplay, pGLXFBConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffer);
		 	glXGetFBConfigAttrib(gpDisplay, pGLXFBConfigs[i], GLX_SAMPLE_BUFFERS, &samples);
			printf("Mathcing\n");

			if(bestFBConfig < 0 || sampleBuffer && samples > bestNumberOfSamples)
			{
				bestFBConfig = i;
				bestNumberOfSamples = samples;
			}

			if(worstNumberOfSamples < 0 || !sampleBuffer || samples < worstNumberOfSamples)
			{
				worstFBConfig = i;
				worstNumberOfSamples = samples;
			}
		}
		XFree(pTempXVisualInfo);
	}	

	bestGLXFBConfig = pGLXFBConfigs[bestFBConfig];
	gGLXFBConfig = bestGLXFBConfig;

	XFree(pGLXFBConfigs);

    gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, bestGLXFBConfig);
	printf("Chosen Visual ID = 0X%lu\n", gpXVisualInfo -> visualid);

    winAttribs.border_pixel = 0;
    winAttribs.background_pixmap = 0;
    winAttribs.colormap = XCreateColormap(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo -> screen), gpXVisualInfo ->visual, AllocNone);
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

    XStoreName(gpDisplay, gWindow, "First Window");

    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
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
	fprintf(gpFile, "Starting Initialization");
	void Resize(int, int);

	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB((GLubyte*) "glXCreateContextAttribsARB");

	GLint attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
		GLX_CONTEXT_PROFILE_MASK_ARB,
		GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0
	};

	gGLXContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attribs);
	
	// fallback to safe old style 2.4
	if(!gGLXContext)
	{
		GLint attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			0
		};
		printf("Failed to create GLX 4.5 context");
		gGLXContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attribs);
	}
	else
	{
		printf("OpenGL context 4.5 is created \n");
	}
	
	// verify if this cotext is direct context
	if(!glXIsDirect(gpDisplay, gGLXContext))
	{
		printf("Indirect GLX Rendering context obtained\n");
	}
	else
	{
		printf("Direct GLX Rendering context obtained\n");
	}
 
	glXMakeCurrent(gpDisplay, gWindow, gGLXContext);

	glShadeModel(GL_SMOOTH);
	
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_CULL_FACE);
	
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

	Resize(giWindowWidth, giWindowHeight);

	fprintf(gpFile, "Exiting Initialization");
}

void Render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glXSwapBuffers(gpDisplay, gWindow);
}

void Resize(int width, int height)
{
	if(height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
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

