#pragma once

extern "C" void getSphereVertexData(float spherePositionCoords[1146], float sphereNormalCoords[1146], float sphereTexCoords[764], unsigned short sphereElements[2280]);

extern "C" unsigned int getNumberOfSphereVertices(void);

extern "C" unsigned int getNumberOfSphereElements(void);
