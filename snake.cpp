#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <list>
#include <thread>
#include <chrono>
#include <cstring>
#include "number.h"
#ifdef __WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#endif
using namespace std;
struct color{
	GLclampf r,g,b,a;
};
struct position{
	int x,y;
};
struct node{
	GLFWwindow* window;
	position pos;
};

const int UP=0;
const int DOWN=1;
const int LEFT=2;
const int RIGHT=3;
const int MAP_EMPTY=6;
const int MAP_APPLE=7;
const int MAP_SNAKE=8;
const int WINDOW_WIDTH=80;
const int WINDOW_HEIGHT=80;
const int DEFAULT_MAP_WIDTH=30;
const int DEFAULT_MAP_HEIGHT=30;
const int step_per_sec=3;
const char WINDOW_NAME[]="The Snake Game";
const color APPLE_COLOR={1.f,0.25f,0.25f,1.f};
const color HEAD_COLOR={0.25f,0.25f,255,1.f};
const color BODY_COLOR={0.5f,0.5f,0.9f,0.9f};

void error_callback(int,const char*);
void window_close_callback(GLFWwindow*);
void key_callback(GLFWwindow*,int,int,int,int);

void game_main();

void output_map();//JUST FOR TESTING
void console_mode_input();
void initialize_window(GLFWwindow*);
void update_cursor();
void delay(int);
position random_pos(GLFWwindow*);
void set_window_pos(GLFWwindow*,position);
void generate_apple();
void refresh(GLFWwindow*);
void log_error(const char*);
void end(int);

list<node> snake;
node apple;
GLFWmonitor *monitor;
GLFWvidmode *vidmode;
GLFWcursor *cursor;
GLFWimage cursor_image;

int map_width=DEFAULT_MAP_WIDTH, map_height=DEFAULT_MAP_HEIGHT, direction, direction_pre, score, **map;
bool time_to_exit=false, snake_forward=false, disable_white_background=false, disable_opengl=false;
unsigned char cursor_pixels[4*32*32];
char *white_background=(char*)"\033[47m";

