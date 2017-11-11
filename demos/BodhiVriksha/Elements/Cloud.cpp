#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>
#include <tchar.h>

#define PI 3.14f

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "GLU32.lib")

void DrawCircle(float radius)
{
	glBegin(GL_TRIANGLE_FAN);
	for (float i = 0.0f; i < 2 * PI; i += 0.001f)
	{
		glVertex3f(sin(i)*radius, cos(i)*radius, 0.0f);
	}
	glEnd();
}

void DrawCloudsType1(GLfloat x, GLfloat y, GLfloat z, GLfloat radius)
{
	glLoadIdentity();
	glTranslatef(x, y, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.5f, y - 0.2f, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.4f, y + 0.2f, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.6f, y, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.01f, y - 0.25f, z);
	DrawCircle(radius - 0.1f);
}

void DrawCloudsType2(GLfloat x, GLfloat y, GLfloat z, GLfloat radius)
{
	glLoadIdentity();
	glTranslatef(x, y, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.5f, y - 0.2f, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.4f, y + 0.2f, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.6f, y, z);
	DrawCircle(radius);
}


void DrawCloudsType3(GLfloat x, GLfloat y, GLfloat z, GLfloat radius)
{
	glLoadIdentity();
	glTranslatef(x, y, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.5f, y - 0.2f, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.4f, y + 0.2f, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x + 0.6f, y + 0.1f, z);
	DrawCircle(radius);

	glLoadIdentity();
	glTranslatef(x, y - 0.35f, z);
	DrawCircle(radius - 0.1f);

}

void DrawClouds()
{
	glPushMatrix();
	GLfloat radius = 0.5f;
	GLfloat initial_x = -3.5f, initial_y = 1.5f, initial_z = -6.0f;
	glColor3f(0.752941, 0.752941, 0.752941);
	DrawCloudsType1(initial_x, initial_y, initial_z, radius - 0.1f);
	DrawCloudsType2(initial_x + 2.5f, initial_y + 0.2f, initial_z, radius - 0.15f);
	DrawCloudsType2(initial_x + 4.5f, initial_y - 0.3f, initial_z, radius - 0.1f);
	DrawCloudsType3(initial_x + 6.5f, initial_y + 0.4f, initial_z, radius - 0.1f);
	glPopMatrix();
}