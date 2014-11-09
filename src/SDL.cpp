#include <cstdlib>
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <string>
#include <map>
#include <time.h>


using namespace std;

static TTF_Font *font;
static SDL_Surface *screen = NULL;

void sdl_init(int width, int height) {
	SDL_Init( SDL_INIT_EVERYTHING);

	SDL_WM_SetCaption("Infection Simulation", NULL);

//Set up the screen
	screen = SDL_SetVideoMode(width, height, 32,
			SDL_DOUBLEBUF);
	SDL_ShowCursor(SDL_DISABLE);

   // Initialize SDL_ttf library
   if (TTF_Init() != 0) {
      SDL_Quit();
   }

   // Load a font
   font = TTF_OpenFont("src/FreeSans.ttf", 24);
   if (font == NULL) {
      TTF_Quit();
      SDL_Quit();
   }

}

void sdl_draw_text(const string& text, int x, int y) {
	SDL_Colour col = {0x00, 0x00, 0x00};
	SDL_Surface* surface  = TTF_RenderText_Solid(font, text.c_str(), col);
	SDL_Rect target_rect = surface->clip_rect;
	target_rect.x = x, target_rect.y = y;
	SDL_BlitSurface(surface, &surface->clip_rect, screen, &target_rect);
	SDL_FreeSurface(surface);
}

void sdl_fill_rect(int x, int y, int w, int h, unsigned int colour) {
	SDL_Rect rect {(Sint16)x, (Sint16)y, (Uint16)w, (Uint16)h};
	SDL_FillRect(screen, &rect, colour);
}

void sdl_delay(int ms) {
	SDL_Event event;
    /* Check for new events */
    while(SDL_PollEvent(&event)) {
        /* If a quit event has been sent */
        if (event.type == SDL_QUIT) {
        	exit(0);
        }
    }
	SDL_Delay(ms);
}

void sdl_predraw() {
	if (screen == NULL) {
		sdl_init(640,480);
	}

	SDL_FillRect(screen, NULL, 0);
}

void sdl_postdraw() {
	SDL_Flip(screen);
}
#include <iostream>
