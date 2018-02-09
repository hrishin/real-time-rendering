
#include"myHeader.h"
#include"resource.h"

HDC								g_hDC = NULL;
HGLRC							g_hRC = NULL;  //Rendering context
extern		bool				g_bIsFullScreen;
extern		HWND				g_hWnd;
extern		WINDOWPLACEMENT		g_hPrevWndPlace;
extern		cLog				&fout;
float							g_Angle = 0.0f;

extern		float				g_angle;
extern		float				g_lx;
extern      float				g_lz;
extern		float				g_x;
extern		float				g_z;
extern		float				eyex;
extern		float				eyey;
extern		float				eyez;
extern		float				centerx;
extern		float				centery;
extern		float				centerz;
extern		float				upmtrx;
extern		float				upmtry;
extern		float				upmtrz;


float 		g_Mercury			= 0.0f;
float 		g_Venus				= 0.0f;
float 		g_Earth				= 0.0f;
float 		g_Mars				= 0.0f;
float 		g_Jupiter			= 0.0f;
float 		g_Saturn			= 0.0f;
float 		g_Uranus			= 0.0f;
float 		g_Neptune			= 0.0f;

/* define array requires for lighting*/
GLfloat					hAmbientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat					hDefuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat					hSpecularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat					hLightPosition[] = { 0.0f, 0.0f, 1.0f, 0.0f }; /*Consider that Light is coming from outside the theatre(from Front, thats why, Z is one ) */
GLfloat					hMaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat					hShininess = 50.0f;
GLuint					gbTexture[10];
GLUquadric				*pQuadric[9];
void update()
{
	g_Angle = g_Angle + 0.1f;
	if (g_Angle >= 360)
		g_Angle = 0.1f;

	g_Mercury	=	g_Mercury + 0.7;
	g_Venus		=	g_Venus +	0.3;
	g_Earth		=	g_Earth +	0.5;
	g_Mars		=	g_Mars +	0.3;
	g_Jupiter	=	g_Jupiter + 0.5;
	g_Saturn	=	g_Saturn + 0.6;
	g_Uranus	=	g_Uranus + 0.7;
	g_Neptune	=	g_Neptune + 0.5;


}


void doInitialize(void)
{
	void		resize(int, int);
	int LoadTexture(GLuint *, TCHAR[]);
	PIXELFORMATDESCRIPTOR		hPFD;
	int							iPixelFormatIndex;
	ZeroMemory(&hPFD, sizeof(PIXELFORMATDESCRIPTOR));
	hPFD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	hPFD.nVersion = 1;
	hPFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; /*What is the use of PFD_DOUBLEBUFFERDONTCARE*/
	hPFD.iPixelType = PFD_TYPE_RGBA;
	hPFD.cColorBits = 32;
	hPFD.cRedBits = 8;
	hPFD.cGreenBits = 8;
	hPFD.cBlueBits = 8;
	hPFD.cAlphaBits = 8;
	hPFD.cDepthBits = 32;

	g_hDC = GetDC(g_hWnd); // obtain the painter of local windowing.
	iPixelFormatIndex = ChoosePixelFormat(g_hDC, &hPFD);
	if (iPixelFormatIndex == 0) {
		fout << "pixel format index could not obtain";
		ReleaseDC(g_hWnd, g_hDC);
		g_hDC = NULL;
	}
	SetPixelFormat(g_hDC, iPixelFormatIndex, &hPFD);

	/* Lets form the bridge between local windowing and OpenGL on Windows*/
	g_hRC = wglCreateContext(g_hDC); // obtain the painter for rendering. /* what is the use of wglLayerContext()*/
	if (g_hRC == NULL) {
		fout << "Bride could not form between Windowing and OpenGL";
		ReleaseDC(g_hWnd, g_hDC);
		g_hDC = NULL;
	}
	wglMakeCurrent(g_hDC, g_hRC);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_2D);

	glLightfv(GL_LIGHT0, GL_AMBIENT, hAmbientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, hDefuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, hSpecularLight);
	glLightfv(GL_LIGHT0, GL_POSITION, hLightPosition);

	glMaterialfv(GL_FRONT, GL_SPECULAR, hMaterialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, hShininess);

	glEnable(GL_LIGHT0);
	LoadTexture(&gbTexture[0], MAKEINTRESOURCE(EARTH_BITMAP));
	LoadTexture(&gbTexture[1], MAKEINTRESOURCE(MERCURY_BITMAP));
	LoadTexture(&gbTexture[2], MAKEINTRESOURCE(VENUS_BITMAP));
	LoadTexture(&gbTexture[3], MAKEINTRESOURCE(MARS_BITMAP));
	LoadTexture(&gbTexture[4], MAKEINTRESOURCE(JUPITER_BITMAP));
	LoadTexture(&gbTexture[5], MAKEINTRESOURCE(SATURN_BITMAP));
	LoadTexture(&gbTexture[6], MAKEINTRESOURCE(URANUS_BITMAP));
	LoadTexture(&gbTexture[7], MAKEINTRESOURCE(NEPTUNE_BITMAP));
	LoadTexture(&gbTexture[8], MAKEINTRESOURCE(SUN_BITMAP));
	LoadTexture(&gbTexture[9], MAKEINTRESOURCE(STARS_BITMAP));


	for ( int i=0; i<9;i++)
		pQuadric[i] = gluNewQuadric();
	doResizing(WIN_WIDTH, WIN_HEIGHT);
	return;
}

