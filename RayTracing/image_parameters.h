
#pragma once

#ifndef IMAGE_PARAMETERS_H
#define IMAGE_PARAMETERS_H

#include "chunk.h"
#include <list>

//class chunk;

struct image_parameters {

	//Image dimensions
	int nx = 1000;
	int ny = 1000;

	//Number of Samples
	int ns = 1000;
	float *sampleReciprocals = { 0 };
	float **sampleReciprocalsPtr = &sampleReciprocals;


	//Sets size of chunk
	int chunkSize = 50;

	//Output Storage
	float *outputArray = { 0 };
	float **outputArrayPtr = &outputArray;
	std::list<chunk> chunkList;
	std::list<chunk>::iterator iter;

	//Iterative mode settings
	bool iterative_mode = true;
	int sample_number = 1;
	int previous_samples = 0;

	//Save Options
	bool savePPM = false;
	bool saveHDR = true;


};
#endif //IMAGE_PARAMETERS_H