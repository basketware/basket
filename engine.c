// the brain.

#include <SDL2/SDL_messagebox.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>

#define BASKET_INTERNAL
#include "basket.h"

static f64 hz;

static bool running = true;
static bool focused = true;

static u16 window_width  = 960;
static u16 window_height = 700;
static bool is_debug = false;

void event(SDL_Event event, SDL_Window *window) {
    static bool fullscreen = false;

    inp_event(event);

    switch (event.type) {
        case SDL_QUIT: {
            running = false;

            break;
        }

        case SDL_KEYDOWN: {
            switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_F11: {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(window, fullscreen);
                }

                default: break;
            }
            break;
        }

        case SDL_WINDOWEVENT: {
            switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    window_width = event.window.data1;
                    window_height = event.window.data2;
                    break;
                }

                case SDL_WINDOWEVENT_FOCUS_LOST: {
                    // cs_set_global_pause(true);
                    focused = false;
                    break;
                }

                case SDL_WINDOWEVENT_FOCUS_GAINED: {
                    // cs_set_global_pause(false);
                    focused = true;
                    break;
                }

                default: break;
            }
        }

        default: break;
    }
}

void eng_close(void) {
    running = false;
}


// TODO: This shit is not future proof.
void eng_window_size(u16 *w, u16 *h) {
    if (w != NULL)
        *w = window_width;

    if (h != NULL)
        *h = window_height;
}


void eng_tickrate(f64 _hz) {
    hz = _hz;
}

bool eng_is_focused(void) {
    return focused;
}

void eng_set_debug(bool debug) {
    is_debug = debug;
}

bool eng_is_debug(void) {
    return is_debug;
}

#define ENG_CALL_IF_VALID(func, ...) { \
    ret = 0;                           \
    if (func)                          \
        ret = func(__VA_ARGS__);       \
                                       \
    if (ret && app.close)              \
        ret = app.close(app.userdata, ret); \
}

static int eng_headless(Application app) {
    int ret = SDL_Init( SDL_INIT_TIMER );
    if (ret)
        ERR_FATAL("couldn't get sdl2 to init.");

    // filesystem.
    ret = fs_init();
    if (ret)
        ERR_FATAL("couldn't init filesystem!");

    eng_tickrate(30);

    ENG_CALL_IF_VALID(app.init, app.userdata);

    while (running) {
        uint64_t frame_start = SDL_GetPerformanceCounter();

        ENG_CALL_IF_VALID(app.tick, app.userdata, 1.0/hz);

        uint64_t frame_end = SDL_GetPerformanceCounter();
        uint64_t frame_time = frame_end - frame_start;

        uint64_t frequency = SDL_GetPerformanceFrequency();
        uint64_t target = frequency / hz;

        if (frame_time < target) {
            Uint64 delay_time = target - frame_time;
            SDL_Delay((Uint32)((delay_time * 1000) / frequency));
        }
    }

    ENG_CALL_IF_VALID(app.close, app.userdata, ret);

    return ret;
}

