#include <iostream> 
#include <fstream>   // for file I/O (e.g. loading ROM)
#include <cstdint>  
#include <cstring>  
#include <chrono>    
#include <thread>    
#include <stack>
#include <string>
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
std::stack<uint16_t> stack;

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

u16 fetch(){
    u16 left = memory[PC];
    u16 right = memory[PC+1];
    u16 opcode = (left << 4) | right;
    PC=PC+2;
    return opcode;
}

void decode(u16 instruction){
    u8 first = (instruction >> 12) & 0x0F; //1
    u8 X = (instruction >> 8) & 0x0F; //2
    u8 Y = (instruction >> 4) & 0x0F; //3
    u8 N = (instruction) & 0x0F; // 4
    u8 NN = (instruction) & 0xFF; // 8bit number
    u16 NNN = (instruction >> 4) & 0xFFF; // 12 bit address

    switch (first)
    {
    case 0: //
        if(N == 0xE){ //00EE instruction
            PC = stack.top();
            stack.pop();
        }
        else{ //clear screen instruction
            for(int i = 0; i < 64; i++){
                for(int j = 0; j < 32; j++)
                {
                    framebuffer[i][j] = 0;
                }
            }
        }
        break;
    case 1: // jmp
        PC == NNN;
        break;
    case 2:
        stack.push(PC);
        PC = NNN;
        break;
    case 3:
        
        break;
    case 6: //6XNN set register VX to NN
        registers[X] = NN;
        break;
    case 7:
        registers[X] += NN;
        break;
    case 0xA:
        break;
    case 0xD:
        break;
    default:
        break;
    }
}

void execute(){}


int main(){
    for(int i = 0; i < 0x200; i++){ 
        memory[i] = font[i];
    }

};