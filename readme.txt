Firmware for control board for JediCut serial interface

Usable for 8bit AVR with hardware USB support. It's tested on custom board with AT90USB162, but any USB AVR should work.

Based on great LUFA library for USB suuport.

If you wish to use this, use AVR studio or WinAVR to compile it.
You might want to:
	- change MCU type, frequency etc in makefile
	- write own LED driver to Board/LEDs.h
		-change LED's meaning in JediCutFirmware.h (if you have less then 4leds)
	- reconfigure stepper's ports. Driver with 4 control lines is used in this source code (ie controling directly motor's phases, instead of DIR and STEP)
		- configure ports to output in JediCutFirmware.c -> setupHardware
		- change steppers port mappings in Stepper/StepperMotorConfig.h
	- check timer config (but should be written universally)
	- compile using AVR studio 6 or with WinAVR
	- you might need to install driver for virtual serial, use ini in driver folder
	 

