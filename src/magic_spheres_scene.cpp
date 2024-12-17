
#include "magic_spheres_scene.h"

namespace raytrace {

	MagicSpheresScene::MagicSpheresScene() {
		for (int i = 0; i < num_magic_spheres; i++) {
			Sphere s(vec3(1.0f,0.0f,0.0f), 0.05f, Color(0xFF, 0xFF, 0xFF), 900, 0.3f);
			std::shared_ptr<Sphere> s_ref = std::make_shared<Sphere>(s);
			AddSphere(s_ref);
		}
		AddSphere(floor_ref);
		AddLight(al_ref);
		AddLight(pl_ref);
		AddLight(dl_ref);
		camera_.pitch = 90.0f;
		camera_.position = vec3(0.0f, 4.0f, 0.0f);
	}
	void MagicSpheresScene::Update(float delta_time) {
		camera_.pitch = 90.0f;
		camera_.position = vec3(0.0f, 4.0f, 0.0f);
		float interval = 360.0f / num_magic_spheres;
		float period = 1000.0f;
		float b = ((2.0f * (float)M_PI) / period); // Period of 1 second
		float x = (float)SDL_GetTicks64();
		float sin_lerp = sin(b * x) * 0.5f + 0.5f; // map sin of time o [0,1]
		float cos_lerp = cos(b * x) * 0.5f + 0.5f; // map cos of time to [0,1]
		float radius = 1.0f;

		float rot_speed = 0.1f;
		int num_sides = 0;
		num_sides = (SDL_GetTicks64() / 1000) % 8;
		float shift_amplitude = period / (float)num_magic_spheres;
		for (int i = 0; i < num_magic_spheres; i++) {
			vec3 current_center = (*spheres[i]).center;
			(*spheres[i]).center = (RotationAboutY(interval * i + x / (1 / rot_speed)) * vec3(radius, 0.0f, 0.0f));
			(*spheres[i]).center.y = sin(b * (x + (i * (shift_amplitude * num_sides)))) * 0.5f + 0.5f;
		}
		(*pl_ref).position = vec3(0.0f, 1.0f, 0.0f) * (5.0f * (sin((b / 2) * x) * 0.5f + 0.5f));
		(*floor_ref).color = Color((u8)(130.0f + 125.0f * sin_lerp), (u8)(155.0f + 100.0f * cos_lerp), 0xFF);
	}
}