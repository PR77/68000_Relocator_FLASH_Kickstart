#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*****************************************************************************/
/* Defines *******************************************************************/
/*****************************************************************************/

#define SECTOR_SIZE		        0xFFFF
#define MAX_SECTORS             8

#define TOGGLE_STATUS           0x8080
#define EXCEEDED_TIME           0x2020

#define FLASH_RESET_ADDR_1      (0x0000 << 1)
#define FLASH_RESET_DATA_1      (0xF0F0)

#define FLASH_ERASE_ADDR_1      (0x0555 << 1)
#define FLASH_ERASE_DATA_1      (0x8080)
#define FLASH_SECTOR_DATA_1     (0x3030)

#define FLASH_PROGRAM_ADDR_1    (0x0555 << 1)
#define FLASH_PROGRAM_DATA_1    (0xA0A0)

#define FLASH_UNLOCK_ADDR_1     (0x0555 << 1)
#define FLASH_UNLOCK_DATA_1     (0xAAAA)
#define FLASH_UNLOCK_ADDR_2     (0x02AA << 1)
#define FLASH_UNLOCK_DATA_2     (0x5555)

#define FLASH_AUTOSEL_ADDR_1    (0x0555 << 1)
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
/* Globals *******************************************************************/
/*****************************************************************************/

struct Library *ExpansionBase = NULL;

/*****************************************************************************/
/* Prototypes ****************************************************************/
/*****************************************************************************/

tFlashCommandStatus checkFlashStatus(ULONG address);
tFlashCommandStatus unlockFlashDevice(ULONG address);
tFlashCommandStatus readManufactureID(ULONG address, UWORD * pManufactureID);
tFlashCommandStatus readDeviceID(ULONG address, UWORD * pDeviceID);
tFlashCommandStatus resetFlashDevice(ULONG address);
tFlashCommandStatus eraseFlashSector(ULONG address, UBYTE sectorNumber);
tFlashCommandStatus writeFlashByte(ULONG address, UWORD data);
tFlashCommandStatus programFlash(ULONG address, ULONG size, UWORD * pData);
int main(int argc, char **argv);

/*****************************************************************************/
/* Code **********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Function:    checkFlashStatus()                                           */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address                                                */
/* Description: Polls Flash device for completion or timeout status          */
/*****************************************************************************/
tFlashCommandStatus checkFlashStatus(ULONG address)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;
    UWORD currentWord, previousWord;
    UBYTE loopDelay;
    
    currentWord = *(UWORD *)address;

    do {
        loopDelay++;
    } while (loopDelay < 100);
    
    flashCommandStatus = flashBusy;

    do {
        
        previousWord = currentWord;
        currentWord = *(UWORD *)address;
        
        if (0 != (previousWord ^ currentWord) & TOGGLE_STATUS)
        {
            if (currentWord & EXCEEDED_TIME)
                flashCommandStatus = flashStatusError;
        }
        else
        {
            flashCommandStatus = flashOK;
        }

    } while ((flashBusy == flashCommandStatus) || (flashStatusError == flashCommandStatus));
    
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    unlockFlashDevice()                                          */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address                                                */
/* Description: Sends initial unlock command sequence to flash device        */
/*****************************************************************************/
tFlashCommandStatus unlockFlashDevice(ULONG address)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: unlockFlashDevice(ULONG address 0x%X)\n", address);
#endif
    
    *(UWORD *)(address + FLASH_UNLOCK_ADDR_1) = FLASH_UNLOCK_DATA_1;
#ifndef NDEBUG
    printf("FLOW: FLASH_UNLOCK_ADDR_1 0x%X, FLASH_UNLOCK_DATA_1 0x%X\n", (address + FLASH_UNLOCK_ADDR_1), FLASH_UNLOCK_DATA_1);
#endif
    *(UWORD *)(address + FLASH_UNLOCK_ADDR_2) = FLASH_UNLOCK_DATA_2;
#ifndef NDEBUG
    printf("FLOW: FLASH_UNLOCK_ADDR_2 0x%X, FLASH_UNLOCK_DATA_2 0x%X\n", (address + FLASH_UNLOCK_ADDR_2), FLASH_UNLOCK_DATA_2);
#endif

    flashCommandStatus = flashOK;
    
#ifndef NDEBUG
    printf("EXIT: unlockFlashDevice(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);    
}

