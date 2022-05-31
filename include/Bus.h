#pragma once
#include "array"
#include "stdint.h"
#include "cpu.h"
#include "ppu.h"
#include "cart.h"

class Bus {
private:
    uint8_t wRam[0x2000];
    uint8_t hRam[126];

    uint8_t serial[2];
    
public:
    Bus();
    void write(uint16_t address, uint8_t byte);
    uint8_t read(uint16_t address);
    void interruptFlags(uint8_t flag); //ORs input flags with IF register

    uint8_t joypad;
    uint8_t joypad_state; //top 4 bits are action buttons / bottom 4 bits are directions
    bool PPU_read;
};

