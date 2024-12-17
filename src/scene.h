#pragma once
#ifndef RAYTRACE_SCENE_H_
#define	RAYTRACE_SCENE_H_

#include <vector>

#include <SDL.h>

#include "camera.h"
#include "sphere.h"
#include "shader.h"
#include "types.h"

namespace raytrace {
	class Scene {
	public:
		static inline const int SPHERES_BUFFER_SIZE = 4816; // Holds 100 spheres
		static inline const int LIGHTS_BUFFER_SIZE = 4816; // Holds 100 lights

		static int Init(Shader& shader);
		Scene();

		int AddSphere(std::shared_ptr<Sphere>& sphere);
		int AddLight(std::shared_ptr<Light>& light);
		int RemoveLight(std::shared_ptr<Light>& light);
		void WriteSphereBuffer();
		void WriteLightBuffer();
		virtual void Update(float delta_time) {};

		// All scenes share the same buffer on GPU
		static inline GLuint ubo_sphere_buffer_;
		static inline GLuint ubo_light_buffer_;
		static inline u8 serialized_spheres_[SPHERES_BUFFER_SIZE];
		static inline u8 serialized_lights_[LIGHTS_BUFFER_SIZE];
		static inline Shader* shader_ = nullptr;

		std::vector<std::shared_ptr<Sphere>> spheres;
		std::vector<std::shared_ptr<Light>> lights;
		Camera camera_ = Camera(vec3(0.0f, 0.0f, 0.0f));
	};
} // namespace raytrace
#endif // RAYTRACE_SCENE_H_
