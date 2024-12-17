#pragma once
#ifndef RAYTRACE_SPHERE_H_
#define	RAYTRACE_SPHERE_H_

#include <vector>

#include <SDL.h>

#include "types.h"

namespace raytrace {
	class Sphere {
	public:
		const int DEFAULT_SPECULAR = 50;
		const float DEFAULT_REFLECTIVE = 0.3f;
		const static int SPHERE_SIZE_STD140 =
			sizeof(vec4) // Center position			- offset - 0
			+ sizeof(vec4) // Color					- offset - 16
			+ sizeof(float) // Radius 20
			+ sizeof(int) // Specular
			+ sizeof(float) // Reflective
			+ 4 // bring offset to 48, getting to base alignment 4N (???????????? maybe not needed)????
			;
		static void WriteUniformBuffer(u8* buffer_start, std::vector<std::shared_ptr<Sphere>> spheres) { // Caller is responsible for buffer size
			int offset = 0;
			// num_entries
			int num_spheres = (int)spheres.size();
			memcpy(buffer_start, &num_spheres, sizeof(int));
			offset += 16; // pad to 16, requirement of struct
			// spheres
			for (int i = 0; i < (int)spheres.size(); i++) {
				(*spheres[i]).std140_serialize(buffer_start + offset + i * SPHERE_SIZE_STD140);
			}
		}
		// Order is as listed in SPHERE_SIZE_STD140
		void std140_serialize(u8* dst) {
			int offset = 0;

			// Copy center position
			vec4 center4(center);
			memcpy(dst + offset, &center4.x, sizeof(center4.x));
			offset += sizeof(center4.x);
			memcpy(dst + offset, &center4.y, sizeof(center4.y));
			offset += sizeof(center4.y);
			memcpy(dst + offset, &center4.z, sizeof(center4.z));
			offset += sizeof(center4.z);
			memcpy(dst + offset, &center4.w, sizeof(center4.w));
			offset += sizeof(center4.w);

			// Color
			vec4 color4 = color.ToFloat();
			memcpy(dst + offset, &color4.x, sizeof(color4.x));
			offset += sizeof(color4.x);
			memcpy(dst + offset, &color4.y, sizeof(color4.y));
			offset += sizeof(color4.y);
			memcpy(dst + offset, &color4.z, sizeof(color4.z));
			offset += sizeof(color4.z);
			memcpy(dst + offset, &color4.w, sizeof(color4.w));
			offset += sizeof(color4.w);

			// Radius
			memcpy(dst + offset, &radius, sizeof(radius));
			offset += sizeof(radius);

			// Specular
			memcpy(dst + offset, &specular, sizeof(specular));
			offset += sizeof(specular);

			// Reflective
			memcpy(dst + offset, &reflective, sizeof(reflective));
			offset += sizeof(reflective);

		}
		
		Sphere(vec3 center, float radius) :center(center), radius(radius), color(Color(0xFF,0x0,0x0)), specular(DEFAULT_SPECULAR), reflective(DEFAULT_REFLECTIVE) {};
		Sphere(vec3 center, float radius, Color color) :center(center), radius(radius), color(color), specular(DEFAULT_SPECULAR), reflective(DEFAULT_REFLECTIVE) {};
		Sphere(vec3 center, float radius, Color color, int specular) :center(center), radius(radius), color(color), specular(specular), reflective(DEFAULT_REFLECTIVE) {};
		Sphere(vec3 center, float radius, Color color, int specular, float reflective) :center(center), radius(radius), color(color), specular(specular), reflective(reflective) {};


		Sphere(float x, float y, float z, float radius) :center(vec3(x,y,z)), radius(radius), color(Color(0xFF, 0x0, 0x0)), specular(DEFAULT_SPECULAR), reflective(DEFAULT_REFLECTIVE) {};
		Sphere(float x, float y, float z, float radius, Color color) :center(vec3(x, y, z)), radius(radius), color(color), specular(DEFAULT_SPECULAR), reflective(DEFAULT_REFLECTIVE) {};
		Sphere(float x, float y, float z, float radius, Color color, int specular) :center(vec3(x, y, z)), radius(radius), color(color), specular(specular), reflective(DEFAULT_REFLECTIVE) {};
		Sphere(float x, float y, float z, float radius, Color color, int specular, float reflective) :center(vec3(x, y, z)), radius(radius), color(color), specular(specular), reflective(reflective) {};



		vec3 center; // position of the sphere
		Color color;
		float radius;
		int specular; // Specular exponent, ~500 is shiny, ~10 is a little bit shiny.
		float reflective; // How much the sphere reflects light [0.0, 1.0]
	private:

	};
} // namespace raytrace
#endif // RAYTRACE_SPHERE_H_
