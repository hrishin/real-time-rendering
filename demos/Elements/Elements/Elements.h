#ifndef _Elements_h
#define _Elements_h
#pragma once

typedef struct _position
{
	float topLeft;
	float topRight;
	float bottomRight;
	float bottomLeft;
} POSITION;

typedef struct _color
{
	float r;
	float g;
	float b;
} COLOR;

void drawMedicalIcon(POSITION position, float zIndex, COLOR color);
void drawDramaIcon(POSITION position, float zIndex, COLOR color);
void drawAstroIcon(POSITION position, float zIndex, COLOR color);
void drawTeacherIcon(POSITION position, float zIndex, COLOR color);
void drawWin32Icon(POSITION position, float zIndex, COLOR color);
void drawUnixIcon(POSITION position, float zIndex, COLOR color);
void drawAppleIcon(POSITION position, float zIndex, COLOR color);
void drawAppleCutProIcon(POSITION position, float zIndex, COLOR color);
void drawAndroidIcon(POSITION position, float zIndex, COLOR color);
void drawOpenGlIcon(POSITION position, float zIndex, COLOR color);

#endif // !_Elements_h