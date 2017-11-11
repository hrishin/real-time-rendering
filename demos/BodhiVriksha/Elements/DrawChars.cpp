#include <iostream>
#include <iomanip>
#include "DrawChars.h"

using namespace std;


DrawChars:: DrawChars()
{
    mNumOfLines = 1;
    mWordSpacing.resize(1);
    mCharSpacing.resize(1);
    mStrVecDisplayContent.resize(1);
    mStrVecDisplayContent[0].resize(1);
    mStrVecDisplayContent[0][0] = "  ";

    mGridPts = new GLfloat**[nx];
    for (int i = 0; i < nx; i++)
    {
        mGridPts[i] = new GLfloat*[ny];
        for (int j = 0; j < ny; j++)
        {
            mGridPts[i][j] = new GLfloat[3];
        }
    }

	mNewGridPts = new GLfloat**[nxNew];
	for (int i = 0; i < nxNew; i++)
	{
		mNewGridPts[i] = new GLfloat*[nyNew];
		for (int j = 0; j < nyNew; j++)
		{
			mNewGridPts[i][j] = new GLfloat[3];
		}
	}


    generatePoints(nx, ny, min_Xdim, max_Xdim,
        min_Ydim, max_Ydim, xdim_inc,
        ydim_inc);
	
	generatePointsNew(nxNew, nyNew, min_Xdim, max_Xdim,
		min_Ydim, max_Ydim, xdim_inc,
		ydim_inc);

}


DrawChars:: ~DrawChars()
{
    for (int i = 0; i < nx; i++)
    {
        for (int j = 0; j < ny; j++)
        {
            delete[] mGridPts[i][j];
        }
        delete[] mGridPts[i];
    }
    delete[] mGridPts;
}



void DrawChars :: generatePoints(int nx, int ny,
    GLfloat min_Xdim, GLfloat max_Xdim,
    GLfloat min_Ydim, GLfloat max_Ydim,
    GLfloat xdim_inc, GLfloat ydim_inc)
{

    int nxdim = nx, nydim = ny;
    GLfloat minx = min_Xdim, miny = min_Ydim;
    GLfloat	maxx = max_Xdim, maxy = max_Ydim;
    GLfloat minz = 0, maxz = 0;
    GLfloat xinc = xdim_inc, yinc = ydim_inc, zinc = 0;
    int index = 0;
    GLfloat y = miny, x = minx, z = minz;

    for (int j = 0; j < nydim; j++)
    {
        x = minx;
        for (int i = 0; i < nxdim; i++)
        {
            //index = i + (nxdim)*j;
            mGridPts[i][j][0] = x;
            mGridPts[i][j][1] = y;
            mGridPts[i][j][2] = z;
            x = x + xdim_inc;
        }
        y = y + ydim_inc;
    }
}

void DrawChars::generatePointsNew(int nx, int ny,
	GLfloat min_Xdim, GLfloat max_Xdim,
	GLfloat min_Ydim, GLfloat max_Ydim,
	GLfloat xdim_inc, GLfloat ydim_inc)
{

	int nxdim = nx, nydim = ny;
	GLfloat minx = min_Xdim, miny = min_Ydim;
	GLfloat	maxx = max_Xdim, maxy = max_Ydim;
	GLfloat minz = 0, maxz = 0;
	GLfloat xinc = xdim_inc, yinc = ydim_inc, zinc = 0;
	int index = 0;
	GLfloat y = miny, x = minx, z = minz;

	for (int j = 0; j < nydim; j++)
	{
		x = minx;
		for (int i = 0; i < nxdim; i++)
		{
			//index = i + (nxdim)*j;
			mNewGridPts[i][j][0] = x;
			mNewGridPts[i][j][1] = y;
			mNewGridPts[i][j][2] = z;
			x = x + xdim_inc;
		}
		y = y + ydim_inc;
	}
}

void DrawChars:: readFontData()
{
     char filename[] = "./NewFont.txt";
    ifstream inputfile;
    int last_position = 0, filesize = 0;
    inputfile.open(filename, ios::in);
    if (inputfile.fail())
    {
        cerr << "1. The file does not exist.\n";
        exit(1);
    }
    else
    {
        string input;
        inputfile.seekg(0, ios::beg);
        vector<vector<vector<int> >> charPtsVec; //store all m and d values
        vector<vector<int>> mdVector; //will store m and d values in 1 iteration 
        string keyChar;//A,B,C etc
        stringstream tokenizer;

        while (!inputfile.eof())
        {

            std::getline(inputfile, input);
            if (input.find("\"") != string::npos)
            {
                if (!mdVector.empty()) //for final m and d  to add .
                {
                    charPtsVec.push_back(mdVector);
                    mdVector.clear();
                }
                if (!charPtsVec.empty())
                {
                    mCharPtsMap[keyChar] = charPtsVec;
                    charPtsVec.clear();
                    mdVector.clear();
                }
                input.erase(input.begin() + input.find("\""));
                input.erase(input.begin() + input.find("\""));
                keyChar = input;
                cout << keyChar << endl;

                if (keyChar == "d" || keyChar == "m")
                {
                    continue;
                }
            }
            if (input.find("m") != string::npos)
            {
                if (!mdVector.empty()) //when more than one "m" found.
                {
                    charPtsVec.push_back(mdVector);
                    mdVector.clear();
                }
                input.erase(input.begin() + input.find("m"));
                input.erase(input.begin() + input.find(","));
                tokenizer.clear();
                tokenizer.str(input);
                vector<int> tempVect;
                tempVect.resize(2);
                tokenizer >> tempVect[0]; tokenizer >> tempVect[1];
                mdVector.push_back(tempVect);
            }
            if (input.find("d") != string::npos)
            {
                input.erase(input.begin() + input.find("d"));
                input.erase(input.begin() + input.find(","));
                tokenizer.clear();
                tokenizer.str(input);
                vector<int> tempVect;
                tempVect.resize(2);
                tokenizer >> tempVect[0]; tokenizer >> tempVect[1];
                mdVector.push_back(tempVect);
            }

        }//end of file loop

        if (!mdVector.empty()) //for final char.
        {
            charPtsVec.push_back(mdVector);
            mdVector.clear();
        }
        if (!charPtsVec.empty())
        {
            mCharPtsMap[keyChar] = charPtsVec;
            charPtsVec.clear();
            mdVector.clear();
        }
    }
}




