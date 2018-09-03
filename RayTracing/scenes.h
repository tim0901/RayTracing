#pragma once

#ifndef SCENES_H
#define SCENES_H

//Stores various scenes for testing

//Generates a scene full of spheres for testing
hitable* random_scene() {
	int n = 500;
	hitable **list = new hitable*[n + 1];

	texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));

	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = drand48();
			vec3 center(a + 0.9*drand48(), 0.2, b + 0.9*drand48());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new sphere(center, 0.2, new lambertian(new constant_texture(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48()))));
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(center, 0.2,
						new metal(vec3(0.5*(1 + drand48()), 0.5*(1 + drand48()), 0.5*(1 + drand48())), 0.5*drand48()));
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(vec3(1.0, 1.0, 1.0), 1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(vec3(1.0, 1.0, 1.0), 1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	//return new hitable_list(list, i);
	return new bvh_node(list, i, 0.0, 0.0);
}

hitable *two_spheres() {
	texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));
	int n = 50;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -10, 0), 10, new lambertian(checker));
	list[1] = new sphere(vec3(0, 10, 0), 10, new lambertian(checker));

	return new hitable_list(list, 2);
}

hitable *test_spheres() {

	int n = 50;
	hitable **list = new hitable*[n + 1];

	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(new constant_texture(vec3(0.1, 0.2, 0.5))));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(new constant_texture(vec3(0.8, 0.8, 0.0))));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(vec3(1.0, 1.0, 1.0), 1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -0.495, new dielectric(vec3(1.0, 1.0, 1.0), 1.5));
	list[5] = new sphere(vec3(-1, 0, 1), 0.5, new lambertian(new constant_texture(vec3(0.3, 0.6, 0.5))));

	//Builds a world from the objects
	return new hitable_list(list, 6);
}

hitable *two_perlin_spheres() {
	texture *pertext = new noise_texture(4);
	hitable**list = new hitable*[2];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
	return new hitable_list(list, 2);
}

#endif // !RANDOM_SCENE_H