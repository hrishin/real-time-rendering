#pragma once
#include"cLog.h"

cLog *cLog::p_hcLog = NULL;
cLog::cLog()
{
}
cLog::cLog(const char *pszFileName):hLog(pszFileName, ios::out)
{
}
cLog::~cLog()
{
	hLog.close();
}
cLog& cLog::getLogHandle(const char *pszFileName)
{
	if (p_hcLog == NULL) {
		p_hcLog = new cLog(pszFileName);
	}
	return (*p_hcLog);
}
void cLog::log(const char *pszLog)
{
	
	hLog <<endl<<pszLog ;
}

void cLog::distroyLog()
{
	if (p_hcLog != NULL)
	{
		delete p_hcLog;
		p_hcLog = NULL;
	}
}
cLog& cLog::operator<<(const char *pszLine)
{
	this->log(pszLine);
	return (*this);
}
cLog& cLog::operator*(void)
{
	return (*this);
}
