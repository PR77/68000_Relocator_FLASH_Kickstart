# 68000 Relocator with FLASH based Kickstart
Forked from https://github.com/AwesomeInferno/68000Relocator and modified to support FLASH Kickstart allowing programming in system.

# Warning
This design has not been compliance tested and will not be. It may cause damage to your A500. I take no responsibility for this. I accept no responsibility for any damage to any equipment that results from the use of this design and its components. IT IS ENTIRELY AT YOUR OWN RISK!

# Overview
To allow for FLASH based Kickstart images to be mapped in the same address space as existing ROM based kickstarts without the need to remove the existing ROM. This design is targeting an accelerated A500(+) system which also require a CPU relocation.

### Appearance
Nice 3D model:
![3D Model](/Images/68000RelocatorFLASHKickstart.png)

... this is not the final design as routing has not been completed. So some components may move, or added.

### How It Works
In principle the operation is fairly simple. A CPLD is used to switch between the ROM Kickstart on the Motherboard or the Flash Kickstart on the CPU Relocator. Switching is performed by an active /RESET (CTRL-A-A) without interruption for longer than 1 second. Shorter /RESET durations will simply just reset the Amiga. After a POR (Power On Reset) by default the Flash Kickstart on the CPU Relocator will be used.

#### ROM based Kickstart

#### FLASH based Kickstart

#### Programming

### Known Issues And Pending Items
As this design is still in-the-making, the following items are pending:

1. PCB Layout and schematic finalisation. Pinning for CPLD is open to allow for routing.
2. CPLD logic verification. Currently fitted to a XC9572XL, however target device is XC9536XL.
3. Testing.
