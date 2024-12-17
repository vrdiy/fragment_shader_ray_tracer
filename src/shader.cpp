#include "shader.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>

namespace raytrace {
	std::string readFile(std::string file_name) {
		std::filesystem::path shader_path = "src\\shaders\\";
		std::filesystem::path full_path = shader_path / file_name;
		std::cout << full_path << std::endl;
		// open file in binary mode
		std::ifstream file(full_path, std::ios::in | std::ios::ate | std::ios::binary);

		// continue if successfully opened
		if (file.is_open()) {

			// get size of file (std::ios::ate starts the get pointer at end of file)
			std::streampos file_size = file.tellg();
			
			std::string file_contents;
			file_contents.resize(file_size);
			// read file contents into string
			file.seekg(0, std::ios::beg);
			file.read(&file_contents[0], file_size);
			// clean up and return
			file.close();
			return file_contents;
		}
		abort();
	}

	// Detaches the shader of type type from the program object
	// Returns 0 if no shader of type type was attached
	int Shader::DetachSource(GLenum type)
	{
		if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER) {
			std::cout << "Shader type not valid" << std::endl;
			return -1;
		}
		if (type == GL_VERTEX_SHADER) {
			if (vertex_shader_id_) {
				glDetachShader(program_id_, vertex_shader_id_);
				return 1;
			}
		}
		if (type == GL_FRAGMENT_SHADER) {
			if (fragment_shader_id_) {
				glDetachShader(program_id_, fragment_shader_id_);
				return 1;
			}
		}
		return 0; // No shaders were attached
	}
	
	// Creates and attaches shader of type type from a path
	// Returns the shader's handle on success, GL_FALSE on failure
	GLuint Shader::AddSource(std::string shader_source, GLenum type) {
		if (type != GL_VERTEX_SHADER && type != GL_FRAGMENT_SHADER) {
			std::cout << "Shader type not valid" << std::endl;
			return GL_FALSE;
		}
		
		// Read the shader from disk
		std::string temp = readFile(shader_source);
		const char* shader_code = temp.c_str();

		// Create the shader object
		GLuint shader_id = glCreateShader(type);
		glShaderSource(shader_id, 1, &shader_code, NULL);
		glCompileShader(shader_id);

		// Show compiler errors
		GLint is_compiled = 0;
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &is_compiled);
		if (is_compiled == GL_FALSE) {
			GLint log_len = 0;
			glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_len);

			std::string error_log(log_len, 'x');
			glGetShaderInfoLog(shader_id, log_len, &log_len, &error_log[0]);
			std::cout << "Shader \"" << shader_source << "\" failed to compile." << std::endl;
			std::cout << error_log << std::endl;
			return GL_FALSE;
		}

		glAttachShader(program_id_, shader_id);
		
		return shader_id;
	}
	Shader::Shader(const char* vertex_shader_file_name, const char* fragment_shader_file_name) {
		// Create program first, AddSource depends on it
		program_id_ = glCreateProgram();
		if (program_id_ == GL_FALSE) {
			abort();
		}
		vertex_shader_id_ = AddSource(vertex_shader_file_name, GL_VERTEX_SHADER);
		if (GL_FALSE == vertex_shader_id_) {
			abort();
		};
		fragment_shader_id_ = AddSource(fragment_shader_file_name, GL_FRAGMENT_SHADER);
		if (GL_FALSE == fragment_shader_id_) {
			abort();
		};
		vertex_shader_file_ = vertex_shader_file_name;
		fragment_shader_file_ = fragment_shader_file_name;
		glLinkProgram(program_id_);
		// Do not delete shaders after linking, this is legal but 
		//	they can be used to recover state when hotloading new shaders.
		// This may change in the future if too complex in favor of creating
		//	new programs on hot reload
		
	}
	
	// Reloads shader programs from source files.
	// Called when source files are modified.
	// Will recover on failure to compile/link.
	int Shader::Reload() {
		// Remove current shaders
		DetachSource(GL_VERTEX_SHADER);
		DetachSource(GL_FRAGMENT_SHADER);

		// Add new shaders to program
		GLuint temp_vertex_shader_id = AddSource(vertex_shader_file_.c_str(), GL_VERTEX_SHADER);
		if (GL_FALSE == temp_vertex_shader_id) {
			glDetachShader(program_id_, temp_vertex_shader_id);
			glDeleteShader(temp_vertex_shader_id);
			Recover();
			return -1;
		}
		GLuint temp_fragment_shader_id = AddSource(fragment_shader_file_.c_str(), GL_FRAGMENT_SHADER);
		if (GL_FALSE == temp_fragment_shader_id) {
			glDetachShader(program_id_, temp_vertex_shader_id);
			glDeleteShader(temp_vertex_shader_id);
			glDeleteShader(temp_fragment_shader_id);
			Recover();
			return -1;
		}

		// Attempt to link
		glLinkProgram(program_id_);

		// Check for success
		GLint link_status = GL_FALSE;
		glGetProgramiv(program_id_, GL_LINK_STATUS, &link_status);
		if (GL_FALSE == link_status) {
			GLsizei log_length;
			glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &log_length);
			GLchar* info_log = new GLchar[log_length];
			glGetProgramInfoLog(program_id_, log_length, NULL, info_log);
			std::cout << info_log << std::endl;
			delete []info_log;
			// Delete temporary shaders
			glDeleteShader(temp_vertex_shader_id);
			glDeleteShader(temp_fragment_shader_id);

			Recover();
			return -1;
		}
		else { // Successful link
			// Delete old shaders
			glDeleteShader(vertex_shader_id_);
			glDeleteShader(fragment_shader_id_);

			// Overwrite old shader ids
			vertex_shader_id_ = temp_vertex_shader_id;
			fragment_shader_id_ = temp_fragment_shader_id;
		}
		return 0;
	}
	int Shader::Enable() {
		glUseProgram(program_id_);
		return 0;
	}
	int Shader::Disable() {
		glUseProgram(0);
		return 0;
	}
	int Shader::Delete() {
		glDeleteShader(vertex_shader_id_);
		glDeleteShader(fragment_shader_id_);
		glDeleteProgram(program_id_);
		vertex_shader_id_ = 0;
		fragment_shader_id_ = 0;
		program_id_ = 0;
		return 0;
	}
	int Shader::Recover() {
		// Detach bad sources
		DetachSource(GL_VERTEX_SHADER);
		DetachSource(GL_FRAGMENT_SHADER);

		// Re-Attach known good shaders
		glAttachShader(program_id_, vertex_shader_id_);
		glAttachShader(program_id_, fragment_shader_id_);

		// ?Recover Uniforms?

		// Re-Link
		glLinkProgram(program_id_);
		// Check for success
		GLint link_status = GL_FALSE;
		glGetProgramiv(program_id_, GL_LINK_STATUS, &link_status);
		if (GL_FALSE == link_status) {
			GLsizei log_length;
			glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &log_length);
			GLchar* info_log = new GLchar[log_length];
			glGetProgramInfoLog(program_id_, log_length, NULL, info_log);
			std::cout << info_log << std::endl;
			delete []info_log;
		}
		return 0;

	}

}