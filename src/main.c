#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define STEP 32

typedef struct {
    SDL_FRect position;
} snake_t;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    bool running;
    snake_t *snake;
} state_t;

snake_t *create_snake(void) {
    snake_t *snake = (snake_t*) malloc(sizeof(snake_t));
    if (!snake) {
        fprintf(stderr, 
            "Couldn't allocate memory for snake: %s\n", SDL_GetError());
        exit(1);
    }
    srand(time(NULL));
    int x = (int) ((rand() % SCREEN_W) / (float) STEP);
    int y = (int) ((rand() % SCREEN_H) / (float) STEP);
    snake->position = (SDL_FRect) { 
        .x = x * STEP, 
        .y = y * STEP, 
        .w = STEP, 
        .h = STEP 
    };
    return snake;
}

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
    state->snake = create_snake();
    return state;
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

void draw_grid(state_t *state) {
    SDL_SetRenderDrawColor(state->renderer, 0x00, 0x00, 0x00, 0xFF);
    int i;
    for (i = STEP; i < SCREEN_W; i += STEP) {
        SDL_RenderDrawLine(state->renderer, i, 0, i, SCREEN_H);
    }
    for (i = STEP; i < SCREEN_H; i += STEP) {
        SDL_RenderDrawLine(state->renderer, 0, i, SCREEN_W, i);
    }
}

void draw_snake(state_t *state) {
    SDL_SetRenderDrawColor(state->renderer, 0x00, 0xFF, 0x00, 0xFF);
    SDL_RenderFillRectF(state->renderer, &state->snake->position);
}

void draw(state_t *state) {
    draw_background(state);
    draw_grid(state);
    draw_snake(state);
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
