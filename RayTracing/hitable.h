#pragma once

#ifndef HITABLEH
#define HITABLEH
#endif // HITABLEH

#include "ray.h"

class material;

//A bunch of arguments together in a struct to be easily sent together
struct hit_record {
	float t;
	vec3 p;
	vec3 normal;
	material *mat_ptr;
};

class hitable {
public:
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;

};