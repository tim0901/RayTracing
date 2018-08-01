#pragma once

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "vec3.h"

/*
class Matrix {
	int* array; 
	int m_width;
public: 
	Matrix(int w, int h) : m_width(w), array(new int[w * h]) {} ~Matrix() { delete[] array; } int at(int x, int y) const { return array[index(x, y)]; }


protected: 
	int index(int x, int y) const { return x + m_width * y; }
};*/




class matrix {

	int m_width;
	vec3* ary;

public:
	matrix() {}
	matrix(int x, int y) { ary = new vec3[((x+1) * (y+1))]; m_width = x; }

	inline vec3 operator[](int i) const { return ary[i]; }
	inline vec3& operator[](int i) { return ary[i]; }

	~matrix() { delete[] ary; }
	
	inline int index(int x, int y) const {

		return x + (m_width * y);

	}
	
	inline vec3 at(int x, int y) const {
		return ary[index(x, y)];
	}
};