#pragma once

#ifndef VEC3_H
#define VEC3_H


#include <math.h>
#include <stdlib.h>
#include <iostream>

//This class defines a vector3 structure and some basic vector operations

class vec3 {
public:

	vec3() {}
	vec3(double e0, double e1, double e2) { e[0] = e0; e[1] = e1; e[2] = e2; }

	//Can use x, y, z or r, g, b to return stored values - vec3 can be used to store either.
	inline double x() const { return e[0]; }
	inline double y() const { return e[1]; }
	inline double z() const { return e[2]; }
	inline double r() const { return e[0]; }
	inline double g() const { return e[1]; }
	inline double b() const { return e[2]; }


	//Defining vector operators
	inline const vec3& operator + () const { return *this; }
	inline vec3 operator - () const { return vec3(-e[0], -e[1], -e[2]); }
	inline double operator [] (int i) const { return e[i]; }
	inline double& operator [] (int i) { return e[i]; };

	inline vec3& operator += (const vec3 &v2);
	inline vec3& operator -= (const vec3 &v2);
	inline vec3& operator *= (const vec3 &v2);
	inline vec3& operator /= (const vec3 &v2);
	inline vec3& operator *= (const float t);
	inline vec3& operator /= (const float t);

	inline double length() const {
		return sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
	}


	inline double squared_length() const {
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	}

	inline void make_unit_vector();

	double e[3];
};

inline std::istream& operator >> (std::istream &is, vec3 &t) {
	is >> t.e[0] >> t.e[1] >> t.e[2];
	return is;
}


inline std::ostream& operator << (std::ostream &os, const vec3 &t) {
	os << t.e[0] << " " << t.e[1] << " " << t.e[2];
	return os;
}

inline void vec3::make_unit_vector() {
	double k = 1.0 / sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
	e[0] *= k; e[1] *= k; e[2] *= k;
}

//Vector addition
inline vec3 operator + (const vec3 &v1, const vec3 &v2) {
	return vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}

//Vector subtraction
inline vec3 operator - (const vec3 &v1, const vec3 &v2) {
	return vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}

//Vector multiplication
inline vec3 operator * (const vec3 &v1, const vec3 &v2) {
	return vec3(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]);
}

//Vector division
inline vec3 operator / (const vec3 &v1, const vec3 &v2) {
	return vec3(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2]);
}

//Scalar multiplication
inline vec3 operator * (float t, const vec3 &v) {
	return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}
inline vec3 operator * (const vec3 &v, float t) {
	return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

//Scalar division
inline vec3 operator / (vec3 v, float t) {
	return vec3(v.e[0] / t, v.e[1] / t, v.e[2] / t);
}

//Dot product
inline double dot(const vec3 &v1, const vec3 &v2) {
	return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
}

//Cross product
inline vec3 cross(const vec3 &v1, const vec3 &v2) {
	return vec3((v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1]),
		(-(v1.e[0] * v2.e[2] - v1.e[2] * v2.e[0])),
		(v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0]));
}

//Returns vector with abs applied to each component
inline vec3 abs(const vec3 &v) {
	return vec3(std::abs(v[0]), std::abs(v[1]), std::abs(v[2]));
}

//Returns the index of the component with the largest value
inline int max_dimension(const vec3 &v) {
	return (v[0] > v[1]) ? ((v[0] > v[2]) ? 0 : 2) : ((v[1] > v[2]) ? 1 : 2);
}

//Permutes coordinates according to index values provided
inline vec3 permute(const vec3 &v, int x, int y, int z) {
	return vec3(v[x], v[y], v[z]);
}

inline vec3& vec3::operator +=(const vec3 &v) {
	e[0] += v.e[0];
	e[1] += v.e[1];
	e[2] += v.e[2];
	return *this;
}

inline vec3& vec3::operator *=(const vec3 &v) {
	e[0] *= v.e[0];
	e[1] *= v.e[1];
	e[2] *= v.e[2];
	return *this;
}

inline vec3& vec3::operator /=(const vec3 &v) {
	e[0] /= v.e[0];
	e[1] /= v.e[1];
	e[2] /= v.e[2];
	return *this;
}

inline vec3& vec3::operator -=(const vec3 &v) {
	e[0] -= v.e[0];
	e[1] -= v.e[1];
	e[2] -= v.e[2];
	return *this;
}

inline vec3& vec3::operator *=(float t) {
	e[0] *= t;
	e[1] *= t;
	e[2] *= t;
	return *this;
}

inline vec3& vec3::operator /=(float t) {
	double k = 1.0 / t;

	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
	return *this;
}

inline vec3 unit_vector(vec3 v) {
	return v / v.length();
}

#endif //VEC3_H