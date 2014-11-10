#ifndef SDL_H_
#define SDL_H_

#include <string>

const unsigned int COL_BLACK = 0;
const unsigned int COL_WHITE = 0xFFFFFFFF;
const unsigned int COL_GREY = 0xFF666699;
const unsigned int COL_RED1 = 0xFFFF0000;
const unsigned int COL_RED2 = 0xFF990000;
const unsigned int COL_ORANGE = 0xFFFF9900;

void sdl_init(int width, int height, int vwidth, int vheight);
void sdl_draw_text(const std::string& text, int x, int y);
void sdl_fill_pixel(int x, int y, unsigned int colour);
void sdl_fill_rect(int x, int y, int w, int h, unsigned int colour);
void sdl_delay(int ms);
void sdl_predraw();
void sdl_copybuffer();
void sdl_postdraw(const std::string& filebase);

#endif /* SDL_H_ */
