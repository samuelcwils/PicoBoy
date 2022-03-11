#include "ppu.h"

ppu::ppu()
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
    totalFrames = 0; 
}

void ppu::connectBus(bus* Bus)
{
    this->Bus = Bus;
}

void ppu::connectCPU(cpu* CPU)
{
    this->CPU = CPU;
}

void ppu::DMA(uint16_t nn)
{
    for(int i = 0; i < 0xa0; i++)
    {
        oam[i] = Bus->read(nn + i);
    }

}

uint8_t ppu::reverseBits(uint8_t n)
{
    uint8_t rev = 0;

    while (n > 0)
    {
        rev <<= 1;

        if (n & 1 == 1)
            rev ^= 1;

        n >>= 1;

    }

    return rev;
}

void ppu::fetchSprite(int id, bool xFlip, bool yFlip, bool palette, int yPos, int shift)
{
    int spriteLine;
    int highline_offset;

    if(yFlip)
    {
       spriteLine = (yPos - 16) - regs.bytes.LY; 
       highline_offset = -1;
    } else {
        spriteLine = regs.bytes.LY - (yPos - 16);
        highline_offset = 1;
    }

    fetcher.sprite_lowLine = Bus->read(0x8000 + (id * 16) + (spriteLine * 2));
    fetcher.sprite_highLine = Bus->read(0x8000 + (id * 16) + (spriteLine * 2) + highline_offset);
    
    if(xFlip){
        fetcher.sprite_lowLine = reverseBits(fetcher.sprite_lowLine);
        fetcher.sprite_highLine = reverseBits(fetcher.sprite_lowLine);
    }

    if(shift > 0)
    {
        fetcher.sprite_lowLine = fetcher.sprite_lowLine << shift;
        fetcher.sprite_highLine = fetcher.sprite_highLine << shift;
    } else {
        shift = abs(shift);
        fetcher.sprite_lowLine = fetcher.sprite_lowLine >> shift;
        fetcher.sprite_highLine = fetcher.sprite_highLine >> shift;
    }

    for(int i = 0; i < 8; i++)
    {
        fetcher.sprite_fullLine[7 - i] = (( (fetcher.sprite_highLine & ((1 << i))) << 1) + (fetcher.sprite_lowLine & (1 << i))) >> i;
    }
    
}

void ppu::fetch()
{
    switch(fetcher.state)
    {
        case getTile:
            fetcher.tileID = Bus->read(fetcher.tileRowAddr + fetcher.tileCollumn);
            
            if(regs.bytes.LCDC & 0b00010000)
            {
                fetcher.dataBase = 0x8000;    
            } else {
                fetcher.dataBase = 0x9000;
            }

            fetcher.state = line0;
            break;
        
        case line0:
            fetcher.lowLine = Bus->read(fetcher.dataBase + (fetcher.tileID * 16) + (fetcher.tileLine * 2));
            fetcher.state = line1;
            break;
        
        case line1:
        {
            fetcher.highLine = Bus->read(fetcher.dataBase + (fetcher.tileID * 16) + (fetcher.tileLine * 2) + 1);
            
            for(int i = 0; i < 8; i++)
            {
                fetcher.fullLine[7 - i] = (( (fetcher.highLine & ((1 << i))) << 1) + (fetcher.lowLine & (1 << i))) >> i;
            }
            
            if(fifoSize == 8 || fifoSize == 0){ 

                fetcherDrawSprite();

                if(!queue0.state)
                {
                    for(int i = 0; i < 8; i++){
                        queue0.data[i] = fetcher.fullLine[i];
                    }
                    queue0.state = true;
                    fifoSize+=8;

                } else if(!queue1.state)
                {
                    for(int i = 0; i < 8; i++){
                        queue1.data[i] = fetcher.fullLine[i];
                    }
                    queue1.state = true;
                    fifoSize+=8;
                }
                
                fetcher.state = getTile;
                fetcher.tileCollumn++;
            } else {
                fetcher.state = idle;
            }
            
            break;
        }
        case idle:
        {

            if(fifoSize == 8 || fifoSize == 0){ 

                fetcherDrawSprite();

                if(!queue0.state)
                {
                    for(int i = 0; i < 8; i++){
                        queue0.data[i] = fetcher.fullLine[i];
                    }
                    queue0.state = true;
                    fifoSize+=8;

                } else if(!queue1.state)
                {
                    for(int i = 0; i < 8; i++){
                        queue1.data[i] = fetcher.fullLine[i];
                    }
                    queue1.state = true;
                    fifoSize+=8;
                }
                
                fetcher.state = getTile;
                fetcher.tileCollumn++;
            } else {
                fetcher.state = idle;
            }

            break;
        }
    }

}

