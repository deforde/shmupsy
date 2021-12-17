#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_image.h>

#ifdef _WIN32
#undef main
#endif

// ============================================================================
// Global definitions
// ============================================================================

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define SPACESHIP_VELOCITY_PPS 320
#define SPACESHIP_FIRERATE_PPS 3

#define MAX_NUM_PROJECTILES 1024
#define PROJECTILE_VELOCITY_PPS 640

const char* spaceship_img = "data/ship.png";
const char* projectile_img = "data/laser-bolts.png";

enum {
    SPACESHIP_STATIONARY_1,
    SPACESHIP_STATIONARY_2,
    SPACESHIP_BANK_LEFT_1,
    SPACESHIP_BANK_LEFT_2,
    SPACESHIP_BANK_HARD_LEFT_1,
    SPACESHIP_BANK_HARD_LEFT_2,
    SPACESHIP_BANK_RIGHT_1,
    SPACESHIP_BANK_RIGHT_2,
    SPACESHIP_BANK_HARD_RIGHT_1,
    SPACESHIP_BANK_HARD_RIGHT_2,
    SPACESHIP_SPRITES_TOTAL
};

enum {
    PROJECTILE_1,
    PROJECTILE_2,
    PROJECTILE_SPRITES_TOTAL
};

SDL_Rect spaceship_sprite_quads[SPACESHIP_SPRITES_TOTAL];
SDL_Rect projectile_sprite_quads[PROJECTILE_SPRITES_TOTAL];

typedef struct {
    int32_t x;
    int32_t y;
} vector_t;

#define ENTITY_STRUCT_BODY                              \
{                                                       \
    vector_t position;                                  \
    vector_t velocity;                                  \
    SDL_Rect* sprite_quad;                              \
    uint32_t sprite_scaling;                            \
    SDL_Rect render_quad;                               \
    int32_t num_animation_frames;                       \
    int32_t animation_idx;                              \
    int32_t num_rendered_frames_per_animation_frame;    \
    int32_t rendered_frame_idx;                         \
}

typedef struct ENTITY_STRUCT_BODY entity_t;

typedef struct {
    struct ENTITY_STRUCT_BODY;

    bool is_firing;
    float time_till_next_shot_s;
} spaceship_t;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Texture* spaceship_texture;
    spaceship_t spaceship;

    SDL_Texture* projectile_texture;
    entity_t projectiles[MAX_NUM_PROJECTILES];
    size_t projectiles_count;
} game_state_t;

game_state_t state;

// ============================================================================
// Forward declarations
// ============================================================================

void init();
void destroy();
void update_state();
void handle_event(const SDL_Event* const event);
void render();

SDL_Texture* load_texture(const char* const filename);

void spawn_projectile();

// ============================================================================
// Function implementations
// ============================================================================

