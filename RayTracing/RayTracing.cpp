#pragma once

#include "stdafx.h"

#include "OpenGL.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <iostream>
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include <cfloat>
#include "camera.h"
#include "material.h"
#include <thread>
#include "chunk.h"
#include <list>
#include <mutex>
#include <math.h>
#include "moving_sphere.h"
#include "drand48.h"
#include <stdlib.h>
#include "aabb.h"
#include "bvh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "scenes.h"
#include "image_parameters.h"
#include "aarect.h"
#include <chrono>
#include "mesh.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"

//Declare functions
void save(image_parameters* image);
vec3 color(const ray& r, hitable *world, hitable *light_shape, int depth);
void render(image_parameters* image, int k, int* chunksRemaining, bool* windowOpen, hitable **world, hitable **light_list, camera **cam);

//Container for list of chunks to be rendered
std::mutex chunkList_mutex;

//Iterator for chunk list

std::mutex outputArray_mutex;

//Mutex used to prevent console write errors. Shouldn't be necessary outside of testing
std::mutex console_mutex;

//gets number of threads of systems
unsigned const int nthreads = 8;// std::thread::hardware_concurrency();

//Declares threads
std::thread t[nthreads+1];

std::mutex samples_mutex;

int main()
{

	//Start timer
	auto start = std::chrono::steady_clock::now();

	int* chunksRemaining = new int(0);

	image_parameters* image = new image_parameters();

	*image->sampleReciprocalsPtr = (float*)calloc(image->ns + 1, sizeof(float));

	image->sampleReciprocals[0] = 0;
	for (float x = 1; x <= image->ns; x++) {
		
		image->sampleReciprocals[int(x)] = 1 / x;
	}

	//Defines array for output
	*image->outputArrayPtr = (float*)calloc((image->nx + 2) * (image->ny + 2) * 4, sizeof(float));

	//Seed random number generator
	srand(time(0));

	//For checking if the viewport should be open
	bool *windowOpen = new bool(true);
	//Initialise viewport on the last thread
	t[nthreads] = std::thread(initialiseWindow, image, windowOpen);

	//Splits the image into n*n squares, each to be rendered by an independent thread

	int xChunks = ((image->nx + image->chunkSize - 1) / image->chunkSize);
	int yChunks = ((image->ny + image->chunkSize - 1) / image->chunkSize);

	//Add chunks to render list;

	for (int i = 0; i < xChunks; i++) {
		for (int j = 0; j < yChunks; j++) {

			chunk tempChunk = chunk(image->nx - i * image->chunkSize - 1, image->nx - (i + 1)*image->chunkSize, image->ny - j * image->chunkSize - 1, image->ny - (j + 1)*image->chunkSize);

			//Deals with edges of strangely sized images
			if (tempChunk.endX() < 0) {

				tempChunk = chunk(tempChunk.startX(), 0, tempChunk.startY(), tempChunk.endY());

			}
			if (tempChunk.endY() < 0) {

				tempChunk = chunk(tempChunk.startX(), tempChunk.endX(), tempChunk.startY(), 0);

			}

			image->chunkList.push_front(tempChunk);
		}
	}

	//Initialise iterator
	std::list<chunk>::iterator iter;
	image->iter = image->chunkList.begin();

	//std::cout << "no. of chunks: " << xChunks << " * " << yChunks << " = " << image->chunkList.size() << std::endl;

	*chunksRemaining = xChunks * yChunks;

	//Builds some objects to render
	hitable *world;
	hitable *light_list;

	//Camera to render through
	camera *cam;

	cornell_box_final(&world, &light_list, image, &cam);

//	testing(&world, &light_list, image, &cam);
	
	*windowOpen = true;

	std::cout << *chunksRemaining << std::endl;
	//Renders out the chunks
	for (int k = 0; k < (nthreads); k++) {
		t[k] = std::thread(render, image, k, chunksRemaining, windowOpen, &world, &light_list, &cam);
	}

	//Joins rendering threads
	for (int k = 0; k < nthreads; ++k) {
		t[k].join();
	}

	//Saves only if the render completed
	if (*chunksRemaining == 0 || image->previous_samples == image->ns) {
		t[0] = std::thread(save, image);
		t[0].join();
	}

	//End timer
	auto end = std::chrono::steady_clock::now();
	auto diff = end - start;
	std::cout << "Frame render time: " << std::chrono::duration <double, std::milli>(diff).count() << "ms" << std::endl;

	//Closes OpenGL render thread
	t[nthreads].join();

	//Cleanup
	terminateWindow();
	delete[] world;
	delete[] image->outputArray;
	delete[] image->sampleReciprocals;
	delete windowOpen;
}

