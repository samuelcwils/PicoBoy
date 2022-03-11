#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "bus.h"

class bus;
class cpu {

private:
    bus* Bus; 

    inline void Zflag(uint8_t a, uint8_t b); 
    inline void Zflag_sub(uint16_t a, int b); 
    inline void Hflag(uint8_t a, uint8_t b); 
    inline void Hflag(uint16_t a, uint16_t b); 
    inline void Hflag_sub(uint8_t a, uint8_t b);
    inline void Cflag(uint8_t a, uint8_t b);
    inline void Cflag(uint16_t a, uint16_t b);
    inline void Cflag_sub(uint8_t a, uint8_t b);

    inline void LD_d16(uint8_t &high, uint8_t &low);
    inline void LD_d8(uint8_t &byte);
    inline void LD_REG1_REG2(uint8_t &a, uint8_t b);
    inline void LD_HL_REG(uint8_t reg);
    inline void LD_REG_HL(uint8_t &reg);
    inline void LD_ADDRESS_A(uint16_t address);
    inline void LD_A_ADDRESS(uint16_t address);
    
    inline void JP_a16();
    inline void JR();
    inline void JP_cond(bool flag);
    inline void JR_cond(bool flag);
    
    inline void POP_16b(uint8_t &high, uint8_t &low);
    inline void PUSH_16b(uint8_t high, uint8_t low);
    inline void CALL();
    inline void CALL_cond(bool flag);

    inline void RST(uint8_t H);
    inline void RET();
    inline void RET_cond(bool flag);
    
    inline void ADD(uint16_t &a, uint16_t b);
    inline void ADD(uint8_t &a, uint8_t b);
    inline void ADC(uint8_t &a, uint8_t b);
    inline void SUB(uint8_t &a, uint8_t b);
    inline void SBC(uint8_t &a, uint8_t b);
    
    inline void AND(uint8_t &a, uint8_t b);
    inline void XOR(uint8_t &a, uint8_t b);
    inline void OR(uint8_t &a, uint8_t b);
    inline void CP(uint8_t &a, uint8_t b);

    inline void INC(uint16_t &value);
    inline void INC(uint8_t &byte);
    inline void DEC(uint16_t &value);
    inline void DEC(uint8_t &byte);
    
    inline void RLC(uint8_t &byte);
    inline void RRC(uint8_t &byte);
    inline void RL(uint8_t &byte);
    inline void RR(uint8_t &byte);
    inline void SLA(uint8_t &byte);
    inline void SRA(uint8_t &byte);
    inline void SWAP(uint8_t &byte);
    inline void SRL(uint8_t &byte);
    
    inline void BIT(uint8_t &byte, uint8_t bitNum); //bitNum should be entered as binary (ex: bit 5 is 0b00100000 )
    
    inline void RES(uint8_t &byte, uint8_t bitNum); //bitNum should be entered as binary (ex: bit 5 is 0b00100000 )
    inline void SET(uint8_t &byte, uint8_t bitNum); //bitNum should be entered as binary (ex: bit 5 is 0b00100000 )

    union {
        struct{ 
        uint8_t f;
        uint8_t a;
        } bytes;
        uint16_t af;
    } af; //register af
    
    union {
        struct{ 
        uint8_t c;
        uint8_t b;
        } bytes;
        uint16_t bc;
    } bc; //register bc
    
    union {
        struct{ 
        uint8_t e;
        uint8_t d;
        } bytes;
        uint16_t de;
    } de; //register de 

    union {
        struct{ 
        uint8_t l;
        uint8_t h;
        } bytes;
        uint16_t hl;
    } hl; //register hl 

    union {
        struct{ 
        uint8_t p;
        uint8_t s;
        } bytes;
        uint16_t sp;
    } sp; //register sp
    
public:
    
    union {
        struct{ 
        uint8_t c;
        uint8_t p;
        } bytes;
        uint16_t pc;
    } pc; //register pc 

    bool debug = false;
    bool bootRomDone;

    uint8_t DIV;
    uint8_t TIMA; //FF05
    uint8_t TMA; //FF06
    uint8_t TAC; //FF07
    int TIMA_speed;
    uint16_t totalTicks_DIV;
    uint16_t totalTicks_TIMA;
    
    FILE* fp;
    bool IME;
    bool IMEdelay;
    uint8_t IE;
    uint8_t IF;
    bool servicingInterrupt;

    uint8_t opcode;
    int cycles; //counts up cycles then emulates speed
    

    cpu(bus* Bus);
    void checkInterrupts();
    void updateTimers();
    void execOP();

};