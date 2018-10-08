#include <GL/freeglut.h>

#define WIN_WIDTH 600
#define WIN_HEIGHT 600

bool gFulllScreen = false;

int main(int argc, char** argv)
{
	int iScreenWidth, iScreenHeight;

	// function prototypes
	void display(void);
	void resize(int, int);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);
	void initialize(void);
	void uninitilize(void);

	//code
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

	iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition((iScreenWidth/2) - (WIN_WIDTH/2),
							(iScreenHeight/2) - (WIN_HEIGHT/2));
	glutCreateWindow("OpenGL GLUT: Concentric Traiangles");

	initialize();

	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(uninitilize);

	glutMainLoop();

	// return (0);
}


void initialize(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void display(void)
{
	void renderConcentricTraiangles(void);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	renderConcentricTraiangles();

	glFlush();
}

void renderConcentricTraiangles(void)
{
	void drawTraingle(GLfloat, GLfloat[]);
	int i;
	GLfloat tringleColors[10][3] = { 
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 1.0f },
		{ 0.2f, 0.8f, 0.8f },
		{ 0.5f, 0.0f, 0.5f },
		{ 0.5f, 0.5f, 0.5f },
		{ 1.0f, 0.5f, 0.2f },
	};

	for (i = 1; i <= 10; i++)
	{
		drawTraingle(i*0.1f, tringleColors[(i - 1)]);
	}
}

void drawTraingle(GLfloat vertexPos, GLfloat colorValue[])
{
	glLineWidth(2);
	glBegin(GL_LINES);
		glColor3f(colorValue[0], colorValue[1], colorValue[2]);

		glVertex3f(0.0f, vertexPos, 0.0f);
		glVertex3f(-vertexPos, -vertexPos, 0.0f);

		glVertex3f(-vertexPos, -vertexPos, 0.0f);
		glVertex3f(vertexPos, -vertexPos, 0.0f);

		glVertex3f(vertexPos, -vertexPos, 0.0f);
		glVertex3f(0.0f, vertexPos, 0.0f);
	glEnd();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:// escape
	case 'Q':
	case 'q':
		glutLeaveMainLoop();
		break;

	case 'F':
	case 'f':
		if (gFulllScreen == false)
		{
			glutFullScreen();
		}
		else 
		{
			glutLeaveFullScreen();
		}
		gFulllScreen = !(gFulllScreen);
		break;

	default:
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		break;
	default:
		break;
	}
}


void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}


void uninitilize()
{

}