/*****************************************************************************/
/* Function:    readManufactureID()                                          */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, UWORD * pManufactureID                        */
/* Description: Reads manufacturer ID from flash device                      */
/*****************************************************************************/
tFlashCommandStatus readManufactureID(ULONG address, UWORD * pManufactureID)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: readManufactureID(ULONG address 0x%X, UWORD * pManufactureID 0x%X)\n", address, pManufactureID);
#endif
    
    flashCommandStatus = unlockFlashDevice(address);
    
    *(UWORD *)(address + FLASH_AUTOSEL_ADDR_1) = FLASH_AUTOSEL_DATA_1;    
#ifndef NDEBUG
    printf("FLOW: FLASH_AUTOSEL_ADDR_1 0x%X, FLASH_AUTOSEL_DATA_1 0x%X\n", (address + FLASH_AUTOSEL_ADDR_1), FLASH_AUTOSEL_DATA_1);
#endif
    *pManufactureID = *(UWORD *)(address + FLASH_MANUFACTOR_ID);
#ifndef NDEBUG
    printf("FLOW: FLASH_MANUFACTOR_ID 0x%X, *pManufactureID 0x%X\n", (address + FLASH_MANUFACTOR_ID), *pManufactureID);
#endif
    flashCommandStatus = resetFlashDevice(address);

