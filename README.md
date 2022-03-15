# WIP
This is a gameboy emulator for the Raspberry Pi Pico and ESP32. Games with LCD STAT interrupts have issues. Feel free to contribute.


It is designed around the Pimoroni Pico Explorer Base (although it should work on any st7789 lcd).
Currently the emulation cannot run at full speed. There is no input setup either.
# How to add games
Put a .gb rom file into the include folder and rename it "gameboy.gb". It will be included into the binary.
