#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH 64

int Init_Window();
void Free_Window();
void Draw_Display(bool display[32][64]);
