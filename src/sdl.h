#ifndef SDL_H_
#define SDL_H_

#include <string>

void sdl_init(int width, int height);
void sdl_draw_text(const std::string& text, int x, int y);
void sdl_fill_rect(int x, int y, int w, int h, unsigned int colour);
void sdl_delay(int ms);
void sdl_predraw();
void sdl_postdraw();

#endif /* SDL_H_ */
