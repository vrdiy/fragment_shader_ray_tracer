#pragma once
#ifndef RAYTRACE_RAYTRACER_H_
#define	RAYTRACE_RAYTRACER_H_

#include <iostream>
#include <SDL.h>

#include "camera.h"
#include "types.h"
#include "scene.h"
#include "shader.h"

namespace raytrace {
	class RayTracer {
	public:
		static int putPixel(SDL_Surface* canvas, int x, int y, Color c);
		static vec2 IntersectRaySphere(vec3 o, vec3 direction, Sphere sphere);

		RayTracer(SDL_Surface* canvas, Scene* default_scene);
		void Render(Scene* scene);
		void RenderGPU(Scene* scene);
		Color TraceRay(vec3 ray_origin, vec3 direction, float t_min, float t_max, int recursion_depth);
		std::tuple<Sphere*, float> ClosestIntersection(vec3 ray_origin, vec3 direction, float t_min, float t_max);
		float ComputeLighting(vec3 point, vec3 normal, vec3 vec_to_camera, int s);
		vec3 CanvasToViewport(int x, int y);

		void SetFov(float degrees);
		float GetFov() const;

		void HandlePressedInputs(SDL_Event& e, float delta_time);
		void HandleHeldInputs(float delta_time);
		
		
	private:
		

		SDL_Surface* canvas_ = NULL;
		Color background_color_ = Color(0x0, 0x0, 0x0);
		//Color background_color_ = Color(0xFF, 0xFF, 0xFF);

		Scene *scene_;
		int viewport_width_ = 1;
		int viewport_height_ = 1;
		float dist_to_viewport_ = 0.5;

	};
} // namespace raytrace
#endif // RAYTRACE_RAYTRACER_H_
