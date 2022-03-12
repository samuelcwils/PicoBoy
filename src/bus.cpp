#include "bus.h"

bus::bus(cart* Cart, ppu* PPU)
{
    memoryMap.Cart = Cart;
    memoryMap.PPU = PPU;
    joypad = 0xcf;
    joypad_state = 0xff;
    PPU_read = false;
}

void bus::connectCPU(cpu* CPU)
{
    this->memoryMap.CPU = CPU;
}

void bus::write(uint16_t address, uint8_t byte)
{
    switch(address & 0xf000)
    {
        case 0x0000:
            memoryMap.Cart->writeRom(address, byte);
            break;
        case 0x1000:
            memoryMap.Cart->writeRom(address, byte);;
            break;
        case 0x2000:
            memoryMap.Cart->writeRom(address, byte);
            break;
        case 0x3000:
            memoryMap.Cart->writeRom(address, byte);
            break;
        case 0x4000:
            memoryMap.Cart->writeRom(address, byte);
            break;
        case 0x5000:
            memoryMap.Cart->writeRom(address, byte);
            break;
        case 0x6000:
            memoryMap.Cart->writeRom(address, byte);
            break;
        case 0x7000:
            memoryMap.Cart->writeRom(address, byte);
            break;
        case 0x8000:
            if(memoryMap.PPU->VRAM_access)
            {
                memoryMap.PPU->vRam.vRam[address - 0x8000] = byte;
            }
            break;
        case 0x9000:
            if(memoryMap.PPU->VRAM_access)
            {
                memoryMap.PPU->vRam.vRam[address - 0x8000] = byte;
            }
            break;
        case 0xa000:
            memoryMap.Cart->ramBank[address - 0xa000] = byte;
            break;
        case 0xb000:
            memoryMap.Cart->ramBank[address - 0xa000] = byte;
            break;
        case 0xc000:
            wRam[address-0xc000] = byte;
            break;
        case 0xd000:
            wRam[address-0xc000] = byte;
            break;
        case 0xf000:
            switch(address & 0xff00)
            {
                case 0xfe00:
                if(memoryMap.PPU->OAM_access)
                {
                    memoryMap.PPU->oam[address - 0xee00] = byte;
                }
                break;
                case 0xff00:
                    switch(address & 0xffff)
                    {
                        case 0xff00:
                            joypad = (byte |= (joypad & 0xcf));
                            break;
                        case 0xff01:
                            serial[0] = byte;
                            break;
                        case 0xff02:
                            serial[1] = byte;
                            break;
                        case 0xff04:
                            memoryMap.CPU->DIV = 0;
                            memoryMap.CPU->totalTicks_DIV = 0;
                            memoryMap.CPU->totalTicks_TIMA = 0;
                            memoryMap.CPU->TIMA = 0;
                            break;
                        case 0xff05:
                            memoryMap.CPU->TIMA = byte;
                            break;
                        case 0xff06:
                            memoryMap.CPU->TMA = byte;
                            break;
                        case 0xff07:
                        {
                            memoryMap.CPU->TAC = byte;
                            int lookup[] = {1024,16, 64, 256};
                            memoryMap.CPU->TIMA_speed = lookup[(memoryMap.CPU->TAC & 0b00000011)];
                            break;
                        }
                        case 0xff0f:
                            memoryMap.CPU->IF = byte;
                            break;
                        case 0xff40:
                            memoryMap.PPU->regs.bytes.LCDC = byte;
                            break;
                        case 0xff41:
                            memoryMap.PPU->regs.bytes.STAT = byte;
                            break;
                        case 0xff42:
                            memoryMap.PPU->regs.bytes.SCY = byte;
                            break;
                        case 0xff43:
                            memoryMap.PPU->regs.bytes.SCX = byte;
                            break;
                        case 0xff44:
                            memoryMap.PPU->regs.bytes.LY = byte;
                            break;
                        case 0xff45:
                            memoryMap.PPU->regs.bytes.LYC = byte;
                            break;
                        case 0xff46:
                            memoryMap.PPU->DMA(((uint16_t)byte) << 8);
                            break;
                        case 0xff47:
                            memoryMap.PPU->regs.bytes.BGP = byte;
                            break;
                        case 0xff48:
                            memoryMap.PPU->regs.bytes.OBP0 = byte;
                            break;
                        case 0xff49:
                            memoryMap.PPU->regs.bytes.OBP1 = byte;
                            break;
                        case 0xff4a:
                            memoryMap.PPU->regs.bytes.WY = byte;
                            break;
                        case 0xff4b:
                            memoryMap.PPU->regs.bytes.WX = byte;
                            break;
                        case 0xffff:
                            memoryMap.CPU->IE = byte;
                            break;
                    }
                hRam[address - 0xff81] = byte;
                break;
                
            }
        break;
    }

    if(address <= 0x7fff)
    {
    memoryMap.Cart->writeRom(address, byte);
        
    } else if(address <= 0x9fff){

        if(memoryMap.PPU->VRAM_access)
        {
            memoryMap.PPU->vRam.vRam[address - 0x8000] = byte;
        }

    } else if(address <= 0xbfff){

        memoryMap.Cart->ramBank[address - 0xa000] = byte;

    } else if(address <= 0xdfff){

        wRam[address-0xc000] = byte;
    
    } else if(address <= 0xedff){

        wRam[address-0xe000] = byte; //echo of work ram

    } else if(address <= 0xfe9f){

        if(memoryMap.PPU->OAM_access)
        {
            memoryMap.PPU->oam[address - 0xee00] = byte;
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

        memoryMap.CPU->DIV = 0;
        memoryMap.CPU->totalTicks_DIV = 0;
        memoryMap.CPU->totalTicks_TIMA = 0;
        memoryMap.CPU->TIMA = 0;
    
    } else if(address == 0xff05){

        memoryMap.CPU->TIMA = byte;

    } else if(address == 0xff06){

        memoryMap.CPU->TMA = byte;

    } else if(address == 0xff07){

        memoryMap.CPU->TAC = byte;
        int lookup[] = {1024,16, 64, 256};
        memoryMap.CPU->TIMA_speed = lookup[(memoryMap.CPU->TAC & 0b00000011)];

    } else if(address == 0xff0f){

        memoryMap.CPU->IF = byte;

    
    } else if(address >= 0xff40 && address <= 0xff4b){

        if(address == 0xff46)
        {
            memoryMap.PPU->DMA(((uint16_t)byte) << 8);
        }

        memoryMap.PPU->regs.regs[(address - 0xff40)] = byte;

    } else if(address <= 0xfffe){

        hRam[address - 0xff81] = byte;

    } else if(address == 0xffff){
        
        memoryMap.CPU->IE = byte;
    
    }

}

uint8_t bus::read(uint16_t address)
{ 
    switch(address & 0xf000)
    {
        case 0x0000:
            return memoryMap.Cart->rom[address];
            break;
        case 0x1000:
            return memoryMap.Cart->rom[address];
            break;
        case 0x2000:
            return memoryMap.Cart->rom[address];
            break;
        case 0x3000:
            return memoryMap.Cart->rom[address];
            break;
        case 0x4000:
            return memoryMap.Cart->rom[address + ((memoryMap.Cart->romBankNum - 1) * 0x4000)];
            break;
        case 0x5000:
            return memoryMap.Cart->rom[address + ((memoryMap.Cart->romBankNum - 1) * 0x4000)];
            break;
        case 0x6000:
            return memoryMap.Cart->rom[address + ((memoryMap.Cart->romBankNum - 1) * 0x4000)];
            break;
        case 0x7000:
            return memoryMap.Cart->rom[address + ((memoryMap.Cart->romBankNum - 1) * 0x4000)];
            break;
        case 0x8000:
            if(memoryMap.PPU->VRAM_access || (PPU_read))
            {
                return memoryMap.PPU->vRam.vRam[address - 0x8000];
            } else {
                return 0xff;
            }
            break;
        case 0x9000:
            if(memoryMap.PPU->VRAM_access || (PPU_read))
            {
                return memoryMap.PPU->vRam.vRam[address - 0x8000];
            } else {
                return 0xff;
            }
            break;
        case 0xa000:
            return memoryMap.Cart->cartRam[address - 0xa000];
            break;
        case 0xb000:
            return memoryMap.Cart->cartRam[address - 0xa000];
            break;
        case 0xc000:
            return wRam[address-0xc000];
            break;
        case 0xd000:
            return wRam[address-0xc000];
            break;
        case 0xf000:
            switch(address & 0xff00)
            {
                case 0xfe00:
                if(memoryMap.PPU->OAM_access || (PPU_read))
                {
                    return memoryMap.PPU->oam[address - 0xee00];
                } else {
                    return 0xff;
                }
                break;
                case 0xff00:
                    switch(address & 0xffff)
                    {
                        case 0xff00:
                        joypad &= 0xf0;

                        if(!(joypad & 0b00010000))
                        {
                            return joypad | (joypad_state & 0x0f);
                        }

                        if(!(joypad & 0b00100000))
                        {
                            return joypad | ((joypad_state & 0xf0) >> 4);
                        }
                        break;
                        case 0xff01:
                            return serial[0];
                            break;
                        case 0xff02:
                            return serial[1];
                            break;
                        case 0xff04:
                            return memoryMap.CPU->DIV;
                            break;
                        case 0xff05:
                            return memoryMap.CPU->TIMA;
                            break;
                        case 0xff06:
                            return memoryMap.CPU->TMA;
                            break;
                        case 0xff07:
                            return memoryMap.CPU->TAC;
                            break;
                        case 0xff0f:
                            return memoryMap.CPU->IF;
                            break;
                        case 0xff40:
                            return memoryMap.PPU->regs.bytes.LCDC;
                            break;
                        case 0xff41:
                            return memoryMap.PPU->regs.bytes.STAT;
                            break;
                        case 0xff42:
                            return memoryMap.PPU->regs.bytes.SCY;
                            break;
                        case 0xff43:
                            return memoryMap.PPU->regs.bytes.SCX;
                            break;
                        case 0xff44:
                            return memoryMap.PPU->regs.bytes.LY;
                            break;
                        case 0xff45:
                            return memoryMap.PPU->regs.bytes.LYC;
                            break;
                        case 0xff46:
                            return memoryMap.PPU->regs.bytes.DMA;
                            break;
                        case 0xff47:
                            return memoryMap.PPU->regs.bytes.BGP;
                            break;
                        case 0xff48:
                            return memoryMap.PPU->regs.bytes.OBP0;
                            break;
                        case 0xff49:
                            return memoryMap.PPU->regs.bytes.OBP1;
                            break;
                        case 0xff4a:
                            return memoryMap.PPU->regs.bytes.WY;
                            break;
                        case 0xff4b:
                            return memoryMap.PPU->regs.bytes.WX;
                            break;
                        case 0xffff:
                            return memoryMap.CPU->IE;
                            break;
                    }
                return hRam[address - 0xff81];
                break;
                
            }
        break;
    }
    return 0xff;
}

void bus::interruptFlags(uint8_t flag)
{
    if(!(memoryMap.CPU->servicingInterrupt) && flag != 2)
    {
        memoryMap.CPU->IF |= flag;
    }
    
}