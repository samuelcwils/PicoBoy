#include "cpu.h"
#include "opcodes.cpp"
    
    cpu::cpu(bus* Bus)
    {           
        fp = fopen ("file.txt", "w+");
        this->Bus = Bus;
        cycles = 0;
        af.af = 0x01b0;
        bc.bc = 0x0013;
        de.de = 0x00d8;
        sp.sp = 0xfffe;
        pc.pc = 0x0100;
        IF = 0xe1;
        IE = 0;
        IME = false;
        bootRomDone = false;
        DIV = 0xab;
        TIMA = 0;
        TMA = 0;
        TAC = 0xf8;
        TIMA_speed = 1024;
        totalTicks_DIV = 0;
        totalTicks_TIMA = 0;
        IME = true;
        IMEdelay = false;
        servicingInterrupt = false;
        cycles = 0;
        for(int i = 0; i < 256; i++)
        {
            opcodeTimes[i] = 0;
        }
    }

    void cpu::checkInterrupts()
    {
        if(IME)
        {
            for (int i = 0; i < 5; i++) {
                if (IE & IF & (1 << i)) {
                    IME = 0;
                    IF &= ~(1 << i);
                    
                    servicingInterrupt = true;
                    
                    sp.sp--;
                    Bus->write(sp.sp, pc.bytes.p);
                    sp.sp--;
                    Bus->write(sp.sp, pc.bytes.c);

                    pc.pc = 0x40 + (i*0x8);
                    cycles += 20;
                    
                }
            }
        }
    }

void cpu::updateTimers()
{
    int ticks = cycles;
    totalTicks_DIV += ticks;
    DIV = ((uint8_t)((totalTicks_DIV & 0xff00) >> 8));
    
    if(TAC & 0b00000100)
    {    
        int newTicks = 0;  
        totalTicks_TIMA += ticks;
        if(totalTicks_TIMA / TIMA_speed)
        {
            newTicks = totalTicks_TIMA / TIMA_speed;
            totalTicks_TIMA -= TIMA_speed;
        }

        int temp = ((uint16_t)((TIMA + newTicks))) & 0xff00;

        if(temp){
            IF |= 0b00000100;
            TIMA = TMA;
            return;
        }

        TIMA += newTicks;
    }

}

