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

enum romType {
    ROM_TYPE_UNKNOWN,
    ROM_TYPE_256,
    ROM_TYPE_512,
};

struct romInfo {
    ULONG id;
    WORD major;
    WORD minor;
};

enum romErrCode {
    ERR_NOT_ROM
};

int getRomInfo(BYTE *address, struct romInfo *info);
int displayRomInfo(struct romInfo *info);
#endif /* __ROMINFO */
