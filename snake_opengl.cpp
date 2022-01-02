#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <list>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

struct pos{
	int x,y;
};

void error_callback(int, const char *);
void framebuffer_size_callback(GLFWwindow*, int, int);
void key_callback(GLFWwindow*, int, int, int, int);

void init_gl();
void init_game();
void game_thread();
void random_apple();
glm::mat4 model_pos(pos);
void delay(int);
void log_error(const char *);
void end(int);


const int MAP_WIDTH = 30;
const int MAP_HEIGHT = 30;
const int NODE_WIDTH = 20;
const int NODE_HEIGHT = 20;
const char *WINDOW_TITLE = "Snake OpenGL";
const int STEP_PER_SECOND = 10;
const int UP = 0;
const int DOWN = 1;
const int LEFT = 2;
const int RIGHT = 3;
const float APPLE_COLOR[] = {1.f, 0.2f, 0.2f};
const float HEAD_COLOR[] = {0.2f, 0.4f, 1.f};
const float BODY_COLOR[] = {0.3f, 0.5f, 1.f};

const char *vertex_shader_source = "#version 330 core\n"
"in vec2 a_pos;\n"
"in vec2 a_tex_coord;\n"
"out vec2 tex_coord;\n"
"uniform mat4 my_model;\n"
"void main() {\n"
"	gl_Position = my_model * vec4(a_pos, 0.f, 1.f);\n"
"	tex_coord = a_tex_coord;\n"
"}";

const char *fragment_shader_source = "#version 330 core\n"
"in vec2 tex_coord;\n"
"uniform sampler2D my_background;\n"
"uniform bool my_render_background;\n"
"uniform vec3 my_render_color;\n"
"void main() {\n"
"	if (my_render_background){\n"
"		gl_FragColor = texture(my_background, tex_coord);\n"
"	} else {\n"
"		gl_FragColor = vec4(my_render_color, 1.f);\n"
"	}\n"
"}";

float vertices[] = {
	-1.f, -1.f, 0.f, 0.f, 
	-1.f, 1.f, 0.f, 1.f, 
	1.f, 1.f, 1.f, 1.f, 
	1.f, -1.f, 1.f, 0.f
};
unsigned int indices[] = {
	0, 1, 2, 
	0, 2, 3
};

list<pos> snake;
pos apple;
GLFWwindow *window;
int direction, direction_pre, score;
unsigned int vertex_array, vertex_buffer, element_buffer, vertex_shader, fragment_shader, shader_program, background;
bool snake_lock;