//Color of pixel

vec3 color(const ray& r, hitable *world, hitable *light_shape, int depth) {
	hit_record hrec;
	if (world->hit(r, 0.001, FLT_MAX, hrec)) {
		scatter_record srec;
		vec3 emitted = hrec.mat_ptr->emitted(r, hrec, hrec.u, hrec.v, hrec.p);
		if (depth < 50 && hrec.mat_ptr->scatter(r, hrec, srec)) {
			if (srec.is_specular) {
				return srec.attenuation * color(srec.specular_ray, world, light_shape, depth + 1);
			}
			else {
				hitable_pdf plight(light_shape, hrec.p);
				mixture_pdf p(&plight, srec.pdf_ptr);
				ray scattered = ray(hrec.p, p.generate(), r.time());
				float pdf_val = p.value(scattered.direction());
				delete srec.pdf_ptr;
				return emitted + srec.attenuation*hrec.mat_ptr->scattering_pdf(r, hrec, scattered)*color(scattered, world, light_shape, depth + 1) / pdf_val;
			}
		}
		else {
			return emitted;
		}
	}
	else {
		/*vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0);
		return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);*/
		return vec3(0, 0, 0);
	}
}

void save(image_parameters* image) {

	std::cout << "Saving..." << std::endl;

	if (image->savePPM == true) {

		//Required for console output
		FILE *stream;

		//Sends console output to file out.ppm 
		freopen_s(&stream, "out.ppm", "w", stdout);

		//PPM file header
		std::cout << "P3\n" << image->nx << " " << image->ny << "\n255\n";

		//Outputs colours as required by file format
		for (int j = image->ny - 1; j >= 0; j--) {
			for (int i = 0; i < image->nx; i++) {

				int ir = 255.99*image->outputArray[((i + (image->nx * j)) * 4)];
				int ig = 255.99*image->outputArray[((i + (image->nx * j)) * 4) + 1];
				int ib = 255.99*image->outputArray[((i + (image->nx * j)) * 4) + 2];

				std::cout << ir << " " << ig << " " << ib << "\n";
			}
		}

		//Closes out.ppm file at end out output.
		fclose(stdout);

	}
	if (image->saveHDR == true) {

		//Exports as .hdr - loses alpha channel data. 
		stbi_flip_vertically_on_write(1);

		stbi_write_hdr("out.hdr", image->nx, image->ny, 4, image->outputArray);

	}
	if (image->savePPM == false) {
		std::cout << "Save complete" << std::endl;
	}
}

inline vec3 de_nan(const vec3& c) {
	vec3 temp = c;
	if (!(temp[0] == temp[0])) temp[0] = 0;
	if (!(temp[1] == temp[1])) temp[1] = 0; 
	if (!(temp[2] == temp[2])) temp[1] = 0;
	return temp;
}

