#pragma once

#ifndef SCENES_H
#define SCENES_H


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "aarect.h"
#include "box.h"
#include "texture.h"
#include "constant_medium.h"
#include "mesh.h"

//Stores various scenes for testing
void simple_light(hitable **world, hitable **light_list, image_parameters *image, camera **cam) {

	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0,1,0);
	vec3 vup(0, 1, 0); // vector that is "up" for the camera
	float dist_to_focus = 10.0; //(lookfrom - lookat).length();
	int fov = 25;
	float aperture = 0.0;
	float startTime = 0.0;
	float endTime = 1.0;
	float aspect_ratio = float(image->nx) / float(image->ny);
	*cam = new camera(lookfrom, lookat, vup, fov, aspect_ratio, aperture, dist_to_focus, startTime, endTime);

	hitable **list = new hitable*[4];
	int i = 0;
	texture *pertext = new noise_texture(4);
	list[i++] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
	list[i++] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
	//list[i++] = new sphere(vec3(0, 7, 0), 2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	list[i++] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	
	*world = new hitable_list(list, i);
	int j = 0;
	hitable **a = new hitable*[2];
	a[j++] = new xz_rect(3, 5, 1, 3, -2, 0);
	*light_list = new hitable_list(a, j);
}

void cornell_box(hitable **world, hitable **light_list, image_parameters *image,camera **cam) {

	vec3 lookfrom(278, 278, -800);
	vec3 lookat(278, 278, 0);
	vec3 vup(0, 1, 0); // vector that is "up" for the camera
	float dist_to_focus = 10.0; //(lookfrom - lookat).length();
	int fov = 40;
	float aperture = 0.0;
	float startTime = 0.0;
	float endTime = 1.0; 
	float aspect_ratio = float(image->nx) / float(image->ny);
	*cam = new camera(lookfrom, lookat, vup, fov, aspect_ratio, aperture, dist_to_focus, startTime, endTime);

	hitable **list = new hitable*[9];
	int i = 0;
	material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
	material *light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new flip_normals(new xz_rect(213, 343, 227, 332, 554, light));
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
	material *aluminium = new metal(vec3(0.8, 0.85, 0.88), 0.0);
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), aluminium), 15), vec3(265, 0, 295));
	
	*world = new hitable_list(list, i);

	int j = 0;
	hitable **a = new hitable*[2];
	a[j++] = new xz_rect(213, 343, 227, 332, 554, 0);
	*light_list = new hitable_list(a, j);
}


void cornell_box_final(hitable **world, hitable **light_list, image_parameters *image, camera **cam) {

	//Camera
	vec3 lookfrom(278, 278, -800);
	vec3 lookat(278, 278, 0);

	vec3 vup(0, 1, 0); // vector that is "up" for the camera
	float dist_to_focus = 10.0; //(lookfrom - lookat).length();
	int fov = 40;
	float aperture = 0.0;
	float startTime = 0.0;
	float endTime = 1.0;
	float aspect_ratio = float(image->nx) / float(image->ny);
	*cam = new camera(lookfrom, lookat, vup, fov, aspect_ratio, aperture, dist_to_focus, startTime, endTime);

	//Materials
	hitable **list = new hitable*[8];
	hitable **temp = new hitable*[1];
	material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
	material *light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));
	material *aluminium = new metal(vec3(0.8, 0.85, 0.88), 0.0);
	material *redmetal = new metal(vec3(0.65, 0.05, 0.05), 0.0);
	material *glass = new dielectric(1.5);

	//Map texture
	int nx, ny, nn;
	unsigned char *tex_data = stbi_load("earth-map.jpg", &nx, &ny, &nn, 0);
	material *emat = new lambertian(new image_texture(tex_data, nx, ny));

	int i = 0;
	//Walls
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	
	//Top Light
	list[i++] = new flip_normals(new xz_rect(203, 353, 237, 322, 554, light));

	//Globe
	//list[i++] = new sphere(vec3(90, 90, 350), 50, emat);

	//Glass Sphere
	//list[i++] = new sphere(vec3(190, 90, 190), 90, glass);
	//list[i++] = new sphere(vec3(190, 89, 190), -89, glass);

	//Tall Box
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

	//Mesh

	mesh *test = new mesh(load_mesh("minicooper.obj", red));

	temp[0] = new translate(new rotate_z(new rotate_x(new rotate_y(test, -90), 130), -90), vec3(150, 0, 240));
	list[i++] = new bvh_node(temp, 1, 0, 1);
	
	*world = new hitable_list(list, i);

	//Lights
	int j = 0;
	hitable **a = new hitable*[2];
	a[j++] = new xz_rect(203, 353, 237, 322, 554, 0);
	//a[j++] = new sphere(vec3(190, 90, 190), 90, 0);
	//a[j++] = new sphere(vec3(190, 89, 190), -89, 0);
	
	//a[j++] = new bvh_node(temp, 1, 0, 1);//new translate(new rotate_z(new rotate_x(new rotate_y(test, -90), 0), -90), vec3(285, 250, 295));
	
	//a[j++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), 0), 15), vec3(265, 0, 295));
	*light_list = new hitable_list(a, j);
}