int LoadTexture(GLuint *texture, TCHAR resource[])
{
	HBITMAP hBitmap;
	BITMAP bmp;
	int iStatus = FALSE;

	glGenTextures(1, texture);
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), resource, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBitmap)
	{
		iStatus = TRUE;
		GetObject(hBitmap, sizeof(bmp), &bmp);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		DeleteObject(hBitmap);
	}

	return(iStatus);
}

void mercury()
{
	glPushMatrix();
	glRotatef(g_Mercury, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.15f, 0.0f, -1.0);
	glScalef(0.02f, 0.02f, 0.02f);

	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(g_Angle, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[1]);
	gluQuadricTexture(pQuadric[1], 1);
	gluSphere(pQuadric[1], 0.75f, 100, 100);

	
	
	GLint CirclePoints = 1000;
	const float pi = 3.14f;
	float angle = 0.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	//glColor3f(GREEN_COLOR);
	for (int i = 0; i < 1000; i++)
	{
		angle = (2 * pi * i) / 1000;   // Used this step to do the 1000 partitions of particular circle into points so that it would clear
		glVertex3f(0.15*cos(angle), 0.15* sin(angle), -1.0f);
	}
	glEnd();
	glPopMatrix();
}

void venus()
{
	glPushMatrix();
	//glTranslatef(0.6f, 1.5f, -7.0);
	glRotatef(g_Venus, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.24f, 0.0f, -1.0); 
	glScalef(0.05f, 0.05f, 0.05f);
	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(g_Angle, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[2]);
	gluQuadricTexture(pQuadric[2], 1);
	gluSphere(pQuadric[2], 0.75f, 100, 100);

	//glRotatef(g_Venus, 0.0f, 0.0f, 1.0f);
	//glTranslatef(-0.3f, 0.0f, -1.0);

	GLint CirclePoints = 1000;
	const float pi = 3.14f;
	float angle = 0.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	//glColor3f(GREEN_COLOR);
	for (int i = 0; i < 1000; i++)
	{
		angle = (2 * pi * i) / 1000;   // Used this step to do the 1000 partitions of particular circle into points so that it would clear
		glVertex3f(0.24*cos(angle), 0.24* sin(angle), -1.0f);
	}
	glEnd();
	glPopMatrix();
}

void earth()
{
	glPushMatrix();
	glRotatef(g_Earth, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.33f, 0.0f, -1.0);
	glScalef(0.05f, 0.05f, 0.05f);
	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);

	glRotatef(g_Angle, 0.0, 0.0, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[0]);
	gluQuadricTexture(pQuadric[0], 1);
	gluSphere(pQuadric[0], 0.75f, 100, 100);

	GLint CirclePoints = 1000;
	const float pi = 3.14f;
	float angle = 0.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	//glColor3f(GREEN_COLOR);
	for (int i = 0; i < 1000; i++)
	{
		angle = (2 * pi * i) / 1000;   // Used this step to do the 1000 partitions of particular circle into points so that it would clear
		glVertex3f(0.33*cos(angle), 0.33* sin(angle), -1.0f);
	}
	glEnd();
	glPopMatrix();
}



void mars()
{
	glPushMatrix();
	//glTranslatef(2.2f, 1.5f, -7.0);
	glRotatef(g_Mars, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.43f, 0.0f, -1.0);
	glScalef(0.05f, 0.05f, 0.05f);
	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(g_Angle, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[3]);
	gluQuadricTexture(pQuadric[3], 1);
	gluSphere(pQuadric[3], 0.75f, 100, 100);

	GLint CirclePoints = 1000;
	const float pi = 3.14f;
	float angle = 0.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	//glColor3f(GREEN_COLOR);
	for (int i = 0; i < 1000; i++)
	{
		angle = (2 * pi * i) / 1000;   // Used this step to do the 1000 partitions of particular circle into points so that it would clear
		glVertex3f(0.43*cos(angle), 0.43* sin(angle), -1.0f);
	}
	glEnd();
	glPopMatrix();
}

void jupiter()
{
	glPushMatrix();
	//glTranslatef(-2.7f, -1.5f, -7.0);
	glRotatef(g_Jupiter, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.53f, 0.0f, -1.0);

	glScalef(0.05f, 0.05f, 0.05f);
	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(g_Angle, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[4]);
	gluQuadricTexture(pQuadric[4], 1);
	gluSphere(pQuadric[4], 0.75f, 100, 100);

	
	GLint CirclePoints = 1000;
	const float pi = 3.14f;
	float angle = 0.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	//glColor3f(GREEN_COLOR);
	for (int i = 0; i < 1000; i++)
	{
		angle = (2 * pi * i) / 1000;   // Used this step to do the 1000 partitions of particular circle into points so that it would clear
		glVertex3f(0.53*cos(angle), 0.53* sin(angle), -1.0f);
	}
	glEnd();
	glPopMatrix();
}