void init()
{
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_1].x = 0;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_1].y = 0;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_1].w = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_1].h = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_LEFT_1].x = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_LEFT_1].y = 0;
    spaceship_sprite_quads[SPACESHIP_BANK_LEFT_1].w = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_LEFT_1].h = 24;
    spaceship_sprite_quads[SPACESHIP_STATIONARY_1].x = 32;
    spaceship_sprite_quads[SPACESHIP_STATIONARY_1].y = 0;
    spaceship_sprite_quads[SPACESHIP_STATIONARY_1].w = 16;
    spaceship_sprite_quads[SPACESHIP_STATIONARY_1].h = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_RIGHT_1].x = 48;
    spaceship_sprite_quads[SPACESHIP_BANK_RIGHT_1].y = 0;
    spaceship_sprite_quads[SPACESHIP_BANK_RIGHT_1].w = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_RIGHT_1].h = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_1].x = 64;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_1].y = 0;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_1].w = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_1].h = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_2].x = 0;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_2].y = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_2].w = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_2].h = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_LEFT_2].x = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_LEFT_2].y = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_LEFT_2].w = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_LEFT_2].h = 24;
    spaceship_sprite_quads[SPACESHIP_STATIONARY_2].x = 32;
    spaceship_sprite_quads[SPACESHIP_STATIONARY_2].y = 24;
    spaceship_sprite_quads[SPACESHIP_STATIONARY_2].w = 16;
    spaceship_sprite_quads[SPACESHIP_STATIONARY_2].h = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_RIGHT_2].x = 48;
    spaceship_sprite_quads[SPACESHIP_BANK_RIGHT_2].y = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_RIGHT_2].w = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_RIGHT_2].h = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_2].x = 64;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_2].y = 24;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_2].w = 16;
    spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_2].h = 24;

    projectile_sprite_quads[PROJECTILE_1].x = 0;
    projectile_sprite_quads[PROJECTILE_1].y = 0;
    projectile_sprite_quads[PROJECTILE_1].w = 16;
    projectile_sprite_quads[PROJECTILE_1].h = 32;
    projectile_sprite_quads[PROJECTILE_2].x = 16;
    projectile_sprite_quads[PROJECTILE_2].y = 0;
    projectile_sprite_quads[PROJECTILE_2].w = 16;
    projectile_sprite_quads[PROJECTILE_2].h = 32;

    state.window = NULL;
    state.renderer = NULL;

    state.spaceship_texture = NULL;
    state.projectile_texture = NULL;

    state.spaceship.sprite_scaling = 2;
    state.spaceship.position.x = SCREEN_WIDTH / 2;
    state.spaceship.position.y = SCREEN_HEIGHT - 1 - spaceship_sprite_quads[SPACESHIP_STATIONARY_1].h * state.spaceship.sprite_scaling / 2;
    state.spaceship.velocity.x = 0;
    state.spaceship.velocity.y = 0;
    state.spaceship.sprite_quad = NULL;
    state.spaceship.render_quad.x = 0;
    state.spaceship.render_quad.y = 0;
    state.spaceship.render_quad.w = 0;
    state.spaceship.render_quad.h = 0;
    state.spaceship.num_animation_frames = 2;
    state.spaceship.animation_idx = 0;
    state.spaceship.num_rendered_frames_per_animation_frame = 4;
    state.spaceship.rendered_frame_idx = 0;
    state.spaceship.is_firing = false;
    state.spaceship.time_till_next_shot_s = 0.0F;

    state.projectiles_count = 0;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    state.window = SDL_CreateWindow(
        "sdl-test",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if(state.window == NULL) {
        fprintf(stderr, "SDL window could not be created: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(state.renderer == NULL) {
        fprintf(stderr, "SDL renderer could not be created: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags)) {
        fprintf(stderr, "SDL image could no be initialised: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    state.spaceship_texture = load_texture(spaceship_img);
    state.projectile_texture = load_texture(projectile_img);
}

void destroy()
{
    SDL_DestroyTexture(state.projectile_texture);
    state.projectile_texture = NULL;

    SDL_DestroyTexture(state.spaceship_texture);
    state.spaceship_texture = NULL;

    SDL_DestroyRenderer(state.renderer);
    state.renderer = NULL;

    SDL_DestroyWindow(state.window);
    state.window = NULL;

    IMG_Quit();
    SDL_Quit();
}

void handle_event(const SDL_Event* const event)
{
    // Set the spaceship's velocity
    if(event->type == SDL_KEYDOWN && event->key.repeat == 0) {
        switch(event->key.keysym.sym) {
            case SDLK_UP: state.spaceship.velocity.y -= SPACESHIP_VELOCITY_PPS; break;
            case SDLK_DOWN: state.spaceship.velocity.y += SPACESHIP_VELOCITY_PPS; break;
            case SDLK_LEFT: state.spaceship.velocity.x -= SPACESHIP_VELOCITY_PPS; break;
            case SDLK_RIGHT: state.spaceship.velocity.x += SPACESHIP_VELOCITY_PPS; break;
            case SDLK_SPACE: state.spaceship.is_firing = true; break;
        }
    }
    else if(event->type == SDL_KEYUP && event->key.repeat == 0) {
        switch(event->key.keysym.sym) {
            case SDLK_UP: state.spaceship.velocity.y += SPACESHIP_VELOCITY_PPS; break;
            case SDLK_DOWN: state.spaceship.velocity.y -= SPACESHIP_VELOCITY_PPS; break;
            case SDLK_LEFT: state.spaceship.velocity.x += SPACESHIP_VELOCITY_PPS; break;
            case SDLK_RIGHT: state.spaceship.velocity.x -= SPACESHIP_VELOCITY_PPS; break;
            case SDLK_SPACE:
                state.spaceship.is_firing = false;
                state.spaceship.time_till_next_shot_s = 0.0F;
                break;
        }
    }
}

void update_state()
{
    // Get the time delta since the last update
    const uint32_t current_time_ms = SDL_GetTicks();
    static bool time_initialised = false;
    static uint32_t last_update_time_ms = 0;
    if(!time_initialised) {
        last_update_time_ms = current_time_ms;
        time_initialised = true;
    }
    const float time_delta_s = (current_time_ms - last_update_time_ms) / 1000.0F;
    last_update_time_ms = current_time_ms;

    // Select the correct spaceship sprite and set the rendering quad dimensions accordingly
    state.spaceship.sprite_quad = &spaceship_sprite_quads[SPACESHIP_STATIONARY_1 + state.spaceship.animation_idx];
    if(state.spaceship.velocity.x < 0) {
        state.spaceship.sprite_quad = &spaceship_sprite_quads[SPACESHIP_BANK_HARD_LEFT_1 + state.spaceship.animation_idx];
    }
    else if(state.spaceship.velocity.x > 0) {
        state.spaceship.sprite_quad = &spaceship_sprite_quads[SPACESHIP_BANK_HARD_RIGHT_1 + state.spaceship.animation_idx];
    }
    state.spaceship.render_quad.w = state.spaceship.sprite_quad->w * state.spaceship.sprite_scaling;
    state.spaceship.render_quad.h = state.spaceship.sprite_quad->h * state.spaceship.sprite_scaling;

    // Update the spaceship's position
    state.spaceship.position.x += (int32_t)(state.spaceship.velocity.x * time_delta_s);
    state.spaceship.position.y += (int32_t)(state.spaceship.velocity.y * time_delta_s);
    const int32_t spaceship_min_x = (state.spaceship.render_quad.w / 2);
    const int32_t spaceship_max_x = SCREEN_WIDTH - (state.spaceship.render_quad.w / 2);
    const int32_t spaceship_min_y = (state.spaceship.render_quad.h / 2);
    const int32_t spaceship_max_y = SCREEN_HEIGHT - (state.spaceship.render_quad.h / 2);
    state.spaceship.position.x = state.spaceship.position.x < spaceship_min_x ? spaceship_min_x : state.spaceship.position.x;
    state.spaceship.position.x = state.spaceship.position.x >= spaceship_max_x ? (spaceship_max_x - 1) : state.spaceship.position.x;
    state.spaceship.position.y = state.spaceship.position.y < spaceship_min_y ? spaceship_min_y : state.spaceship.position.y;
    state.spaceship.position.y = state.spaceship.position.y >= spaceship_max_y ? (spaceship_max_y - 1) : state.spaceship.position.y;

    // Update the spaceship's rendering quad origin
    state.spaceship.render_quad.x = state.spaceship.position.x - state.spaceship.render_quad.w / 2;
    state.spaceship.render_quad.y = state.spaceship.position.y - state.spaceship.render_quad.h / 2;

    // Update the spaceship's animation indices as needed
    ++state.spaceship.rendered_frame_idx;
    if(state.spaceship.rendered_frame_idx == state.spaceship.num_rendered_frames_per_animation_frame) {
        state.spaceship.rendered_frame_idx = 0;
        ++state.spaceship.animation_idx;
        state.spaceship.animation_idx %= state.spaceship.num_animation_frames;
    }

    // If the ship is firing, and is ready to generate a new projectile, then do so now
    if(state.spaceship.is_firing) {
        if(state.spaceship.time_till_next_shot_s <= 0.0F) {
            spawn_projectile();
            state.spaceship.time_till_next_shot_s = 1.0F / SPACESHIP_FIRERATE_PPS;
        }
        else {
            state.spaceship.time_till_next_shot_s -= time_delta_s;
        }
    }

    // Update all projectile's positions and animations
    for(size_t i = 0; i < state.projectiles_count; ++i) {
        entity_t* projectile = &state.projectiles[i];
        projectile->sprite_quad = &projectile_sprite_quads[PROJECTILE_1 + projectile->animation_idx];

        projectile->position.y += (int32_t)(projectile->velocity.y * time_delta_s);
        projectile->render_quad.y = projectile->position.y - projectile->render_quad.h / 2;

        ++projectile->rendered_frame_idx;
        if(projectile->rendered_frame_idx == projectile->num_rendered_frames_per_animation_frame) {
            projectile->rendered_frame_idx = 0;
            ++projectile->animation_idx;
            projectile->animation_idx %= projectile->num_animation_frames;
        }
    }
    // Remove all projectiles that have exited the screen
    for(size_t i = 0; i < state.projectiles_count;) {
        if(state.projectiles[i].position.y < 0) {
            state.projectiles[i] = state.projectiles[state.projectiles_count - 1];
            state.projectiles_count--;
            continue;
        }
        ++i;
    }
}

void render()
{
    SDL_SetRenderDrawColor(state.renderer, 0x0, 0x0, 0x0, 0xFF);
    SDL_RenderClear(state.renderer);

    SDL_RenderCopy(state.renderer, state.spaceship_texture, state.spaceship.sprite_quad, &state.spaceship.render_quad);
    for(size_t i = 0; i < state.projectiles_count; ++i) {
        entity_t* projectile = &state.projectiles[i];
        SDL_RenderCopy(state.renderer, state.projectile_texture, projectile->sprite_quad, &projectile->render_quad);
    }

    SDL_RenderPresent(state.renderer);
}

SDL_Texture* load_texture(const char* const filename)
{
    SDL_Surface* surface = IMG_Load(filename);
    if(surface == NULL) {
        fprintf(stderr, "Failed to load image from: \"%s\": %s\n", filename, IMG_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(state.renderer, surface);
    if(texture == NULL) {
        fprintf(stderr, "SDL texture could not be created: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_FreeSurface(surface);

    return texture;
}

void spawn_projectile()
{
    if(state.projectiles_count < MAX_NUM_PROJECTILES) {
        entity_t* projectile = &state.projectiles[state.projectiles_count++];

        projectile->position.x = state.spaceship.position.x;
        projectile->position.y = state.spaceship.position.y + state.spaceship.render_quad.h / 2;

        projectile->velocity.x = 0;
        projectile->velocity.y = -PROJECTILE_VELOCITY_PPS;

        projectile->sprite_quad = &projectile_sprite_quads[PROJECTILE_1];
        projectile->sprite_scaling = 2;

        projectile->render_quad.w = projectile->sprite_quad->w * projectile->sprite_scaling;
        projectile->render_quad.h = projectile->sprite_quad->h * projectile->sprite_scaling;
        projectile->render_quad.x = projectile->position.x - projectile->render_quad.w / 2;
        projectile->render_quad.y = projectile->position.y - projectile->render_quad.h / 2;

        projectile->num_animation_frames = 2;
        projectile->animation_idx = 0;
        projectile->num_rendered_frames_per_animation_frame = 4;
        projectile->rendered_frame_idx = 0;
    }
}

// ============================================================================
// Main entry point
// ============================================================================

int main(int argc, char* argv[])
{
    init();

    bool running = true;
    SDL_Event event;

    while(running) {
        // Poll for events
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
            }
            else {
                handle_event(&event);
            }
        }

        // Update game state
        update_state();

        // Render
        render();
    }

    destroy();

    return EXIT_SUCCESS;
}
