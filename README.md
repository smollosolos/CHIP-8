# CHIP-8
CHIP-8 Interpreter / Emulator made with C and SDL3
![image](https://github.com/user-attachments/assets/16e2b3e6-7271-41cc-8a8c-80036d12f78a)

## Overview
CHIP-8 is an interpreted programming language developed in the 1970s. It was initially used on the COSMAC VIP and Telmac 1800 8-bit microcomputers to make game programming easier. This emulator implements the original CHIP-8 interpreter, allowing you to run classic games and programs written for the platform.
More information : https://en.wikipedia.org/wiki/CHIP-8

## Features
- Complete implementation of all CHIP-8 instructions
- Configurable sound using SDL3
- Keyboard input mapping matching the original COSMAC VIP layout (more information below)
- Configurable display rendering FPS
- Support for loading and running ROM files through command-line
- Configurable instruction execution speed
- Configurable colors - monochrome display
- Configurable quirks - (subtle differences in the way CHIP-8 implementations interpret bytecode)

## Usage
The interpreter was made with C17 and SDL3 3.2
run from command-line:
```
make (or whatever command you need to run) file="YOURROMDIRECTORY"
```
Example:
```
mingw32-make file="chip8-roms\programs\IBM Logo.ch8"
```
![image](https://github.com/user-attachments/assets/a801aa58-1b94-4236-9b77-711908bc8b11)

Many roms are provided in this repository but feel free to use your own!

## Keyboard
The original CHIP-8 used a 16-key hexadecimal keypad (on the left). This emulator maps those keys to your keyboard as follows:

![keyboard layout](https://github.com/user-attachments/assets/2c0ba486-3e78-41c4-81ae-52cb8f1ec75a)

## Notes
This is my first emulator/interperter, it was used for learning emulation.  One quirk that is not implemented is waiting for next interrupt. At that point the interrupt routine occurs (during which the display is refreshed). More information can be found here: https://www.laurencescotford.net/2020/07/19/chip-8-on-the-cosmac-vip-drawing-sprites/

## Thanks
- [Guide: Writing a CHIP-8 Emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/) by Tobias V. Langhoff - This guide provided the foundational knowledge and step-by-step guidance for implementing the CHIP-8 system.
- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) - A  technical reference that helped ensure accurate implementation of all CHIP-8 instructions.
- [CHIP-8 Test Suite](https://github.com/Timendus/chip8-test-suite) by Timendus - Used to verify the correctness of the emulator's implementation.
- [CHIP-8 ROMS](https://github.com/kripod/chip8-roms/tree/c723a9ed1205a215c5b1e45e994eb54acc243c9e) by Kripod - A collection of CHIP-8 / SuperChip / MegaChip8 programs.
- [Octo ROMS](https://johnearnest.github.io/chip8Archive/) by John Earnest - A archive of CHIP-8 programs made for octojams.


Demo:

![Demo-ezgif com-crop](https://github.com/user-attachments/assets/4a210cff-fd89-416c-a617-c19b20977b65)

Thanks for reading! Have fun learning, gaming or coding!
