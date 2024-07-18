#include <SDL2/SDL_error.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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

typedef struct node {
    SDL_FRect position;
    struct node *next;
} node_t;

typedef struct {
    node_t *head;
    node_t *tail;
    SDL_FPoint velocity;
    direction_t direction;
} snake_t;

typedef struct {
    SDL_FPoint position;
} apple_t;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    bool running;
    snake_t *snake;
    apple_t *apple;
    int score;
    TTF_Font *font;
    SDL_Surface *text_surface;
    SDL_Texture *text_texture;
} state_t;

void set_random_position(int *x, int *y) {
    srand(time(NULL));
    *x = (int) ((rand() % SCREEN_W) / (float) STEP) * STEP;
    *y = (int) ((rand() % SCREEN_H) / (float) STEP) * STEP;
}

snake_t *create_snake(void) {
    snake_t *snake = (snake_t *) malloc(sizeof(snake_t));
    if (!snake) {
        fprintf(stderr, 
            "Couldn't allocate memory for snake: %s\n", SDL_GetError());
        exit(1);
    }
    node_t *head = (node_t *) malloc(sizeof(node_t));
    if (!head) {
        fprintf(stderr, 
            "Couldn't allocate memory for head: %s\n", SDL_GetError());
        exit(1);
    }
    head->position = (SDL_FRect) { 
        .x = 320, 
        .y = SCREEN_H - STEP, 
        .w = STEP, 
        .h = STEP 
    };
    snake->head = head;
    snake->tail = head;
    snake->velocity = (SDL_FPoint) { 
        .x = snake->head->position.x, 
        .y = snake->head->position.y 
    };
    snake->direction = UP;
    return snake;
}

apple_t *create_apple(void) {
    apple_t *apple = (apple_t*) malloc(sizeof(apple_t));
    if (!apple) {
        fprintf(stderr, 
            "Couldn't allocate memory for apple: %s", SDL_GetError());
        exit(1);
    }
    int x, y;
    set_random_position(&x, &y);
    apple->position.x = x;
    apple->position.y = y;
    return apple;
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
    state->apple = create_apple();
    state->score = 0;
    last_frame_time = SDL_GetTicks();
    if (TTF_Init() < 0) {
        fprintf(stderr, "Couldn't initialize TTF: %s\n", TTF_GetError());
        exit(1);
    }
    state->font = TTF_OpenFont("assets/fonts/OpenSans-Regular.ttf", 12);
    if (!state->font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        exit(1);
    }

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

void out_off_bound(state_t *state) {
    if (state->snake->velocity.x + STEP <= 0) { 
        state->snake->velocity.x = state->snake->head->position.x = SCREEN_W - STEP; 
    }
    if (state->snake->velocity.x > SCREEN_W) {
        state->snake->velocity.x = state->snake->head->position.x = 0;
    }
    if (state->snake->velocity.y + STEP <= 0) { 
        state->snake->velocity.y = state->snake->head->position.y = SCREEN_H - STEP; 
    }
    if (state->snake->velocity.y > SCREEN_H) {
        state->snake->velocity.y = state->snake->head->position.y = 0;
    }
}

void add_node(snake_t *snake) {
    node_t *node = (node_t *) malloc(sizeof(node_t));
    if (!node) {
        fprintf(stderr, 
            "Couldn't allocate memory for tail: %s\n", SDL_GetError());
        exit(1);
    }
    fprintf(stdout, "h_x: %.2f, h_y: %.2f, t_x: %.2f, t_y: %.2f\n", 
            snake->head->position.x, snake->head->position.y,
            snake->tail->position.x, snake->tail->position.y);
    float x, y; 
    switch (snake->direction) {
        case UP: 
            x = snake->tail->position.x;
            y = snake->tail->position.y + STEP;
            break;
        case DOWN:
            x = snake->tail->position.x;
            y = snake->tail->position.y - STEP;
            break; 
        case LEFT:
            x = snake->tail->position.x + STEP;
            y = snake->tail->position.y;
            break; 
        case RIGHT:
            x = snake->tail->position.x - STEP;
            y = snake->tail->position.y;
            break; 
    }
    node->position.x = x;
    node->position.y = y;
    node->position.w = STEP;
    node->position.h = STEP;
    node->next = NULL;

    snake->tail->next = node;
    snake->tail = node;
}

void hit(state_t *state) {
    state->score++;
    add_node(state->snake);

    int x, y;
    set_random_position(&x, &y);
    state->apple->position.x = x;
    state->apple->position.y = y;
    fprintf(stdout, "score: %d\n", state->score);
}

void check_collision(state_t *state) {
    if ((int) state->snake->head->position.x == (int) state->apple->position.x
        && (int) state->snake->head->position.y == (int) state->apple->position.y) {
        hit(state);
    }
}

void update_snake(state_t *state, float delta_time) {
    switch (state->snake->direction) {
        case UP: state->snake->velocity.y -= STEP * delta_time; break;
        case DOWN: state->snake->velocity.y += STEP * delta_time; break;
        case LEFT: state->snake->velocity.x -= STEP * delta_time; break;
        case RIGHT: state->snake->velocity.x += STEP * delta_time; break;
    }
    out_off_bound(state);
    if ((int) state->snake->velocity.x % STEP == 0) {
        state->snake->head->position.x = state->snake->velocity.x;
    }
    if ((int) state->snake->velocity.y % STEP == 0) {
        state->snake->head->position.y = state->snake->velocity.y;
    }
    node_t *current = state->snake->head;
    while (current->next) {
        switch (state->snake->direction) {
            case UP: 
                current->next->position.x = current->position.x;
                current->next->position.y = current->position.y + STEP; 
                break;
            case DOWN: 
                current->next->position.x = current->position.x;
                current->next->position.y = current->position.y - STEP; 
                break;
            case LEFT: 
                current->next->position.x = current->position.x + STEP; 
                current->next->position.y = current->position.y; 
                break;
            case RIGHT: 
                current->next->position.x = current->position.x - STEP; 
                current->next->position.y = current->position.y; 
                break;
        }
        current = current->next;
    }
    check_collision(state);
}

void update(state_t *state) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks();
    update_snake(state, delta_time);
}