void DrawChars::initDrawChar( vector<double> wordSpacing, vector<double> charSpacing, vector<double> charWdth, 
                              vector<double>traslateXParam, vector<double>traslateYParam, vector<double> traslateZParam,
                              vector<vector<double>> rgbValueVec, vector<vector<double>> scaleVect, vector<string> strDisplayContent)
{
    mWordSpacing = wordSpacing;
    mCharSpacing = charSpacing;
    mNumOfLines  = strDisplayContent.size();
    mCharWdth    = charWdth;
    mRgbValueVec = rgbValueVec;

    mTraslateXParam = traslateXParam;
    mTraslateYParam = traslateYParam;
    mTraslateZParam = traslateZParam;
	mScaleVect = scaleVect;

    mStrVecDisplayContent.clear();
    mStrVecDisplayContent.resize(mNumOfLines);

    for (int i = 0; i < mNumOfLines; i++)
    {
        //Getting num of words in each line.
        int space = 0;
        for (unsigned int ii = 0; ii < strDisplayContent[i].length(); ii++)
        {

            if (strDisplayContent[i][ii] == ' ' || strDisplayContent[i][ii] == '\t')
                space++;
        }

        stringstream tokenizer;
        tokenizer.clear();
        tokenizer.str(strDisplayContent[i]);

        vector<string>  tempStringVec;
        tempStringVec.resize(space + 1);
        mStrVecDisplayContent[i].resize(space + 1);

        for (int j = 0; j < space + 1; j++)
        {
            tokenizer >> mStrVecDisplayContent[i][j]; //Got actual word
        }
    }
    readFontData();
    //writeOutput();
}

void DrawChars::setWordToDisplayOnScreen()
{
    for (unsigned int  i = 0; i < mStrVecDisplayContent.size(); i++)
    {
        GLfloat   yValue = 0.0;
		GLfloat   xValue = mTraslateXParam[i];
		bool newWord = false;
        for (unsigned int j = 0; j < mStrVecDisplayContent[i].size(); j++)
        {
            string displyWord = mStrVecDisplayContent[i][j];  

			if (mStrVecDisplayContent[i].size() > 1 && newWord)
			{
				xValue = xValue+ mWordSpacing[i];
			}

            for (unsigned int ii = 0; ii < mStrVecDisplayContent[i][j].length(); ii++)
            {
                xValue = xValue + mCharSpacing[i];
				glLineWidth(GLfloat(mCharWdth[i]));
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glColor3f(GLfloat(mRgbValueVec[i][0]), GLfloat(mRgbValueVec[i][1]), GLfloat(mRgbValueVec[i][2]));
                glTranslatef(xValue, GLfloat(mTraslateYParam[i]), GLfloat(mTraslateZParam[i]));
				glScalef(mScaleVect[i][0], mScaleVect[i][1], mScaleVect[i][2]);
                drawCharOnScreen( mStrVecDisplayContent[i][j][ii]);
            }
			newWord = true;
        }
    }
}


void DrawChars::drawCharOnScreen(char displayChar)
{
	string keyChar = " ";
	keyChar = displayChar;
    map < string, vector<vector<vector<int>>>> ::iterator mapItr;
    mapItr = mCharPtsMap.find(keyChar);

    if ( mapItr != mCharPtsMap.end())
    {
        vector<vector<vector<int>>> charPtsVec = (*mapItr).second;

        for (vector<vector<vector<int>>>::iterator it = charPtsVec.begin(); it != charPtsVec.end(); it++)
        {
            vector<vector<int>> mdPtsVec = *it;
            glBegin(GL_LINE_STRIP);
            for (vector<vector<int>> ::iterator i = mdPtsVec.begin(); i != mdPtsVec.end(); i++)
            {
                vector<int> temp = *i;
                int xPt = 0, yPt = 0;
                if (temp[0] != 0) xPt = temp[0]-1;
                if (temp[1] != 0) yPt = temp[1]-1;
                //glVertex3f(mGridPts[xPt][yPt][0], mGridPts[xPt][yPt][1], 0.0f);

				glVertex3f(mNewGridPts[xPt][yPt][0], mNewGridPts[xPt][yPt][1], 0.0f);
            }
            glEnd();
        }
     }
}