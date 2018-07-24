// RayTracing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include <cfloat>
#include "camera.h"
#include "material.h"
#include <thread>


//Required for console output?
FILE *stream;

//gets number of threads of systems
unsigned const int nthreads = 8;// std::thread::hardware_concurrency();

static vec3 image[800][400];




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

void render(int k, int chunksize[nthreads], int remainder, int nx, int ny, hitable* world, int ns) {

	camera cam;
	int startingRow = ny;

	if (k != 0) {
		for (int i = 0; i < k; i++) {

			startingRow -= chunksize[i];

		}
	}
	for (int j = startingRow; j >= startingRow - chunksize[k]; j--) {
		
		for (int i = 0; i < nx; i++) {

			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {

				//Applies anti-aliasing with the random number here - very slow!
				float u = float(i + (rand() / (RAND_MAX + 1.0))) / float(nx);
				float v = float(j + (rand() / (RAND_MAX + 1.0))) / float(ny);
				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);

				//col = render(cam, i, j, nx, ny, col, world);

			}

			col /= float(ns);
			image[i][j] = col;

		}
	}
	std::cout << "Thread: " << k << " finished." << std::endl;
}

int main()
{
	std::thread t[nthreads];

	//Defines size of image
	int nx = 200;
	int ny = 100;
	//Rays per pixel
	int ns = 100;

	//Chunksize for each thread
	int chunksize[8];

	hitable *list[4];
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.3));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new metal(vec3(0.8, 0.8, 0.8), 1.0));

	hitable *world = new hitable_list(list, 4);

	for (int i = 0; i < nthreads; i++) {

		chunksize[i] = ny / nthreads;

	}
	int remainder = ny % nthreads;

	for (int i = remainder; i != 0; i--) {

		chunksize[i]++;

	}


	//Image information
	for (int k = 0; k < nthreads; k++) {

		t[k] = std::thread(render, k, chunksize, remainder, nx, ny, world, ns);

	}
	for (int k = 0; k < nthreads; ++k) {
		t[k].join();
	}

	//Sends console output to file out.ppm 
	freopen_s(&stream, "out.ppm", "w", stdout);


	//PPM header
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";



	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {

			vec3 col = vec3(sqrt(image[i][j][0]), sqrt(image[i][j][1]), sqrt(image[i][j][2]));

			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";

		}
	}

	//Closes out.ppm file at end out output.
	fclose(stdout);
}
