#pragma once

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vec2.h"

class mesh;

class triangle :public hitable {
public:
	triangle() {}
	triangle(vec3* p0, vec3* p1, vec3* p2, material *m, vec3* n0, vec3* n1, vec3* n2) : point0(p0), point1(p1), point2(p2), mat_ptr(m), normal0(n0), normal1(n1), normal2(n2) {}
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;
	
	virtual float pdf_value(const vec3& o, const vec3& v) const {
		hit_record rec;
		if (this->hit(ray(o, v), 0.001, FLT_MAX, rec)) {			
			float area = ((*point1 - *point0).length())*((*point2 - *point0).length())*0.5;
			//float distance_squared = rec.t*rec.t*v.squared_length();
			//float cosine = fabs(dot(v, rec.normal) / v.length());
			
			return 1 / area;
			//return (distance_squared / (cosine*area));
		}
		else {
			return 0;
		}
	}
	virtual vec3 random(const vec3& o) const {

		float a = drand48();
		float b = drand48();
		float su0 = sqrt(a);
		vec3 random_on_triangle = vec3(1 - su0, b*su0, 0);

		vec3 random_point = *point0 + a*(*point1 - *point0) + b*(*point2 - *point0); //(*point0 + drand48() * (*point1-*point0), *point2 + drand48()*(*point2-*point1), *point1 + drand48()*(*point2 - *point1));
		return random_on_triangle - o;
		
	}
	vec3* point0;
	vec3* point1;
	vec3* point2;
	vec3* normal0;
	vec3* normal1;
	vec3* normal2;
	material *mat_ptr;
};

bool triangle::hit(const ray& r, float tmin, float tmax, hit_record& rec) const {
	
	/*
	//Rejects back facing triangles
	vec3 edge1 = *point1 - *point0;//020 022
	vec3 edge2 = *point2 - *point0;//022 002
	//cross product ray direction and edge 2
	vec3 pvec = cross(r.direction(), edge2);
	float test = dot(edge1, pvec);
	const float EPSILON = 1e-8;
	if (test < EPSILON) {
		return false;
	}
	else if (fabs(test) < EPSILON) {
		//reject if ray and triangle are parallel
		return false;
	}*/

	// Translate vertices based on ray origin
	vec3 p0 = *point0 - vec3(r.origin());
	vec3 p1 = *point1 - vec3(r.origin());
	vec3 p2 = *point2 - vec3(r.origin());

	// Permute components of triangle vertices and ray direction
	int kz = max_dimension(abs(r.direction()));
	int kx = kz + 1;
	if (kx == 3) kx = 0;
	int ky = kx + 1;
	if (ky == 3) ky = 0;
	vec3 d = permute(r.direction(), kx, ky, kz);
	p0 = permute(p0, kx, ky, kz);
	p1 = permute(p1, kx, ky, kz);
	p2 = permute(p2, kx, ky, kz);

	// Apply shear transformation to translated vertex positions
	float Sx = -d.x() / d.z();
	float Sy = -d.y() / d.z();
	float Sz = 1.f / d.z();
	p0[0] += Sx * p0.z();
	p0[1] += Sy * p0.z();
	p1[0] += Sx * p1.z();
	p1[1] += Sy * p1.z();
	p2[0] += Sx * p2.z();
	p2[1] += Sy * p2.z();

	// Compute edge function coefficients _e0_, _e1_, and _e2_
	float e0 = p1.x() * p2.y() - p1.y() * p2.x();
	float e1 = p2.x() * p0.y() - p2.y() * p0.x();
	float e2 = p0.x() * p1.y() - p0.y() * p1.x();

	// Fall back to double precision test at triangle edges
	if (sizeof(float) == sizeof(float) &&
		(e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)) {
		double p2xp1y = (double)p2.x() * (double)p1.y();
		double p2yp1x = (double)p2.y() * (double)p1.x();
		e0 = (float)(p2yp1x - p2xp1y);
		double p0xp2y = (double)p0.x() * (double)p2.y();
		double p0yp2x = (double)p0.y() * (double)p2.x();
		e1 = (float)(p0yp2x - p0xp2y);
		double p1xp0y = (double)p1.x() * (double)p0.y();
		double p1yp0x = (double)p1.y() * (double)p0.x();
		e2 = (float)(p1yp0x - p1xp0y);
	}

	// Perform triangle edge and determinant tests
	if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
		return false;
	float det = e0 + e1 + e2;
	if (det == 0) return false;

	// Compute scaled hit distance to triangle and test against ray $t$ range
	p0[2] *= Sz;
	p1[2] *= Sz;
	p2[2] *= Sz;
	float tScaled = e0 * p0.z() + e1 * p1.z() + e2 * p2.z();
	
	if (det < 0 && (tScaled >= 0 || tScaled < FLT_MAX * det))
		return false;
	else if (det > 0 && (tScaled <= 0 || tScaled > FLT_MAX * det))
		return false;

	// Compute barycentric coordinates and $t$ value for triangle intersection
	float invDet = 1 / det;
	float b0 = e0 * invDet;
	float b1 = e1 * invDet;
	float b2 = e2 * invDet;
	float t = tScaled * invDet;

	// Compute triangle partial derivatives
	vec3 dpdu, dpdv;
	vec2 uv[3];
	//TODO proper UV calculation
	uv[0] = vec2(0, 0);
	uv[1] = vec2(1, 0);
	uv[2] = vec2(1, 1);

	// Compute deltas for triangle partial derivatives
	vec2 duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
	vec3 dp02 = *point0 - *point2, dp12 = *point1 - *point2;
	float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
	bool degenerateUV = std::abs(determinant) < 1e-8;
	if (!degenerateUV) {
		float invdet = 1 / determinant;
		dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
		dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
	}
	if (degenerateUV || cross(dpdu, dpdv).squared_length() == 0) {
		// Handle zero determinant for triangle partial derivative matrix
		vec3 ng = cross(*point2 - *point0, *point1 - *point0);
		if (ng.squared_length() == 0)
			// The triangle is actually degenerate; the intersection is
			// bogus.
			return false;

		coordinate_system(unit_vector(ng), &dpdu, &dpdv);
	}
	
	// Interpolate $(u,v)$ parametric coordinates and hit point
	vec3 pHit = b0 * *point0 + b1 * *point1 + b2 * *point2;
	vec2 uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];

	rec.mat_ptr = mat_ptr;
	rec.p = pHit;
	rec.t = t;
	rec.normal = vec3(0, 0, 0);
	rec.u = uvHit.u();
	rec.v = uvHit.v();
	
	//broken?
	if (normal0 == (0,0,0)) {
		rec.normal = unit_vector(b0* *normal0 + b1 * *normal1 + b2 * *normal2);
	}
	else {
		rec.normal = vec3(unit_vector(cross(dp02, dp12)));
	}


	return true;
}

bool triangle::bounding_box(float t0, float t1, aabb& box) const {
	vec3 p0 = *point0;
	vec3 p1 = *point1;
	vec3 p2 = *point2;

	vec3 pmin(std::min(p0.x(), std::min(p1.x(), p2.x())), std::min(p0.y(), std::min(p1.y(), p2.y())), std::min(p0.z(), std::min(p1.z(), p2.z())));
	vec3 pmax(std::max(p0.x(), std::max(p1.x(), p2.x())), std::max(p0.y(), std::max(p1.y(), p2.y())), std::max(p0.z(), std::max(p1.z(), p2.z())));
	
	box = aabb(pmin, pmax);
	return true;
}

#endif // !TRIANGLE_H
