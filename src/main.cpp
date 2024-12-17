#include <algorithm>
#include <iomanip>
#include <iostream>
#include <math.h>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "magic_spheres_scene.h"
#include "rainbow_spheres_scene.h"
#include "raytracer.h"
#include "sphere.h"
#include "shader.h"
#include "util.h"


const int CANVAS_WIDTH = 500;
const int CANVAS_HEIGHT = 500;
using namespace raytrace;

int main(int argc, char* argv[]) {

	SDL_Window* window = NULL;
	SDL_Surface* canvas = NULL;

	if (0 > SDL_Init(SDL_RENDERER_ACCELERATED)) {
		std::cout << SDL_GetError() << std::endl;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("rt demo",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return -1;
	}
	canvas = SDL_GetWindowSurface(window);

	// Create the OpenGL Context
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		std::cout << "OpenGL context failed: " << SDL_GetError() << std::endl;
		return -1;
	}

	// Initialize GLEW
	GLenum glew = glewInit();
	if (glew != GLEW_OK) {
		std::cout << "GLEW failed to initialize: " << glewGetErrorString(glew) << std::endl;
	}

	// Change settings
	SDL_GL_SetSwapInterval(-1); // VSync

	float fullscreen_quad[] = {
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	};

	// setup vao
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// setup vbo
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreen_quad), fullscreen_quad, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);
	
	Shader shader_program("default.vert", "default.frag");
	shader_program.Enable();
	GLuint u_time = glGetUniformLocation(shader_program.GetProgramID(), "u_Time");
	GLuint u_Camera_Rotation_Matrix_X = glGetUniformLocation(shader_program.GetProgramID(), "u_Camera_Rotation_Matrix_X");
	GLuint u_Camera_Rotation_Matrix_Y = glGetUniformLocation(shader_program.GetProgramID(), "u_Camera_Rotation_Matrix_Y");
	GLuint u_Camera_Position = glGetUniformLocation(shader_program.GetProgramID(), "u_Camera_Position");


	// Initialize Scene buffers
	Scene::Init(shader_program);
	// Demo scene
	
	Scene book_demo;
	Sphere red_sphere = Sphere(vec3(0.0f, -1.0f, 3.0f), 1.0f, Color(0xFF, 0x0, 0x0), 500, 0.2f);
	Sphere blue_sphere = Sphere(vec3(2.0f, 0.0f, 4.0f), 1.0f, Color(0x0, 0x0, 0xFF), 500, 0.3f);
	Sphere green_sphere = Sphere(vec3(-2.0f, 0.0f, 4.0f), 1.0f, Color(0x0, 0xFF, 0x0), 10, 0.4f);
	Sphere yellow_sphere = Sphere(vec3(0.0f, -5001.0f, 0.0f), 5000.0f, Color(0xFF, 0xFF, 0x0), 1000, 0.5f);

	std::shared_ptr<Sphere> red_sphere_ref = std::make_shared<Sphere>(red_sphere);
	std::shared_ptr<Sphere> blue_sphere_ref = std::make_shared<Sphere>(blue_sphere);
	std::shared_ptr<Sphere> green_sphere_ref = std::make_shared<Sphere>(green_sphere);
	std::shared_ptr<Sphere> yellow_sphere_ref = std::make_shared<Sphere>(yellow_sphere);

	Light ambient_light = Light::AmbientLight(0.2f);
	Light point_light = Light::PointLight(0.6f, vec3(2, 1, 0));
	Light directional_light = Light::DirectionalLight(0.2f, vec3(1, 4, 4));

	std::shared_ptr<Light> ambient_light_ref = std::make_shared<Light>(ambient_light);
	std::shared_ptr<Light> point_light_ref = std::make_shared<Light>(point_light);
	std::shared_ptr<Light> directional_light_ref = std::make_shared<Light>(directional_light);

	book_demo.AddSphere(red_sphere_ref);
	book_demo.AddSphere(blue_sphere_ref);
	book_demo.AddSphere(green_sphere_ref);
	book_demo.AddSphere(yellow_sphere_ref);

	book_demo.AddLight(ambient_light_ref);
	book_demo.AddLight(point_light_ref);
	book_demo.AddLight(directional_light_ref);
	

	MagicSpheresScene magic_sphere_scene;
	RainbowSpheresScene rainbow_sphere_scene;

	RayTracer rt(canvas, &magic_sphere_scene);
	// Setup Scene ============================================================
	
	// Setup Scene ============================================================

	SDL_Event event;
	bool exit = false;
	glClearColor((float)0 / 255, (float)255 / 255, (float)0 / 255, 1.0f);
	u64 current_time = SDL_GetPerformanceCounter();
	u64 previous_time = 0;
	float delta_time = 0;
	SDL_SetRelativeMouseMode(SDL_TRUE);
	bool RENDER_CPU = false;
	Scene* active_scene = &magic_sphere_scene;
	while (!exit) {
		previous_time = current_time;
		current_time = SDL_GetPerformanceCounter();
		delta_time = ((current_time - previous_time) * 1000 / (float)SDL_GetPerformanceFrequency());
		//SDL_PumpEvents();
		rt.HandleHeldInputs(delta_time * 0.001f);
		while (SDL_PollEvent(&event))
		{
			switch (event.type) {
			case SDL_QUIT:
				std::cout << "Exiting..." << std::endl;
				exit = true;
				break;
			case SDL_KEYDOWN:
				{
					SDL_Keycode key = event.key.keysym.sym;
					if (key == SDLK_ESCAPE) {
						std::cout << "Exiting..." << std::endl;
						exit = true;
					}
					else if (key == SDLK_F1) {
						active_scene = &book_demo;
						std::cout << "Scene 1 Loaded." << std::endl;
					}
					else if (key == SDLK_F2) {
						active_scene = &magic_sphere_scene;
						std::cout << "Scene 2 Loaded." << std::endl;
					}
					else if (key == SDLK_F3) {
						active_scene = &rainbow_sphere_scene;
						std::cout << "Scene 3 Loaded." << std::endl;
					}
					else if (key == SDLK_5) {
						RENDER_CPU = !RENDER_CPU;
					}
				}
				break;
			case SDL_MOUSEMOTION:
				active_scene->camera_.yaw -= event.motion.xrel;
				active_scene->camera_.pitch += event.motion.yrel;
				break;
			default:
				break;
			}
			rt.HandlePressedInputs(event, delta_time * 0.001f);
		}

		
		// Update Uniforms
		glUniform1f(u_time, (float)SDL_GetTicks64());
		GLfloat mx[16];
		memcpy(mx, &active_scene->camera_.RotationX(), 64);
		GLfloat my[16];
		memcpy(my, &active_scene->camera_.RotationY(), 64);
		glUniformMatrix4fv(u_Camera_Rotation_Matrix_X, 1, GL_FALSE, mx);
		glUniformMatrix4fv(u_Camera_Rotation_Matrix_Y, 1, GL_FALSE, my);
		GLfloat cam_pos[4] = { 1.0f };
		memcpy(cam_pos, &active_scene->camera_.position, sizeof(vec3));
		glUniform4fv(u_Camera_Position,1,cam_pos);

		// Manipulate scene ============================================================

		
//		rt.camera_.pitch = 90.0f;
		// Manipulate scene ============================================================
		active_scene->Update(delta_time);
		if (RENDER_CPU) {
			rt.Render(active_scene);
			SDL_UpdateWindowSurface(window);
		}
		else {
			rt.RenderGPU(active_scene);
			SDL_GL_SwapWindow(window);
		}


	}
	return 0;
}