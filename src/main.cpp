#include "bus.h"
#include "cart.h"
#include "cpu.h"
#include "incbin.h"
#include "hardware/timer.h"
#include "hardware/vreg.h"
#include "pico/stdio.h"
#include "pico/multicore.h"
#include "pico_explorer.hpp"
#include "pico_graphics.hpp"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "cpu.cpp"
#include <chrono>
#include <thread>

extern "C" INCBIN(Rom, "gameboy.gb"); 

using namespace std::chrono_literals;
using namespace std::chrono;

bool frameDone = false;
unsigned long totalInstructions = 0;

using namespace pimoroni;

static cart* cartridge;
static ppu* PPU;
static bus* Bus;
static cpu* CPU;
static PicoExplorer* Pico_p;

bool CPU_active = false;

#define PLL_SYS_KHZ (133 * 1000)

int main()
{     

    vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(420000, true);
       
    clock_configure(
    clk_peri,
    0,                                                // No glitchless mux
    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
    PLL_SYS_KHZ * 1000,                               // Input frequency
    PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );

    stdio_init_all();

    int frames = 0;

    cartridge = new cart( (uint8_t*)gRomData, (uint32_t)gRomSize);
   
    PPU = new ppu();

    Bus = new bus(cartridge, PPU);
    CPU = new cpu(Bus);

    PPU->connectBus(Bus);
    PPU->connectCPU(CPU);
    PPU->connectPico(Pico_p);
    Bus->connectCPU(CPU);
    
    uint16_t* buffer = new uint16_t[PicoExplorer::WIDTH * PicoExplorer::HEIGHT];

    for(int i = 0; i < 57600; i++)
    {
        buffer[i] = 0;
    }

    PicoExplorer Pico(buffer);
    Pico_p = &Pico;
    Pico.init();

    PPU->frameBuffer = buffer;

    cartridge->printCart();

    while(true)
    {
        uint64_t start = time_us_64();
        while(!PPU->frameDone)
        {             
            CPU->execOP();
            CPU->updateTimers();
            PPU->tick();
    
            CPU->cycles = 0;
        }
        uint64_t stop = time_us_64();    
        uint64_t length = stop - start;

        printf("Emulation: %llu\n", length);

        frameDone = true;

        start = time_us_64();

        Pico.update();
        
        stop = time_us_64();
        length = stop - start;

        printf("CLK_SYS: %u\n", clock_get_hz(clk_sys));
        printf("Screen: %llu\n", length);

        PPU->frameDone = false;
            
        frames++;    

    }

    return 0;   
}