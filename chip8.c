#include "chip8.h"

#define FONT_SIZE 80

//Quirks - If you are using a ROM that matches the descriptions on the right,
// consider switching if something is not working properly
#define FLAG_REGISTER_RESET 0
#define INCREMENT_INDEX 1
#define CLIPPING 0
#define SHIFT 0
#define ALT_JUMP 0
//#define WAIT_SCREEN_REFRESH 0 TODO: Drawing sprites to the display waits for the vertical blank interrupt, limiting their speed to max 60 sprites per second.

//If Fx0A waits for the key press or the key to release
//before resuming instruction
#define WAIT_KEY_RELEASE 0


const char chip8_fontset[FONT_SIZE] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};



cpu cpu_init(cpu self){
    srand(time(NULL));

    self.program_counter = 512;
    self.opcode = 0;
    self.index = 0;
    self.stack_pointer = 0;
    self.delay_timer = 0;
    self.sound_timer = 0;
    self.key_pressed = 0;


    //graphics
    for(int i = 0; i < 64 * 32; i++){
        self.graphics[i] = 0;
    }

    //registers
    for(int i = 0; i < 16; i++){
        self.registers[i] = 0;
    }

    //stack
    for(int i = 0; i < 16; i++){
        self.stack[i] = 0;
    }

    //keys
    for(int i = 0; i < 16; i++){
        self.keys[i] = 0;
    }

    //store font in memory
    for (int i = 0; i < FONT_SIZE; i++){
        self.memory[i] = chip8_fontset[i];
    }
    return self;
}


void cpu_increment_pc(cpu* self){
    self->program_counter += 2;
}