int main(int argc, char **argv){
#ifdef __WIN32
	system("cls");
#endif
	for(int i=1;i<argc;i++){
		if(strcmp(argv[i], "--no-background") == 0){
			disable_white_background=true;		//FOR WINDOWS
			white_background=(char*)"\033[0m";	//FOR UNIX
		}else if(strcmp(argv[i], "--console-mode") == 0){
			disable_opengl=true;
		}
	}
	if(!disable_white_background){
		fprintf(stderr, "Call \"%s --no-background\" to disable map white background.\n", argv[0]);
	}
	if(!disable_opengl){
		fprintf(stderr, "Call \"%s --console-mode\" to disable OpenGL.\n", argv[0]);
		glfwSetErrorCallback(error_callback);
		glfwInit();
		monitor=glfwGetPrimaryMonitor();
		vidmode=(GLFWvidmode*)glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_DECORATED,GLFW_FALSE);
		glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
		map_width=vidmode->width/WINDOW_WIDTH;
		map_height=vidmode->height/WINDOW_HEIGHT;
		map=new int*[map_width];
		for(int x=0;x<map_width;x++){
			map[x]=new int[map_height];
			for(int y=0;y<map_height;y++){
				map[x][y]=MAP_EMPTY;
			}
		}
		glfwWindowHint(GLFW_FOCUSED,GLFW_TRUE);
		apple.window=glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,WINDOW_NAME,NULL,NULL);
		glfwWindowHint(GLFW_FOCUSED,GLFW_FALSE);
		initialize_window(apple.window);
		if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			log_error("Failed to load OpenGL");
			end(1);
		}
		glClearColor(APPLE_COLOR.r,APPLE_COLOR.g,APPLE_COLOR.b,APPLE_COLOR.a);
		srand((unsigned int)time(NULL));
		apple.pos=random_pos(apple.window);
		map[apple.pos.x][apple.pos.y]=MAP_APPLE;
	
		GLFWwindow *head_window=glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,WINDOW_NAME,NULL,NULL);
		initialize_window(head_window);
		node head={head_window,random_pos(head_window)};
		map[head.pos.x][head.pos.y]=MAP_SNAKE;
		snake.push_back(head);
		direction=rand()%4;
		direction_pre=direction;
	
		cursor_image.width=32;
		cursor_image.height=32;
		cursor_image.pixels=cursor_pixels;
		update_cursor();
	} else {
		fprintf(stderr, "Map width %d height %d\n", map_width, map_height);
		map=new int*[map_width];
		for(int x=0;x<map_width;x++){
			map[x]=new int[map_height];
			for(int y=0;y<map_height;y++){
				map[x][y]=MAP_EMPTY;
			}
		}
		apple.window=NULL;
		srand((unsigned int)time(NULL));
		apple.pos=random_pos(NULL);
		map[apple.pos.x][apple.pos.y]=MAP_APPLE;
		node head={NULL, random_pos(NULL)};
		map[head.pos.x][head.pos.y]=MAP_SNAKE;
		snake.push_back(head);
		direction=rand()%4;
		direction_pre=direction;
	}
	thread game_thread(game_main);
	
	while(!time_to_exit){
		if(disable_opengl) {
			console_mode_input();
		} else {
			glfwMakeContextCurrent(apple.window);
			refresh(apple.window);
			for(list<node>::iterator iter=snake.begin();iter!=snake.end();iter++){
				glfwMakeContextCurrent(iter->window);
				if(iter==snake.begin()){
					glClearColor(HEAD_COLOR.r,HEAD_COLOR.g,HEAD_COLOR.b,HEAD_COLOR.a);
				}else{
					glClearColor(BODY_COLOR.r,BODY_COLOR.g,BODY_COLOR.b,BODY_COLOR.a);
				}
				refresh(iter->window);
			}
			glfwPollEvents();
		}
		if(snake_forward){
			snake_forward=false;
			position pos,old_pos=snake.front().pos;
			switch(direction){
				case UP:
					pos={old_pos.x,old_pos.y-1};
					break;
				case DOWN:
					pos={old_pos.x,old_pos.y+1};
					break;
				case LEFT:
					pos={old_pos.x-1,old_pos.y};
					break;
				case RIGHT:
					pos={old_pos.x+1,old_pos.y};
					break;
			}
			if(pos.x<0)pos.x=map_width-1;
			if(pos.x>=map_width)pos.x=0;
			if(pos.y<0)pos.y=map_height-1;
			if(pos.y>=map_height)pos.y=0;
			node head;
			if(disable_opengl) {
				head={NULL, pos};
			} else {
				GLFWwindow *win=glfwCreateWindow(WINDOW_WIDTH,WINDOW_HEIGHT,WINDOW_NAME,NULL,NULL);
				initialize_window(win);
				glfwSetWindowPos(win,pos.x*WINDOW_WIDTH,pos.y*WINDOW_HEIGHT);
				head={win, pos};
			}
			snake.push_front(head);
			map[head.pos.x][head.pos.y]=MAP_SNAKE;

			if(pos.x==apple.pos.x&&pos.y==apple.pos.y){
				generate_apple();
				score++;
				//printf("Apple eaten detected, now score %d\n",score);
				if(!disable_opengl) {
					update_cursor();
				}
			}else{
				node tail=snake.back();
				if(!disable_opengl) {
					glfwDestroyWindow(tail.window);
				}
				if(head.pos.x!=tail.pos.x||head.pos.y!=tail.pos.y) {
					map[tail.pos.x][tail.pos.y]=MAP_EMPTY;
				}
				snake.pop_back();
			}
			list<node>::iterator iter=snake.begin();
			for(iter++;iter!=snake.end();iter++){
				if(pos.x==iter->pos.x&&pos.y==iter->pos.y){
					char over[100];
					sprintf(over, "You died with score %d", score);
					log_error(over);
					end(0);
				}
			}
			output_map();
		}
	}
	end(0);
	return 0;
}
void game_main(){
	delay(1000/step_per_sec);
	while(true){
		delay(1000/step_per_sec);
		direction=direction_pre;
		snake_forward=true;
	}
}


