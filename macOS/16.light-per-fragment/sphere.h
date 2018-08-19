#ifndef SPHERE_H
#define SPHERE_H

#define S_VERTICES 1146
#define S_NORMALS 1146
#define S_TEXTURES 764
#define S_ELEMENTS 2280

extern void getSphereVertexData(float[S_VERTICES], float[S_NORMALS], float[S_TEXTURES], unsigned short[S_ELEMENTS]);
extern unsigned int getNumberOfSphereVertices(void);
extern unsigned int getNumberOfSphereElements(void);

#endif