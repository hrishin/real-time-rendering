#ifndef _Elements_h
#define _Elements_h
#pragma once

typedef struct _position
{
	float x;
	float y;
	float z;
} POSITION;

typedef struct _color
{
	float r;
	float g;
	float b;
} COLOR;

void drawMedicalIcon(POSITION position, COLOR color);
void drawDramaIcon(POSITION position, COLOR color);
void drawAstroIcon(POSITION position, COLOR color);
void drawTeacherIcon(POSITION position, COLOR color);
void drawWin32Icon(POSITION position, COLOR color);
void drawUnixIcon(POSITION position, COLOR color);
void drawAppleIcon(POSITION position, COLOR color);
void drawiOSIcon(POSITION position, COLOR color);
void drawAndroidIcon(POSITION position, COLOR color);
void drawOpenGlIcon(POSITION position, COLOR color);

#endif // !_Elements_h