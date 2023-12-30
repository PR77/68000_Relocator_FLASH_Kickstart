/*
    This file is part of FLASH_KICKSTART originally designed by
    Paul Raspa.

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

#ifndef __FLASH_DEVICE_SST39__
#define __FLASH_DEVICE_SST39__

/*****************************************************************************/
/* Macros ********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Defines *******************************************************************/
/*****************************************************************************/

#define SECTOR_SIZE		        0x1000 // 4KB
#define MAX_SECTORS             128    // 128 sectors for 040 flash

#define TOGGLE_STATUS           0x4040

#define FLASH_RESET_ADDR_1      (0x0000 << 1)
#define FLASH_RESET_DATA_1      (0xF0F0)

#define FLASH_ERASE_ADDR_1      (0x5555 << 1)
#define FLASH_ERASE_DATA_1      (0x8080)
#define FLASH_SECTOR_DATA_1     (0x3030)
#define FLASH_COMPLETE_DATA_1   (0x1010)

#define FLASH_PROGRAM_ADDR_1    (0x5555 << 1)
#define FLASH_PROGRAM_DATA_1    (0xA0A0)

#define FLASH_UNLOCK_ADDR_1     (0x5555 << 1)
#define FLASH_UNLOCK_DATA_1     (0xAAAA)
#define FLASH_UNLOCK_ADDR_2     (0x2AAA << 1)
#define FLASH_UNLOCK_DATA_2     (0x5555)

#define FLASH_AUTOSEL_ADDR_1    (0x5555 << 1)
#define FLASH_AUTOSEL_DATA_1    (0x9090)

#define FLASH_MANUFACTOR_ID     (0x0000 << 1)
#define FLASH_DEVICE_ID         (0x0001 << 1)

#define EXPECTED_MANUFAC_ID     (UWORD)0xBFBF
#define EXPECTED_DEVICE_ID      (UWORD)0xB6B6

/*****************************************************************************/
/* Types *********************************************************************/
/*****************************************************************************/

typedef enum {

    flashIdle = 0,
    flashBusy,
    flashStatusError,
    flashEraseError,
    flashProgramError,
    flashProgramRetry,
    flashProgramTimeout,
    flashOK

} tFlashCommandStatus;

/*****************************************************************************/
/* Prototypes ****************************************************************/
/*****************************************************************************/

extern tFlashCommandStatus checkFlashStatus(ULONG address);
extern tFlashCommandStatus unlockFlashDevice(ULONG address);
extern tFlashCommandStatus readManufactureID(ULONG address, UWORD * pManufactureID);
extern tFlashCommandStatus readDeviceID(ULONG address, UWORD * pDeviceID);
extern tFlashCommandStatus resetFlashDevice(ULONG address);
extern tFlashCommandStatus eraseFlashSector(ULONG address, ULONG sectorNumber);
extern tFlashCommandStatus eraseCompleteFlash(ULONG address);
extern tFlashCommandStatus writeFlashWord(ULONG baseAddress, ULONG writeAddress, UWORD data);

#endif /* __FLASH_DEVICE_SST39__ */
