#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define SCREEN_W 640
#define SCREEN_H 480
#define STEP 32
#define FPS 60
#define FRAME_TARGET_TIME (1000.f / FPS)

int last_frame_time = 0;

typedef enum {
    UP,
    DOWN,
    LEFT, 
    RIGHT,
} direction_t;

typedef struct {
    SDL_FRect position;
    SDL_FPoint velocity;
    direction_t direction;
} snake_t;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    bool running;
    snake_t *snake;
} state_t;

void set_random_position(int *x, int *y) {
    srand(time(NULL));
    *x = (int) ((rand() % SCREEN_W) / (float) STEP) * STEP;
    *y = (int) ((rand() % SCREEN_H) / (float) STEP) * STEP;
}

snake_t *create_snake(void) {
    snake_t *snake = (snake_t*) malloc(sizeof(snake_t));
    if (!snake) {
        fprintf(stderr, 
            "Couldn't allocate memory for snake: %s\n", SDL_GetError());
        exit(1);
    }
    int x, y;
    // set_random_position(&x, &y);
    snake->position = (SDL_FRect) { 
        .x = 320, 
        .y = SCREEN_H - STEP, 
        .w = STEP, 
        .h = STEP 
    };
    snake->velocity = (SDL_FPoint) { 
        .x = snake->position.x, 
        .y = snake->position.y 
    };
    snake->direction = UP;
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
    last_frame_time = SDL_GetTicks();
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
            case SDLK_UP: state->snake->direction = UP; break;
            case SDLK_DOWN: state->snake->direction = DOWN; break;
            case SDLK_LEFT: state->snake->direction = LEFT; break;
            case SDLK_RIGHT: state->snake->direction = RIGHT; break;
            default: break;
        }
    }
}

void update(state_t *state) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();

    switch (state->snake->direction) {
        case UP: state->snake->velocity.y -= STEP * delta_time; break;
        case DOWN: state->snake->velocity.y += STEP * delta_time; break;
        case LEFT: state->snake->velocity.x -= STEP * delta_time; break;
        case RIGHT: state->snake->velocity.x += STEP * delta_time; break;
    }
    if ((int) state->snake->velocity.x % STEP == 0) {
        state->snake->position.x = state->snake->velocity.x;
    }
    if ((int) state->snake->velocity.y % STEP == 0) {
        state->snake->position.y = state->snake->velocity.y;
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
    SDL_RenderDrawRectF(state->renderer, &state->snake->position);
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
    free(state->snake);
    free(state);
}

int main(void) {
    state_t *state = init();
    while (state->running) {
        handle_input(state);
        update(state);
        draw(state);
        SDL_Delay(20);
    }
    clear(state);

    return 0;
}
