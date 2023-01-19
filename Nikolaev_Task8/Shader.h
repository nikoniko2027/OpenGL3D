#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glad\glad.h>
#include <map>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>

class shader {
private:
	std::string readTextFile(const char* fileName) {
		std::ifstream shaderFile;
		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			shaderFile.open(fileName);
			std::stringstream shaderStream;
			shaderStream << shaderFile.rdbuf();
			shaderFile.close();
			return shaderStream.str();
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ:" << fileName << std::endl;
		}
	}
public:
	GLuint shaderID;


	shader(const char* fileName, GLuint shaderType) {
		std::string fileStd = readTextFile(fileName);
		const char* shaderSource = fileStd.c_str();
		shaderID = glCreateShader(shaderType);

		glShaderSource(shaderID, 1, &shaderSource, NULL);
		glCompileShader(shaderID);

		GLint success;

		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar infoLog[512];
			glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
			std::string strShaderType;
			switch (shaderType) {
			case GL_VERTEX_SHADER: strShaderType = "VERTEX"; break;
			case GL_GEOMETRY_SHADER: strShaderType = "GEOMETRY"; break;
			case GL_FRAGMENT_SHADER: strShaderType = "FRAGMENT"; break;
			}
			std::cout << "ERROR::SHADER::" << strShaderType << "::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}

	void clear() {
		glDeleteShader(shaderID);
	}
};

class program {
public:
	GLuint programID;
	std::map<std::string, GLint> uniforms;

	program(shader  vertexShader, shader fragmentShader) {
		programID = glCreateProgram();
		glAttachShader(programID, vertexShader.shaderID);
		glAttachShader(programID, fragmentShader.shaderID);
		glLinkProgram(programID);

		GLint success;
		glGetProgramiv(programID, GL_LINK_STATUS, &success);
		if (!success) {
			GLchar infoLog[512];
			glGetProgramInfoLog(programID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
		}
	}

	void use() {
		glUseProgram(programID);
	}

	void useUniform(std::string uniformName) {
		uniforms[uniformName] = glGetUniformLocation(programID, uniformName.c_str());
	}

	void setUniform(std::string uniformName, glm::vec3 value) {
		glUniform3fv(uniforms[uniformName], 1, glm::value_ptr(value));
	}

	void setUniform(std::string uniformName, glm::mat4 value) {
		glUniformMatrix4fv(uniforms[uniformName], 1, GL_FALSE, glm::value_ptr(value));
	}

	void setUniform(std::string uniformName, float value) {
		glUniform1f(uniforms[uniformName], value);
	}
};
