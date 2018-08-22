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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"

//Declare functions
void save();
hitable* random_scene();
vec3 color(const ray& r, hitable *world, int depth);
void render(int k, hitable* world, camera cam);

//Required for console output
FILE *stream;

//gets number of threads of systems
unsigned const int nthreads = 8;// std::thread::hardware_concurrency();

//Container for list of chunks to be rendered
std::list<chunk> chunkList;
std::mutex chunkList_mutex;

//Defines size of image
int nx = 500;
int ny = 500;

//Rays per pixel
int ns = 1;

//Thread iterator
int n = 0;

unsigned char *outputArray = { 0 };
unsigned char **outputArrayPtr = &outputArray;

std::mutex outputArray_mutex;

std::mutex console_mutex;

//For checking if the viewport should be open
bool *windowOpen = new bool(true);

//Initialises threads
std::thread t[nthreads];

//Generates a scene full of spheres for testing
hitable* random_scene() {
	int n = 500;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = (rand() / (RAND_MAX + 1.0));
			vec3 center(a + 0.9* (rand() / (RAND_MAX + 1.0)), 0.2, b + 0.9* (rand() / (RAND_MAX + 1.0)));
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) { //diffuse material
					list[i++] = new sphere(center, 0.2, new lambertian(vec3((rand() / (RAND_MAX + 1.0)) *  (rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0))* (rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0))* (rand() / (RAND_MAX + 1.0)))));

				}
				else if (choose_mat < 0.95) { //metal material
					list[i++] = new sphere(center, 0.2, new metal(vec3(0.5*(1 + (rand() / (RAND_MAX + 1.0))), 0.5*(1 + (rand() / (RAND_MAX + 1.0))), 0.5*(1 + (rand() / (RAND_MAX + 1.0)))), 0.5* (rand() / (RAND_MAX + 1.0))));

				}
				else { //dielectric material
					list[i++] = new sphere(center, 0.2, new dielectric(vec3(1.0, 1.0, 1.0), 1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(vec3(1.0, 1.0, 1.0), 1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list, i);
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

void save() {

	std::cout << "Saving..." << std::endl;

	//Sends console output to file out.ppm 
	freopen_s(&stream, "out.ppm", "w", stdout);

	//PPM file header
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	//Outputs colours as required by file format
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {

			int ir = outputArray[((i + (nx * j)) * 4)];
			int ig = outputArray[((i + (nx * j)) * 4) + 1];
			int ib = outputArray[((i + (nx * j)) * 4) + 2];

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
	//Closes out.ppm file at end out output.
	fclose(stdout);

	//Exports as .jpg - loses alpha channel data. Occasionally crashes with read access violation. Unsure of cause. 
	//stbi_flip_vertically_on_write(1);
	//stbi_write_jpg("out.jpg", nx, ny, 4, outputArray, 100);
	n--;
}

void render(int k, hitable* world, camera cam) {

	//Checks list has contents
	while (chunkList.size() != 0 && *windowOpen) {

		//Locks list from access by other threads
		chunkList_mutex.lock();
		//Removes first chunk on the list
		chunk toBeRendered = chunkList.front();
		chunkList.pop_front();

		//DEBUG
		console_mutex.lock();
		std::cout << "sX: " << toBeRendered.startX() << " eX: " << toBeRendered.endX() << " sY: " << toBeRendered.startY() << " eY: " << toBeRendered.endY() << std::endl;
		console_mutex.unlock();

		//Unlocks list
		chunkList_mutex.unlock();

		for (int j = int(toBeRendered.startY()); j >= int(toBeRendered.endY()); j--) {

			for (int i = int(toBeRendered.endX()); i <= int(toBeRendered.startX()); i++) {

				vec3 col(0, 0, 0);
				for (int s = 0; s < toBeRendered.ns(); s++) {

					//Applies anti-aliasing with the random number here - very slow!
					float u = float(i + (rand() / (RAND_MAX + 1.0))) / float(nx);
					float v = float(j + (rand() / (RAND_MAX + 1.0))) / float(ny);
					ray r = cam.get_ray(u, v);
					//vec3 p = r.point_at_parameter(2.0);
					col += color(r, world, 0);

				}

				col /= float(toBeRendered.ns());

				//Color correction
				col = vec3(sqrt(col.r()), sqrt(col.g()), sqrt(col.b()));


				int ir = int(255.99*col[0]);
				int ig = int(255.99*col[1]);
				int ib = int(255.99*col[2]);

				outputArray_mutex.lock();
				outputArray[(i + (nx * j)) * 4] = ir;
				outputArray[((i + (nx * j)) * 4) + 1] = ig;
				outputArray[((i + (nx * j)) * 4) + 2] = ib;
				outputArray[((i + (nx * j)) * 4) + 3] = 1;
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
	}
	//Close thread
	console_mutex.lock();
	std::cout << "Thread: " << k << " finished." << std::endl;
	console_mutex.unlock();
	n--;
}

int main()
{
	//Initialise viewport
	t[nthreads] = std::thread(initialiseWindow, nx, ny, n, outputArrayPtr, windowOpen);

	//Allocate output array
	*outputArrayPtr = (unsigned char*)malloc((nx + 2) * (ny + 2) * 4);

	//Splits the image into n*n squares, each to be rendered by an independent thread
	//Sets size of chunk
	//Sometimes crashes if chunkSize < 10, unsure how to solve. Performance scales badly at this level so is unadvised anyway.
	int chunkSize = 50;

	int xChunks = ((nx + chunkSize - 1) / chunkSize);
	int yChunks = ((ny + chunkSize - 1) / chunkSize);

	//Add chunks to render list;

	for (int i = 0; i < xChunks; i++) {
		for (int j = 0; j < yChunks; j++) {

			chunk tempChunk = chunk(nx - i * chunkSize - 1, nx - (i + 1)*chunkSize, ny - j * chunkSize + 1, ny - (j + 1)*chunkSize, ns);

			//Deals with edges of strangely sized images
			if (tempChunk.endX() < 0) {

				tempChunk = chunk(tempChunk.startX(), 0, tempChunk.startY(), tempChunk.endY(), tempChunk.ns());

			}
			if (tempChunk.endY() < 0) {

				tempChunk = chunk(tempChunk.startX(), tempChunk.endX(), tempChunk.startY(), 0, tempChunk.ns());

			}

			chunkList.push_front(tempChunk);
		}
	}
	std::cout << "no. of chunks: " << xChunks << " * " << yChunks << " = " << chunkList.size() << std::endl;

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
	//world = random_scene();

	//Camera to render through

	vec3 lookfrom(-2, 2, 1);
	vec3 lookat(0, 0, -1);
	float dist_to_focus = (lookfrom - lookat).length();
	float aperture = 0.1;
	float aspect_ratio = float(nx) / float(ny);

	camera cam(lookfrom, lookat, vec3(0, 1, 0), 30, aspect_ratio, aperture, dist_to_focus);
	*windowOpen = true;

	//Renders out the chunks
	for (int k = 0; k < (nthreads); k++) {
		t[k] = std::thread(render, k, world, cam);
		n++;
	}

	//Joins rendering threads
	for (int k = 0; k < nthreads; ++k) {
		t[k].join();
		std::cout << "Thread " << k << " joined." << std::endl;
	}

	//Saves only if the render completed
	if (chunkList.size() == 0) {
		n++;
		t[0] = std::thread(save);
		t[0].join();
	}

	t[nthreads].join();

	//Cleanup
	terminateWindow();
	delete[] world;
	delete[] outputArray;
	delete windowOpen;
}
