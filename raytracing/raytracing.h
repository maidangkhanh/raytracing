#pragma once
#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <cmath>
#include <limits>
#include <memory>
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

// Common Headers

#include "ray.h"
#include "vec3.h"

#endif