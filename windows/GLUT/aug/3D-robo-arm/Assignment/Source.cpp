#include <GL/freeglut.h>
#include <math.h>

#define PI 3.1415926535898
#define WIN_WIDTH 600
#define WIN_HEIGHT 600

bool gFulllScreen = false;
int shoulder = 0;
int elbow = 0;

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
	glutCreateWindow("OpenGL GLUT: Robo Arm");

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
	glTranslatef(0.0f, 0.0f, -5.0f);
	gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
	glPushMatrix();
	glTranslatef(-1.0f, 0.0f, 0.0f);
	glRotatef((GLfloat)shoulder, 0.0, 0.0, 1.0);
	glTranslatef(1.0, 0.0, 0.0);
	glPushMatrix();	
	glScalef(2.0, 0.4, 1.0);
	glutWireCube(1.0);
	glPopMatrix();

	glTranslatef(1.0f, 0.0f, 0.0f);
	glRotatef((GLfloat) elbow, 0.0f, 0.0f, 1.0f);
	glTranslatef(1.0f, 0.0f, 0.0f);
	glPushMatrix();
	glScalef(2.0f, 0.4f, 1.0f);
	glutWireCube(1.0);
	glPopMatrix();

	glPopMatrix();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{

	case 's':
		shoulder = (shoulder + 5) % 360;
		glutPostRedisplay();
		break;

	case 'S':
		shoulder = (shoulder - 5) % 360;
		glutPostRedisplay();
		break;

	case 'e':
		elbow = (elbow + 5) % 360;
		glutPostRedisplay();
		break;

	case 'E':
		elbow = (elbow - 5) % 360;
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