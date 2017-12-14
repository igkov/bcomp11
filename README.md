# bcomp11 - on-board computer

Car on-board computer based on LPC11C14. Specified for Mitsubishi Pajero Sport 2nd gen.

Configurations
-----------------------------------------

The Keil MDK project have some configurations:

<b>bcomp11 (Debug)</b> - old interface version, debug build.<br>
<b>bcomp11 (Release)</b> - old interface version, release build.<br>
<b>bcomp11v2 (Debug)</b> - new (actual) interface version, debug build.<br>
<b>bcomp11v2 (Release)</b> - new (actual) interface version, release build.<br>
<b>bcomp11v2 (ReleaseF)</b> - new (actual) interface version, release build for SH1106 and gauge meter.<br>

The sub-folder /pc contains HAL-level and build-script for compilation in MinGW-Win32. This build use for easy interface debug.

Full description
-----------------------------------------

http://igorkov.org/bcomp11 (old version)
http://igorkov.org/bcomp11v2 (actual version)

Hardware description 
-----------------------------------------

Computer based on NXP LPC11C14 with HW CAN.

Scheme:
<img src="http://igorkov.org/images/bcomp11-scheme-v2.png">

Device view*:
<img src="http://igorkov.org/images/bcomp11-tests.jpg">
*on photo 2 board with LPC11C14: this computer, and test device for automotive CAN-NET: http://github.com/igkov/ecu-emulator
