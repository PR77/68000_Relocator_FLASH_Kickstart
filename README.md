# 68000 Relocator with FLASH based Kickstart
Forked from https://github.com/PR77/68000_Relocator_FLASH_Kickstart and the software and firmware have been modified.

It now supports two flashed kickstart ROMs and has a cleaner interface.

**NOTE:** If you are using FK 3.0 you will need to use the 3.0 Xilinx firmware too. Otherwise hi/lo ROMs will be the mapped to the same address.

# Warning
This design has not been compliance tested and will not be. It may cause damage to your A500. I take no responsibility for this. I accept no responsibility for any damage to any equipment that results from the use of this design and its components. IT IS ENTIRELY AT YOUR OWN RISK!

# Overview
To allow for Flash Kickstart images to be mapped in the same address space as existing ROM Kickstart without the need to remove the existing ROM. This design is targeting an accelerated A500(+) system which also require a CPU relocation.

### Appearance
Nice 3D model:
![3D Model](/Images/68000RelocatorFLASHKickstart.png)

... this is the final design.

### Final Hardware
Here is the final hardware loaded and installed with my Rev-2 Accelerator (https://github.com/PR77/A500_ACCEL_RAM_IDE-Rev-2). I was not able to easily source new AMD AM29F040B Flash devices so I changed over to SST39SF0x0A devices. They are JEDEC compliant so no issue with the pinout. Just watch out for the device identification in the SW.

![Final PCB](/Images/FinalHardware.jpg)

### BOM
For those wanting to build their own hardware, here is the BOM;

| Reference(s) | Value      | Footprint                                                      |
|--------------|------------|----------------------------------------------------------------|
| C1           | 10uF       | Capacitors_Tantalum_SMD:CP_Tantalum_Case-B_EIA-3528-21_Hand    |
| C2           | 10uF       | Capacitors_Tantalum_SMD:CP_Tantalum_Case-B_EIA-3528-21_Hand    |
| C3           | 100nF      | Capacitors_SMD:C_0805_HandSoldering                            |
| C4           | 100nF      | Capacitors_SMD:C_0805_HandSoldering                            |
| C5           | 100nF      | Capacitors_SMD:C_0805_HandSoldering                            |
| C6           | 100nF      | Capacitors_SMD:C_0805_HandSoldering                            |
| J3           | BLOCK      | Pin_Headers:Pin_Header_Straight_1x02_Pitch2.54mm               |
| J4           | JTAG       | Pin_Headers:Pin_Header_Straight_1x06_Pitch2.54mm               |
| R1           | 10K        | Resistors_SMD:R_0805_HandSoldering                             |
| R2           | 10K        | Resistors_SMD:R_0805_HandSoldering                             |
| R3           | 10K        | Resistors_SMD:R_0805_HandSoldering                             |
| U1           | 68000D     | Pin_Headers:Pin_Header_Straight_2x32_Pitch2.54mm               |
| U2           | 68000D     | Housings_DIP:DIP-64_W22.86mm_Socket_LongPads                   |
| U3           | SST39SF040 | Housings_DIP:DIP-32_W15.24mm_Socket                            |
| U4           | SST39SF040 | Housings_DIP:DIP-32_W15.24mm_Socket                            |
| U5           | XC9572VQ44 | Housings_QFP:TQFP-44_10x10mm_Pitch0.8mm                        |
| U6           | LM1117-3.3 | TO_SOT_Packages_SMD:SOT-223-3_TabPin2                          |

### In Operation
My test Amiga 500 has a ROM Kickstart version 1.2, so the boot screen looks like this:

![ROM Kickstart 1.2](/Images/Kickstart1.2.jpg)

Then when loading into RAM Kickstart version 3.1 the software will make a simple dump of the RAM to the console to ensure it has been written correctly (data blurred to avoid any potential (C) issues). Essentially some basic debugging which looks like this:

![Application Output](/Images/KickstartApplication.jpg)

... so now, using the magic sequence, I have Kickstart version 3.1 as shown here:

![ROM Kickstart 3.1](/Images/Kickstart3.1.jpg)

### How It Works
In principle the operation is fairly simple. A CPLD is used to switch between the ROM Kickstart on the Motherboard or the two Flash Kickstart on the CPU Relocator. Switching is performed by an active /RESET (CTRL-A-A) without interruption for longer than 1 second. Shorter /RESET durations will simply just reset the Amiga. After a POR (Power On Reset) by default the Kickstart on the Motherboard will be used.

#### ROM based Kickstart
When ROM Kickstart is being used, the hardware simply passed /AS to the Amiga Motherboard to allow GARY perform the address decode and chip select of the internal ROM. During this process, the Flash Kickstart is simply not used - except when programming. A Zorro II space of 512K or 1MB (depending on jumper) is requested and allocated by the Amiga which maps to the Flash devices.

When programming the Flash devices, access is via this Zorro II space. Programming of the Flash devices can only be performed when ROM Kickstart is being used.

#### FLASH based Kickstart
When Flash Kickstart is being used, the hardware simply blocks /AS to the Amiga Motherboard and the CPLD will perform the operation of Flash device control. As /AS is not asserted on the Amiga Motherboard, and therefore no GARY decode, the Relocator CPLD will then also assert the /DTACK after placing data on the bus - this is needed to complete the CPU cycle.

#### Overlay Interception
When Flash Kickstart mode is being used, the CPLD will also intercept the overlay of the Kickstart in the 0x000000 memory space until the first access to the custom chip register is performed. This ensures the correct SP and IP initialisation is performed directly after POR (Power On Reset) as the Flash Kickstart is used by default.
