#ifndef __FLASH_DEVICE_SST39__
#define __FLASH_DEVICE_SST39__

/*****************************************************************************/
/* Macros ********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Defines *******************************************************************/
/*****************************************************************************/

#define SECTOR_SIZE		        0x10000
#define MAX_SECTORS             8

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

/*****************************************************************************/
/* Types *********************************************************************/
/*****************************************************************************/

typedef enum {
    
    flashIdle = 0,
    flashBusy,
    flashStatusError,
    flashEraseError,
    flashProgramError,
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
extern tFlashCommandStatus eraseFlashSector(ULONG address, UBYTE sectorNumber);
extern tFlashCommandStatus eraseCompleteFlash(ULONG address);
extern tFlashCommandStatus writeFlashWord(ULONG baseAddress, ULONG writeAddress, UWORD data);

#endif /* __FLASH_DEVICE_SST39__ */