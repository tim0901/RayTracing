
#pragma once

#ifndef IMAGE_PARAMETERS_H
#define IMAGE_PARAMETERS_H

#include "chunk.h"
#include <list>

//class chunk;

struct image_parameters {
	int nx;
	int ny;
	int ns;

	unsigned char *outputArray = { 0 };
	unsigned char **outputArrayPtr = &outputArray;
	std::list<chunk> chunkList;
	std::list<chunk>::iterator iter;

};
#endif //IMAGE_PARAMETERS_H