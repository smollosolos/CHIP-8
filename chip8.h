#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

typedef struct{
    uint16_t opcode;
    uint8_t  memory[4096];

    uint8_t  graphics[32 * 64];
    
    uint8_t  registers[16];

    uint16_t index;
    uint16_t program_counter;
    
    //timers
    uint8_t delay_timer;
    uint8_t sound_timer;

    clock_t time_before;
    clock_t sound_before;
    
    //stack
    uint16_t stack[16];
    uint16_t stack_pointer;
    
    //keyboard
    uint8_t keys[16];
    //check if keyboard as already been pressed for Fx0A
    uint8_t key_pressed;
    } cpu;

    cpu cpu_init(cpu self);

    void cpu_increment_pc(cpu* self);

    void cpu_cycle(cpu* self);
    