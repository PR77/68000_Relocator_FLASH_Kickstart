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
#include <stdlib.h>
#include <stdbool.h>

#include "RomInfo.h"

static void getDiagRom(UBYTE *address, struct romInfo *info)
{
    UBYTE *ptr = address + 0xC8;
    UBYTE data = *ptr;
    char *endptr = NULL;
    if (data != 0x56) // V
    {
        return;
    }
    ptr++;
    info->major = strtoul((const char*)ptr, &endptr, 10);
    if (!endptr)
    {
        return;
    }
    data = *endptr;
    if (data != '.')
    {
        info->minor = 0;
        return;
    }
    endptr++;
    info->minor = strtoul(endptr, &endptr, 10);
    if (!endptr)
    {
        return;
    }
    info->isDiagRom = true;
    data = *endptr;
    if (data != '.')
    {
        info->extra = 0;
        return;
    }
    endptr++;
    info->extra = strtoul(endptr, NULL, 10);
}

int getRomInfo(UBYTE *address, struct romInfo *info)
{
    UBYTE *ptr = address;
    UBYTE data = *ptr;
    info->isDiagRom = false;

    if (data == 0xff)
    {
        address+= 256*1024;
        ptr = address;
        data = *ptr;
        // We would only hit here with Flash ROMs
        if (data == 0xff)
        {
            info->id = ROM_TYPE_BLANK;
            return 0;
        }
    }
    if (data != 0x11)
    {
        info->id = ROM_TYPE_UNKNOWN;

        return ERR_NOT_ROM;
    }
    ptr++;
    data = *ptr;
    switch (data)
    {
        case 0x11:
            info->id = ROM_TYPE_256;
            break;
        case 0x14:
        case 0x16: // kick40063.A600
            info->id = ROM_TYPE_512;
            break;
        default:
            info->id = ROM_TYPE_UNKNOWN;
            return ERR_ROM_UNKNOWN;
            break;
    }
    ptr++;
    data = *ptr;
    if (data != 0x4E) // 256K byte swapped
    {
        return ERR_NOT_ROM;
    }
    memcpy(&info->major, address+12, 2);
    memcpy(&info->minor, address+14, 2);
    // We hit part of a memory ptr for DiagROM, it will be > 200
    if (info->major > 100)
    {
        getDiagRom(address, info);
    }
    return 0;
}

void displayRomInfo(struct romInfo *info, char **output)
{
    const char *kversion;
    const char *size;

    if (info->id == ROM_TYPE_BLANK)
    {
        if (output)
        {
            char *ret = strdup("Erased");
            *output = ret;
        }
        else
        {
            printf("Erased\n");
        }
        return;
    }
    if (!info->isDiagRom)
    {
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
            case 34:
                kversion = "Kickstart 1.3";
                break;
            case 35:
                kversion = "Kickstart 1.4";
                break;
            case 36:
                kversion = "Kickstart 2.0";
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
                kversion = "Kickstart 3.1.4";
                break;
            default:
                kversion = "Unknown";
                break;
        }
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

    if (output)
    {
        char *ret = malloc(64 * sizeof(char));
        *output = ret;
        if (info->isDiagRom)
        {
            snprintf(ret, 64, "DiagRom V%hu.%hu.%hu %s", info->major, info->minor, info->extra, size);
        }
        else
        {
            snprintf(ret, 64, "%s (%hu.%hu) %s", kversion, info->major, info->minor, size);
        }
    }
    else
    {
        if (info->isDiagRom)
        {
            printf("DiagRom V%hu.%hu.%hu %s\n", info->major, info->minor, info->extra, size);
        }
        else
        {
            printf("%s (%hu.%hu) %s\n", kversion, info->major, info->minor, size);
        }
    }
}
