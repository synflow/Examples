#include "SDL.h"
#include <stdio.h> /* for fprintf() */
#include <stdlib.h> /* for atexit() */

static SDL_Window *showSourceImage() {
	SDL_Surface *surface = SDL_LoadBMP("data/lena512.bmp");
	if (surface == NULL) {
		// In the event that the window could not be made...
		printf("Could not load bitmap: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Window *window = SDL_CreateWindow("source image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, 0);
	if (window == NULL) {
		// In the event that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	SDL_DestroyRenderer(renderer);

	SDL_DestroyTexture(texture);

	return window;
}

static SDL_Window *createTargetWindow() {
	SDL_Window *window = SDL_CreateWindow("target image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, 0);
	if (window == NULL) {
		// In the event that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		exit(1);
	}
	return window;
}

int main(int argc, char** a2rgv) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr,
			"\nUnable to initialize SDL:  %s\n",
			SDL_GetError()
			);
		return 1;
	}
	atexit(SDL_Quit);

	SDL_Window *winSource = showSourceImage();
	SDL_Window *winTarget = createTargetWindow();

	while (1) {
		SDL_Event event;
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT) {
			break;
		}
		else if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				// closing any of the windows quits the application
				break;
			}
		}
	}

	// Close and destroy windows
	SDL_DestroyWindow(winSource);
	SDL_DestroyWindow(winTarget);

	return 0;
}