void draw_background(state_t *state) {
    SDL_SetRenderDrawColor(state->renderer, 54, 69, 79, 0xFF);
    SDL_RenderClear(state->renderer);
}

void draw_grid(state_t *state) {
    SDL_SetRenderDrawColor(state->renderer, 137, 148, 153, 0xFF);
    int i;
    for (i = STEP; i < SCREEN_W; i += STEP) {
        SDL_RenderDrawLine(state->renderer, i, 0, i, SCREEN_H);
    }
    for (i = STEP; i < SCREEN_H; i += STEP) {
        SDL_RenderDrawLine(state->renderer, 0, i, SCREEN_W, i);
    }
}

void draw_score(state_t *state) {
    SDL_Color white = {255, 255, 255};
    char *text = (char *) malloc(sizeof(char) * 10);
    sprintf(text, "score: %2d", state->score);
    state->text_surface = TTF_RenderText_Solid(state->font, text, white);
    state->text_texture = SDL_CreateTextureFromSurface(
            state->renderer, state->text_surface);
    SDL_Rect score_rect = {
        .x = SCREEN_W / 2 - 50,
        .y = 0,
        .w = 100,
        .h = 50,
    };
    SDL_RenderCopy(state->renderer, state->text_texture, NULL, &score_rect);
}

void draw_snake(state_t *state) {
    SDL_SetRenderDrawColor(state->renderer, 80, 200, 120, 0xFF);

    node_t *n = state->snake->head;
    while (n) {
        if (SDL_RenderDrawRectF(state->renderer, &n->position) < 0) {
            fprintf(stderr, "Couldn't draw node: %s\n", SDL_GetError());
            exit(1);
        }
        if (SDL_RenderFillRectF(state->renderer, &n->position) < 0) {
            fprintf(stderr, "Couldn't fill rect: %s\n", SDL_GetError());
            exit(1);
        }
        n = n->next;
    }
}

void draw_apple(state_t *state) {
    SDL_SetRenderDrawColor(state->renderer, 210, 43, 43, 0xFF);
    SDL_FRect rect = { 
        .x = state->apple->position.x, 
        .y = state->apple->position.y,
        .w = STEP,
        .h = STEP,
    };
    SDL_RenderDrawRectF(state->renderer, &rect);
    SDL_RenderFillRectF(state->renderer, &rect);
}

void draw(state_t *state) {
    draw_background(state);
    draw_grid(state);
    draw_score(state);
    draw_snake(state);
    draw_apple(state);
    SDL_RenderPresent(state->renderer);
}

void clear(state_t *state) {
    // SDL_FreeSurface(state->text_surface);
    // SDL_DestroyTexture(state->text_texture);
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    TTF_Quit();
    SDL_Quit();
    free(state->snake);
    free(state->apple);
    free(state);
}

int main(void) {
    state_t *state = init();
    while (state->running) {
        handle_input(state);
        update(state);
        draw(state);
    }
    clear(state);

    return 0;
}
