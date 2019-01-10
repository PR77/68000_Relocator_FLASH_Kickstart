# 68000 Relocator with FLASH based Kickstart
Forked from https://github.com/AwesomeInferno/68000Relocator and modified to support FLASH Kickstart allowing programming in system.

# Warning
This design has not been compliance tested and will not be. It may cause damage to your A500. I take no responsibility for this. I accept no responsibility for any damage to any equipment that results from the use of this design and its components. IT IS ENTIRELY AT YOUR OWN RISK!

# Overview
To allow for Flash based Kickstart images to be mapped in the same address space as existing ROM based kickstarts without the need to remove the existing ROM. This design is targeting an accelerated A500(+) system which also require a CPU relocation.

### Appearance
Nice 3D model:
![3D Model](/Images/68000RelocatorFLASHKickstart.png)

... this is not the final design as routing has not been completed. So some components may move, or added.

### Prototype
Drawing in from the experience and usefulness of my prototyping jig created for my Rev-1 Accelerator (https://github.com/PR77/A500_ACCEL_RAM_IDE-Rev-1), I decided to do the same for this hardware expansion.

So the prototype looks like this:

![Jumpered PCB](/Images/PrototypeHardware.jpg)

To keep the prototype simple, instead of using Flash devices (AMD AM29F040B are my target has I have these and experience with the devices) I used simple SRAMs (also from my Accelerator). There was also the learning curve with using VBCC (http://eab.abime.net/showthread.php?t=83113) to develop the software to control the hardware, but with that sorted out I managed to finally get the prototype to work.

My test Amiga 500 has a ROM Kickstart version 1.2, so the boot screen looks like this:

![ROM Kickstart 1.2](/Images/Kickstart1.2.jpg)

Then when loading into RAM Kickstart version 3.1 the software will make a simple dump of the RAM to the console to ensure it has been written correctly (data blurred to avoid any potential (C) issues). Essentially some basic debugging which looks like this:

![Application Output](/Images/KickstartApplication.jpg)

... so now, using the magic sequence, I have Kickstart version 3.1 as shown here:

![ROM Kickstart 3.1](/Images/Kickstart3.1.jpg)

### How It Works
In principle the operation is fairly simple. A CPLD is used to switch between the ROM Kickstart on the Motherboard or the Flash Kickstart on the CPU Relocator. Switching is performed by an active /RESET (CTRL-A-A) without interruption for longer than 1 second. Shorter /RESET durations will simply just reset the Amiga. After a POR (Power On Reset) by default the Flash Kickstart on the CPU Relocator will be used.

#### ROM based Kickstart

#### FLASH based Kickstart

#### Programming

### Known Issues And Pending Items
As this design is still in-the-making, the following items are pending:

1. PCB Layout and schematic finalisation.
2. Support for ROMs based at address 0xF00000.
3. Test with different ROMs.
