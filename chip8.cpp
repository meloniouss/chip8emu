#include <iostream> 
#include <fstream>
#include <cstdint>  
#include <cstring>  
#include <chrono>    
#include <thread>    
#include <stack>
#include <string>
#include <limits.h>
#include <SDL2/SDL.h>
#define u8 uint8_t
#define u16 uint16_t
#define i8 int8_t
#define i16 int16_t

uint8_t memory[4096];
bool framebuffer[64][32]; //screen
u16 PC = 0x200;
uint16_t idx; //index pointer
uint8_t registers[16]; //0 through 15 in decimal, called V0 through VF -> vf is flag register
uint8_t delay_timer;
uint8_t sound_timer;
std::stack<u16> stack;
bool running = false; //for sdl

u8 font[] = {0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
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
    0xF0, 0x80, 0xF0, 0x80, 0x80};

    const SDL_Keycode keymap[16] = {
        SDLK_x, // 0
        SDLK_1, // 1
        SDLK_2, // 2
        SDLK_3, // 3
        SDLK_q, // 4
        SDLK_w, // 5
        SDLK_e, // 6
        SDLK_a, // 7
        SDLK_s, // 8
        SDLK_d, // 9
        SDLK_z, // A
        SDLK_c, // B
        SDLK_4, // C
        SDLK_r, // D
        SDLK_f, // E
        SDLK_v  // F
    };
        
u16 fetch(){
    u16 left = memory[PC];
    u16 right = memory[PC+1];
    u16 opcode = (left << 8) | right;
    PC=PC+2;    
    return opcode;
}
void loadROM(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open ROM file!" << std::endl;
        return;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&memory[0x200]), size);
    file.close();
    std::cout << "ROM loaded successfully!" << std::endl;
}

