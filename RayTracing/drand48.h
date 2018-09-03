#pragma once

#ifndef DRAND48_H
#define DRAND48_H


inline float drand48() {

	return (rand() / (RAND_MAX + 1.0));
}

#endif //DRAND48_H