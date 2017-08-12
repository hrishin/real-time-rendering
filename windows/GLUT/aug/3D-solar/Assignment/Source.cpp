#include <GL/freeglut.h>
#include <math.h>

#define PI 3.1415926535898
#define WIN_WIDTH 600
#define WIN_HEIGHT 600

bool gFulllScreen = false;
int year = 1;
int day = 1;

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

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition((iScreenWidth/2) - (WIN_WIDTH/2),
							(iScreenHeight/2) - (WIN_HEIGHT/2));
	glutCreateWindow("OpenGL GLUT: Solar System");

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
	glPushMatrix();
	
	glutWireSphere(1.0, 20, 16);
	glRotatef((GLfloat) year, 0.0, 1.0, 0.0);
	glTranslatef(2.0, 0.0, 0.0);
	glRotatef((GLfloat) day, 0.0, 1.0, 0.0);
	glutWireSphere(0.2, 10, 8);
	glPopMatrix();

	//glFlush();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{

	case 'd':
		day = (day +10) % 360;
		glutPostRedisplay();
		break;
	case 'D':
		day = (day -10) % 360;
		glutPostRedisplay();
		break;
	case 'y':
		year = (year +5) % 360;
		glutPostRedisplay();
		break;
	case 'Y':
		year = (year -5) % 360;
		glutPostRedisplay();
		break;
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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat) width/ (GLfloat) height, 1.0, 20.0);
}


void uninitilize()
{

}