int main(int argc, char **argv) {
	glfwSetErrorCallback(error_callback);
	glfwInit();
	window = glfwCreateWindow(MAP_WIDTH * NODE_WIDTH, MAP_HEIGHT * NODE_HEIGHT, WINDOW_TITLE, NULL, NULL);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		log_error("Load OpenGL Failed");
		end(12);
	}
	glfwSwapInterval(1);
	init_gl();
	init_game();
	thread t(game_thread);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(shader_program);
		glBindVertexArray(vertex_array);
		
		glUniform1i(glGetUniformLocation(shader_program, "my_render_background"), GL_TRUE);
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "my_model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		glUniform1i(glGetUniformLocation(shader_program, "my_render_background"), GL_FALSE);
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "my_model"), 1, GL_FALSE, glm::value_ptr(model_pos(apple)));
		glUniform3fv(glGetUniformLocation(shader_program, "my_render_color"), 1, APPLE_COLOR);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		while (snake_lock);
		snake_lock = true;
		list<pos>::iterator iter = snake.begin();
		if (iter != snake.end()) {
			glUniform3fv(glGetUniformLocation(shader_program, "my_render_color"), 1, HEAD_COLOR);
			glUniformMatrix4fv(glGetUniformLocation(shader_program, "my_model"), 1, GL_FALSE, glm::value_ptr(model_pos(*iter)));
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			iter++;
		}
		for (;iter != snake.end();iter++) {
			glUniform3fv(glGetUniformLocation(shader_program, "my_render_color"), 1, BODY_COLOR);
			glUniformMatrix4fv(glGetUniformLocation(shader_program, "my_model"), 1, GL_FALSE, glm::value_ptr(model_pos(*iter)));
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		snake_lock = false;
		
		glBindVertexArray(0);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	end(0);
	return 0;
}
void init_game(){
	srand((unsigned int)time(NULL));
	random_apple();
	pos head = {rand() % MAP_WIDTH, rand() % MAP_HEIGHT};
	snake.push_back(head);
	direction = rand() % 4;
	direction_pre = direction;
}
void game_thread() {
	delay(1000/STEP_PER_SECOND);
	while (true) {
		delay(1000/STEP_PER_SECOND);
		while (snake_lock);
		snake_lock = true;
		pos head = snake.front();
		direction = direction_pre;
		switch (direction) {
			case UP:
				head.y--;
				break;
			case DOWN:
				head.y++;
				break;
			case LEFT:
				head.x--;
				break;
			case RIGHT:
				head.x++;
				break;
		}
		if (head.x < 0) {
			head.x = MAP_WIDTH - 1;
		} else if (head.x >= MAP_WIDTH) {
			head.x = 0;
		}
		if (head.y < 0) {
			head.y = MAP_HEIGHT - 1;
		} else if (head.y >= MAP_HEIGHT) {
			head.y = 0;
		}
		snake.push_front(head);
		if (head.x == apple.x && head.y == apple.y) {
			score++;
			printf("SCORE: %d\n", score);
			random_apple();
		} else {
			snake.pop_back();
		}
		list<pos>::iterator iter = snake.begin();
		for (iter++;iter != snake.end();iter++) {
			if (iter->x == head.x&&iter->y == head.y) {
				char info[100];
				sprintf(info, "You died with score %d", score);
				log_error(info);
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
		}
		snake_lock = false;
	}
}
void random_apple(){
	bool again=true;
	while(again){
		apple = {rand() % MAP_WIDTH, rand() % MAP_HEIGHT};
		again = false;
		for (list<pos>::iterator iter = snake.begin(); iter != snake.end(); iter++) {
			if (apple.x == iter->x && apple.y == iter->y) {
				again = true;
				break;
			}
		}
	}
}
glm::mat4 model_pos(pos p) {
	glm::mat4 m = glm::mat4(1.0f);
	m = glm::translate(m, glm::vec3(1.f/MAP_WIDTH*(p.x*2-MAP_WIDTH+1.f), -1.f/MAP_HEIGHT*(p.y*2-MAP_HEIGHT+1.f), 0.f));
	m = glm::scale(m, glm::vec3(1.f/MAP_WIDTH, 1.f/MAP_HEIGHT, 1.f));
	return m;
}
void init_gl() {
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glGenBuffers(1, &element_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int texture_width, texture_height, texture_nr_channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *texture_data = stbi_load("res/background.png", &texture_width, &texture_height, &texture_nr_channels, 0);
	glGenTextures(1, &background);
	glBindTexture(GL_TEXTURE_2D, background);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(texture_data);
	
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	unsigned int location = glGetAttribLocation(shader_program, "a_pos");
	glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
	glEnableVertexAttribArray(location);
	location = glGetAttribLocation(shader_program, "a_tex_coord");
	glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(location);
	
	glUseProgram(shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, background);
	location = glGetUniformLocation(shader_program, "my_background");
	glUniform1i(location, 0);
}

void delay(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void log_error(const char *description) {
	fprintf(stderr, "\033[91m\033[1m%s\033[0m\n", description);
}
void end(int code) {
	glfwTerminate();
	exit(code);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_UP:
				if (direction != DOWN) {
					direction_pre = UP;
				}
				break;
			case GLFW_KEY_DOWN:
				if (direction != UP) {
					direction_pre = DOWN;
				}
				break;
			case GLFW_KEY_LEFT:
				if (direction != RIGHT) {
					direction_pre = LEFT;
				}
				break;
			case GLFW_KEY_RIGHT:
				if (direction != LEFT) {
					direction_pre = RIGHT;
				}
				break;
		}
	}
}
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}
void error_callback(int code, const char *description) {
	char output[10240];
	sprintf(output, "Error %d: %s", code, description);
	log_error(output);
}
