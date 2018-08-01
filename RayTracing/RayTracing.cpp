#include "stdafx.h"
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
#include <algorithm>


//Required for console output?
FILE *stream;

//gets number of threads of systems
unsigned const int nthreads =8;// std::thread::hardware_concurrency();

//Container for list of chunks to be rendered
std::list<chunk> chunkList;
std::mutex chunkList_mutex;

//Defines size of image
int nx = 200;
int ny = 100;

//Rays per pixel
int ns = 100;

//Container for output of render
vec3* outputMat;
std::mutex outputMat_mutex;

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

void render(int k, hitable* world, camera cam) {

	int m_width = nx;

	//Checks list has contents
	while (chunkList.size() != 0) {

		
		//Locks list from access by other threads
		chunkList_mutex.lock();
		//Removes first chunk on the list
		chunk toBeRendered = chunkList.front();
		chunkList.pop_front();

		//DEBUG
		std::cout << "sX: " << toBeRendered.startX() << " eX: " << toBeRendered.endX() << " sY: " << toBeRendered.startY() << " eY: " << toBeRendered.endY() << std::endl;

		//Unlocks list
		chunkList_mutex.unlock();

		for (int j = int(toBeRendered.startY()); j >= int(toBeRendered.endY()); j--) {

			for (int i = int(toBeRendered.endX()); i <= int(toBeRendered.startX()); i++) {

				vec3 col(0, 0, 0);
				for (int s = 0; s < toBeRendered.ns(); s++) {

					//Applies anti-aliasing with the random number here - very slow!
					float u = float(i + (rand() / (RAND_MAX + 1.0))) / float(m_width);
					float v = float(j + (rand() / (RAND_MAX + 1.0))) / float(ny);
					ray r = cam.get_ray(u, v);
					vec3 p = r.point_at_parameter(2.0);
					col += color(r, world, 0);

				}

				col /= float(toBeRendered.ns());


				//Stores final pixel value in buffer
				outputMat_mutex.lock();
				outputMat[i + (m_width * j)] = col;
				outputMat_mutex.unlock();

			}
		}

	}
	//Close thread
	std::cout << "Thread: " << k << " finished." << std::endl;
}

int main()
{
	//Output buffer - *2 to stop memory access violations
	outputMat = new vec3[(nx + 2)*(ny + 2)];

	//Initialises threads
	std::thread t[nthreads];

	//Splits the image into n*n squares, each to be rendered by an independent thread
	//Sets size of chunk
	int chunkSize = 50;

	int xChunks = ((nx + chunkSize - 1) / chunkSize);
	int yChunks = ((ny + chunkSize - 1) / chunkSize);

	std::cout << "no. of chunks: " << xChunks << " " << yChunks << std::endl;
	std::list<chunk>::iterator it;

	//Add chunks to render list;
	it = chunkList.begin();

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

			chunkList.insert(it, tempChunk);
			it--;
			std::cout << "List length: " << chunkList.size() << std::endl;
		}
	}

	//Builds some objects to render
	hitable *list[4];
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.3));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new metal(vec3(0.8, 0.8, 0.8), 1.0));

	//Builds a world from the objects
	hitable *world = new hitable_list(list, 4);

	//Camera to render through
	camera cam;


	//Renders out the chunks
	for (int k = 0; k < nthreads; k++) {
		t[k] = std::thread(render, k, world, cam);
	}
	for (int k = 0; k < nthreads; ++k) {
		t[k].join();
	}

	//Stops main thread from saving if rendering is still going!

	std::cout << "Saving..." << std::endl;

	//Sends console output to file out.ppm 
	freopen_s(&stream, "out.ppm", "w", stdout);

	//PPM file header
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	//Outputs colours as required by file format
	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {

			vec3 col = outputMat[i + (nx * j)];

			//Colour correction
			col = vec3(sqrt(col.r()), sqrt(col.g()), sqrt(col.b()));

			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";

		}
	}
	//Closes out.ppm file at end out output.
	fclose(stdout);
	
	//Cleanup
	delete[] outputMat;
	delete[] world;

}