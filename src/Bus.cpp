#include "Bus.h"
extern Cart* cart;
extern PPU* ppu;
extern CPU* cpu;

Bus::Bus()
{
    joypad = 0xcf;
    joypad_state = 0xff;
}

void Bus::write(uint16_t address, uint8_t byte)
{
    if(address <= 0x7fff)
    {
        cart->writeRom(address, byte);
        
    } else if(address <= 0x9fff){

        if(ppu->VRAM_access)
        {
            ppu->vRam[address - 0x8000] = byte;
        }

    } else if(address <= 0xbfff){

        cart->ramBank[address - 0xa000] = byte;

    } else if(address <= 0xdfff){

        wRam[address-0xc000] = byte;
    
    } else if(address <= 0xedff){

        wRam[address-0xe000] = byte; //echo of work ram

    } else if(address <= 0xfe9f){

        if(ppu->OAM_access)
        {
            ppu->oam[address - 0xee00] = byte;
        }

    } else if(address == 0xff00){

        joypad = (byte |= (joypad & 0xcf));

    } else if(address == 0xff01){

        serial[0] = byte;

    } else if(address == 0xff02){

        if(byte & 0b10000000)
        {
            interruptFlags(0b10000000);
        }

        serial[1] = byte;

       // printf("%c", serial[0]) ;


    } else if(address == 0xff04){

        cpu->DIV = 0;
        cpu->totalTicks_DIV = 0;
        cpu->totalTicks_TIMA = 0;
        cpu->TIMA = 0;
    
    } else if(address == 0xff05){

        cpu->TIMA = byte;

    } else if(address == 0xff06){

        cpu->TMA = byte;

    } else if(address == 0xff07){

        cpu->TAC = byte;
        int lookup[] = {1024,16, 64, 256};
        cpu->TIMA_speed = lookup[(cpu->TAC & 0b00000011)];

    } else if(address == 0xff0f){

        cpu->IF = byte;

    
    } else if(address >= 0xff40 && address <= 0xff4b){

        if(address == 0xff46)
        {
            ppu->DMA(((uint16_t)byte) << 8);
        }

        ppu->regs.regs[(address - 0xff40)] = byte;

    } else if(address <= 0xfffe){

        hRam[address - 0xff81] = byte;

    } else if(address == 0xffff){
        
        cpu->IE = byte;
    
    }

}

uint8_t Bus::read(uint16_t address)
{   

    if(address <= 0x3fff)
    {

        return cart->rom[address];

    } else if(address <= 0x7fff)
    {
        return cart->rom[address + ((cart->romBankNum - 1) * 0x4000)];

    } else if(address <= 0x9fff){

        if(ppu->VRAM_access || (PPU_read))
        {
            return ppu->vRam[address - 0x8000];
        } else {
            return 0xff;
        }

    } else if(address <= 0xbfff){

        return cart->cartRam[address - 0xa000];

    } else if(address <= 0xdfff){

        return wRam[address-0xc000];
    
    } else if(address <= 0xedff){

        return wRam[address-0xe000];//echo of work ram

    } else if(address <= 0xfe9f){

        if(ppu->OAM_access || (PPU_read))
        {
            return ppu->oam[address - 0xee00];
        } else {
            return 0xff;
        }
        
    } else if(address == 0xff00){

        joypad &= 0xf0;

        if(!(joypad & 0b00010000))
        {
            return joypad | (joypad_state & 0x0f);
        }

        if(!(joypad & 0b00100000))
        {
            return joypad | ((joypad_state & 0xf0) >> 4);
        }

    } else if(address == 0xff01){

        return serial[0];

    } else if(address == 0xff02){

        return serial[1];

    } else if(address == 0xff04){

        return cpu->DIV;
    
    } else if(address == 0xff05){

        return cpu->TIMA;

    } else if(address == 0xff06){

        return cpu->TMA;

    } else if(address == 0xff07){

        return cpu->TAC;

    } else if(address == 0xff0f){

        return cpu->IF;
      
    } else if(address >= 0xff40 && address <= 0xff4b){

        if(address == 0xff41)
        {
            if(!(ppu->regs.bytes.LCDC & 0b10000000))
            {
                return (ppu->regs.regs[(address - 0xff41)]) & 0b11111100;
            } else {
                return ppu->regs.bytes.STAT;
            }

           int x = 5 + 5;
        }

        return ppu->regs.regs[(address - 0xff40)];

    } else if(address <= 0xfffe){

        return hRam[address - 0xff81];

    } else if(address == 0xffff){
        
        return cpu->IE;

    }

    return 0xff;
}

void Bus::interruptFlags(uint8_t flag)
{
    if(!(cpu->servicingInterrupt) && flag != 2)
    {
        cpu->IF |= flag;
    }
    
}