#pragma once
#include <glm/ext.hpp>

const int NAME_STR_SIZE = 1024;
class Program {
	private:
		unsigned int program;
		bool is_bind = false;
		bool is_named = false;
		char name[NAME_STR_SIZE + 1];
	public:
		bool bind(const char *, const char *);
		bool bind_from_file(const char *, const char *);
		bool set_name(const char *);
		unsigned int get_program();
		bool use();
		void set(const char *, int);
		void set(const char *, float);
		void set(const char *, glm::mat4&);
		void set(const char *, glm::vec3&);
};
