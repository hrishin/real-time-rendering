#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/glew.h>
#include <GL/glx.h>

#include "vmath.h"

#include "ogl_aux.h"

using namespace std;
using namespace vmath;

enum
{
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_COLOR,
    VDG_ATTRIBUTE_NORMAL,
    VDG_ATTRIBUTE_TEXTURE0,
};

// windowing properties
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColorMap;
Window gWindow;
int giWindowWidth = 800;
int giWindowHeight = 600;
const char *gpWindowTitle = "Programmable Pipeline : Diffuse light cube";

GLXContext gGLXContext;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVaoSquare;
GLuint gVboSquarePosition;
GLuint gVboSquareNormal;

GLuint gModelViewMatrixUniform, gProjectionMatrixUniform;
GLuint gLdUniform, gKdUniform, gLightPositionUniform;
GLuint gLKeyPressedUniform;

mat4 gPerspectiveProjectionMatrix;

GLfloat gAngle = 0.0f;

FILE *gpLogFile;

bool gbFullScreen = false;
bool gbAnimate;
bool gbLight;

// entry point function
int main(int argc, char *arg[])
{
    // function prototypes
    void CreateWindow(void);
    void ToggleFullscreen(void);
    void Initialize(void);
    void Render(void);
    void Update(void);
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

    while (bDone == false)
    {
        while (XPending(gpDisplay))
        {
            XNextEvent(gpDisplay, &event);
            switch (event.type)
            {
            case MapNotify:
                break;

            case KeyPress:
                keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                switch (keysym)
                {
                case XK_Escape:
                    bDone = true;
                    break;

                case XK_F:
                case XK_f:
                    if (gbFullScreen == false)
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

                case XK_A:
                case XK_a:
                    gbAnimate = !gbAnimate; 
                    break;

                case XK_L:
                case XK_l:
                    gbLight = !gbLight;
                    break;

                default:
                    break;
                }
                break;

            case ButtonPress:
                switch (event.xbutton.button)
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
        Update();
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

    static int frameBufferAttributes[] =
        {
            GLX_RGBA,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            GLX_ALPHA_SIZE, 1,
            GLX_DEPTH_SIZE, 24,
            GLX_DOUBLEBUFFER, True,
            None};

    gpDisplay = XOpenDisplay(NULL);
    if (gpDisplay == NULL)
    {
        fprintf(stderr, "ERROR : Unable to Open X Display. \nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    defaultScreen = XDefaultScreen(gpDisplay);

    gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
    if (gpXVisualInfo == NULL)
    {
        fprintf(stderr, "ERROR: Unable to get a Visual Info.\nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    winAttribs.border_pixel = 0;
    winAttribs.background_pixmap = 0;
    winAttribs.colormap = XCreateColormap(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), gpXVisualInfo->visual, AllocNone);
    gColorMap = winAttribs.colormap;
    winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
    winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask | StructureNotifyMask;

    styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

    gWindow = XCreateWindow(gpDisplay,
                            RootWindow(gpDisplay, gpXVisualInfo->screen),
                            0,
                            0,
                            giWindowWidth,
                            giWindowHeight,
                            0,
                            gpXVisualInfo->depth,
                            InputOutput,
                            gpXVisualInfo->visual,
                            styleMask,
                            &winAttribs);

    if (!gWindow)
    {
        printf("ERROR : Failed to creat main window.\nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    XStoreName(gpDisplay, gWindow, gpWindowTitle);

    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);

    XMapWindow(gpDisplay, gWindow);
}

void Initialize(void)
{
    void Resize(int, int);
    void Uninitialize(void);
    void logShaderCompilationStatus(GLuint, FILE *, const char *);
    void logProgramCompilationStatus(GLuint, FILE *, const char *);

    gpLogFile = fopen("log.txt", "w");
    if (gpLogFile)
    {
        fprintf(gpLogFile, "Log file created successfully \n");
    }
    else
    {
        fprintf(stderr, "Log file can't be created \n exiting \n");
        exit(EXIT_FAILURE);
    }

    gGLXContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);
    if (gGLXContext == NULL)
    {
        fprintf(stderr, "ERROR: Unable to create OpenGL context.\nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    if (glXMakeCurrent(gpDisplay, gWindow, gGLXContext) == False)
    {
        fprintf(stderr, "ERROR: Unable to set the current context GL context.\nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    // GLEW initialization
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        fprintf(stderr, "ERROR: Unable to initialize extension wranglers (GLEW).\nExiting now...\n");
        Uninitialize();
        exit(EXIT_FAILURE);
    }

    // VERTEX SHADER
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vertexShaderSourceCode =
		"#version 130" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_LKeyPressed;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Kd;" \
		"uniform vec4 u_light_position;" \
		"out vec3 diffuse_light;" \
		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoordinates = u_model_view_matrix * vPosition;"\
		"vec3 tnorm = normalize(mat3(u_model_view_matrix) * vNormal);" \
		"vec3 s = normalize(vec3(u_light_position - eyeCoordinates));" \
		"diffuse_light = u_Ld * u_Kd * max(dot(s, tnorm), 0.0);" \
		"}" \
		"gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;" \
		"}";
    glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    glCompileShader(gVertexShaderObject);
    LogShaderCompilationStatus(gVertexShaderObject, gpLogFile, "Vertex Shader Compilation Log");

    // FRAGMENT SHADER
    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderSourceCode =
		"#version 130" \
		"\n" \
		"in vec3 diffuse_light;" \
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;" \
		"void main(void)" \
		"{" \
		"vec4 color;" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"color = vec4(diffuse_light, 1.0);"
		"}" \
		"else" \
		"{" \
		"color = vec4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = color;" \
		"}";

    glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
    glCompileShader(gFragmentShaderObject);
    LogShaderCompilationStatus(gFragmentShaderObject, gpLogFile, "Fragment Shader Compilation Log");

    // SHADER PROGRAM
    gShaderProgramObject = glCreateProgram();

    // attach vertex shader to shader program
    glAttachShader(gShaderProgramObject, gVertexShaderObject);

    // attach vertex shader to shader program
    glAttachShader(gShaderProgramObject, gFragmentShaderObject);

    // pre-link binding of shader program object with vertex shader "position" attribute
    glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_VERTEX, "vPosition");

    // pre-link binding of shader program object with vertex shader "normals" attribute
    glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_NORMAL, "vNormal");

    // link shader
    glLinkProgram(gShaderProgramObject);
    LogProgramCompilationStatus(gShaderProgramObject, gpLogFile, "Shader Program Link Log");

    // get MVP uniform location
	gModelViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");

	gLdUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	gKdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");

	gLightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

    	// Cube VAO
	glGenVertexArrays(1, &gVaoSquare);
	glBindVertexArray(gVaoSquare);

	// Position
	const GLfloat cubeVertices[] =
	{
		1.0f, 1.0f, -1.0f, // TOP
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, -1.0f, -1.0f, // bootom 
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, // front
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, 1.0f, -1.0f, // back
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, 1.0f, -1.0f, // right
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, 1.0f, -1.0f, // left
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f
	};	

	glGenBuffers(1, &gVboSquarePosition);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSquarePosition);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Normals
	const GLfloat cubeNormals[] =
	{
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

	glGenBuffers(1, &gVboSquareNormal);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSquareNormal);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // perspective matrix to identity matrix
    gPerspectiveProjectionMatrix = mat4::identity();

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glEnable(GL_CULL_FACE);

    gbAnimate = false;
	gbLight = false;

    // warm up
    Resize(giWindowWidth, giWindowHeight);
}

void ToggleFullscreen(void)
{
    Atom wm_state;
    Atom fullscreen;
    XEvent xev = {0};

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
               RootWindow(gpDisplay, gpXVisualInfo->screen),
               False,
               StructureNotifyMask,
               &xev);
}

void Render(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gShaderProgramObject);

	if (gbLight)
	{
		glUniform1i(gLKeyPressedUniform, 1);

		glUniform3f(gLdUniform, 1.0f, 1.0f, 1.0f);
		glUniform3f(gKdUniform, 0.5f, 0.5f, 0.5f);

		float lightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};
		glUniform4fv(gLightPositionUniform, 1, (GLfloat*) lightPosition);
	}
	else 
	{
		glUniform1i(gLKeyPressedUniform, 0);
	}

	// set all matrices to identity
	mat4 modelMatrix = mat4::identity();
	mat4 modelViewMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	// translate z axis
	modelMatrix = translate(0.0f, 0.0f, -5.0f);

	// all axes rotation by gAngle angle
	rotationMatrix = rotate(gAngle, gAngle, gAngle);

	modelViewMatrix = modelMatrix * rotationMatrix; // order is important

	// pass the modelViewMatrix to vertex shader variable
	glUniformMatrix4fv(gModelViewMatrixUniform, 1, GL_FALSE, modelViewMatrix);

	// pass the projectionlViewMatrix to vertex shader variable
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	// bind VAO
	glBindVertexArray(gVaoSquare);

	// draw 6 faces for cube
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	
    // unbind VAO
	glBindVertexArray(0);

    glUseProgram(0);

    glXSwapBuffers(gpDisplay, gWindow);
}

void Update()
{
    if (!gbAnimate)
    {
        return;
    }

	gAngle = gAngle + 0.3f;

	if (gAngle >= 360.0f)
		gAngle = gAngle - 360.0f;
}

void Resize(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    gPerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void Uninitialize(void)
{
    GLXContext currentGLXContext;

    if (gVaoSquare)
    {
        glDeleteVertexArrays(1, &gVaoSquare);
        gVaoSquare = 0;
    }

    if (gVboSquarePosition)
    {
        glDeleteBuffers(1, &gVboSquarePosition);
        gVboSquarePosition = 0;
    }

    if (gVboSquareNormal)
    {
        glDeleteBuffers(1, &gVboSquareNormal);
        gVboSquareNormal = 0;
    }

    if (gShaderProgramObject && gVertexShaderObject)
    {
        glDetachShader(gShaderProgramObject, gVertexShaderObject);
    }

    if (gShaderProgramObject && gFragmentShaderObject)
    {
        glDetachShader(gShaderProgramObject, gFragmentShaderObject);
    }

    if (gVertexShaderObject)
    {
        glDeleteShader(gVertexShaderObject);
        gVertexShaderObject = 0;
    }

    if (gFragmentShaderObject)
    {
        glDeleteShader(gFragmentShaderObject);
        gFragmentShaderObject = 0;
    }

    if (gShaderProgramObject)
    {
        glDeleteProgram(gShaderProgramObject);
        gShaderProgramObject = 0;
        glUseProgram(0);
    }

    currentGLXContext = glXGetCurrentContext();

    if (currentGLXContext != NULL && currentGLXContext != gGLXContext)
    {
        glXMakeCurrent(gpDisplay, 0, 0);
    }

    if (gGLXContext)
    {
        glXDestroyContext(gpDisplay, gGLXContext);
    }

    if (gWindow)
    {
        XDestroyWindow(gpDisplay, gWindow);
    }

    if (gColorMap)
    {
        XFreeColormap(gpDisplay, gColorMap);
    }

    if (gpXVisualInfo)
    {
        free(gpXVisualInfo);
        gpXVisualInfo = NULL;
    }

    if (gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }

    if (gpLogFile)
    {
        fprintf(gpLogFile, "Log file is successfully closed\n");
        fclose(gpLogFile);
        gpLogFile = NULL;
    }
}
