# 68000 Relocator with FLASH based Kickstart
Forked from https://github.com/PR77/68000_Relocator_FLASH_Kickstart and the software and firmware have been modified as well as a GUI tool created.

It now supports two flashed kickstart ROMs and has a cleaner interface.

**NOTE:** If you are using FK 3.0 command line tool or FlashKickstart GUI tool you will need to use the 3.0 Xilinx firmware too. Otherwise hi/lo ROMs will be the mapped to the same address.

# Warning
This design has not been compliance tested and will not be. It may cause damage to your A500. I take no responsibility for this. I accept no responsibility for any damage to any equipment that results from the use of this design and its components. IT IS ENTIRELY AT YOUR OWN RISK!

# Overview
To allow for Flash Kickstart images to be mapped in the same address space as existing ROM Kickstart without the need to remove the existing ROM. This design is targeting an accelerated A500(+) system which also require a CPU relocation.

# Flashing new ROMs
It is recommended you use the included FlashKickstart GUI tool. To use this you may need to install the reqtools library in your LIBS: drive. If you are using Kickstart 1.3 this is available in the lib13 directory of the software, for 2.0 onwards use the version in libs20 directory.

You can then load "FlashKickstart" and you should see the window similar to below:

![FlashKickstart](/Images/shot1.png)

You need to erase the ROMs before flashing a new image. Erasing will clear both the flash ROM slots.

![Erasing](/Images/shot4.png)

You can then program with any ROM you wish. You should not use byte-swapped ROMs.

![File browsing](/Images/shot3.png)

![Flashing](/Images/shot2.png)

# Using the ROMs
In principle the operation is fairly simple. A CPLD is used to switch between the ROM Kickstart on the Motherboard or the two Flash Kickstart on the CPU Relocator. Switching is performed by an active /RESET (CTRL-A-A) without interruption for longer than 1 second. Shorter /RESET durations will simply just reset the Amiga. After a POR (Power On Reset) by default the Kickstart on the Motherboard will be used.

## ROM based Kickstart
When ROM Kickstart is being used, the hardware simply passed /AS to the Amiga Motherboard to allow GARY perform the address decode and chip select of the internal ROM. During this process, the Flash Kickstart is simply not used - except when programming. A Zorro II space of 512K or 1MB (depending on jumper) is requested and allocated by the Amiga which maps to the Flash devices.

When programming the Flash devices, access is via this Zorro II space. Programming of the Flash devices can only be performed when ROM Kickstart is being used.

## FLASH based Kickstart
When Flash Kickstart is being used, the hardware simply blocks /AS to the Amiga Motherboard and the CPLD will perform the operation of Flash device control. As /AS is not asserted on the Amiga Motherboard, and therefore no GARY decode, the Relocator CPLD will then also assert the /DTACK after placing data on the bus - this is needed to complete the CPU cycle.

### Overlay Interception
When Flash Kickstart mode is being used, the CPLD will also intercept the overlay of the Kickstart in the 0x000000 memory space until the first access to the custom chip register is performed. This ensures the correct SP and IP initialisation is performed directly after POR (Power On Reset) as the Flash Kickstart is used by default.
