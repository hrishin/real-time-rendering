#ifndef OGL_AUX_H
#define OGL_AUX_H

extern void Uninitialize(void);

void LogShaderCompilationStatus(GLuint, FILE *, const char *);
void LogProgramCompilationStatus(GLuint, FILE *, const char *);

#endif