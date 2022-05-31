#include "PPU.h"

extern Bus* bus;
extern CPU* cpu;

PPU::PPU()
{  
    regs.bytes.LY = 0;
    OAM_access = true;
    VRAM_access = true;
    scrollingLeft = 0;
    xPos = 0;
    totalTicks = 0;
    queue0.state = false;
    queue1.state = false;
    fifoSize = 0;
    for(int i = 0; i < 12; i++)
    {
        regs.regs[i] = 0;
    }
    regs.bytes.LCDC = 0x91;
    regs.bytes.BGP = 0xfc;
    regs.bytes.OBP0 = 0xff;
    regs.bytes.OBP1 = 0xff;
}

void PPU::DMA(uint16_t nn)
{
    for(int i = 0; i < 0xa0; i++)
    {
        oam[i] = bus->read(nn + i);
    }

}

void PPU::drawLine()
{
    if(regs.bytes.LCDC & 0b00001000)
    {
        fetcher.BG_mapBase = 0x1c00;
    } else {
        fetcher.BG_mapBase = 0x1800;
    }

    if(regs.bytes.LCDC & 0b01000000)
    {
        fetcher.WIN_mapBase = 0x1c00;
    } else {
        fetcher.WIN_mapBase = 0x1800;
    }

    if(regs.bytes.LCDC & 0b00010000)
    {
        fetcher.dataBase = 0;    
    } else {
        fetcher.dataBase = 0x1000;
    }

    fetcher.tileLine = (regs.bytes.LY + regs.bytes.SCY) % 8;
    fetcher.tileRowAddr = fetcher.BG_mapBase + ( ( (  ( (regs.bytes.LY + regs.bytes.SCY) % 256 )/8) ) * 32);
    fetcher.tileCollumn = 0;
    fetcher.state = 0;
    xPos = 0;
    uint16_t lookup[4] = {0x6c93, 0x432e, 0x2a0a, 0x1105};

    int i = 0;

    int yPos = ((regs.bytes.LY) * 240);

    while(i != 20)
    {
        fetcher.tileID = vRam[fetcher.tileRowAddr + fetcher.tileCollumn];
        fetcher.lowLine = vRam[fetcher.dataBase + (fetcher.tileID * 16) + (fetcher.tileLine * 2)];
        fetcher.highLine = vRam[fetcher.dataBase + (fetcher.tileID * 16) + (fetcher.tileLine * 2) + 1];

        for(int i = 7; i > -1; i--)
        {
            frameBuffer[yPos + xPos] = lookup[((( (fetcher.highLine & ((1 << i))) << 1) + (fetcher.lowLine & (1 << i))) >> i)];
            xPos++;
        }

        fetcher.tileCollumn++;

        i++;
    }

    if(regs.bytes.LCDC & 0b00000010) //if sprite enable
    {
        bool spriteSize = regs.bytes.LCDC & 0b00000100;
        
        for(int i = 156; i >= 0; i-=4) //scan through to see if sprites are visible
        {
            bool yVisibility;

            if(spriteSize)
            {
                yVisibility = (oam[i] - regs.bytes.LY - 16) < 17 && (oam[i] - regs.bytes.LY - 16) > 0;
            } else {
                yVisibility = (oam[i] - regs.bytes.LY - 8) < 9 && (oam[i] - regs.bytes.LY - 8) > 0;
            }

            if(yVisibility)
            {
                int id = oam[i+2];
                int highline_offset = 0;
                int spriteLine;

                bool yFlip = oam[i + 3] & 0b01000000;

                if(yFlip)
                {
                    spriteLine = (oam[i] - 16) - regs.bytes.LY; 
                    highline_offset = -1;
                } else {
                    spriteLine = regs.bytes.LY - (oam[i] - 16);
                    highline_offset = 1;
                }

                uint8_t lowLine = vRam[(id* 16) + (spriteLine * 2)];
                uint8_t highLine = vRam[(id * 16) + (spriteLine * 2) + highline_offset];

                int sprite_xPos = oam[i + 1] - 1; //offset by one

                for(int i = 0; i < 8; i++)
                {
                    frameBuffer[yPos + sprite_xPos] = lookup[((( (highLine & ((1 << i))) << 1) + (lowLine & (1 << i))) >> i)];
                    sprite_xPos--;
                }    
            }
        }
    }
}

void PPU::tick()
{
    if(!(regs.bytes.LCDC & 0b10000000))
    {
        return;
    }

    totalTicks += cpu->cycles;
    statusMode = regs.bytes.STAT & 0b00000011;

    if(regs.bytes.LY > 144)
    {
        if(statusMode != vBlank)
        {
            VRAM_access = true;
            OAM_access = true;
            cpu->IF |= 0b00000001; //sends vblank interrupt
            regs.bytes.STAT &= 0b00000100;
            regs.bytes.STAT |= vBlank;
            regs.bytes.STAT |= 0b00010000; //interrupt source
            bus->interruptFlags(0b00000010);
        }

        if(totalTicks >= 456)
        {
            totalTicks -= 456;
            regs.bytes.LY++;
        }

        if(regs.bytes.LY == 153)
        {
            regs.bytes.LY = 0; 
            frameDone = true;
        }
    
    } else if(totalTicks < 80){
       
        if(statusMode != OAM)
        {
            VRAM_access = true;
            OAM_access = false;
            regs.bytes.STAT &= 0000000100;
            regs.bytes.STAT |= OAM;
            regs.bytes.STAT |= 0b00100000; //interrupt source
            bus->interruptFlags(0b00000010);
        }

    } else if(totalTicks < 253){
        
        if(statusMode != Transfer)
        {
            drawLine();
            VRAM_access = false;
            OAM_access = false;
            regs.bytes.STAT &= 0000000100;
            regs.bytes.STAT |= Transfer;
            regs.bytes.STAT |= 0b00001000; //interrupt source
            bus->interruptFlags(0b00000010);

        }

    } else if(totalTicks < 456){
        
        if(statusMode != hBlank)
        {
            VRAM_access = true;
            OAM_access = true;
            regs.bytes.STAT &= 0000000100;
            regs.bytes.STAT |= hBlank;
            regs.bytes.STAT |= 0b00001000; //interrupt source
            bus->interruptFlags(0b00000010);
        }

    } else {
        totalTicks -= 456;
        regs.bytes.LY++;
    }
}