int eng_main(Application app, bool headless) {
    printf("hello world, i'm basket.\n");

    err_init();

    if (headless)
        return eng_headless(app);

    int ret = SDL_Init( 0
        | SDL_INIT_VIDEO
        | SDL_INIT_EVENTS
        | SDL_INIT_AUDIO
        | SDL_INIT_TIMER
        | SDL_INIT_JOYSTICK
        | SDL_INIT_GAMECONTROLLER
    );
    if (ret)
        ERR_FATAL("couldn't get sdl2 to init.");

  	SDL_Window *window = SDL_CreateWindow(
        "socks",
  	    SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
  	    window_width, window_height,
  	    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
  	);
    if (window == NULL)
        ERR_FATAL("can't initialize window\n%s", SDL_GetError());

    // filesystem.
    if (fs_init())
        ERR_FATAL("couldn't init filesystem!\nget sure you didn't delete any files");

    // audio.
    if (aud_init())
        ERR_FATAL("couldn't init audio!");

    // renderer.
    if (ren_init(window))
        ERR_FATAL("couldn't init graphics!");

    // input
    if (inp_init())
        ERR_FATAL("couldn't init input!");

    inp_bind((RawBindings) {
        .up     = (char *[]) {"w", 0},
        .down   = (char *[]) {"s", 0},
        .left   = (char *[]) {"a", 0},
        .right  = (char *[]) {"d", 0},

        .action = (char *[]) {"j", "space", 0},
        .quick  = (char *[]) {"k", 0},
        .menu   = (char *[]) {"l", 0}
    });

    inp_bind((RawBindings) {
        .up     = (char *[]) {"up"   , 0},
        .down   = (char *[]) {"down" , 0},
        .left   = (char *[]) {"left" , 0},
        .right  = (char *[]) {"right", 0},

        .action = (char *[]) {"z", "space", 0},
        .quick  = (char *[]) {"x", 0},
        .menu   = (char *[]) {"c", 0}
    });

    inp_bind((RawBindings) {
        .up     = (char *[]) {"gamepad:dpup",    0},
        .down   = (char *[]) {"gamepad:dpdown",  0},
        .left   = (char *[]) {"gamepad:dpleft",  0},
        .right  = (char *[]) {"gamepad:dpright", 0},

        .action = (char *[]) {"gamepad:a", 0},
        .quick  = (char *[]) {"gamepad:b", 0},
        .menu   = (char *[]) {"gamepad:x", 0}
    });

    eng_tickrate(30);

    ENG_CALL_IF_VALID(app.init, app.userdata)

    // for our delta stuff
    printf("setting up timer.\n");
    u64 now = SDL_GetPerformanceCounter();
    u64 last = now;

    static f64 deltas[8];
    u8 delta_idx = 0;
    u8 delta_len = 0;

    f32 lag = 1.0/hz;

    running = true;
    while (running && ret == 0) {
        f32 timestep = 1.0/hz;

        // calculate delta
        now = SDL_GetPerformanceCounter();
        f64 real_delta = (double)(now - last) / (double)SDL_GetPerformanceFrequency();
        last = now;

        // delta smoothing:
        deltas[delta_idx++] = real_delta;
        delta_len = max(delta_len, delta_idx);
        if (delta_idx == 8) delta_idx = 0;

        f64 delta = 0.0;
        for (u8 i=0; i < delta_len; i++)
            delta += deltas[i];
        delta /= (f64)delta_len;

        #define TICK(step) {                        \
            SDL_Event ev;                           \
            while (SDL_PollEvent(&ev))              \
                event(ev, window);                  \
                                                    \
            if (focused)                            \
                ENG_CALL_IF_VALID(app.tick, app.userdata, step)   \
                                                    \
            inp_update(step);                       \
        }

        if (hz == 0)
            TICK(delta)

        else {
            // fixed timesteps
            lag += real_delta;
            u8 n = 0;
            while (lag > timestep) {
                lag -= timestep;

                TICK(timestep);

                // computer is too god damn slow, sorry.
                if (n++ == 3) {
                    lag = 0.0;
                    printf("Could not hit %.1fhz!\n", hz);
                    break;
                }
            }

        }

        ENG_CALL_IF_VALID(app.frame, app.userdata, lag / timestep, delta)

        if (!focused) {
            u16 w, h;
            ren_size(&w, &h);

            ren_rect(-(i32)(w/2), -(i32)(w/2), h*2, h*2, (Color){ .full = 0x00000055 });
        }

        if (ren_frame())
            ERR_FATAL("renderer fuckup! sorry");

        SDL_GL_SwapWindow(window);
    }

    if (app.close)
        ret = app.close(app.userdata, ret);

    printf("[OFFLINE] cerebellum\n");
    inp_byebye();

    printf("[OFFLINE] temporal lobe\n");
    aud_byebye();

    printf("[OFFLINE] occipital lobe\n");
    ren_byebye();

    printf("[OFFLINE] frontal lobe\n");
    SDL_DestroyWindow(window);

    printf("[OFFLINE] central executive network\n");
    SDL_Quit();

    printf("the end.\n");

    return ret;
}