#ifndef NDEBUG
    printf("EXIT: readManufactureID(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    readDeviceID()                                               */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, UWORD * pDeviceID                             */
/* Description: Reads device ID from flash device                            */
/*****************************************************************************/
tFlashCommandStatus readDeviceID(ULONG address, UWORD * pDeviceID)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: readDeviceID(ULONG address 0x%X, UWORD * pManufactureID 0x%X)\n", address, pDeviceID);
#endif
    
    flashCommandStatus = unlockFlashDevice(address);
    
    *(UWORD *)(address + FLASH_AUTOSEL_ADDR_1) = FLASH_AUTOSEL_DATA_1;    
#ifndef NDEBUG
    printf("FLOW: FLASH_AUTOSEL_ADDR_1 0x%X, FLASH_AUTOSEL_DATA_1 0x%X\n", (address + FLASH_AUTOSEL_ADDR_1), FLASH_AUTOSEL_DATA_1);
#endif
    *pDeviceID = *(UWORD *)(address + FLASH_DEVICE_ID);
#ifndef NDEBUG
    printf("FLOW: FLASH_DEVICE_ID 0x%X, *pDeviceID 0x%X\n", (address + FLASH_DEVICE_ID), *pDeviceID);
#endif
    flashCommandStatus = resetFlashDevice(address);
    
#ifndef NDEBUG
    printf("EXIT: readDeviceID(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    resetFlashDevice()                                           */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address                                                */
/* Description: Sends reset command sequence to flash device                 */
/*****************************************************************************/
tFlashCommandStatus resetFlashDevice(ULONG address)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: resetFlashDevice(ULONG address 0x%X)\n", address);
#endif
    
    *(UWORD *)(address + FLASH_RESET_ADDR_1) = FLASH_RESET_DATA_1;    

    flashCommandStatus = flashOK;

#ifndef NDEBUG
    printf("EXIT: resetFlashDevice(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    eraseFlashSector()                                           */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, UBYTE sectorNumber                            */
/* Description: Erases a specific sector number from flash device            */
/*****************************************************************************/
tFlashCommandStatus eraseFlashSector(ULONG address, UBYTE sectorNumber)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

    if (sectorNumber < MAX_SECTORS)
    {
        if (flashOK == unlockFlashDevice(address + (SECTOR_SIZE * sectorNumber)))
        {
            *(ULONG *)(address + FLASH_ERASE_ADDR_1) = FLASH_ERASE_DATA_1;    
            
            if (flashOK == unlockFlashDevice(address + (SECTOR_SIZE * sectorNumber)))
            {
                *(ULONG *)(address + (sectorNumber << 16)) = FLASH_SECTOR_DATA_1;

                flashCommandStatus = flashOK;                
            }
        }
    }
    else
    {
        flashCommandStatus = flashEraseError;
    }
    
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    writeFlashByte()                                             */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, UWORD data                                    */
/* Description: Writes a specific word to the flash device                   */
/*****************************************************************************/
tFlashCommandStatus writeFlashByte(ULONG address, UWORD data)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

    flashCommandStatus = unlockFlashDevice(address);
    
    *(UWORD *)(address + FLASH_PROGRAM_ADDR_1) = FLASH_PROGRAM_DATA_1;    
    *(UWORD *)(address) = data;
    
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    programFlash()                                               */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, ULONG size, UWORD * pData                     */
/* Description: Writes a specific array of data the flash device             */
/*****************************************************************************/
tFlashCommandStatus programFlash(ULONG address, ULONG size, UWORD * pData)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;
    ULONG currentWord = 0;
    
    do {

        if (flashOK == writeFlashByte((address + currentWord), pData[currentWord]))
        {
            if (flashOK == checkFlashStatus(address + currentWord))
            {
                currentWord++;
                currentWord++;
                continue;
            }
        }
        
        resetFlashDevice(address);
        flashCommandStatus = flashProgramError;
        break;            
                
    } while (currentWord < size);
    
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Main **********************************************************************/
/*****************************************************************************/
int main(int argc, char **argv)
{
    struct ConfigDev *myCD = NULL;
    BPTR fileHandle = 0L;
    UWORD flashManufactureID, flashDeviceID;

    /* Check if application has been started with correct parameters */
    if (argc != 2)
    {
        printf("usage: FlashKickstart <filename>\n");
        exit(RETURN_FAIL);
    }
    
    /* Open any version expansion.library to read in ConfigDevs */
    ExpansionBase = OpenLibrary("expansion.library", 0L);
    
    /* Check if opened correctly, otherwise exit with message and error */
    if (NULL == ExpansionBase)
    {
        printf("Failed to open expansion.library\n");
        exit(RETURN_FAIL);
    }
 
    /*----------------------------------------------------*/
    /* FindConfigDev(oldConfigDev, manufacturer, product) */
    /* oldConfigDev = NULL for the top of the list        */
    /* manufacturer = -1 for any manufacturer             */
    /* product      = -1 for any product                  */
    /*----------------------------------------------------*/
  
    /* Check if correct Zorro II hardware is present. FLASH Kickstart */
    myCD = FindConfigDev(0L, 1977, 104);

    /* Check if opened correctly, otherwise exit with message and error */
    if (NULL == myCD)
    {
        printf("Failed to identify FLASH Kickstart Hardware\n");
        CloseLibrary(ExpansionBase);
        exit(RETURN_FAIL);
    }
    else
    /* Opened correctly, so print out the configuration details */
    {
        printf("FLASH Kickstart Hardware identified with configuration:\n");
        printf("cd_BoardAddr = 0x%X\n", myCD->cd_BoardAddr);
        printf("cd_BoardSize = 0x%X (%ldK)\n", myCD->cd_BoardSize,((ULONG)myCD->cd_BoardSize)/1024);
        printf("Identyfing FLASH Devices:\n");
        
        /* Print out some details now about the Flash Chips - Manufacturer ID */
        if (flashOK == readManufactureID((ULONG)myCD->cd_BoardAddr, &flashManufactureID))
        {
            printf("Manufacturing ID: Hign Device 0x%X, Low Device 0x%X\n", ((flashManufactureID & 0xFF00) >> 8), (flashManufactureID & 0xFF));
        }
        else
        {
            printf("Failed to identify FLASH Manufacturer ID\n");
            CloseLibrary(ExpansionBase);
            exit(RETURN_FAIL);            
        }
        
        /* Print out some details now about the Flash Chips - Device ID */
        if (flashOK == readDeviceID((ULONG)myCD->cd_BoardAddr, &flashDeviceID))
        {
            printf("Device ID: Hign Device 0x%X, Low Device 0x%X\n", ((flashDeviceID & 0xFF00) >> 8), (flashDeviceID & 0xFF));
        }
        else
        {
            printf("Failed to identify FLASH Device ID\n");
            CloseLibrary(ExpansionBase);
            exit(RETURN_FAIL);            
        }
    }
    
    fileHandle = Open(argv[1], MODE_OLDFILE);
    
    if (0L != fileHandle)
    {      
        // TODO: read kickstart image
        // TODO: erase flash() with progress indicator
        // TODO: write flash() with progress indicator
        // TODO: close fileHandle
        // TODO: print nice success message       
    }
    else
    {
        printf("Could not open specific Kickstart image [%s]\n", argv[1]);
    }

    CloseLibrary(ExpansionBase);
}
