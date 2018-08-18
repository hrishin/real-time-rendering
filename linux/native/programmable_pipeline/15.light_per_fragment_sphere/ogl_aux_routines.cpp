#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glx.h>
#include "ogl_aux.h"

void LogShaderCompilationStatus(GLuint shaderObject, FILE *pLogFile, const char *logPrefix)
{
    GLint iInfoLogLength = 0;
    GLint iShaderCompiledStatus = 0;
    char *szInfoLog = NULL;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
    if (iShaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(shaderObject, iInfoLogLength, &written, szInfoLog);
                fprintf(pLogFile, "%s : %s\n", logPrefix, szInfoLog);
                free(szInfoLog);
                Uninitialize();
                exit(EXIT_FAILURE);
            }
        }
    }
}

void LogProgramCompilationStatus(GLuint programObject, FILE *pLogFile, const char *logPrefix)
{
    GLint iInfoLogLength = 0;
    char *szInfoLog = NULL;
    GLint iShaderProgramLinkStatus = 0;
    glGetProgramiv(programObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
    if (iShaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(programObject, iInfoLogLength, &written, szInfoLog);
                fprintf(pLogFile, "%s : %s\n", logPrefix, szInfoLog);
                free(szInfoLog);
                Uninitialize();
                exit(0);
            }
        }
    }
}
