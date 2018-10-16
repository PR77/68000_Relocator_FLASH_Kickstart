#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// TODO: Add missing headers once I figure out this stupic enum issue!

/*****************************************************************************/
/* Defines *******************************************************************/
/*****************************************************************************/

#define SECTOR_SIZE		        0xFFFF
#define MAX_SECTORS             8

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
void main(int argc, char **argv);

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
    UWORD loopDelay, previousWord, toggleStatus;
    
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
    
    *(UWORD *)(address + FLASH_UNLOCK_ADDR_1) = FLASH_UNLOCK_DATA_1;
    *(UWORD *)(address + FLASH_UNLOCK_ADDR_2) = FLASH_UNLOCK_DATA_2;

    flashCommandStatus = flashOK;
    
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
    
    flashCommandStatus = unlockFlashDevice(address);
    
    *(UWORD *)(address + FLASH_AUTOSEL_ADDR_1) = FLASH_AUTOSEL_DATA_1;    
    *pManufactureID = *(UWORD *)(address + FLASH_MANUFACTOR_ID);
    *(UWORD *)(address + FLASH_RESET_ADDR_1) = FLASH_RESET_DATA_1;    
    
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
    
    flashCommandStatus = unlockFlashDevice(address);
    
    *(UWORD *)(address + FLASH_AUTOSEL_ADDR_1) = FLASH_AUTOSEL_DATA_1;    
    *pDeviceID = *(UWORD *)(address + FLASH_DEVICE_ID);
    *(UWORD *)(address + FLASH_RESET_ADDR_1) = FLASH_RESET_DATA_1;    
    
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
    
    *(UWORD *)(address + FLASH_RESET_ADDR_1) = FLASH_RESET_DATA_1;    

    flashCommandStatus = flashOK;

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
        if (flashOK == unlockFlashDevice())
        {
            *(ULONG *)(address + FLASH_ERASE_ADDR_1) = FLASH_ERASE_DATA_1;    
            
            if (flashOK == unlockFlashDevice())
            {
                *(ULONG *)(address + (sectorNumber << 16) = FLASH_SECTOR_DATA_1;

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
    
    *(ULONG *)(address + FLASH_PROGRAM_ADDR_1) = FLASH_PROGRAM_DATA_1;    
    *(ULONG *)(address) = data;
    
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

        if (flashOK == writeFlashByte(pData[currentWord]))
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
                
    } while (currentWord < size)
    
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Main **********************************************************************/
/*****************************************************************************/
void main(int argc, char **argv)
{
    struct Library *ExpansionBase = NULL;
    struct ConfigDev *myCD = NULL;
    BPTR fileHandle = NULL;
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
    myCD = FindConfigDev(myCD, 1977, 104);

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
        printf("cd_BoardAddr = %lx\n", myCD->cd_BoardAddr);
        printf("cd_BoardSize = %lx (%ldK)\n", myCD->cd_BoardSize,((ULONG)myCD->cd_BoardSize)/1024);
        printf("Identyfing FLASH Devices:\n");
        
        /* Print out some details now about the Flash Chips - Manufacturer ID */
        if (flashOK == readManufactureID(myCD->cd_BoardAddr, &flashManufactureID))
        {
            printf("Manufacturing ID: Hign Device %1x, Low Device %1x\n", ((flashManufactureID & 0xFF00) >> 8), (flashManufactureID & 0xFF));
        }
        else
        {
            printf("Failed to identify FLASH Manufacturer ID\n");
            CloseLibrary(ExpansionBase);
            exit(RETURN_FAIL);            
        }
        
        /* Print out some details now about the Flash Chips - Device ID */
        if (flashOK == readDeviceID(myCD->cd_BoardAddr, &flashDeviceID))
        {
            printf("Device ID: Hign Device %1x, Low Device %1x\n", ((flashDeviceID & 0xFF00) >> 8), (flashDeviceID & 0xFF));
        }
        else
        {
            printf("Failed to identify FLASH Device ID\n");
            CloseLibrary(ExpansionBase);
            exit(RETURN_FAIL);            
        }
    }
    
    fileHandle = Open(argv[1], MODE_OLDFILE);
    
    if (NULL != fileHandle)
    {      
        // TODO: read kickstart image
        // TODO: erase flash() with progress indicator
        // TODO: write flash() with progress indicator
        // TODO: close fileHandle
        // TODO: print nice success message       
    }
    else
    {
        print("Could not open specific Kickstart image [%s]", argv[1]);
    }

    CloseLibrary(ExpansionBase);
}
