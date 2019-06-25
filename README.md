# **PicProg - Programming Microchip PIC16(L)F184XX based on STM32F3Discovery board**## Notes:- PicProg consists of two parts - STM32F303VC code part (folder picprog4) and Windows10 interface part (folders picprog and WPicProg) .### STM32F303VC- MCLR, ICSPCLK and ICSPDAT of PIC must be connected to PB2, PB4 and PB5 of STM32F303VC- Use no optimization while compiling (-O0)### Interface- PicProg interface uses ".elf" file as a source of code to be programmed.- PicProg interface uses [ELFIO](http://serge1.github.io/ELFIO) library. Please include path to ELFIO files in your compiler -I option.- This interface version runs under Windows 10. **picprog.exe** - command line version and **WPicProg.exe** - GUI version.- Command line version:picprog.exe  COMname  <full path to .elf file>COMname - COM1, COM2 ….. COM99 - virtual COM port, connected to STM32F3Discovery. 