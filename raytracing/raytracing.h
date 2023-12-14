#pragma once
#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include "interval.h"

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const double pi = 3.1415926535897932385;


// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_double();
}

// Common Headers

#include "ray.h"
#include "vec3.h"

#endif