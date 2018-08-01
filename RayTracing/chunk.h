#pragma once

#include "vec3.h"
#include "camera.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>

//defines a chunk as a container for the start and end x and y dimensions
//Also contains the number of samples to be taken per pixel

class chunk {
public:

	int position[5];

	chunk() {};
	chunk(int startX, int endX, int startY, int endY, int ns) { position[0] = startX; position[1] = endX; position[2] = startY; position[3] = endY; position[4] = ns; }

	inline int startX() const { return position[0]; }
	inline int endX() const { return position[1]; }
	inline int startY() const { return position[2]; }
	inline int endY() const { return position[3]; }
	inline int ns() const { return position[4]; }
};