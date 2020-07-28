#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "gameboy/cpu.h"
#include "gameboy/irq.h"
#include "gameboy/mem.h"
#include "gameboy/ppu.h"

#define DISPLAY_X   160
#define DISPLAY_Y   144
#define SCALE       4

uint8_t aBootROM[256];
uint8_t aGameROM[32 * 1024];

static int load_bootrom(const char *pFilename)
{
    FILE *pBootROM = fopen(pFilename, "rb");
    if (NULL == pBootROM)
    {
        printf("Cannot open BootROM file: %s\n", pFilename);
        return EXIT_FAILURE;
    }

    size_t size_read = fread(&aBootROM[0], sizeof(uint8_t), 256, pBootROM);
    if (size_read != 256)
    {
        printf("Error loading BootROM: %d\n", (int) size_read);
        return EXIT_FAILURE;
    }
    fclose(pBootROM);
    return EXIT_SUCCESS;

}static int load_gamerom(const char *pFilename)
{
    FILE *pGameROM = fopen(pFilename, "rb");
    if (NULL == pGameROM)
    {
        printf("Cannot open GameROM file: %s\n", pFilename);
        return EXIT_FAILURE;
    }

    size_t size_read = fread(&aGameROM[0], sizeof(uint8_t), 32 * 1024, pGameROM);
    if (size_read != 32 * 1024)
    {
        printf("Error loading GameROM: %d\n", (int) size_read);
        return EXIT_FAILURE;
    }
    fclose(pGameROM);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    // SDL variables
    SDL_Window* pWindow = NULL;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_PixelFormat *pPixelFormat;
    SDL_Event event;
    uint8_t *pPixels;
    int pitch;

    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0 )
    {
        fprintf(stdout, "SDL_Init failed(%s)\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    /* Create window */
    pWindow = SDL_CreateWindow("SDL Gameboy",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               DISPLAY_X * SCALE,
                               DISPLAY_Y * SCALE,
                               SDL_WINDOW_OPENGL);
    if(NULL == pWindow)
    {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    pRenderer = SDL_CreateRenderer(pWindow, -1, 0);
    pTexture = SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                 DISPLAY_X, DISPLAY_Y);
    pPixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    // Load bootrom
    if (load_bootrom("DMG_ROM.bin"))
        return EXIT_FAILURE;

    // Load gamerom
    if (load_gamerom("Tetris.bin"))
        return EXIT_FAILURE;

    // Init gameboy emulator
    cpu_init();
    irq_init();
    mem_init();
    ppu_init();

    // Overwrite mem values
    mem_set_bootrom(&aBootROM[0]);
    mem_set_gamerom(&aGameROM[0], 0);
    mem_set_gamerom(&aGameROM[1024 * 16], 1);

    while(true)
    {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
            break;

        SDL_LockTexture(pTexture, NULL, (void**) &pPixels, &pitch);

        // Test SDL
        for (int y = 0 ; y < DISPLAY_Y ; y++)
        {
            uint32_t *p = (uint32_t *)(pPixels + pitch*y); // cast for a pointer increments by 4 bytes.(RGBA)
            for (int x = 0 ; x < DISPLAY_X ; x++)
            {
                *p = SDL_MapRGBA(pPixelFormat, x*y, x*y, x*y, 255);
                p++;
            }
        }

        // Run Gameboy emulation
        cpu_exec();
        ppu_exec();

        SDL_UnlockTexture(pTexture);

        SDL_RenderClear(pRenderer);
        SDL_RenderCopy(pRenderer, pTexture, NULL, NULL);
        SDL_RenderPresent(pRenderer);
    }

    // Close and destroy the window
    SDL_DestroyWindow(pWindow);

    // Clean up
    SDL_Quit();
    return EXIT_SUCCESS;
}