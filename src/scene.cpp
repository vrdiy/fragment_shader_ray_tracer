
#include "scene.h"

namespace raytrace {

	int Scene::Init(Shader& shader) {
		glGenBuffers(1, &Scene::ubo_sphere_buffer_);
		glBindBuffer(GL_UNIFORM_BUFFER, Scene::ubo_sphere_buffer_);
		glBufferData(GL_UNIFORM_BUFFER, SPHERES_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &Scene::ubo_light_buffer_);
		glBindBuffer(GL_UNIFORM_BUFFER, Scene::ubo_light_buffer_);
		glBufferData(GL_UNIFORM_BUFFER, LIGHTS_BUFFER_SIZE, NULL, GL_STATIC_DRAW);
		
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, Scene::ubo_sphere_buffer_);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, Scene::ubo_light_buffer_);
		// Dependency inject shader
		shader_ = &shader;

		// Bind ubos to shader
		GLuint ubo_sphere_buffer_index = glGetUniformBlockIndex((*shader_).GetProgramID(), "ubo_Spheres");
		GLuint ubo_light_buffer_index = glGetUniformBlockIndex((*shader_).GetProgramID(), "ubo_Lights");
		// Bind ubo_Spheres to ubo index 0 in shader
		glUniformBlockBinding((*shader_).GetProgramID(), ubo_sphere_buffer_index, 0);
		// Bind ubo_Lights to ubo index 1 in shader
		glUniformBlockBinding((*shader_).GetProgramID(), ubo_light_buffer_index, 1);
		return 0;
	}
	Scene::Scene() {
	}
	
	int Scene::AddSphere(std::shared_ptr<Sphere>& sphere) {
		spheres.emplace_back(sphere);
		return 0;
	};
	void Scene::WriteSphereBuffer() {
		Sphere::WriteUniformBuffer(Scene::serialized_spheres_, spheres);
		glBindBuffer(GL_UNIFORM_BUFFER, Scene::ubo_sphere_buffer_);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, SPHERES_BUFFER_SIZE, Scene::serialized_spheres_);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	void Scene::WriteLightBuffer() {
		Light::WriteUniformBuffer(Scene::serialized_lights_, lights);
		glBindBuffer(GL_UNIFORM_BUFFER, Scene::ubo_light_buffer_);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, LIGHTS_BUFFER_SIZE, Scene::serialized_lights_);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	int Scene::AddLight(std::shared_ptr<Light>& light) {
		lights.emplace_back(light);
		return 0;
	}
	int Scene::RemoveLight(std::shared_ptr<Light>& light) {
		auto light_to_delete = std::find(lights.begin(), lights.end(), light);
		if (light_to_delete == lights.end()) {
			return -1;
		}
		lights.erase(light_to_delete);
		return 0;
	}
}