/*
    This file is part of FLASH_KICKSTART originally designed by
    Paul Raspa.

    This file was created by Andrew (LinuxJedi) Hutchings.

    FLASH_KICKSTART is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    FLASH_KICKSTART is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLASH_KICKSTART. If not, see <http://www.gnu.org/licenses/>.
*/

#include <clib/intuition_protos.h>
#include <string.h>
#include <stdio.h>

#include "RomInfo.h"

int getRomInfo(BYTE *address, struct romInfo *info)
{
    BYTE data = *address;
    if (data != 0x11)
    {
        return ERR_NOT_ROM;
    }
    address++;
    data = *address;
    switch (data)
    {
        case 0x11:
            info->id = ROM_TYPE_256;
            break;
        case 0x14:
            info->id = ROM_TYPE_512;
            break;
        default:
            info->id = ROM_TYPE_UNKNOWN;
            return 1;
            break;
    }
    memcpy(&info->major, address+11, 2);
    memcpy(&info->minor, address+13, 2);
    return 0;
}

int displayRomInfo(struct romInfo *info)
{
    const char *kversion;
    const char *size;
    switch(info->major)
    {
        case 30:
            kversion = "Kickstart 1.0";
            break;
        case 31:
            kversion = "Kickstart 1.1";
            break;
        case 33:
            kversion = "Kickstart 1.2";
            break;
        case 36:
            if (info->minor < 28)
            {
                kversion = "Kickstart 1.4";
            } else {
                kversion = "Kickstart 2.0";
            }
            break;
        case 37:
            kversion = "Kickstart 2.04";
            break;
        case 39:
            kversion = "Kickstart 3.0";
            break;
        case 40:
            kversion = "Kickstart 3.1";
            break;
        case 45:
            kversion = "Kickstart 3.x";
            break;
        case 46:
            kversion = "Kickstart 3.1.2";
            break;
        default:
            kversion = "Unknown";
            break;
    }
    switch (info->id)
    {
        case ROM_TYPE_256:
            size = "256KB";
            break;
        case ROM_TYPE_512:
            size = "512KB";
            break;
        default:
            size = "";
            break;
    }

    printf("%s (%hu.%hu) %s\n", kversion, info->major, info->minor, size);
}