void ppu::fetcherDrawSprite()
{
    if(regs.bytes.LCDC & 0b00000010) //if sprite enable
    {
        bool spriteSize = regs.bytes.LCDC & 0b00000100;
        
        for(int i = 156; i >= 0; i-=4) //scan through to see if sprites are visible
        {
            bool yVisibility;

            if(spriteSize)
            {
                yVisibility = ((int)oam[i] - regs.bytes.LY - 16) < 17 && ((int)oam[i] - regs.bytes.LY - 16) > 0;
            } else {
                yVisibility = ((int)oam[i] - regs.bytes.LY - 8) < 9 && ((int)oam[i] - regs.bytes.LY - 8) > 0;
            }

            if(yVisibility)
            {
                int shift = (xPos + 16 - ((int)oam[i+1])); // or xVisibility. Finds how much a line in a sprite will be shifted (left) in order to fit in it's proper location. 
                
                if((shift < 8) && (shift > - 8))
                {
                    fetchSprite(oam[i + 2], oam[i + 3] & 0b00100000, oam[i + 3] & 0b01000000, oam[i + 3] & 0b10000000, oam[i], shift);
                    for(int i = 0; i < 8; i++)
                    {
                        if(fetcher.sprite_fullLine[i] == 0)
                        {
                            //do not write pixel if transparent
                        } else {
                            fetcher.fullLine[i] = fetcher.sprite_fullLine[i];
                        }
                        
                    }
                }


                
            }
        }
    }
            
}

uint16_t ppu::getPixel()
{
    int colorIndex;
    if(fifoSize <= 8)
    {
        colorIndex = queue0.data[8 - fifoSize];
    } else {
        colorIndex = queue1.data[16 - fifoSize];
    }


    int colorID = (((regs.bytes.BGP & (0b00000011 << (colorIndex * 2))) >> (colorIndex * 2)));
    uint16_t lookup[4] = {0x6c93, 0x432e, 0x2a0a, 0x1105};
    return lookup[colorID];
}

