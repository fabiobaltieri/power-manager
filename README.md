USB Power Manager
=================

http://fabiobaltieri.com/2013/xx/xx/usb-power-manager/

This project is an USB power controller, and allows to control the power lines
of two USB ports and a generic supply line.

Main features:
- Two controllable USB ports (5V)
- One controllable generic supply line, up to 26V
- Overcurrent protection for USB ports
- Voltage and current measurement on both USB and power ports, before and after
  the switch
- Two additional independent relay lines (for switch simulation)

License
-------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Contents
--------

    COPYING     text version of the GPL
    README      this file
    bootloader/ source directory for bootloader
    firmware/   source directory for main firmware
    hardware/   source directory for the hardware design

Firmware and Bootloader
-----------------------

Building requires an avr-gcc toolchain, in the firmware/ or bootloader/
directory, to build run:

    make

Flashing the firmware on the device requires avrdude and a compatible hardware
programmer. Default configuration is stored at the beginning of the Makefile.
To program with the default configuration, run:

    make flash

Fuses can be configured running:

    make fuses

To program the main firmware from the bootloader, use the command:

    make boot

Hardware
--------

All hardware files (schematic, layout and libraries) are in CadSoft Eagle
format.
