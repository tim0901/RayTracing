#pragma once

#ifndef RAY_H
#define RAY_H

#include "vec3.h"

//defines the point p =  A + B*t
// A = origin, B = direction, t = time

class ray {

public:

	vec3 A, B;
	float _time;

	ray() {};
	ray(const vec3& a, const vec3& b, float ti = 0.0) { A = a; B = b; _time = ti; }
	vec3 origin() const { return A; }
	vec3 direction() const { return B; }
	float time() const { return _time; }
	vec3 point_at_parameter(float t) const { return A + t * B; }
};

#endif //RAY_H