void ppu::tick()
{
    int ticks = CPU->cycles;

    uint64_t start;
    uint64_t stop;
    uint64_t length;

    while(ticks > 0)
    {
        Bus->PPU_read = true;

        statusMode = regs.bytes.STAT & 0b00000011;

        switch(statusMode)
        {
            
            case OAM:
            {
                OAM_access = false;
                if(totalTicks == 80){
                    if(regs.bytes.LCDC & 0b00001000)
                    {
                        fetcher.BG_mapBase = 0x9c00;
                    } else {
                        fetcher.BG_mapBase = 0x9800;
                    }

                    if(regs.bytes.LCDC & 0b01000000)
                    {
                        fetcher.WIN_mapBase = 0x9c00;
                    } else {
                        fetcher.WIN_mapBase = 0x9800;
                    }

                    fetcher.tileLine = (regs.bytes.LY + regs.bytes.SCY) % 8;
                    fetcher.tileRowAddr = fetcher.BG_mapBase + ( ( (  ( (regs.bytes.LY + regs.bytes.SCY) % 256 )/8) ) * 32);
                    
                    for(int i = 8; i > 0; i--) //clear FIFO from last line
                    {
                        queue0.data[i] = 0;
                        queue1.data[i] = 0;
                    }
                    queue0.state = false;
                    queue1.state = false;

                    scrollingLeft = regs.bytes.SCX;
                    fetcher.state = 0;
                    fetcher.tileCollumn = 0;
                    OAM_access = true;
                    regs.bytes.STAT &= 0b11111100;
                    regs.bytes.STAT |= Transfer;
                }
                break;

            }
            case Transfer:
            {
                if(!(totalFrames % 3))
                {
                    if(totalTicks % 2)
                    {
                        fetch();
                    }
                    
                    VRAM_access = false;
                    OAM_access = false;

                    if((fifoSize > 8))
                    {      
                        int amountScrolled = 0;
                        while(scrollingLeft > 0 && ((fifoSize - amountScrolled) > 8))
                        {
                            fifoSize--;
                            scrollingLeft--;
                            amountScrolled++;
                        }
                        
                        frameBuffer[( (regs.bytes.LY + 48) * 240) + (xPos + 40)] = getPixel();
                        fifoSize--;
                        xPos++;

                        if(fifoSize == 8)
                        {
                            queue1.state = false;
                        }

                        if(fifoSize == 0)
                        {
                            queue0.state = false;
                        }

                        if(regs.bytes.WY == regs.bytes.LY)
                        {
                            if(xPos == regs.bytes.WX)
                            {
                                for(int i = 8; i > 0; i--) //clear FIFO from last line
                                {
                                    queue0.data[i] = 0;
                                    queue1.data[i] = 0;
                                }
                                queue0.state = false;
                                queue1.state = false;

                                fifoSize = 0;

                                fetcher.tileRowAddr = fetcher.WIN_mapBase + (((regs.bytes.LY) ) / 8) * 32;
                                fetcher.state = getTile;
                            }
                        }

                        if(xPos == 160){
                            xPos = 0;
                            totalTicks+=2;
                            regs.bytes.STAT &= 0b00000100;
                            regs.bytes.STAT |= hBlank;
                            regs.bytes.STAT |= 0b00001000; //interrupt source
                            CPU->IF |= 0b00000010;
                            VRAM_access = true;
                            OAM_access = true;
                        }
                    }

                } else {
                    
                    if(totalTicks == 253)
                    {
                        regs.bytes.STAT &= 0b00000100;
                        regs.bytes.STAT |= hBlank;
                        regs.bytes.STAT |= 0b00001000; //interrupt source
                        CPU->IF |= 0b00000010;
                        VRAM_access = true;
                        OAM_access = true;
                    }
                }

                break;
            }
            case hBlank:
            {

                VRAM_access = true;
                OAM_access = true;
                if(totalTicks == 456)
                {
                    totalTicks = 0; 
                    regs.bytes.LY++;

                    if(regs.bytes.LY == 144)
                    {
                        CPU->IF |= 0b00000001; //sends vblank interrupt
                        
                        regs.bytes.STAT &= 0b00000100;
                        regs.bytes.STAT |= vBlank;
                        regs.bytes.STAT |= 0b00010000; //interrupt source
                        CPU->IF |= 0b00000010;
                    
                    } else {
                        regs.bytes.STAT &= 0b00000100;
                        regs.bytes.STAT |= OAM;
                        regs.bytes.STAT |= 0b00100000; //interrupt source
                        CPU->IF |= 0b00000010;
                    }

                }   

                break;
            }
            case vBlank:
            {

                VRAM_access = true;
                OAM_access = true;
                
                if(totalTicks == 456)
                {
                    totalTicks = 0; 
                    regs.bytes.LY++;
                    
                    if(regs.bytes.LY == 153)
                    {
                        regs.bytes.LY = 0; 
                        
                        regs.bytes.STAT &= 0b00000100;
                        regs.bytes.STAT |= OAM;
                        regs.bytes.STAT |= 0b00100000; //interrupt source
                        CPU->IF |= 0b00000010;
                        
                        frameDone = true;
                        totalFrames++;
                    }
                }

                break;
            }
        }

        totalTicks++; 

        if(regs.bytes.LY == regs.bytes.LYC)
        {
            regs.bytes.STAT &= 0b00000011;
            regs.bytes.STAT |= 0b01000100;
            CPU->IF |= 0b00000010;
            
        } else 
        {
            regs.bytes.STAT &= 0b11111011;
        }

        ticks--;

    }

}
