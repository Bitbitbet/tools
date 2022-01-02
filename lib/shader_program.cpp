#include "shader_program.h"
#include "utils.h"
#include <glad/glad.h>
#include <glm/ext.hpp>
#include <cstdio>

bool Program::bind(const char *vertex_shader_source, const char *fragment_shader_source) {
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	int success;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		char *info = new char[10240];
		glGetShaderInfoLog(vertex_shader, 10240, NULL, info);
		if(is_named) {
			log_error("%s : Errors detected while compiling vertex shader: %s", name, info);
		} else {
			log_error("Errors detected while compiling vertex shader: %s", info);
		}
		delete[] info;
		return false;
	}
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		char *info = new char[10240];
		glGetShaderInfoLog(fragment_shader, 10240, NULL, info);
		if(is_named) {
			log_error("%s : Errors detected while compiling fragment shader: %s", name, info);
		} else {
			log_error("Errors detected while compiling fragment shader: %s", info);
		}
		delete[] info;
		return false;
	}

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	is_bind = true;
	return true;
}
bool Program::bind_from_file(const char *vertex_shader_source_path, const char *fragment_shader_source_path) {
	FILE *vertex_shader_file = fopen(vertex_shader_source_path, "r");
	if (vertex_shader_file == nullptr) {
		log_error("Can't open vertex shader source for reading : %s", vertex_shader_source_path);
		return false;
	}
	FILE *fragment_shader_file = fopen(fragment_shader_source_path, "r");
	if (fragment_shader_file == nullptr) {
		log_error("Can't open fragment shader source for reading : %s", fragment_shader_source_path);
		return false;
	}
	fseek(vertex_shader_file, 0, SEEK_END);
	char *vertex_shader_source = new char[ftell(vertex_shader_file) + 1];
	fseek(vertex_shader_file, 0, SEEK_SET);
	char c = fgetc(vertex_shader_file);
	int index = 0;
	while (c != (char)EOF) {
		vertex_shader_source[index] = c;
		index++;
		c = fgetc(vertex_shader_file);
	}
	vertex_shader_source[index] = '\0';

	fseek(fragment_shader_file, 0, SEEK_END);
	char *fragment_shader_source = new char[ftell(fragment_shader_file) + 1];
	fseek(fragment_shader_file, 0, SEEK_SET);
	c = fgetc(fragment_shader_file);
	index = 0;
	while (c != (char)EOF) {
		fragment_shader_source[index] = c;
		index++;
		c = fgetc(fragment_shader_file);
	}
	fragment_shader_source[index] = '\0';
	bool result = bind(vertex_shader_source, fragment_shader_source);
	delete[] vertex_shader_source;
	delete[] fragment_shader_source;
	fclose(vertex_shader_file);
	fclose(fragment_shader_file);
	return result;
}

unsigned int Program::get_program() {
	if(is_bind) {
		return program;
	} else {
		log_error("Tried to get program without binding");
		return 4294967295;
	}
}

bool Program::use() {
	if(is_bind) {
		glUseProgram(program);
	} else {
		log_error("Tried to use program without binding");
		return false;
	}
	return true;
}

bool Program::set_name(const char *name) {
	int length = strlen(name);
	if(length > NAME_STR_SIZE) {
		log_error("Given name is too long");
		return false;
	}
	memcpy(this->name, name, (length + 1) * sizeof(char));
	is_named = true;
	return true;
}

void Program::set(const char *u_name, int value) {
	glUniform1i(glGetUniformLocation(program, u_name), value);
}
void Program::set(const char *u_name, float value) {
	glUniform1f(glGetUniformLocation(program, u_name), value);
}

void Program::set(const char *u_name, glm::mat4 &value){
	glUniformMatrix4fv(glGetUniformLocation(program, u_name), 1, GL_FALSE, glm::value_ptr(value));
}
void Program::set(const char *u_name, glm::vec3 &value){
	glUniform3fv(glGetUniformLocation(program, u_name), 1, glm::value_ptr(value));
}
