# bcomp11 - on-board computer

Car on-board computer based on LPC11C14. Specified for Mitsubishi Pajero Sport 2nd gen.

Configurations
-----------------------------------------

The Keil MDK project have some configurations:

<b>bcomp11 (Debug)</b> - old interface version, debug build.<br>
<b>bcomp11 (Release)</b> - old interface version, release build.<br>
<b>bcomp11v2 (Debug)</b> - new (actual) interface version, debug build.<br>
<b>bcomp11v2 (Release)</b> - new (actual) interface version, release build.<br>

For build in GCC-ARM use Makefile. GCC build didn't full functionality, because GCC generate more code. 

The sub-folder /pc contains HAL-level and build-script for compilation in MinGW-Win32. This build use for easy interface debug.

Full description
-----------------------------------------

http://igorkov.org/bcomp11 (old version)
http://igorkov.org/bcomp11v2 (actual version)

Hardware description 
-----------------------------------------

Computer based on NXP LPC11C14 with HW CAN. Full hardware project here: https://github.com/igkov/bcomp11hw

Scheme:
<img src="http://igorkov.org/images/bcomp11v2-scheme.png">

Device view (this is first hardware version)*:
<img src="http://igorkov.org/images/bcomp11-tests.jpg">
*on photo 2 board with LPC11C14: this computer, and test device for automotive CAN-NET: http://github.com/igkov/ecu-emulator

Compilation
---------

Use <a href="http://www.keil.com">Keil MDK</a> or <a href="https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads">GCC ARM</a>. Recommended version of Keil MDK - 4.23 or 4.24, newer versions have other editor kernel and cann't show Cyrillic in CP-1251 (only in Unicode/UTF-8), thats problem for show/edit Cyrillic text constants.

Program MCU
---------

For program MCU, please use USB-UART converter with DTR/RST control and programming utility as <a href="http://www.flashmagictool.com/">Flash Magic</a> or <a href="https://sourceforge.net/projects/lpc21isp/">lpc21isp</a>.

Donate project
---------

You can donate this project <a href="https://www.paypal.me/kovalenkoi">here</a>.