void cpu_cycle(cpu* self){
    self->opcode = self->memory[self->program_counter] << 8 | self->memory[self->program_counter + 1];
    uint8_t first = self->opcode >> 12;

    if(self->delay_timer > 0){
        if(((double)(clock() - self->time_before) / CLOCKS_PER_SEC) >= 0.0167){ //60 hz
            self->delay_timer--;
            self->time_before = clock();
        }
    }

    if(self->sound_timer > 0){
        if(((double)(clock() - self->sound_before) / CLOCKS_PER_SEC) >= 0.0167){ //60 hz
            self->sound_timer--;
            self->sound_before = clock();
        }
    }

    switch (first)
        {
        case 0x0:
         {   if (self->opcode == 0x00E0){
                for(int i = 0; i < 64 * 32; i++){
                    self->graphics[i] = 0;
                }
            }
            else if (self->opcode == 0x00EE){
                self->stack_pointer--;
                self->program_counter = self->stack[self->stack_pointer];
            }
            cpu_increment_pc(self);
            break;
        }
        
        case 0x1:
        {
            self->program_counter = self->opcode & 0x0FFF;
            break;
        }
        case 0x2:
        {        
            self->stack[self->stack_pointer] = self->program_counter;
            self->stack_pointer++;
            self->program_counter = self->opcode & 0x0FFF;
            break;
        }
        case 0x3:
        {        
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            if (self->registers[x] == ((self->opcode) & 0x00FF)){
                cpu_increment_pc(self);
            }
            cpu_increment_pc(self);
            break;
        }
        case 0x4:
        {
            uint8_t x = ((self->opcode) & 0x0F00) >> 8;
            if (self->registers[x] !=((self->opcode) & 0x00FF)){
                cpu_increment_pc(self);
            }
            cpu_increment_pc(self);
            break;
        }

        case 0x5:
        {
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            uint8_t y = (self->opcode & 0x00F0) >> 4;
            if (self->registers[x] == self->registers[y]){
                cpu_increment_pc(self);
            }
            cpu_increment_pc(self);
            break;
        }
        case 0x6:
        {
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            self->registers[x] = (self->opcode & 0x00FF);
            cpu_increment_pc(self);
            break;
        }
            
        case 0x7:
        {
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            self->registers[x] += self->opcode & 0x00FF;
            cpu_increment_pc(self);
            break;
        }

        case 0x8:
        {
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            uint8_t y = (self->opcode & 0x00F0) >> 4;
            uint8_t m = (self->opcode & 0x000F);
            switch (m)
            {
            case 0:
                self->registers[x] = self->registers[y];
                break;
            case 1:
                self->registers[x] |= self->registers[y];
                if (FLAG_REGISTER_RESET)
                    self->registers[0xF] = 0;
                break;
            case 2:
                self->registers[x] &= self->registers[y];
                if (FLAG_REGISTER_RESET)
                    self->registers[0xF] = 0;
                break;
            case 3:
                self->registers[x] ^= self->registers[y];
                if (FLAG_REGISTER_RESET)
                    self->registers[0xF] = 0;
                break;
            case 4:
                {
                int temp = self->registers[x];
                self->registers[x] += self->registers[y];
                self->registers[0xF] = ((self->registers[y]) + (temp)) > 255;
                break;
                }
            case 5:
                {
                int temp = self->registers[x];
                self->registers[x] -= self->registers[y];
                self->registers[0xF] = (temp) >= (self->registers[y]);
                break;
                }
            case 6:
            {
                if (!SHIFT){
                    self->registers[x] = self->registers[y];
                }
                int temp = self->registers[x];
                self->registers[x] /= 2;
                self->registers[0xF] = temp & 1;
                break;
            }
            case 7:
            {
                int temp = self->registers[x];
                self->registers[x] = self->registers[y] - self->registers[x];
                self->registers[0xF] = self->registers[y] >= temp;
                break;
            }
            case 0xE:
                if (!SHIFT){
                    self->registers[x] = self->registers[y];
                }
                int temp = self->registers[x];
                self->registers[x] *= 2;
                self->registers[0xF] = (temp & 0x80) >> 7;
                break;
            }
            cpu_increment_pc(self);
            break;  
        }

        case 0x9:
        {
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            uint8_t y = (self->opcode & 0x00F0) >> 4;
            if (self->registers[x] != self->registers[y]){
                cpu_increment_pc(self);
            }
            cpu_increment_pc(self);
            break;
        }

        case 0xA:
        {
            self->index = ((self->opcode) & 0x0FFF);
            cpu_increment_pc(self);
            break;
        }

        case 0xB:
        {
            if (ALT_JUMP){
                uint8_t x = (self->opcode & 0x0F00) >> 8;
                self->program_counter = (self->opcode & 0x0FFF) + self->registers[x];
            }
            else
                self->program_counter = (self->opcode & 0x0FFF) + self->registers[0];
            break;
        }
        case 0xC:
        {
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            self->registers[x] = (rand() % 255) & (self->opcode & 0x00FF);
            cpu_increment_pc(self);
            break;
        }
        case 0xD:
        {
            uint8_t n = ((self->opcode) & 0x000F);

            uint8_t x = (self->opcode & 0x0F00) >> 8;
            uint8_t y = (self->opcode & 0x00F0) >> 4;
            int reg_x = self->registers[x];
            int reg_y = self->registers[y];
            self->registers[0xF] = 0;
                
            for (int height = 0; height < n; height++){
                uint8_t sprite_row = self->memory[self->index + height];
                
                for (int width = 0; width < 8; width++){

                    uint8_t pixel = sprite_row & (0x80 >> width);
                    if(CLIPPING){
                       if ((reg_x % 64) + width >= 64 || (reg_y % 32) + height >= 32){
                            pixel = 0;
                       }
                    }
                    if (pixel != 0){
                        int indx = (((height + reg_y) % 32) * 64) + ((reg_x + width) % 64);
                        if (self->graphics[indx] == 1){
                            self->registers[0xF] = 1;
                        }
                        self->graphics[indx] ^= 1;
                    }
                }
            }
            cpu_increment_pc(self);
            break;
        } 
        case 0xE:
        {
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            switch ((self->opcode & 0x00F0) >> 4){
                case 9:
                    if(self->keys[self->registers[x]] != 0){
                        cpu_increment_pc(self);
                    }
                    break;
                case 0xA:
                    if(self->keys[self->registers[x]] == 0){
                        cpu_increment_pc(self);
                    }
                    break;
            }
            cpu_increment_pc(self);
            break;
        }
        case 0xF:
        {
            uint8_t x = (self->opcode & 0x0F00) >> 8;
            switch(self->opcode & 0x00FF){
                case 0x07:
                    self->registers[x] = self->delay_timer;
                    break;
                case 0x0A:
                    if(!self->key_pressed)
                    {
                    int key_pressed = 0;
                    for (int i = 0; i < 16; i++){
                        if (self->keys[i] != 0){
                            self->registers[x] = i;
                            key_pressed = 1;
                            break;
                        }
                    }
                    if (!key_pressed)
                        return;
                    }

                    if(WAIT_KEY_RELEASE)
                    {
                    self->key_pressed = 1;
                    int key_released = 0;
                    for (int i = 0; i < 16; i++){
                        if (self->keys[self->registers[x]] == 0){
                            key_released = 1;
                        }
                    }
                    if(!key_released)
                        return;
                    self->key_pressed = 0;
                    }
                    break;
                case 0x15:
                    self->delay_timer = self->registers[x];
                    self->time_before = clock();
                    break;
                case 0x18:
                    self->sound_timer = self->registers[x];
                    self->sound_before = clock();
                    break;
                case 0x1E:
                    self->index += self->registers[x];
                    break;
                case 0x29:
                    self->index = 5 * self->registers[x];
                    break;
                case 0x33:
                    self->memory[self->index] = self->registers[x] / 100;
                    self->memory[self->index + 1] = (self->registers[x] / 10) % 10;
                    self->memory[self->index + 2] = self->registers[x] % 10;
                    break;
                case 0x55:
                    for (int i = 0; i <= x; i++){
                        if (INCREMENT_INDEX){
                            self->memory[(self->index)] = self->registers[i];
                            self->index++;
                        }
                        else{
                            self->memory[(self->index + i)] = self->registers[i];
                        }
                    }
                    break;
                case 0x65:
                    for (int i = 0; i <= x; i++){
                        if (INCREMENT_INDEX){
                            self->registers[i] = self->memory[(self->index)];
                            self->index++;
                        }
                        else{
                            self->registers[i] = self->memory[(self->index + i)];
                        }
                    }
                    break;
            }
            cpu_increment_pc(self);
            break;
        }   
    
    }  
}