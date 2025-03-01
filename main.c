#include <SDL3\SDL.h>
#include "chip8.h"

//To run from command-line:
// make (or whatever command you need to run) file="YOURROMDIRECTORY"
// Ex: mingw32-make file="chip8-roms\programs\IBM Logo.ch8"

//SETTINGS - more settings can be changed in chip8.c
#define USE_FREQUENCY 1 //set this to 0 if you want the game loop to run without delay - recommended for modern/octo games
#define FREQUENCY 800 //If greater than 1000 turn USE_FREQUENCY off instead
#define REFRESH_RATE 30 //display refresh rate
#define SOUND_FREQ 138.59
#define OFF 0xE2E2E2FF //color for off pixel in RGBA
#define ON  0x1A1A1AFF //color for on pixel in RGBA

void load_rom(cpu *chip8, const char *rom_filename){
    FILE *file = fopen(rom_filename, "rb");
    if (file != NULL)
    {
        uint8_t byte;
        int i = 0;
        while (fread(&byte, sizeof(byte), 1, file))
        {
            chip8->memory[0x200 + i] = byte;
            i++;
        }
        fclose(file);
    }
}

const SDL_Scancode keymap[16] = {
    SDL_SCANCODE_X,
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_W,
    SDL_SCANCODE_E,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_C,
    SDL_SCANCODE_4,
    SDL_SCANCODE_R,
    SDL_SCANCODE_F,
    SDL_SCANCODE_V,
};


SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

static SDL_AudioStream *stream = NULL;
static int current_sine_sample = 0;

void init(){
    
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
        printf("SDL unable to initialize!\n");
        SDL_Log("ERROR: %s", SDL_GetError());
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8", 1024, 512, 0);

    if (window == NULL){
        printf("Can't create window!");
    }

    renderer = SDL_CreateRenderer(window,NULL);
    if (renderer == NULL){
        printf("Can't create renderer!");
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    if (texture == NULL){
        printf("Can't create texture!");
    }

    //Sound
    SDL_AudioSpec spec;
    spec.channels = 1;
    spec.format = SDL_AUDIO_F32;
    spec.freq = 8000;
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!stream) 
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
}

void deinit(){
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[])
{

    init();
    cpu chip8;
    chip8 = cpu_init(chip8);
    load_rom(&chip8, argv[1]);
    clock_t before = clock();
    
    bool opened = true;
    while(opened){

        cpu_cycle(&chip8);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type){
                case SDL_EVENT_KEY_DOWN:
                {
                    for (int i = 0; i<16; i++){
                        if(event.key.scancode == keymap[i])
                            chip8.keys[i] = 1;
                    }
                    break;
                }
                case SDL_EVENT_KEY_UP:
                {
                    for (int i = 0; i<16; i++){
                        if(event.key.scancode == keymap[i])
                            chip8.keys[i] = 0;
                    }
                    break;
                }
                case SDL_EVENT_QUIT:
                    opened = false;
                    break;
            }
        }

        
        //draw pixels
        if(((double)(clock() - before) / CLOCKS_PER_SEC) >= (1.0/REFRESH_RATE)){
            
            SDL_RenderClear(renderer);
            int pitch = 0;
            int *pixels = NULL;

            SDL_LockTexture(texture, NULL, (void **) &pixels, &pitch);
                for (int i = 0; i < ((long)sizeof(chip8.graphics) / (long)sizeof(chip8.graphics[0])); i++){
                    if (chip8.graphics[i])
                        pixels[i] = ON;
                    else
                        pixels[i] = OFF;
                }
            
            SDL_UnlockTexture(texture);
            SDL_RenderTexture(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            before = clock();
        }

        //Sound
        if(chip8.sound_timer > 0 && SDL_AudioStreamDevicePaused(stream))
            SDL_ResumeAudioStreamDevice(stream);
        if(chip8.sound_timer <= 0)
            SDL_PauseAudioStreamDevice(stream);
        const int minimum_audio = (8000 * sizeof (float)) / 2; 
        if (SDL_GetAudioStreamAvailable(stream) < minimum_audio) {
        static float samples[512]; 

        for (int i = 0; i < SDL_arraysize(samples); i++) {
            const int freq = SOUND_FREQ;
            const float phase = current_sine_sample * freq / 8000.0f;
            samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
            current_sine_sample++;
        }

        current_sine_sample %= 8000;

        SDL_PutAudioStreamData(stream, samples, sizeof (samples));
    }

        if (USE_FREQUENCY){
            Sleep(((1000.0/FREQUENCY)));  
        }
    }
    return 0;
   }