void final(hitable **world, hitable **light_list, image_parameters *image, camera **cam) {

	vec3 lookfrom(478, 278, -600);
	vec3 lookat(278, 278, 0);
	vec3 vup(0, 1, 0); // vector that is "up" for the camera
	float dist_to_focus = 10;//(lookfrom - lookat).length();
	int fov = 40;
	float aperture = 0.0;
	float startTime = 0.0;
	float endTime = 1.0;
	float aspect_ratio = float(image->nx) / float(image->ny);
	*cam = new camera(lookfrom, lookat, vup, fov, aspect_ratio, aperture, dist_to_focus, startTime, endTime);


	int nb = 20;
	hitable **list = new hitable*[30];
	hitable **boxlist = new hitable*[10000];
	hitable **boxlist2 = new hitable*[10000];
	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *ground = new lambertian(new constant_texture(vec3(0.48, 0.83, 0.53)));
	int b = 0;
	for (int i = 0; i < nb; i++) {
		for (int j = 0; j < nb; j++) {
			float w = 100;
			float x0 = -1000 + i * w;
			float z0 = -1000 + j * w;
			float y0 = 0;
			float x1 = x0 + w;
			float y1 = 100 * (drand48() + 0.01);
			float z1 = z0 + w;
			boxlist[b++] = new box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
		}
	}
	int l = 0;
	list[l++] = new bvh_node(boxlist, b, 0, 1);
	material *light = new diffuse_light(new constant_texture(vec3(7, 7, 7)));
	list[l++] = new flip_normals(new xz_rect(123, 423, 147, 412, 554, light));
	vec3 center(400, 400, 200);
	list[l++] = new moving_sphere(center, center + vec3(30, 0, 0), 0, 1, 50, new lambertian(new constant_texture(vec3(0.7, 0.3, 0.1))));
	list[l++] = new sphere(vec3(260, 150, 45), 50, new dielectric(1.5));
	list[l++] = new sphere(vec3(0, 150, 145), 50, new metal(vec3(0.8, 0.8, 0.9), 10.0));
	hitable *boundary = new sphere(vec3(360, 150, 145), 70, new dielectric(1.5));
	list[l++] = boundary;
	list[l++] = new constant_medium(boundary, 0.2, new constant_texture(vec3(0.2, 0.4, 0.9)));
	boundary = new sphere(vec3(0, 0, 0), 5000, new dielectric(1.5));
	list[l++] = new constant_medium(boundary, 0.0001, new constant_texture(vec3(1.0, 1.0, 1.0)));
	int nx, ny, nn;
	unsigned char *tex_data = stbi_load("earth-map.jpg", &nx, &ny, &nn, 0);
	material *emat = new lambertian(new image_texture(tex_data, nx, ny));
	list[l++] = new sphere(vec3(400, 200, 400), 100, emat);
	texture *pertext = new noise_texture(0.1);
	list[l++] = new sphere(vec3(220, 280, 300), 80, new lambertian(pertext));
	int ns = 1000;
	for (int j = 0; j < ns; j++) {
		boxlist2[j] = new sphere(vec3(165 * drand48(), 165 * drand48(), 165 * drand48()), 10, white);
	}
	list[l++] = new translate(new rotate_y(new bvh_node(boxlist2, ns, 0.0, 1.0), 15), vec3(-100, 270, 395));
	*world = new hitable_list(list, l);


	int k = 0;
	hitable **a = new hitable*[3];
	a[k++] = new xz_rect(123, 423, 147, 412, 554, 0);
	a[k++] = new sphere(vec3(260, 150, 45), 50, 0);
	a[k++] = new sphere(vec3(360, 150, 145), 70, 0);
	*light_list = new hitable_list(a, k);
}