void decode(u16 instruction){
    u8 first = (instruction >> 12) & 0x0F; //1
    u8 X = (instruction >> 8) & 0x0F; //2
    u8 Y = (instruction >> 4) & 0x0F; //3
    u8 N = (instruction) & 0x0F; // 4
    u8 NN = (instruction) & 0xFF; // 8bit number
    u16 NNN = instruction & 0x0FFF;
    u8 a, b, x_cord, y_cord,x,y,temp;
    switch (first)
    {
    case 0: //
        if(N == 0xE){ //00EE instruction
            if (!stack.empty()) {
                PC = stack.top();
                stack.pop();
            } else {
                std::cerr << "Error: stack is empty, cannot return from subroutine!" << std::endl;
                PC = 0; 
            }
        }
        else if (NN == 0xE0){ //clear screen instruction
            for(int i = 0; i < 64; i++){
                for(int j = 0; j < 32; j++)
                {
                    framebuffer[i][j] = 0;
                }
            }
        }
        break;
    case 1: // jmp
        PC = NNN;
        break;
    case 2:
        stack.push(PC);
        PC = NNN;
        break;
    case 3: //3XNN
        if(registers[X] == NN){
            PC = PC + 2;
        }
        break;
    case 4:
        if (registers[X] != NN){
            PC = PC + 2;
        }
        break;
    case 5:
        if(registers[X] == registers[Y]){
            PC = PC + 2;
        }
    break;
    case 6: //6XNN set register VX to NN
        registers[X] = NN;
        break;
    case 7:
        registers[X] += NN;
        break;
    case 8:
        switch (N)
        {
        case 0: //set
            registers[X] = registers[Y];
            break;
        case 1: //binary OR
            registers[X] = (registers[X] | registers[Y]);
            break;
        case 2: //binary AND
            registers[X] = (registers[X] & registers[Y]);
            break;
        case 3: //logical XOR (bitwise)
            registers[X] = (registers[X] ^ registers[Y]);
            break;
        case 4: //add with overflow
            registers[X] = registers[X] + registers[Y];
            if (registers[X] > 0 && registers[Y] > INT_MAX - registers[X]){ //overflow
                registers[15] = 1; 
            }
            else{
                registers[15] = 0;
            }
            break;
        case 5:
            x = registers[X];
            y = registers[Y];
            if (x >= y) {
                registers[15] = 1;
            } else {
                registers[15] = 0; 
            }        
            registers[X] = x - y;
            break;
        case 6:
            registers[X] = registers[Y];
            registers[0xF] = registers[X] & 0x1;
            registers[X] >>= 1;
            break;
        case 7:
            a = registers[X];
            b = registers[Y];
            if (y >= a) {
                registers[15] = 1;
            } else {
                registers[15] = 0; 
            }
            registers[X] = b - a;
            break;
        case 0xE:
            registers[X] = registers[Y];
            registers[0xF] = (registers[X] & 0x80) >> 7;
            registers[X] <<= 1;
            break;
        default:
            break;
        }
    break;
    case 9:
        if(registers[X] != registers[Y]){
            PC=PC+2;
        }
        break;
    case 0xA:
        idx = NNN;
        break;
    case 0xB: //jump to NNN + V0
        PC = NNN + registers[0];
        break;
    case 0xD:
        x_cord = registers[X] % 64;
        y_cord = registers[Y] % 32;
        registers[15] = 0; //vf
        //n-th byte from registers[idx]
        for (int row = 0; row < N; row++) {
            u8 sprite_byte = memory[idx + row];
    
            for (int col = 0; col < 8; col++) {
                if (x_cord + col >= 64) break;
    
                u8 sprite_pixel = (sprite_byte >> (7 - col)) & 1;
                x = (x_cord + col) % 64;
                y = (y_cord + row) % 32;
    
                if (sprite_pixel) {
                    if (framebuffer[x][y] == 1) {
                        registers[15] = 1;
                    }
                    framebuffer[x][y] ^= 1;
                }
            }
            if (y_cord + row >= 31) break;
        }
        break;
    case 0xF:
        if(N == 7){
            registers[X] = delay_timer;
        }
        else if (N == 5){
            if(Y == 1){delay_timer = registers[X];}
            else if(NN==0x55){ //fx55
                temp = idx;
                for(int i = 0; i <= X; i++){
                    memory[temp] = registers[i];
                    temp++;
                }
            }
            else if(NN == 0x65){ //fx65
                temp = idx;
                for(int i = 0; i <= X; i++){
                    registers[i] = memory[temp];
                    temp++;
                }
            }
        }
        else if(N == 8){
            sound_timer = registers[X];
        }
    
        else if (N == 0xE){
            if (idx > 0xFFF - registers[X]) {
                registers[15] = 1;
            } else {
                registers[15] = 0;
            }
            idx += registers[X];
        }

        else if (NN == 0x0A) { // Fx0A - Wait for a key press, store in VX
            SDL_Event event;
            bool key_pressed = false;
            while (!key_pressed) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        running = false;
                        return;
                    }
                    if (event.type == SDL_KEYDOWN) {
                        for (int i = 0; i < 16; ++i) {
                            if (event.key.keysym.sym == keymap[i]) {
                                registers[X] = i;
                                key_pressed = true;
                                break;
                            }
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Prevent 100% CPU usage
            }
        }
        
        else if (N == 9){ //font char
            idx = registers[X] + 5;
        }
        else if(N==3){ //  Binary-coded decimal conversion
            memory[idx] = registers[X] / 100;
            memory[idx+1] = (registers[X] / 10) % 10;
            memory[idx+2] = registers[X] % 10;
        } 
        break;
    case 0xE:
        if(Y == 9){
            if (keymap[registers[X]] == 1) {
                PC += 2; 
            }
        }
        else if(Y == 0xA){
            if (keymap[registers[X]] == 0) {
                PC += 2;
            }
        }
    break;
    default:
        break;
    }
}

void render() {
    // SDL boilerplate 
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    uint32_t pixelBuffer[64 * 32];
    
    running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }
        
        u16 instruction = fetch();
        decode(instruction);
        
        //refresh rate
        if (delay_timer > 0) delay_timer--;
        if (sound_timer > 0) sound_timer--;
        
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 64; x++) {
                pixelBuffer[y * 64 + x] = framebuffer[x][y] ? 0xFFFFFFFF : 0xFF000000;
            }
        }
        
        SDL_UpdateTexture(texture, NULL, pixelBuffer, 64 * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(){
    for(int i = 0; i < 80; i++){ 
        memory[0x050 + i] = font[i];
    }
    loadROM("test_opcode.ch8");
    render();
    return 0;
}