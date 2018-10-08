#include <GL/freeglut.h>
#include <math.h>

#define PI 3.1415926535898
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
	glutCreateWindow("OpenGL GLUT: Deathly Hollow");

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
	void renderSymbol(void);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	renderSymbol();

	glFlush();
}

void renderSymbol(void)
{
	void drawTraingle();
	void drawCircle();
	void drawLine();

	drawTraingle();
	drawCircle();
	drawLine();
}

void drawTraingle()
{
	glLineWidth(1);
	glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);

		glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glVertex3f(1.0f, -1.0f, 0.0f);
		
		glVertex3f(1.0f, -1.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
	glEnd();	
}

void drawCircle()
{
	GLfloat getDistance(GLfloat, GLfloat, GLfloat, GLfloat);
	GLfloat angle;
	GLfloat sideA, sideB, sideC, centerX, centerY, p, s, radius;
	int i;
	static GLint circle_points;

	sideA = getDistance(0.0f, 1.0f, 1.0f, -1.0f);
	sideB = getDistance(0.0f, 1.0f, -1.0f, -1.0f);
	sideC = getDistance(-1.0f, -1.0f, 1.0f, -1.0f);
	p = sideA + sideB + sideC;
	centerX = ((sideA * (-1.0f)) + (sideB * 1.0f) + (sideC * 0.0f)) / p;
	centerY = ((sideA * (-1.0f)) + (sideB * (-1.0f)) + (sideC * 1.0f)) / p;
	s = p / 2;
	radius = sqrtf(s * (s - sideA) * (s - sideB) * (s - sideC)) / s;

	glLineWidth(1);
	circle_points = 1000; 
	glBegin(GL_POINTS); 
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 0; i < circle_points; i++)
		{
			angle = 2 * PI * i / circle_points;
			glVertex3f(radius * cos(angle) + centerX, radius * sin(angle) + centerY, 1.0f);
		}
	glEnd();
}

GLfloat getDistance(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	GLfloat x = pow(x2 - x1, 2);
	GLfloat y = pow(y2 - y1, 2);
	return(sqrtf( x + y));
}

void drawLine()
{
	glLineWidth(1);
	glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, -1.0f, 0.0f);
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