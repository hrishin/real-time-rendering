#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>


#include "Elements.h"
#include "DrawChars.h"

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "GLU32.lib")

static GLfloat DEG2RAD = 3.14159 / 180;

DrawChars drawChar;

void drawMedicalIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.0;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b};
	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };
	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);
	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "Medical Science";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawDramaIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.0;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };
	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };
	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);
	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "Drama Theater";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawTeacherIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.0;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };
	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };
	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);
	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "Teacher";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawAstroIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.0;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };
	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };

	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);

	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "Astrology";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawAppleIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.5;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };
	
	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;
	
	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };
	
	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);

	scaleValueVec[0] = scaleVec1;
	
	vector<string>  textToDraw;
	std::string currentChars = "Apple Cut Pro";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawWin32Icon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.5;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };

	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };

	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);

	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "Win 32, COM, .NET";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawUnixIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 3.0;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };

	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };

	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);

	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "UNIX";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawAndroidIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.5;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };

	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };

	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);

	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "android";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawOpenGlIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 1.0f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.0;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };

	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };

	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);

	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "OpenGL";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}

void drawiOSIcon(POSITION position, COLOR color)
{
	vector<double> wordSpacVec;
	wordSpacVec.resize(10);
	wordSpacVec[0] = 0.0f;

	vector<double> charSpacVec;
	charSpacVec.resize(10);
	charSpacVec[0] = 0.8f;

	vector<double> charWidth;
	charWidth.resize(10);
	charWidth[0] = 1.0;

	vector<double> traslateXParam;
	traslateXParam.resize(10);
	traslateXParam[0] = position.x;

	vector<double> traslateYParam;
	traslateYParam.resize(10);
	traslateYParam[0] = position.y;

	vector<double> traslateZParam;
	traslateZParam.resize(10);
	traslateZParam[0] = position.z;

	vector<double> rgbVec1{ color.r, color.g, color.b };

	vector<vector<double>>rgbValueVec;
	rgbValueVec.resize(10);
	rgbValueVec[0] = rgbVec1;

	vector<double> scaleVec1{ 0.1f, 0.1f, 1.0f };

	vector<vector<double>>scaleValueVec;
	scaleValueVec.resize(10);

	scaleValueVec[0] = scaleVec1;

	vector<string>  textToDraw;
	std::string currentChars = "iOS";
	textToDraw.push_back(currentChars);

	drawChar.initDrawChar(wordSpacVec, charSpacVec, charWidth, traslateXParam, traslateYParam,
		traslateZParam, rgbValueVec, scaleValueVec, textToDraw);

	drawChar.setWordToDisplayOnScreen();
}