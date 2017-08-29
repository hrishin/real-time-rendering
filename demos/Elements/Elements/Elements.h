#ifndef _Elements_h
#define _Elements_h
#pragma once


struct _position 
{
	float topLeft;
	float topRight;
	float bottomRight;
	float bottomLeft;
};
typedef _position Position;

struct _color 
{
	float r;
	float g;
	float b;
};
typedef _color Color;

void drawMedicalIcon(Position position, float zIndex, Color color);
void drawDramaIcon(Position position, float zIndex, Color color);
void drawPratibimbIcon(Position position, float zIndex, Color color);
void drawAstroIcon(Position position, float zIndex, Color color);
void drawTeacherIcon(Position position, float zIndex, Color color);
void drawWin32Icon(Position position, float zIndex, Color color);
void drawUnixIcon(Position position, float zIndex, Color color);
void drawAppleIcon(Position position, float zIndex, Color color);
void drawAppleCutProIcon(Position position, float zIndex, Color color);
void drawAndroidIcon(Position position, float zIndex, Color color);
void drawOpenGlIcon(Position position, float zIndex, Color color);

#endif // !_Elements_h