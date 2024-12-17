#pragma once
#ifndef RAYTRACE_UTIL_H_
#define	RAYTRACE_UTIL_H_
#include "types.h"
// Assumes 0,0 origin centered on canvas
namespace raytrace {

	float Degrees(float radians);
	float Radians(float degrees);
	void PrintBufferValues(u8* buffer, int size);
	Mat4 RotationAboutX(float degrees);
	Mat4 RotationAboutY(float degrees);
	Mat4 RotationAboutZ(float degrees);

}// namespace raytrace
#endif // RAYTRACE_UTIL_H_