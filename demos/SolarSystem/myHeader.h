#pragma once
#include		<Windows.h>
#include		<gl/GL.h>
#include		<gl/GLU.h>
#include		"Log\cLog.h"

#pragma			comment			(lib, "opengl32.lib")
#pragma			comment			(lib, "glu32.lib")

#define			WIN_WIDTH 800
#define			WIN_HEIGHT 600
#define			WIN_X_POSITION 100
#define			WIN_Y_POSITION 100

#define			BLACK_COLOR 0.0f, 0.0f, 0.0f

#define			RED_COLOR 1.0f, 0.0f, 0.0f
#define			GREEN_COLOR 0.0f, 0.5f, 0.0f
#define			BLUE_COLOR 0.0f, 0.0f, 1.0f
#define			CYAN_COLOR 0.0f, 1.0f, 1.0f
#define			MAGENTA_COLOR 1.0f, 0.0f, 1.0f
#define			YELLOW_COLOR 1.0f, 1.0f, 0.0f
#define			WHITE_COLOR 1.0f, 1.0f, 1.0f
#define			ORANGE_COLOR 1.0f, 0.5f, 0.0f
#define			GRAY_COLOR 0.5f, 0.5f, 0.5f
#define			BROWN_COLOR 0.35f, 0.16f, 0.14f
#define			CORN_FLOWER_COLOR 0.39f, 0.58f, 0.93f
enum			colors { RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, WHITE, ORANGE, GRAY, BROWN };

LRESULT			CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void			doResizing(int, int);
void			doToggleFullScreen(void);
void			update();
void			display();