void initialize_window(GLFWwindow *window){
	glfwMakeContextCurrent(window);
	glfwSetWindowCloseCallback(window,window_close_callback);
	glfwSetKeyCallback(window,key_callback);
	glfwSetCursor(window,cursor);
	glfwSwapInterval(0);
}
void update_cursor(){
	if(cursor!=NULL){
		glfwDestroyCursor(cursor);
	}
	int l=score/10%10;
	int r=score%10;
	unsigned char *ptr=cursor_pixels;
	for(int y=0;y<32;y++){
		for(int x=0;x<32;x++){
			bool index=x<16?numbers[l][y/2][x/2]:numbers[r][y/2][x/2-8];
			if(index){
				*ptr=0xff;
				*(++ptr)=0xff;
				*(++ptr)=0xff;
				*(++ptr)=0xff;
			}else{
				*ptr=0x00;
				*(++ptr)=0x00;
				*(++ptr)=0x00;
				*(++ptr)=0x00;
			}
			ptr++;
		}
	}
	cursor=glfwCreateCursor(&cursor_image,0,0);
	glfwSetCursor(apple.window,cursor);
	for(list<node>::iterator iter=snake.begin();iter!=snake.end();iter++){
		glfwSetCursor(iter->window,cursor);
	}
}
void error_callback(int code,const char *description){//DO NOT CALL
	char output[10240];
	sprintf(output, "Error %d: %s", code, description);
	log_error(output);
}
void window_close_callback(GLFWwindow *window){
	log_error("Now exiting...");
	time_to_exit=true;
}
void key_callback(GLFWwindow* window,int key,int scancode,int action,int mods){
	if(action==GLFW_PRESS){
		switch(key){
			case GLFW_KEY_UP:
				if(direction!=DOWN){
					direction_pre=UP;
				}
				break;
			case GLFW_KEY_DOWN:
				if(direction!=UP){
					direction_pre=DOWN;
				}
				break;
			case GLFW_KEY_LEFT:
				if(direction!=RIGHT){
					direction_pre=LEFT;
				}
				break;
			case GLFW_KEY_RIGHT:
				if(direction!=LEFT){
					direction_pre=RIGHT;
				}
				break;
		}
	}
}

void console_mode_input(){/*
//#ifdef __WIN32
	while(kbhit()){
		switch(getch())
	}*/
}

void output_map(){
#ifdef __WIN32
	HANDLE handle=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	WORD general=info.wAttributes;
	CONSOLE_CURSOR_INFO cursor_info;
	cursor_info.bVisible=false;
	SetConsoleCursorInfo(handle, &cursor_info);
	for(int y=0;y<map_height;y++){
		for(int x=0;x<map_width;x++){
			switch(map[x][y]){
				case MAP_EMPTY:
					if(!disable_white_background){
						SetConsoleTextAttribute(handle, 0xff);
					}
					printf("  ");
					break;
				case MAP_APPLE:
					SetConsoleTextAttribute(handle, BACKGROUND_RED);
					printf("  ");
					if(disable_white_background) {
						SetConsoleTextAttribute(handle, general);
					}
					break;
				case MAP_SNAKE:
					SetConsoleTextAttribute(handle, BACKGROUND_BLUE);
					printf("  ");
					if(disable_white_background) {
						SetConsoleTextAttribute(handle, general);
					}
					break;
			}
		}
		SetConsoleTextAttribute(handle, general);
		putchar('\n');
	}
	SetConsoleCursorPosition(handle,info.dwCursorPosition);
	cursor_info.bVisible=true;
	SetConsoleCursorInfo(handle, &cursor_info);
#else
	for(int y=0;y<map_height;y++){
		printf("%s",white_background);
		for(int x=0;x<map_width;x++){
			switch(map[x][y]){
				case MAP_EMPTY:
					printf("  ");
					break;
				case MAP_APPLE:
					printf("\033[41m  %s",white_background);
					break;
				case MAP_SNAKE:
					printf("\033[44m  %s",white_background);
					break;
			}
		}
		puts("\033[0m\033[K");
	}
	printf("\033[K\033[%dA",map_height);
#endif
}

void delay(int ms){
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
position random_pos(GLFWwindow *window){
	position p={rand()%map_width,rand()%map_height};
	set_window_pos(window,p);
	return p;
}
void set_window_pos(GLFWwindow *window,position p){
	if(window!=NULL){
		glfwSetWindowPos(window,WINDOW_WIDTH*p.x,WINDOW_HEIGHT*p.y);
	}
}
void generate_apple(){
	//map[apple.pos.x][apple.pos.y]=MAP_EMPTY;It should be SNAKE HEAD
	bool flag=true;
	while(flag){
		position apple_pos=random_pos(NULL);
		if(map[apple_pos.x][apple_pos.y]==MAP_EMPTY){
			set_window_pos(apple.window,apple_pos);
			apple.pos=apple_pos;
			map[apple.pos.x][apple.pos.y]=MAP_APPLE;
			flag=false;
		}
	}
}
void refresh(GLFWwindow *window){
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);
}
void log_error(const char *info){
#ifdef __WIN32
	HANDLE handle=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO console_screen_buffer_info;
	GetConsoleScreenBufferInfo(handle, &console_screen_buffer_info);
	SetConsoleTextAttribute(handle, FOREGROUND_RED);
	printf("%s", info);
	SetConsoleTextAttribute(handle, console_screen_buffer_info.wAttributes);
	putchar('\n');
#else
	fprintf(stderr,"\033[91m\033[1m%s\n\033[0m",info);
#endif
}
void end(int code){
	glfwTerminate();
	exit(code);
}
