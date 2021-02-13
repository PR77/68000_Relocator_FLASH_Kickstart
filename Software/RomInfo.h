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

#ifndef __ROMINFO__
#define __ROMINFO__

#include <stdbool.h>

enum romType {
    ROM_TYPE_UNKNOWN,
    ROM_TYPE_256,
    ROM_TYPE_512,
    ROM_TYPE_BLANK
};

struct romInfo {
    ULONG id;
    UWORD major;
    UWORD minor;
    UWORD extra;
    bool isDiagRom;
};

enum romErrCode {
    ERR_NO_ERR,
    ERR_NOT_ROM,
    ERR_ROM_UNKNOWN
};

int getRomInfo(UBYTE *address, struct romInfo *info);
void displayRomInfo(struct romInfo *info, char **output);
#endif /* __ROMINFO */
