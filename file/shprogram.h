#pragma once
#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <iostream>

class ShaderProgram
{
	GLuint program_id; 	// The program ID
public:
	// Constructor reads and builds the shader
	ShaderProgram(const GLchar* vertexPath, const GLchar* fragmentPath);

	// Use the program
	void Use() const
	{
		glUseProgram(get_programID());
	}

	// returns program ID
	GLuint get_programID() const
	{
		return program_id;
	}

	void setMat4Uniform(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(get_programID(), name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void setVec2Uniform(const std::string &name, const glm::vec2 &vec) const
	{
		int a = glGetUniformLocation(get_programID(), name.c_str());
		glUniform2fv(a, 1, &vec[0]);
	}
	
	void setVec3Uniform(const std::string &name, const glm::vec3 &vec) const
	{
	
		glUniform3fv(glGetUniformLocation(get_programID(), name.c_str()), 1, &vec[0]);
	}
	
	void setVec4Uniform(const std::string &name, const glm::vec4 &vec) const
	{
	
		glUniform4fv(glGetUniformLocation(get_programID(), name.c_str()), 1, &vec[0]);
	}
};