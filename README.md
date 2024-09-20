This is a gameboy emulator for the Raspberry Pi Pico. Games with LCD STAT interrupts have issues. Feel free to contribute.

It is designed around the Pimoroni Pico Explorer Base (although it should work on any st7789 lcd).
Runs at full speed with all inputs setup.
This does overclock your device!

# How to add games
Put a .gb rom file into the include folder and rename it "gameboy.gb". It will be included into the binary.

# Note
Requires git submodule init and git submodule update to get dependancies]

There are definitely some things I would have done differently if I made this project now. Not having a function to pass in pointers of the components (eg. connectCPU) is one example.
