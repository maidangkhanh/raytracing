#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <random>

// C++ std usings
using std::make_shared;
using std::shared_ptr;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Ultility Functions
inline double degrees_to_radians(double degrees)
{
	return degrees * pi / 180.0;
}

inline double rand_double()
{
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937 generator;
	return distribution(generator);
}
inline double rand_double(double min, double max)
{
	return (min + (max - min) * rand_double());
}

// Common Headers

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif // !RAYTRACING_H