void triangle_test(hitable **world, hitable **light_list, image_parameters *image, camera **cam) {

	vec3 lookfrom(-15, -5, -5);
	vec3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0); // vector that is "up" for the camera
	float dist_to_focus = 10.0; //(lookfrom - lookat).length();
	int fov = 25;
	float aperture = 0.0;
	float startTime = 0.0;
	float endTime = 1.0;
	float aspect_ratio = float(image->nx) / float(image->ny);
	*cam = new camera(lookfrom, lookat, vup, fov, aspect_ratio, aperture, dist_to_focus, startTime, endTime);
	
	int i = 0;
	hitable **list = new hitable*[80];
	material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *blue = new lambertian(new constant_texture(vec3(0.05, 0.05, 0.65)));
	material *green = new lambertian(new constant_texture(vec3(0.05, 0.65, 0.05)));
	texture *checker = new checker_texture(new constant_texture(vec3(0.05, 0.05, 0.65)), new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *mat = new lambertian(checker);
	material *light = new diffuse_light(new constant_texture(vec3(5, 5, 5)));
	int nx, ny, nn;
	unsigned char *tex_data = stbi_load("earth-map.jpg", &nx, &ny, &nn, 0);
	material *emat = new lambertian(new image_texture(tex_data, nx, ny));

	vec3* point0 = new vec3(1, 0, -1);
	vec3* point1 = new vec3(1, 0.5, -0.5);
	vec3* point2 = new vec3(1, 0, 0);
	
	
	list[i++] = new xy_rect(-3, 7, -1, 8, -2, light);
	//list[i++] = new mesh(light);
	list[i++] = new sphere(vec3(0, 0, 0), 0.2f, emat);
	list[i++] = new sphere(vec3(2, 0, 0), 0.2f, red);
	list[i++] = new sphere(vec3(0, 2, 0), 0.2f, blue);
	list[i++] = new sphere(vec3(0, 0, 2), 0.2f, green);

	*world = new hitable_list(list, i);

	int j = 0;
	hitable **a = new hitable*[3];
	a[j++] = new xz_rect(-3, 7, -1, 8, -2, 0);
	*light_list = new hitable_list(a, j);
}

void testing(hitable **world, hitable **light_list, image_parameters *image, camera **cam) {
	vec3 lookfrom(15, 5, 5);
	vec3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0); // vector that is "up" for the camera
	float dist_to_focus = 10.0; //(lookfrom - lookat).length();
	int fov = 25;
	float aperture = 0.0;
	float startTime = 0.0;
	float endTime = 1.0;
	float aspect_ratio = float(image->nx) / float(image->ny);
	*cam = new camera(lookfrom, lookat, vup, fov, aspect_ratio, aperture, dist_to_focus, startTime, endTime);

	int i = 0;
	hitable **list = new hitable*[80];
	material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *blue = new lambertian(new constant_texture(vec3(0.05, 0.05, 0.65)));
	material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
	material *green = new lambertian(new constant_texture(vec3(0.05, 0.65, 0.05)));
	texture *checker = new checker_texture(new constant_texture(vec3(0.05, 0.05, 0.65)), new constant_texture(vec3(0.65, 0.05, 0.05)));
	material *mat = new lambertian(checker);
	material *light = new diffuse_light(new constant_texture(vec3(5, 5, 5)));
	int nx, ny, nn; 
	//unsigned char *tex_data = stbi_load("earth-map.jpg", &nx, &ny, &nn, 0);
	//material *emat = new lambertian(new image_texture(tex_data, nx, ny));

	list[i++] = new rotate_y(new mesh(load_mesh("bunny.obj", red)),180);
	
	//list[i++] = new sphere(vec3(0, 0, 0), 0.2f, emat);
	//list[i++] = new sphere(vec3(2, 0, 0), 0.2f, red);
	//list[i++] = new sphere(vec3(0, 2, 0), 0.2f, blue);
	//list[i++] = new sphere(vec3(0, 0, 2), 0.2f, green);

	list[i++] = new xy_rect(-3, 7, -1, 8, -5, light);

	*world = new hitable_list(list, i);

	int j = 0;
	hitable **a = new hitable*[3];
	a[j++] = new xz_rect(-3, 7, -1, 8, -5, 0);
	*light_list = new hitable_list(a, j);
}





#endif // !RANDOM_SCENE_H