#pragma once
#include "stdint.h"
#include "bus.h"
#include "cpu.h"
#include "pico/util/queue.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "cpu.h"
#include <queue>
#include <cstdlib>
#include <cmath>

class cpu;
class bus;

class ppu {
public:
    
    union{
        struct{
            uint8_t tileData[0x1800];
            struct{
                uint8_t map1[0x400];
                uint8_t map2[0x400];
            } maps;
        } blocks;
        uint8_t vRam[0x2000];
    } vRam;

    uint8_t oam[160];

    uint16_t* frameBuffer;
    bool frameDone;
    bool OAM_access;
    bool VRAM_access;
    bool disableFlag;

    union{
        struct {
            uint8_t LCDC; //FF40
            uint8_t STAT; //FF41
            uint8_t SCY;  //FF42
            uint8_t SCX;  //FF43
            uint8_t LY;   //FF44
            uint8_t LYC;  //FF45
            uint8_t DMA;  //FF46
            uint8_t BGP;  //FF47
            uint8_t OBP0; //FF48
            uint8_t OBP1; //FF49
            uint8_t WY;   //FF4A
            uint8_t WX;   //FF4B   
        } bytes;
        uint8_t regs[12];

    } regs;

    ppu();

    void connectBus(bus* Bus);
    void connectCPU(cpu* CPU);
    void initQueue();

    void DMA(uint16_t nn);

    void tick();

    void drawTile(int x, int y, int index); //debug thing

    void drawTiles(); //debug thing

    uint64_t totalLength = 0;

    int totalFrames = 0;

    bool core1_done;

    struct{
        uint16_t data[8];
        bool state; //true = full
    }queue0;

    struct{
        uint16_t data[8];
        bool state; //true = full
    }queue1;

    int fifoSize;
    int scrollingLeft;
    int amountScrolled;
    int xPos;
   
    struct{
        uint8_t lowLine;
        uint8_t highLine;
        uint16_t fullLine[8];
        uint8_t sprite_lowLine;
        uint8_t sprite_highLine;
        uint16_t sprite_fullLine[8];

        int state;
        int tileID;
        int tileLine;
        uint16_t tileRowAddr;
        uint16_t BG_mapBase;
        uint16_t WIN_mapBase;
        uint16_t dataBase; //not for sprites
        uint8_t tileCollumn;
    } fetcher;

    enum {
    getTile  = 0,
    line0    = 1,
    line1    = 2,
    idle     = 3,
    };
    
    uint8_t reverseBits(uint8_t n);

    void fetchSprite(int id, bool xFlip, bool yFlip, bool palette, int yPos, int shift); // palette: 0=OBP0, 1=OBP1

    void fetch();

    void fetcherDrawSprite();

    int getColorID(int colorIndex);

    uint16_t getPixel();

    enum {
	hBlank   = 0,
	vBlank   = 1,
	OAM      = 2,
	Transfer = 3,
    };


    uint8_t statusMode;
    int totalTicks;
    bus* Bus;
    cpu* CPU;

};