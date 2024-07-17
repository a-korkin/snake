#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define STEP 32

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    bool running;
} state_t;

state_t *init(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    state_t *state = (state_t*) malloc(sizeof(state_t));
    if (!state) {
        fprintf(stderr, 
            "Couldn't allocate memory for state: %s\n", SDL_GetError());
        exit(1);
    }
    state->window = SDL_CreateWindow(
        "Snake",
         SDL_WINDOWPOS_CENTERED,
         SDL_WINDOWPOS_CENTERED,
         SCREEN_W, 
         SCREEN_H,
         SDL_WINDOW_SHOWN);
    if (!state->window) {
        fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
        exit(1);
    }
    state->renderer = SDL_CreateRenderer(
        state->window, -1, SDL_RENDERER_SOFTWARE);
    if (!state->renderer) {
        fprintf(stderr, "Couldn't create renderer: %s\n", SDL_GetError());
        exit(1);
    }
    state->running = true;
}

void handle_input(state_t *state) {
    SDL_PollEvent(&state->event);
    if (state->event.type == SDL_QUIT) {
        state->running = false;
    }
    if (state->event.type == SDL_KEYDOWN) {
        switch (state->event.key.keysym.sym) {
            case SDLK_ESCAPE: state->running = false; break;
            case SDLK_UP: fprintf(stdout, "up\n"); break;
            case SDLK_DOWN: fprintf(stdout, "down\n"); break;
            case SDLK_LEFT: fprintf(stdout, "left\n"); break;
            case SDLK_RIGHT: fprintf(stdout, "right\n"); break;
            default: break;
        }
    }
}

void draw_background(state_t *state) {
    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(state->renderer);
}

void draw(state_t *state) {
    draw_background(state);
    SDL_RenderPresent(state->renderer);
}

void clear(state_t *state) {
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    SDL_Quit();
    free(state);
}

int main(void) {
    state_t *state = init();
    while (state->running) {
        handle_input(state);
        draw(state);
    }
    clear(state);

    return 0;
}
