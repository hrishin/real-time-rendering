#include <GL/freeglut.h>

#define WIN_WIDTH 800
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
	glutCreateWindow("OpenGL GLUT: Horizontal Line");

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
	void drawHorisontalLine(void);
	void drawVarticalLine(void);
	void drawHMultipleLines(int num);
	void drawVMultipleLines(int num);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	drawHorisontalLine();
	drawVarticalLine();
	drawHMultipleLines(20);
	drawVMultipleLines(20);

	glFlush();
}

void drawHorisontalLine()
{
	glLineWidth(3);
	glBegin(GL_LINES);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-1.0f, 0.0f, 0.0f);
	glEnd();
}

void drawVarticalLine()
{
	glLineWidth(3);
	glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, -1.0f, 0.0f);
	glEnd();
}

void drawHMultipleLines(int num)
{
	int i;
	GLfloat yPosition;

	glLineWidth(1);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);
	yPosition = -0.05f;
	for (i = 0; i < 20; i++)
	{
		glVertex3f(-1.0f, yPosition, 0.0f);
		glVertex3f(1.0f, yPosition,0.0f);
		yPosition = yPosition - 0.05f;
	}
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f);
	yPosition = 0.05f;
	for (i = 0; i < 20; i++)
	{
		glVertex3f(1.0f, yPosition, 0.0f);
		glVertex3f(-1.0f, yPosition, 0.0f);
		yPosition = yPosition + 0.05f;
	}
	glEnd();
}

void drawVMultipleLines(int num)
{
	int i;
	GLfloat xPosition;

	glLineWidth(1);
	glBegin(GL_LINES);
		glColor3f(0.0f, 0.0f, 1.0f);
		xPosition = -0.05f;
		for (i = 0; i < num; i++)
		{
			glVertex3f(xPosition, -1.0f, 0.0f);
			glVertex3f(xPosition , 1.0f, 0.0f);
			xPosition = xPosition - 0.05f;
		}
	glEnd();

	glBegin(GL_LINES);
		glColor3f(0.0f, 1.0f, 0.0f);
		xPosition = 0.05f;
		for (i = 0; i < num; i++)
		{
			glVertex3f(xPosition, 1.0f, 0.0f);
			glVertex3f(xPosition , -1.0f, 0.0f);
			xPosition = xPosition + 0.05f;
		}
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