#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#include "Elements.h"

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "GLU32.lib")

static GLfloat DEG2RAD = 3.14159 / 180;

void drawMedicalIcon(POSITION position, float zIndex, COLOR color)
{
	int i;
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(9.0f);

	// draw circle
	glBegin(GL_POLYGON);
	glColor3f(1.0f, 0.0, 0.0);
	for (i = 0; i < 360; i++)
	{
		GLfloat rad = i * DEG2RAD;
		glVertex3f(cos(rad) * 0.9f, sin(rad) * 0.9f, 0.0);
	}
	glEnd();

	// draw plus sign
	glLineWidth(12.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.7f, 0.0f);
	glVertex3f(0.0, -0.7f, 0.0f);
	glVertex3f(0.7f, 0.0, 0.0f);
	glVertex3f(-0.7f, 0.0, 0.0f);
	glEnd();

}

void drawDramaIcon(POSITION position, float zIndex, COLOR color)
{
	int i;

	// sad face
	glPushMatrix();
		glTranslatef(0.40f, 0.0, 0.0);
		glRotatef(-15.0f, 0.0f, 0.0f, 1.0f);

		// face
		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.9f, 0.9f);
		for (i = 161; i < 380; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 0.47, sin(rad) * 0.95);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i = 379; i > 160; i--)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 0.65, (sin(rad) * 0.95) + 0.85);
		}
		glEnd();

		// smile
		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i = 0; i < 180; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 0.20, (sin(rad) * 0.25) - 0.80);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.9f, 0.9f);
		for (i = 0; i < 181; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 0.20, (sin(rad) * 0.15) - 0.82);
		}
		glEnd();

		// eyes
		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 40; i < 201; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f((cos(rad) * 0.10) - 0.20, (sin(rad) * 0.12) - 0.35);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 161; i < 320; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(-(cos(rad) * 0.10) + 0.20, -(sin(rad) * 0.12) - 0.35);
		}
		glEnd();

	glPopMatrix();


	// happy face
	glPushMatrix();

		glTranslatef(-0.40, 0.0, 0.0);
		glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
	
		// face
		glBegin(GL_POLYGON);
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 161; i < 380; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 0.47, sin(rad) * 0.95);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i = 379; i > 160; i--)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 0.65, (sin(rad) * 0.95) + 0.85);
		}
		glEnd();

		// smile
		glBegin(GL_POLYGON);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i = 181; i < 360; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 0.25, (sin(rad) * 0.25) - 0.60);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(1.0f, 1.0f, 1.0f);
		for (i = 181; i < 360; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(cos(rad) * 0.32, (sin(rad) * 0.18) - 0.50);
		}
		glEnd();

		// eyes
		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 161; i < 320; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f(-(cos(rad) * 0.10) - 0.20, -(sin(rad) * 0.12) - 0.35);
		}
		glEnd();

		glBegin(GL_POLYGON);
		glColor3f(1.0f, 0.0f, 0.0f);
		for (i = 40; i < 201; i++)
		{
			GLfloat rad = i * DEG2RAD;
			glVertex2f((cos(rad) * 0.10) + 0.20, (sin(rad) * 0.12) - 0.35);
		}
		glEnd();

	glPopMatrix();
}

void drawAstroIcon(POSITION position, float zIndex, COLOR color)
{
	int i;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(9.0f);

	// draw circle
	glBegin(GL_POLYGON);
	glColor3f(1.0f, 0.0, 0.0);
	for (i = 0; i < 360; i++)
	{
		GLfloat rad = i * DEG2RAD;
		glVertex3f(cos(rad) * 0.8f, sin(rad) * 0.8f, 1.0);
	}
	glEnd();

	// draw inner circle
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
	glColor3f(0.0f, 0.0, 0.0);
	for (i = 0; i < 360; i++)
	{
		GLfloat rad = i * DEG2RAD;
		glVertex3f((cos(rad) * 0.2f) - 0.3f, (sin(rad) * 0.2f) + 0.3f, 1.0);
	}
	glEnd();

	// draw outline
	glRotatef(-60.0f, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_POLYGON);
	glColor3f(1.0f, 0.0, 0.0);
	for (i = 0; i < 360; i++)
	{
		GLfloat rad = i * DEG2RAD;
		glVertex3f(cos(rad) * 0.3f, sin(rad) * 1.9f, 0.0);
	}
	glEnd();
}

void drawAppleIcon(POSITION position, float zIndex, COLOR color)
{
	int i;
	
	glLineWidth(9.0f);

	// draw circle
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
	glColor3f(1.0f, 0.0, 0.0);
	for (i = 0; i < 360; i++)
	{
		GLfloat rad = i * DEG2RAD;
		glVertex3f(cos(rad) * 0.5, sin(rad) * 0.5, 1.0);
	}
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1.0f, 0.0, 0.0);
	for (i = 0; i < 360; i++)
	{
		GLfloat rad = i * DEG2RAD;
		glVertex3f((cos(rad) * 0.4) - 0.3, (sin(rad) * 0.3) + 0.25, 1.0);
	}
	glEnd();

	glRotatef(23.0f, 0.0, 0.0, 1.0);
	glBegin(GL_POLYGON);
	glColor3f(1.0f, 0.0, 0.0);
	for (i = 0; i < 360; i++)
	{
		GLfloat rad = i * DEG2RAD;
		glVertex3f((cos(rad) * 0.3) - 0.4, (sin(rad) * 0.5) + 0.12, 1.0);
	}
	glEnd();

	

}