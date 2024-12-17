#ifndef RAYTRACE_SHADER_H_
#define	RAYTRACE_SHADER_H_

#include <fstream>
#include <string>

#include "GL/glew.h"

namespace raytrace {

std::string readFile(const char* fileName);
class Shader {
public:
	
	Shader(const char* vertex_shader_file_name, const char* fragment_shader_file_name);
	int Enable();
	int Disable();
	int Reload();
	int Delete();
	constexpr GLuint GetProgramID() const { return program_id_; };

private:
	GLuint AddSource(std::string shader_source, GLenum type);
	int DetachSource(GLenum type);
	int Recover();
	GLuint program_id_ = 0;

	// vertex_shader_id_ and fragment_shader_id_ are used to 
	// recover state if the modified shader source code fails to compile or link
	GLuint vertex_shader_id_ = 0, fragment_shader_id_ = 0;

	std::string vertex_shader_file_ = "";
	std::string fragment_shader_file_ = "";

};
} // namespace opengl_imp_1
#endif // RAYTRACE_SHADER_H_

