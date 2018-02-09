#pragma once
#include<iostream>
#include<tchar.h>
#include<fstream>
using namespace std;

class cLog
{
private:
	cLog();
	cLog(const char *pszFileName);
	fstream hLog;
	static cLog *p_hcLog;
	~cLog();
public:
	static cLog& getLogHandle(const char *pszFileName);
	void  log(const char *pszLog);
	static void  distroyLog();
	cLog& operator<<(const char *pszLine);
	cLog& operator*(void);
};
