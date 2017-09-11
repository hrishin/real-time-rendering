#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#include "Elements.h"

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "GLU32.lib")

void drawDramaIcon(POSITION position, float zIndex, COLOR color)
{
	int i;
	GLfloat DEG2RAD = 3.14159 / 180;
	
	// sad face
	glPushMatrix();
		glTranslatef(20.0f, 0.0, 0.0);
		//glRotatef(-15.0f, 0.0f, 0.0f, 1.0f);

		// face
		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.9f, 0.9f);
		for (i = 161; i < 380; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 47.0, sin(rad) * 95.0);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i = 379; i > 160; i--)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 65.0, (sin(rad) * 95.0) + 85);
		}
		glEnd();

		// smile
		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i = 0; i < 180; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 20.0, (sin(rad) * 25.0) - 80.0);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.9f, 0.9f);
		for (i = 0; i < 181; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 20.0, (sin(rad) * 15.0) - 82.0);
		}
		glEnd();

		// eyes
		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 40; i < 201; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f((cos(rad) * 10.0) - 20.0, (sin(rad) * 12.0) - 35.0);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 161; i < 320; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(-(cos(rad) * 10.0) + 20.0, -(sin(rad) * 12.0) - 35.0);
		}
		glEnd();

	glPopMatrix();


	// happy face
	glPushMatrix();

		glTranslatef(-20.0, 0.0, 0.0);
		//glRotatef(5.0f, 0.0f, 0.0f, 1.0f);
	
		// face
		glBegin(GL_POLYGON);
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 161; i < 380; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 47.0, sin(rad) * 95.0);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i = 379; i > 160; i--)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 65.0, (sin(rad) * 95.0) + 85);
		}
		glEnd();

		// smile
		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i = 181; i < 360; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 25.0, (sin(rad) * 25.0) - 60.0);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 181; i < 360; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 32.0, (sin(rad) * 18.0) - 50.0);
		}
		glEnd();

		// eyes
		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 161; i < 320; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(-(cos(rad) * 10.0) - 20.0, -(sin(rad) * 12.0) - 35.0);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 40; i < 201; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f((cos(rad) * 10.0) + 20.0, (sin(rad) * 12.0) - 35.0);
		}
		glEnd();

	glPopMatrix();
}