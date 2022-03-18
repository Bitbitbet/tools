#include <SDL2/SDL.h>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
#include <atomic>

constexpr int WINDOW_WIDTH = 600;
constexpr int WINDOW_HEIGHT = 600;

SDL_Color next_color() {
	return {255, 0, 0, 255};
}

SDL_Color next_color(SDL_Color prv) {
	if(prv.r == 255) {
		if(prv.b == 0) {
			if(prv.g == 255) {
				--prv.r;
			} else {
				++prv.g;
			}
		} else {
			--prv.b;
		}
	} else if(prv.g == 255) {
		if(prv.r == 0) {
			if(prv.b == 255) {
				--prv.g;
			} else {
				++prv.b;
			}
		} else {
			--prv.r;
		}
	} else { // prv.b == 255
		if(prv.r == 0) {
			if(prv.g == 0) {
				++prv.r;
			} else {
				--prv.g;
			}
		} else {
			++prv.r;
		}
	}
	return prv;
}

std::atomic_int fps;
std::atomic_bool request_exit = false;

bool software_rendering = false;

void fps_displayer() {
	fps = 0;
	while(!request_exit) {
		std::this_thread::sleep_for(std::chrono::seconds(1));

		printf("Current fps: %d\n", static_cast<int>(fps));
		fps = 0;
	}
}

int main(int, char **) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer *render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(!window) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return 2;
	}

	if(!render) {
		fprintf(stderr, "%s\n", SDL_GetError());

		render = SDL_CreateSoftwareRenderer(SDL_GetWindowSurface(window));
		software_rendering = true;
		if(!render) {
			fprintf(stderr, "%s\n", SDL_GetError());
			return 3;
		}
	}

	std::thread t(fps_displayer);

	SDL_Color color = next_color();
	SDL_Event event;
	while(true) {
		SDL_PollEvent(&event);
		if(event.type == SDL_QUIT || event.type == SDL_MOUSEBUTTONDOWN) {
			request_exit = true;
			break;
		}

		SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);

		SDL_RenderFillRect(render, nullptr);

		if (software_rendering) {
			SDL_UpdateWindowSurface(window);
		} else {
			SDL_RenderPresent(render);
		}

		++fps;

		color = next_color(color);
	}

	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();

	t.join();

	return 0;
}
