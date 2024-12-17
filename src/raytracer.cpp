#include "raytracer.h"

#include <algorithm>
#include <math.h>
#include <tuple>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>


#include "util.h"
#include "sphere.h"

namespace raytrace {
	RayTracer::RayTracer(SDL_Surface* canvas, Scene* default_scene) : canvas_(canvas),scene_(default_scene) {
	};

	// x and y are assumed to be from a centered origin, in the range	x: -width/2 -> width/2 - 1, y: -height/2 -> height/2 - 1
	int RayTracer::putPixel(SDL_Surface* canvas, int x, int y, Color c) {

		x = x + canvas->w / 2;
		// y is subtracted from width/2 because positive values of y in a centered origin 
		// // would index down farther in the array of pixels, tdlr it flips the y.
		// Need to subract 1 from y to make the xy input ranges the same
		// because positive values map from 0->249 and negative from -1->-250
		y = canvas->h / 2 - y - 1;

		if (SDL_PIXELFORMAT_XRGB8888 != canvas->format->format) {
			std::cout << "Idk bout that format" << std::endl;
			return -1;
		}
		u32* pixel = (u32*)((u8*)canvas->pixels + x * canvas->format->BytesPerPixel + y * canvas->pitch);
		*pixel = c.xrgb_pixel;
		return 0;
	}
	vec3 ReflectRay(vec3 ray_to_reflect, vec3 normal_to_reflect_over) {
		return normal_to_reflect_over * (2.0f * normal_to_reflect_over.dot(ray_to_reflect)) - ray_to_reflect;
	}
	// s is the specular exponent
	float RayTracer::ComputeLighting(vec3 point, vec3 normal, vec3 vec_to_camera, int s) {

		float intensity = 0.0f;

		for (int i = 0; i < scene_->lights.size(); i++) {
			Light light = *scene_->lights[i].get();
			if (light.type == LightType::kAmbient) {
				intensity += light.intensity;
			}
			else {
				vec3 light_vec;
				if (light.type == LightType::kPoint) {
					light_vec = light.position - point;
				}
				else { // Directional light
					light_vec = light.direction;
				}

				// Check for shadow
				Sphere* obscuring_sphere = NULL;
				float closest_t;
				std::tie(obscuring_sphere, closest_t) = ClosestIntersection(point, light_vec, 0.01f, FLT_MAX);
				if (obscuring_sphere != NULL) {
					continue;
				}

				// Diffuse 
				float n_dot_l = normal.dot(light_vec);
				if (n_dot_l > 0) {
					intensity += light.intensity * n_dot_l / (vec3::Length(normal)* vec3::Length(light_vec));
				}

				// Specular
				if (s != -1) {
					vec3 reflection = ReflectRay(light_vec, normal);
					float r_dot_v = reflection.dot(vec_to_camera);

					// Reflection is facing the camera.
					// angle between reflection and camera is less than 90 degrees
					if (r_dot_v > 0.05f) { 
						intensity += light.intensity * pow(r_dot_v / (vec3::Length(reflection) * vec3::Length(vec_to_camera)),s);
					}
				}
			}
		}
		return intensity;
	}
	// Returns the solutions as a vec2 of scalars 
	vec2 RayTracer::IntersectRaySphere(vec3 o, vec3 direction, Sphere sphere) {
		float r = sphere.radius;
		vec3 CO = o - sphere.center;

		// Setting up Quadratic Formula
		float a = direction.dot(direction);
		float b = 2 * CO.dot(direction);
		float c = CO.dot(CO) - r * r;

		// The discriminant determines how many solutions to the sphere intersection there are
		// discriminat > 0, two real roots
		// discriminant == 0, one repeated root
		// discrimant < 0, no real roots
		float discriminant = b * b - 4 * a * c;
		if (discriminant < 0) {
			return vec2(FLT_MAX, FLT_MAX);
		}

		// Solve quadratic equation
		float t1 = (-b + sqrt(discriminant)) / (2 * a);
		float t2 = (-b - sqrt(discriminant)) / (2 * a);


		return vec2(t1, t2);
	};

