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

#include "Sphere.h"
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
const char *gpWindowTitle = "Programmable Pipeline : Light per vertex (v KEY) and per fragment (f KEY) sphere";

GLXContext gGLXContext;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gNumElements;
GLuint gNumVertices;
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

GLuint gVaoSphere;
GLuint gVboSpherePosition;
GLuint gVboShereNormal;
GLuint gVboSphereElements;

GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
GLuint gLaUniform, gLdUniform, gLsUniform, gLightPositionUniform;
GLuint gKaUniform, gKdUniform, gKsUniform, gMaterialShinessUniform;
GLuint gLKeyPressedUniform;
GLuint gIsPerVertexUniform;

mat4 gPerspectiveProjectionMatrix;

GLfloat gLightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat gLightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gLightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gLightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };

GLfloat gMaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gMaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gMaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gMaterialShininess = 50.0f;

bool gbLight;
bool gIsPerVertex;

FILE *gpLogFile;

bool gbFullScreen = false;


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
                case XK_Q:
                case XK_q:
                    bDone = true;
                    break;

                case XK_Escape:
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

                case XK_F:
                case XK_f:
                    gIsPerVertex = false;
                    break;

                case XK_V:
                case XK_v:
                    gIsPerVertex = true;
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

	// source
	const GLchar *vertexShaderSourceCode =
		"#version 130" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform int u_lighting_enabled;" \
		"uniform int u_per_vertex;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec4 u_light_position;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"out vec3 phong_ads_color_out;" \
		"out vec3 trasnformed_normals_out;" \
		"out vec3 light_direction_out;" \
		"out vec3 viewer_vector_out;" \
		"void main(void)" \
		"{" \
			"if (u_lighting_enabled == 1)" \
			"{" \
				"if (u_per_vertex == 1)" \
				"{" \
					"vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;"\
					"vec3 trasnformed_normals = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" \
					"vec3 light_direction = normalize(vec3(u_light_position) - eye_coordinates.xyz);" \
					"float tn_dot_ld = max(dot(trasnformed_normals, light_direction), 0.0);" \
					"vec3 ambient = u_La * u_Ka;" \
					"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
					"vec3 reflection_vector = reflect(-light_direction, trasnformed_normals);" \
					"vec3 viewer_vector = normalize(-eye_coordinates.xyz);" \
					"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, viewer_vector) , 0.0) , u_material_shininess);" \
					"phong_ads_color_out = ambient + diffuse + specular;" \
				"}" \
				"else" \
				"{" \
					"vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;"\
					"trasnformed_normals_out = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
					"light_direction_out = vec3(u_light_position) - eye_coordinates.xyz;" \
					"viewer_vector_out = -eye_coordinates.xyz;" \
				"}" \
			"}" \
			"else" \
			"{" \
				"phong_ads_color_out = vec3(1.0, 1.0, 1.0);" \
			"}" \
			"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"}";
    glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);
    glCompileShader(gVertexShaderObject);
    LogShaderCompilationStatus(gVertexShaderObject, gpLogFile, "Vertex Shader Compilation Log");

    // FRAGMENT SHADER
    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fragmentShaderSourceCode =
		"#version 130" \
		"\n" \
		"in vec3 phong_ads_color_out;" \
		"in vec3 trasnformed_normals_out;" \
		"in vec3 light_direction_out;" \
		"in vec3 viewer_vector_out;" \
		"uniform int u_per_vertex;" \
		"uniform int u_lighting_enabled;" \
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
			"if (u_per_vertex == 1)" \
			"{" \
				"FragColor = vec4(phong_ads_color_out, 1.0);" \
			"}" \
			"else" \
			"{" \
				"vec3 phong_ads_color;" \
				"if (u_lighting_enabled == 1)" \
				"{" \
					"vec3 normalized_trasnformed_normals = normalize(trasnformed_normals_out);" \
					"vec3 normalized_light_direction = normalize(light_direction_out);" \
					"float tn_dot_ld = max(dot(normalized_trasnformed_normals, normalized_light_direction), 0.0);" \
					"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_trasnformed_normals);" \
					"vec3 normalized_viewer_vector = normalize(viewer_vector_out);" \
					"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector) , 0.0) , u_material_shininess);" \
					"vec3 ambient = u_La * u_Ka;" \
					"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
					"phong_ads_color = ambient + diffuse + specular;" \
				"}" \
				"else" \
				"{" \
					"phong_ads_color = vec3(1.0, 1.0, 1.0);" \
				"}" \
				"FragColor = vec4(phong_ads_color, 1.0);" \
			"}" \
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
	gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_lighting_enabled");
	gIsPerVertexUniform = glGetUniformLocation(gShaderProgramObject, "u_per_vertex");

	// ambient color of light
	gLaUniform = glGetUniformLocation(gShaderProgramObject, "u_La");
	// diffuse color of light
	gLdUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	// specular color of light
	gLsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
	// light position
	gLightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

	// ambient color of material
	gKaUniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	// diffuse color of material
	gKdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	// specular color of light
	gKsUniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	// shininess of material
	gMaterialShinessUniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");

	// get sphere vertices, normals, textures, elements
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	// Sphere VAO
	glGenVertexArrays(1, &gVaoSphere);
	glBindVertexArray(gVaoSphere);

	glGenBuffers(1, &gVboSpherePosition);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSpherePosition);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Normals
	glGenBuffers(1, &gVboShereNormal);
	glBindBuffer(GL_ARRAY_BUFFER, gVboShereNormal);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Elements
	glGenBuffers(1, &gVboSphereElements);
	glBindBuffer(GL_ARRAY_BUFFER, gVboSphereElements);
	// move data from main memory to graphics memory
	// GL_STATIC_DRAW or GL_DYNAMIC_DRAW : How you want load data run or preloading. example game shows loadingbar and "loading" messages
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

    gbLight = false;
    gIsPerVertex = true;

    glShadeModel(GL_SMOOTH);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    // perspective matrix to identity matrix
    gPerspectiveProjectionMatrix = mat4::identity();

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

	// start using OpenGL program object
	glUseProgram(gShaderProgramObject);

	if (gIsPerVertex)
	{
		glUniform1i(gIsPerVertexUniform, 1);
	}
	else 
	{
		glUniform1i(gIsPerVertexUniform, 0);
	}

	if (gbLight)
	{
		// setting light enabled uniform
		glUniform1i(gLKeyPressedUniform, 1);

		// setting light properties uniform
		glUniform3fv(gLaUniform, 1, gLightAmbient);
		glUniform3fv(gLdUniform, 1, gLightDiffuse);
		glUniform3fv(gLsUniform, 1, gLightSpecular);
		glUniform4fv(gLightPositionUniform, 1, gLightPosition);

		glUniform3fv(gKaUniform, 1, gMaterialAmbient);
		glUniform3fv(gKdUniform, 1, gMaterialDiffuse);
		glUniform3fv(gKsUniform, 1, gMaterialSpecular);
		glUniform1f(gMaterialShinessUniform, gMaterialShininess);
	}
	else 
	{
		glUniform1i(gLKeyPressedUniform, 0);
	}

	// set all matrices to identity
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	// translate z axis
	modelMatrix = translate(0.0f, 0.0f, -2.0f);

	// pass the modelMatrix to vertex shader variable
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);

	// pass the viewMatrix to vertex shader variable
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);

	// pass the projectionlViewMatrix to vertex shader variable
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	// bind VAO
	glBindVertexArray(gVaoSphere);

	// draw sphere
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVboSphereElements);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// unbind VAO
	glBindVertexArray(0);

	// stop using OpenGL program object
	glUseProgram(0);

    glXSwapBuffers(gpDisplay, gWindow);
}

void Update()
{

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

    // destroy vao
	if (gVaoSphere)
	{
		glDeleteVertexArrays(1, &gVaoSphere);
		gVaoSphere = 0;
	}

	// destroy vbo
	if (gVboSpherePosition)
	{
		glDeleteBuffers(1, &gVboSpherePosition);
		gVboSpherePosition = 0;
	}
	if (gVboShereNormal)
	{
		glDeleteBuffers(1, &gVboShereNormal);
		gVboShereNormal = 0;
	}
	if (gVboSphereElements)
	{
		glDeleteBuffers(1, &gVboSphereElements);
		gVboSphereElements = 0;
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
