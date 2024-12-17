
#include "rainbow_spheres_scene.h"

namespace raytrace {

	RainbowSpheresScene::RainbowSpheresScene() {
		for (int i = 0; i < num_spheres; i++) {
			Sphere s(vec3(1.0f, 0.0f, 0.0f), 0.05f, Color(0xFF, 0xFF, 0xFF), 900, 0.3f);
			std::shared_ptr<Sphere> s_ref = std::make_shared<Sphere>(s);
			AddSphere(s_ref);
		}
		AddSphere(floor_ref);
		AddLight(al_ref);
		AddLight(pl_ref);
		AddLight(dl_ref);
		camera_.pitch = 90.0f;
		camera_.position = vec3(0.0f, 1.7f, 0.0f);
	}
	void RainbowSpheresScene::Update(float delta_time) {
		camera_.pitch = 90.0f;
		camera_.position = vec3(0.0f, 1.7f, 0.0f);
		float interval = 360.0f / num_spheres;
		float period = 5000.0f;
		float b = ((2.0f * (float)M_PI) / period); // Period of 1 second
		float x = (float)SDL_GetTicks64();
		float sin_lerp = sin(b * x) * 0.5f + 0.5f; // map sin of time o [0,1]
		float cos_lerp = cos(b * x) * 0.5f + 0.5f; // map cos of time to [0,1]
		float radius = 0.6f;

		float rot_speed = 0.1f;
		float num_sides;
		num_sides = ((float)SDL_GetTicks64() / 5000.0f)* (sin(((2.0f * (float)M_PI) / 10000.0f) * x) * 0.5f + 0.5f);
		
		float shift_amplitude = period / (float)num_spheres;
		for (int i = 0; i < num_spheres; i++) {
			u8 faded_to_black = (u8)((sin((2.0f*(float)M_PI/100)+50)*0.5f+0.5f)*((float)i / (float)num_spheres) * 255.0f);
			u8 color2 = (u8)(sin_lerp*((float)i / (float)num_spheres) * 255.0f);
			u8 color3 = (u8)(cos_lerp*((float)i / (float)num_spheres) * 255.0f);
			u8 color5 = (u8)((sin((2.0f * (float)M_PI / 100) + i*50) * 0.5f + 0.5f) * ((float)i / (float)num_spheres) * 255.0f);

			vec3 current_center = (*spheres[i]).center;
			(*spheres[i]).center = (RotationAboutY(interval * 7 * i + x / (1 / rot_speed)) * vec3(radius*(i/80.0f), 0.0f, 0.0f));
			(*spheres[i]).center.y = sin(b * (x + (i * num_sides))) * 0.5f + 0.5f;
			(*spheres[i]).reflective = cos_lerp;
			(*spheres[i]).radius = ((float)i / num_spheres)*0.3f;
			(*spheres[i]).color = Color(faded_to_black, color2, color3);

		}
		u8 fade = (u8)((((float)sin_lerp*0.2f+0.8f) * 255.0f));
		//(*pl_ref).position = vec3(0.0f, 1.0f, 0.0f) * (5.0f * (sin((b / 2) * x) * 0.5f + 0.5f));
		(*floor_ref).color = Color(fade, fade, fade);
	}
}