void saturn()
{
	glPushMatrix();
	glRotatef(g_Saturn, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.65f, 0.0f, -1.0);
	glScalef(0.05f, 0.05f, 0.05f);
	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(g_Angle, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[5]);
	gluQuadricTexture(pQuadric[5], 1);
	gluSphere(pQuadric[5], 0.75f, 100, 100);

	
	GLint CirclePoints = 1000;
	const float pi = 3.14f;
	float angle = 0.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	//glColor3f(GREEN_COLOR);
	for (int i = 0; i < 1000; i++)
	{
		angle = (2 * pi * i) / 1000;   // Used this step to do the 1000 partitions of particular circle into points so that it would clear
		glVertex3f(0.65*cos(angle), 0.65* sin(angle), -1.0f);
	}
	glEnd();
	glPopMatrix();
}

void uranus()
{
	glPushMatrix();
	glRotatef(g_Uranus, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.73f, 0.0f, -1.0);
	glScalef(0.05f, 0.05f, 0.05f);
	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(g_Angle, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[6]);
	gluQuadricTexture(pQuadric[6], 1);
	gluSphere(pQuadric[6], 0.75f, 100, 100);

	
	GLint CirclePoints = 1000;
	const float pi = 3.14f;
	float angle = 0.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	//glColor3f(GREEN_COLOR);
	for (int i = 0; i < 1000; i++)
	{
		angle = (2 * pi * i) / 1000;   // Used this step to do the 1000 partitions of particular circle into points so that it would clear
		glVertex3f(0.73*cos(angle), 0.73* sin(angle), -1.0f);
	}
	glEnd();
	glPopMatrix();
}

void neptune()
{
	glPushMatrix();
	glRotatef(g_Neptune, 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.83f, 0.0f, -1.0);
	glScalef(0.05f, 0.05f, 0.05f);
	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(g_Angle, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[7]);
	gluQuadricTexture(pQuadric[7], 1);
	gluSphere(pQuadric[7], 0.75f, 100, 100);

	
	GLint CirclePoints = 1000;
	const float pi = 3.14f;
	float angle = 0.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	//glColor3f(GREEN_COLOR);
	for (int i = 0; i < 1000; i++)
	{
		angle = (2 * pi * i) / 1000;   // Used this step to do the 1000 partitions of particular circle into points so that it would clear
		glVertex3f(0.83*cos(angle), 0.83* sin(angle), -1.0f);
	}
	glEnd();
	glPopMatrix();
}


void sun()
{
	glPushMatrix();
	glTranslatef(-0.01f, 0.02f, -2.0);
	glScalef(0.15f, 0.15f, 0.15f);

	glRotatef(95, 1.0, 0.0, 0.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glRotatef(g_Angle, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, gbTexture[8]);
	gluQuadricTexture(pQuadric[8], 1);
	gluSphere(pQuadric[8], 0.75f, 100, 100);
	glPopMatrix();
}



void stars()
{
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -1.0f); // Model Transformation
	//glScalef(0.75f, 0.75f, 0.75f);
	glBindTexture(GL_TEXTURE_2D, gbTexture[9]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f); // right-top corner of top face

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f); // left-top corner of top face

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f); // left-bottom corner of top face

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f); // right-bottom corner of top face

	glEnd();
	glPopMatrix();
}


void display(void)
{
	void earth();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upmtrx, upmtry, upmtrz);
	glTranslatef(0.0, 0.0, -3.0);

	stars();
	sun();
	mercury();
	venus();
	earth();
	mars();
	jupiter();
	saturn();
	uranus();
	neptune();
	
	SwapBuffers(g_hDC);
}
void setPerspectiveProjection(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
	fout << "Perpective projection has set";
}
void doResizing(int width, int height)
{

	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	fout << "ViewPort has changed";

	setPerspectiveProjection(width, height);

	fout << "End doResizing";
	return;
}
void doUnInitialize(void)
{
	fout << "Begin doUnInitialize";
	DWORD dwStyle;
	if (g_bIsFullScreen == true)
	{
		dwStyle = GetWindowLong(g_hWnd, GWL_STYLE);
		SetWindowLong(g_hWnd, GWL_STYLE, dwStyle);
		SetWindowPlacement(g_hWnd, &g_hPrevWndPlace);
		SetWindowPos(g_hWnd, HWND_TOP, 0, 0, WIN_WIDTH, WIN_HEIGHT, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(g_hRC);
	g_hRC = NULL;
	ReleaseDC(g_hWnd, g_hDC);
	g_hDC = NULL;
	DestroyWindow(g_hWnd);
	g_hWnd = NULL;
	fout << "End doUnInitialize";
	return;
}