	// Handle all intersections of a given ray
	Color RayTracer::TraceRay(vec3 ray_origin, vec3 direction, float t_min, float t_max, int recursion_depth) {
		float closest_t = FLT_MAX;
		Sphere* closest_sphere = NULL;
		

		std::tie(closest_sphere, closest_t) = ClosestIntersection(ray_origin, direction, t_min, t_max);

		if (closest_sphere == NULL) {
			return background_color_;
		}
		vec3 point = ray_origin + direction * closest_t;
		vec3 point_normal = point - closest_sphere->center;
		point_normal = point_normal/vec3::Length(point_normal);
		Color local_color = closest_sphere->color * ComputeLighting(point, point_normal, -direction, closest_sphere->specular);

		// If at end of recursion or object is not reflective, exit
		float r = closest_sphere->reflective;
		if ((recursion_depth <= 0) || (r <= 0.0f)) {
			return local_color;
		}

		// Compute reflected color
		vec3 reflected_ray = ReflectRay(-direction, point_normal);
		Color reflected_color = TraceRay(point, reflected_ray, 0.1f, FLT_MAX, recursion_depth - 1);

		return (local_color * (1.0f - r)) + reflected_color * r;
	};
	// Handle all intersections of a given ray
	std::tuple<Sphere*, float> 
	RayTracer::ClosestIntersection(vec3 ray_origin, vec3 direction, float t_min, float t_max) {
		float closest_t = FLT_MAX;
		Sphere* closest_sphere = NULL;

		// Test each sphere in the scene for each ray 
		for (int i = 0; i < scene_->spheres.size(); i++) {

			vec2 intersects = IntersectRaySphere(ray_origin, direction, *scene_->spheres[i].get());


			// Check for closer intersections 
			if (((intersects.x > t_min) && (intersects.x < t_max)) && intersects.x < closest_t) {
				closest_t = intersects.x;
				closest_sphere = scene_->spheres[i].get();
			}
			if (((intersects.y > t_min) && (intersects.y < t_max)) && intersects.y < closest_t) {
				closest_t = intersects.y;
				closest_sphere = scene_->spheres[i].get();
			}
		}
		return std::make_tuple(closest_sphere, closest_t);
	};
	// Takes a canvas coordinate and converts it to a point on the viewport
	// This will be subtracted from the origin/camera to create a vector/ray
	vec3 RayTracer::CanvasToViewport(int x, int y) {
		return vec3((float)x * (float)viewport_width_ / (float)canvas_->w, (float)y * (float)viewport_height_ / (float)canvas_->h, dist_to_viewport_);
	};

	void RayTracer::SetFov(float degrees) {
		float radians = Radians(degrees);
		dist_to_viewport_ = ((float)viewport_width_ / 2) / tanf(radians / 2);
	}
	
	// Get the current field of view in degrees
	float RayTracer::GetFov() const{
		float radians = atanf(((float)viewport_width_ / (float)2) / dist_to_viewport_);
		float degrees = Degrees(radians);
		if (degrees < 0) { degrees += 360; }
		//std::cout << "deg:" << degrees*2 << std::endl;
		return degrees * 2;
	}

	void RayTracer::Render(Scene* scene) {
		scene_ = scene;
		const Color GREEN(0x0, 0xFF, 0x0);
		vec3 O(0, 0, 0);
		int recursion_depth = 2;
		// Canvas has 0,0 at center
		for (int x = -canvas_->w / 2; x < canvas_->w / 2; x++) {
			for (int y = -canvas_->h / 2; y < canvas_->h / 2; y++) {

				// D is the distance from the camera to the viewport
				vec3 D = (scene_->camera_.RotationY() * (scene_->camera_.RotationX() * CanvasToViewport(x, y)));
				Color pixel_color = TraceRay(scene_->camera_.position, D, 1, FLT_MAX, recursion_depth);
				//std::cout << x << ", " << y << std::endl;
				putPixel(canvas_, x, y, pixel_color);
			}
		}
	}
	void RayTracer::RenderGPU(Scene* scene) {
		scene_ = scene;
		scene->WriteLightBuffer();
		scene->WriteSphereBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene->shader_->Enable();
		glDrawArrays(GL_QUADS, 0, 4);
	}
	
	void RayTracer::HandleHeldInputs(float delta_time) {

		const u8* keystates = SDL_GetKeyboardState(NULL);
		if (keystates[SDL_SCANCODE_LSHIFT]) {
			scene_->camera_.speed = 2.0f;
		}
		else {
			scene_->camera_.speed = 1.0f;
		}
		if (keystates[SDL_SCANCODE_W]) {
			scene_->camera_.MoveForward(1.0f * delta_time);
		}
		if (keystates[SDL_SCANCODE_A]) {
			scene_->camera_.MoveRight(1.0f * delta_time);
		}
		if (keystates[SDL_SCANCODE_S]) {
			scene_->camera_.MoveForward(-1.0f * delta_time);
		}
		if (keystates[SDL_SCANCODE_D])
		{
			scene_->camera_.MoveRight(-1.0f * delta_time);
		}
		
	}

	void RayTracer::HandlePressedInputs(SDL_Event& e,float delta_time) {
		switch (e.type) {
		case SDL_KEYDOWN:
		{
			SDL_Keycode key = e.key.keysym.sym;
			
			if (key == SDLK_LEFT) {
				scene_->camera_.yaw += 10.0f;
				std::cout << "Camera yaw: " << scene_->camera_.yaw << std::endl;
			}
			else if (key == SDLK_RIGHT) {
				scene_->camera_.yaw -= 10.0f;
				std::cout << "Camera yaw: " << scene_->camera_.yaw << std::endl;
			}
			else if (key == SDLK_UP) {
				scene_->camera_.pitch -= 10.0f;
				std::cout << "Camera pitch: " << scene_->camera_.pitch << std::endl;
			}
			else if (key == SDLK_DOWN) {
				scene_->camera_.pitch += 10.0f;
				std::cout << "Camera pitch: " << scene_->camera_.pitch << std::endl;
			}
			break;
		}
		default:
			break;
		}
	}
} // namespace raytrace