void render(image_parameters* image, int k, int* chunksRemaining, bool *windowOpen, hitable **world, hitable **light_list,camera **camPtr) {

	camera cam = **camPtr;

	if (image->iterative_mode == false) {
		//Checks list has contents
		while (&chunksRemaining != 0 && *windowOpen) {

			//Locks list from access by other threads
			chunkList_mutex.lock();
			//Removes first chunk on the list

			chunk toBeRendered;
			if (image->chunkList.end() == image->iter || &chunksRemaining < 0)
			{
				chunkList_mutex.unlock();
				break;
			}
			else
			{
				toBeRendered = *image->iter;
			}

			std::advance(image->iter, 1);

			//Unlocks list
			chunkList_mutex.unlock();

			//DEBUG
			//console_mutex.lock();
			//std::cout << "sX: " << toBeRendered.startX() << " eX: " << toBeRendered.endX() << " sY: " << toBeRendered.startY() << " eY: " << toBeRendered.endY() << std::endl;
			//console_mutex.unlock();

			for (int j = int(toBeRendered.startY()); j >= int(toBeRendered.endY()); j--) {

				for (int i = int(toBeRendered.endX()); i <= int(toBeRendered.startX()); i++) {

					vec3 col(0, 0, 0);
					for (int s = 0; s < image->ns; s++) {
						//Applies anti-aliasing with the random number here - very slow!
						float u = float(i + drand48()) / float(image->nx);
						float v = float(j + drand48()) / float(image->ny);
						ray r = cam.get_ray(u, v);
						col += de_nan(color(r, *world, *light_list, 0));
					}
					//col /= float(image->ns);
					col *= image->sampleReciprocals[image->ns];


					//Color correction
					col = vec3(sqrt(col.r()), sqrt(col.g()), sqrt(col.b()));

					image->outputArray[(i + (image->nx * j)) * 4] = col[0];
					image->outputArray[((i + (image->nx * j)) * 4) + 1] = col[1];
					image->outputArray[((i + (image->nx * j)) * 4) + 2] = col[2];
					image->outputArray[((i + (image->nx * j)) * 4) + 3] = 1;

					if (*windowOpen == false) {
						break;
					}
				}

				if (*windowOpen == false) {
					break;
				}
			}

			if (*windowOpen == false) {
				break;
			}
			*chunksRemaining = *chunksRemaining - 1;
		}
	}
	else {

		int chunk_previous_samples = 0;
		int chunk_sample_number = 1;


		while (image->previous_samples < image->ns && *windowOpen) {

			while(image->previous_samples < image->ns) {
				//Locks list from access by other threads
				chunkList_mutex.lock();
				//Removes first chunk on the list

				samples_mutex.lock();
				chunk toBeRendered;
				if (image->chunkList.end() == image->iter || &chunksRemaining < 0)
				{
					image->iter = image->chunkList.begin();
					image->sample_number++;
					image->previous_samples++;
				}
				
				if (image->previous_samples == image->ns) {
					chunkList_mutex.unlock();
					samples_mutex.unlock();
					break;
				}

				chunk_previous_samples = image->previous_samples;
				chunk_sample_number = image->sample_number;
				toBeRendered = *image->iter;
				samples_mutex.unlock();

				std::advance(image->iter, 1);

				//Unlocks list
				chunkList_mutex.unlock();

				//DEBUG
				//console_mutex.lock();
				//std::cout << "sX: " << toBeRendered.startX() << " eX: " << toBeRendered.endX() << " sY: " << toBeRendered.startY() << " eY: " << toBeRendered.endY() << std::endl;
				//console_mutex.unlock();

				for (int j = int(toBeRendered.startY()); j >= int(toBeRendered.endY()); j--) {

					for (int i = int(toBeRendered.endX()); i <= int(toBeRendered.startX()); i++) {
						vec3 col(0, 0, 0);

						col[0] = image->outputArray[(i + (image->nx * j)) * 4];
						col[1] = image->outputArray[((i + (image->nx * j)) * 4) + 1];
						col[2] = image->outputArray[((i + (image->nx * j)) * 4) + 2];
						//col[3] = image->outputArray[((i + (image->nx * j)) * 4) + 3];

						col = vec3((col.r() * col.r())* chunk_previous_samples, (col.g() * col.g())*chunk_previous_samples, (col.b() * col.b())*chunk_previous_samples);

						//Applies anti-aliasing with the random number here - very slow!
						float u = float(i + drand48()) / float(image->nx);
						float v = float(j + drand48()) / float(image->ny);
						ray r = cam.get_ray(u, v);
						col += de_nan(color(r, *world, *light_list, 0));

						//col /= float(chunk_sample_number);

						col *= image->sampleReciprocals[chunk_sample_number];

						//Color correction
						col = vec3(sqrt(col.r()), sqrt(col.g()), sqrt(col.b()));

						image->outputArray[(i + (image->nx * j)) * 4] = col[0];
						image->outputArray[((i + (image->nx * j)) * 4) + 1] = col[1];
						image->outputArray[((i + (image->nx * j)) * 4) + 2] = col[2];
						image->outputArray[((i + (image->nx * j)) * 4) + 3] = 1;

						if (*windowOpen == false) {
							break;
						}
					}

					if (*windowOpen == false) {
						break;
					}
				}

				if (*windowOpen == false) {
					break;
				}
			}
			if (*windowOpen == false) {
				break;
			}
		}
	}
	//Close thread
	console_mutex.lock();
	std::cout << "Thread: " << k << " finished." << std::endl;
	console_mutex.unlock();
}