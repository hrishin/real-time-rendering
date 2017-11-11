#ifndef DRAWCHARS_H
#define DRAWCHARS_H
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include<math.h>

#include <string>
#include <iostream> 
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>

#include<vector>
#include <map>
#include <algorithm> 

using namespace std;


class DrawChars
{

private:
     int                     mNumOfLines;
    vector<double>           mWordSpacing;
    vector<double>           mCharSpacing;
    vector<double>           mCharWdth;

    vector<double>          mTraslateXParam;
    vector<double>          mTraslateYParam;
    vector<double>          mTraslateZParam;
	vector<vector<double>>   mScaleVect;

    vector<vector<double>>   mRgbValueVec;
    vector< vector<string> > mStrVecDisplayContent;
    map < string, vector<vector<vector<int>>>> mCharPtsMap;
    GLfloat                  ***mGridPts;
	GLfloat                  ***mNewGridPts;



    int nx = 16, ny = 16;
    GLfloat ***Pt;
    GLfloat min_Xdim = -2.0, max_Xdim = 2.0f;
    GLfloat min_Ydim = -2.0, max_Ydim = 2.0f;
    GLfloat xdim_inc = 0.25, ydim_inc = 0.25;

	int nxNew = 44, nyNew = 76;


public:
  
    DrawChars();
    void initDrawChar( vector<double> wordSpacing, vector<double> charSpacing,
                       vector<double> charWdth,
                       vector<double>traslateXParam,
                       vector<double>traslateYParam,
                       vector<double> traslateZParam, 
                       vector<vector<double>> rgbValueVec,
		               vector<vector<double>> scaleValueVec,
                       vector<string> strDisplayContent);

    void readFontData();
    

    void generatePoints(int nx, int ny,
        GLfloat min_Xdim, GLfloat max_Xdim,
        GLfloat min_Ydim, GLfloat max_Ydim,
        GLfloat xdim_inc, GLfloat ydim_inc);

	void generatePointsNew(int nx, int ny,
		GLfloat min_Xdim, GLfloat max_Xdim,
		GLfloat min_Ydim, GLfloat max_Ydim,
		GLfloat xdim_inc, GLfloat ydim_inc);


    void setWordToDisplayOnScreen();
    void drawCharOnScreen(char displayChar);

    ~DrawChars();
};

#endif
