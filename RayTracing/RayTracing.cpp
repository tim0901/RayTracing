#pragma once

#include "stdafx.h"

#include "OpenGL.h"

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
#include "random_scene.h"
#include "image_parameters.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"

//Declare functions
void save(image_parameters* image);
hitable* random_scene();
vec3 color(const ray& r, hitable *world, int depth);
void render(image_parameters* image, int k, int* chunksRemaining, bool* windowOpen, hitable* world, camera cam);

//Container for list of chunks to be rendered
std::mutex chunkList_mutex;

//Iterator for chunk list

std::mutex outputArray_mutex;

//Mutex used to prevent console write errors. Shouldn't be necessary outside of testing
std::mutex console_mutex;

//gets number of threads of systems
unsigned const int nthreads = 8;// std::thread::hardware_concurrency();

//Declares threads
std::thread t[nthreads];

int main()
{
	int* chunksRemaining = new int(0);

	image_parameters* image = new image_parameters();

	//Defines size of image
	image->nx = 600;
	image->ny = 300;
	//Rays per pixel
	image->ns = 10;

	//Defines array for output
	*image->outputArrayPtr = (unsigned char*)malloc((image->nx + 2) * (image->ny + 2) * 4);

	//Seed random number generator
	srand(time(0));

	//For checking if the viewport should be open
	bool *windowOpen = new bool(true);
	//Initialise viewport on the last thread
	t[nthreads] = std::thread(initialiseWindow, image, windowOpen);

	//Splits the image into n*n squares, each to be rendered by an independent thread
	//Sets size of chunk
	int chunkSize = 50;

	int xChunks = ((image->nx + chunkSize - 1) / chunkSize);
	int yChunks = ((image->ny + chunkSize - 1) / chunkSize);

	//Add chunks to render list;

	for (int i = 0; i < xChunks; i++) {
		for (int j = 0; j < yChunks; j++) {

			chunk tempChunk = chunk(image->nx - i * chunkSize - 1, image->nx - (i + 1)*chunkSize, image->ny - j * chunkSize + 1, image->ny - (j + 1)*chunkSize);

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

	std::list<chunk>::iterator iter;
	//Initialise iterator
	image->iter = image->chunkList.begin();

	std::cout << "no. of chunks: " << xChunks << " * " << yChunks << " = " << image->chunkList.size() << std::endl;

	*chunksRemaining = xChunks * yChunks;

	//Builds some objects to render
	hitable *list[6];

	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(vec3(1.0, 1.0, 1.0), 1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -0.495, new dielectric(vec3(1.0, 1.0, 1.0), 1.5));
	list[5] = new sphere(vec3(-1, 0, 1), 0.5, new lambertian(vec3(0.3, 0.6, 0.5)));

	//Builds a world from the objects
	hitable *world = new hitable_list(list, 6);

	//Generate random sphere world
	world = random_scene();

	//Camera to render through

	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	float dist_to_focus = 10; //(lookfrom - lookat).length();
	float aperture = 0.0;
	float aspect_ratio = float(image->nx) / float(image->ny);
	int fov = 20;
	float startTime = 0.0;
	float endTime = 0.0;

	camera cam(lookfrom, lookat, vec3(0, 1, 0), fov, aspect_ratio, aperture, dist_to_focus, startTime, endTime);
	*windowOpen = true;

	std::cout << *chunksRemaining << std::endl;
	//Renders out the chunks
	for (int k = 0; k < (nthreads); k++) {
		t[k] = std::thread(render, image, k, chunksRemaining, windowOpen, world, cam);
	}

	//Joins rendering threads
	for (int k = 0; k < nthreads; ++k) {
		t[k].join();
	}

	//Saves only if the render completed
	if (*chunksRemaining == 0) {
		t[0] = std::thread(save, image);
		t[0].join();
	}

	t[nthreads].join();

	//Cleanup
	terminateWindow();
	delete[] world;
	delete[] image->outputArray;
	delete windowOpen;
}

//Color of pixel
vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			return vec3(0, 0, 0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0);
		return (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}

void save(image_parameters* image) {

	//Required for console output
	FILE *stream;

	std::cout << "Saving..." << std::endl;

	//Sends console output to file out.ppm 
	freopen_s(&stream, "out.ppm", "w", stdout);

	//PPM file header
	std::cout << "P3\n" << image->nx << " " << image->ny << "\n255\n";

	//Outputs colours as required by file format
	for (int j = image->ny - 1; j >= 0; j--) {
		for (int i = 0; i < image->nx; i++) {

			int ir = image->outputArray[((i + (image->nx * j)) * 4)];
			int ig = image->outputArray[((i + (image->nx * j)) * 4) + 1];
			int ib = image->outputArray[((i + (image->nx * j)) * 4) + 2];

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}

	//Closes out.ppm file at end out output.
	fclose(stdout);

	//Exports as .jpg - loses alpha channel data. 
	stbi_flip_vertically_on_write(1);
	stbi_write_jpg("out.jpg", image->nx, image->ny, 4, image->outputArray, 100);
}

void render(image_parameters* image, int k, int* chunksRemaining, bool *windowOpen, hitable* world, camera cam) {
	
	//Checks list has contents
	while (&chunksRemaining != 0 && *windowOpen) {

		//Locks list from access by other threads
		chunkList_mutex.lock();
		//Removes first chunk on the listx

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
					col += color(r, world, 0);

				}

				col /= float(image->ns);

				//Color correction
				col = vec3(sqrt(col.r()), sqrt(col.g()), sqrt(col.b()));

				int ir = int(255.99*col[0]);
				int ig = int(255.99*col[1]);
				int ib = int(255.99*col[2]);

				outputArray_mutex.lock();
				image->outputArray[(i + (image->nx * j)) * 4] = ir;
				image->outputArray[((i + (image->nx * j)) * 4) + 1] = ig;
				image->outputArray[((i + (image->nx * j)) * 4) + 2] = ib;
				image->outputArray[((i + (image->nx * j)) * 4) + 3] = 1;
				outputArray_mutex.unlock();


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
	//Close thread
	console_mutex.lock();
	std::cout << "Thread: " << k << " finished." << std::endl;
	console_mutex.unlock();
}