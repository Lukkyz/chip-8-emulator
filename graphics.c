#include "./graphics.h"

SDL_Window *window;
SDL_Renderer *renderer;

int Init_Window() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, DISPLAY_WIDTH * 20,
                            DISPLAY_HEIGHT * 20, SDL_WINDOW_SHOWN);

  if (!window) {
    fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer) {
    fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  return 0;
}

void Free_Window() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
}

void Draw_Display(bool display[32][64]) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  for (int i = 0; i < DISPLAY_WIDTH; i++) {
    for (int j = 0; j < DISPLAY_HEIGHT; j++) {
      bool is_set = display[j][i];
      if (is_set) {
        SDL_Rect rect = {i * 20, j * 20, 18, 18};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
      }
    }
  }
  SDL_RenderPresent(renderer);
}
