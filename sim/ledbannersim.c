#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_video.h"

#define WIDTH 80
#define HEIGHT 8
#define DEPTH 24
#define PIXELS (WIDTH * HEIGHT)
#define BYTES (PIXELS * 3)

#define BIGGER  10

static uint8_t frame[HEIGHT][WIDTH][3];

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SDL_Init(SDL_INIT_VIDEO);

    // get window surface
    SDL_Window *window = 
        SDL_CreateWindow("ledbanner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH*BIGGER, HEIGHT*BIGGER, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    // create surface for led banner frame
    SDL_Surface *frame_surface = 
        SDL_CreateRGBSurfaceFrom(frame, WIDTH, HEIGHT, DEPTH, WIDTH*3, 0x0000FF, 0x00FF00, 0xFF0000, 0);

    // main loop
    for(;;) {
        uint8_t p = frame;
        int n = BYTES;
        
        while(n > 0) {
            int c = read(0, p, n);
            
            if (c == -1) {
                if (errno == EINTR)
                    continue;
                    
                fprintf(stderr, "fail: %s\n", strerror(errno));
                
                break;
            }
            
            if (c == 0) // pipe closed
                break;
                
            p += c;
            n -= c;
        }
        
        if (n)
            break;

        SDL_BlitScaled(frame_surface, NULL, surface, NULL);
        SDL_UpdateWindowSurface(window);
        
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                printf("Quit received...");
                break;
            }
        }
    }    
    
    SDL_FreeSurface(frame_surface);
    SDL_Quit();
    printf("Quit!\n");
    return 0;
}

