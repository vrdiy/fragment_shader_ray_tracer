#include <iostream>
#include <SDL.h>
#include "util.h"
#include "types.h"
namespace raytrace {

	float Degrees(float radians) {
		return (radians * 180.0f) / ((float)M_PI);
	}
	float Radians(float degrees) {
		return (degrees * (float)M_PI) / 180.0f;
	}
	void PrintBufferValues(u8* buffer, int size) {
		for (int i = 0; i < size; i += 4) {
			float val = 0;
			int int_val = 0;
			memcpy(&val, &buffer[i], sizeof(float));
			memcpy(&int_val, &buffer[i], sizeof(int));
			//std::cout << "byte " << i << ": " << serialized_spheres[i] << std::endl;
			//std::cout << "byte hex " << i << ": " <<  std::hex << std::setfill('0') << std::setw(2) << serialized_spheres[i] << std::endl;
			std::cout << "idx: " << i << ", as Float: " << val << ", Int: " << int_val << std::endl;
		}
	}
	Mat4 RotationAboutY(float degrees) {

		float m[4][4] = {
			{cos(Radians(degrees)), 0, -sin(Radians(degrees)), 0},
			{0, 1, 0, 0},
			{sin(Radians(degrees)), 0, cos(Radians(degrees)), 0},
			{0, 0, 0, 1}
		};
		return Mat4(m);
	}
	Mat4 RotationAboutX(float degrees) {

		float m[4][4] = {
			{1, 0, 0, 0},
			{0, cos(Radians(degrees)), -sin(Radians(degrees)), 0},
			{0, sin(Radians(degrees)), cos(Radians(degrees)), 0},
			{0, 0, 0, 1}
		};
		return Mat4(m);
	}
	Mat4 RotationAboutZ(float degrees) {
		float m[4][4] = {
			{cos(Radians(degrees)), -sin(Radians(degrees)), 0, 0},
			{sin(Radians(degrees)), cos(Radians(degrees)), 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		};
		return Mat4(m);
	}
}// namespace raytrace