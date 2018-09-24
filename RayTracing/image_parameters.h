
#pragma once

#ifndef IMAGE_PARAMETERS_H
#define IMAGE_PARAMETERS_H

#include "chunk.h"
#include <list>

//class chunk;

struct image_parameters {

	//Image dimensions
	int nx;
	int ny;

	//Number of Samples
	int ns;

	//Output Storage
	float *outputArray = { 0 };
	float **outputArrayPtr = &outputArray;
	std::list<chunk> chunkList;
	std::list<chunk>::iterator iter;

	//Iterative mode settings
	bool iterative_mode = true;
	int sample_number = 1;
	int previous_samples = 0;


};
#endif //IMAGE_PARAMETERS_H