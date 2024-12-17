#pragma once
#ifndef RAYTRACE_RAINBOW_SPHERES_SCENE_H_
#define	RAYTRACE_RAINBOW_SPHERES_SCENE_H_

#include "scene.h"

namespace raytrace {
	class RainbowSpheresScene : public Scene {
	public:
		static const int num_spheres = 99; // leave room for floor

		RainbowSpheresScene();

		void Update(float delta_time) override;


	private:
		Sphere floor = Sphere(vec3(0.0f, -5001.0f, 0.0f), 5000.0f, Color(0xFF, 0xFF, 0xFF), 800, 0.4f);
		std::shared_ptr<Sphere> floor_ref = std::make_shared<Sphere>(floor);
		Light al = Light::AmbientLight(0.6f);
		Light pl = Light::PointLight(0.6f, vec3(0, 5, 0));
		Light dl = Light::DirectionalLight(0.4f, vec3(0, -1, 0));
		std::shared_ptr<Light> al_ref = std::make_shared<Light>(al);
		std::shared_ptr<Light> pl_ref = std::make_shared<Light>(pl);
		std::shared_ptr<Light> dl_ref = std::make_shared<Light>(dl);
	};
} // namespace raytrace
#endif // RAYTRACE_RAINBOW_SPHERES_SCENE_H_
