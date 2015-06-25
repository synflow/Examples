#include "SDL.h"
#include <stdio.h> /* for fprintf() */
#include <stdlib.h> /* for atexit() */

#include "SobelFilter.h"

SDL_Surface *surface;

static SDL_Window *showSourceImage() {
	surface = SDL_LoadBMP("data/lena512.bmp");
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

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	SDL_DestroyRenderer(renderer);

	SDL_DestroyTexture(texture);

	return window;
}

static SDL_Window *createTargetWindow(SDL_Window *winSource) {
	int x, y, w, h;
	SDL_GetWindowPosition(winSource, &x, &y);
	SDL_GetWindowSize(winSource, &w, &h);

	SDL_Window *window = SDL_CreateWindow("target image", x + w + 20, y, 512, 512, 0);
	if (window == NULL) {
		// In the event that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		exit(1);
	}
	return window;
}

static void execute_stimulus(struct Stimulus *stimulus) {
	static int offset;

	// Get the requested pixel
	void *addr = &((uint8_t *)surface->pixels)[offset];

	uint8_t luminance;
	SDL_PixelFormat *fmt = surface->format;
	if (fmt->BitsPerPixel == 8) {
		luminance = *((uint8_t *)addr);
	}
	else {
		uint32_t pixel = *(uint32_t *)addr;

		Uint32 temp = pixel & fmt->Rmask;  /* Isolate red component */
		temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
		temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
		uint8_t red = (Uint8)temp;

		/* Get Green component */
		temp = pixel & fmt->Gmask;  /* Isolate green component */
		temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
		temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
		uint8_t green = (Uint8)temp;

		/* Get Blue component */
		temp = pixel & fmt->Bmask;  /* Isolate blue component */
		temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
		temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
		uint8_t blue = (Uint8)temp;

		luminance = ((66 * red + 129 * green + 25 * blue + 128) >> 8) + 16;
	}

	stimulus->pixels_next = luminance;
	stimulus->pixels_valid_next = 1;

	// update position
	offset += fmt->BytesPerPixel;
	if (offset == surface->w * surface->h) {
		offset = 0;
	}
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
	SDL_Window *winTarget = createTargetWindow(winSource);
	SDL_Renderer *renderer = SDL_CreateRenderer(winTarget, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderClear(renderer);

	struct SobelFilter_app sobelFilter = { 0 };
	printf("sizeof sobelFilter = %i\n", sizeof(sobelFilter));
	setup_SobelFilter_app(&sobelFilter);

	sobelFilter.stimulus.execute = execute_stimulus;

	sobelFilter.init(&sobelFilter);

	int i = 0;
	while (1) {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
				// closing any of the windows quits the application
				break;
			}
		}

		sobelFilter.execute(&sobelFilter);
		uint8_t pix = (uint8_t)sobelFilter.kernel.res;

		SDL_SetRenderDrawColor(renderer, pix, pix, pix, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawPoint(renderer, i % 512, i / 512);
		if (i % 512 == 0) {
			SDL_RenderPresent(renderer);
		}
		i++;
	}

	SDL_DestroyRenderer(renderer);

	// Close and destroy windows
	SDL_DestroyWindow(winSource);
	SDL_DestroyWindow(winTarget);

	return 0;
}
