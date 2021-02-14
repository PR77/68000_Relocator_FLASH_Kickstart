#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/expansion_protos.h>

#include <proto/dos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlashDeviceSST39.h"
#include "Helpers.h"

/*****************************************************************************/
/* Macros ********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Defines *******************************************************************/
/*****************************************************************************/

#define LOOP_TIMEOUT        (ULONG)10000
#define KICKSTART_256K      (ULONG)(256 * 1024)

/*****************************************************************************/
/* Types *********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Globals *******************************************************************/
/*****************************************************************************/

struct Library *ExpansionBase = NULL;
struct Library *IntuitionBase = NULL;

char programAlertMsg[] = "\x00\xC0\x14 ABOUT TO FLASH KICKSTART CHIPS \x00\x01" \
"\x00\x80\x24 PRESS MOUSEBUTTON:   LEFT=PROCEED   RIGHT=EXIT \x00";
                  
char eraseAlertMsg[] = "\x00\xC0\x14 ABOUT TO ERASE KICKSTART CHIPS \x00\x01" \
"\x00\x80\x24 PRESS MOUSEBUTTON:   LEFT=PROCEED   RIGHT=EXIT \x00";

/*****************************************************************************/
/* Prototypes ****************************************************************/
/*****************************************************************************/

int main(int argc, char **argv);
tFlashCommandStatus programFlashLoop(ULONG fileSize, ULONG baseAddress, APTR * pBuffer);

/*****************************************************************************/
/* Local Code ****************************************************************/
/*****************************************************************************/
tFlashCommandStatus programFlashLoop(ULONG fileSize, ULONG baseAddress, APTR * pBuffer)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;
    ULONG currentWordIndex = 0;
    ULONG breakCount = 0;
    ULONG errorCount = 0;
    
    do {
        writeFlashWord(baseAddress, currentWordIndex << 1, ((UWORD *)pBuffer)[currentWordIndex]);

        breakCount = 0;
        
        do {
            flashCommandStatus = checkFlashStatus(baseAddress + (currentWordIndex << 1));
            breakCount++;

        } while ((flashCommandStatus != flashOK) && (breakCount < LOOP_TIMEOUT));
        
        if (((UWORD *)pBuffer)[currentWordIndex] != (((UWORD *)baseAddress)[currentWordIndex]))
        {
            errorCount++;
            printf("Failed at ADDR: 0x%06X, SRC: 0x%04X, DEST 0x%04X\n", (currentWordIndex << 1), ((UWORD *)pBuffer)[currentWordIndex], ((UWORD *)baseAddress)[currentWordIndex]);
        }
        
        if (breakCount == LOOP_TIMEOUT)
        {
            flashCommandStatus = flashProgramTimeout
            break;
        }
                                                                              
        currentWordIndex += 1;
    } while (currentWordIndex < (fileSize >> 1));
    
    if (errorCount)
    {
        flashCommandStatus = flashProgramRetry;
    }

    return (flashCommandStatus);
}

/*****************************************************************************/
/* Main Code *****************************************************************/
/*****************************************************************************/
int main(int argc, char **argv)
{
    struct ConfigDev *myCD = NULL;
    struct FileInfoBlock myFIB;
    BPTR fileHandle = 0L;

    printf("\nFlashKickstart Tool For FLASH_KICKSTART");
    printf("\nFor details refer to - https://github.com/PR77/68000_Relocator_FLASH_Kickstart\n");
    printf("Developed By: Paul Raspa (PR77), Revision 1.0.1, 2021.02.14\n");

    /* Check if application has been started with correct parameters */
    if (argc <= 1)
    {
        printf("usage: FlashKickstart <option> <filename>\n");
        printf(" -i\tFLASH CHIP INFO\n");
        printf(" -e\tERASE\n");
        printf(" -d\tDUMP <start address [default F80000]> <length [default 64]>\n");
        printf(" -p\tPROGRAM <filename>\n");
        printf(" -t\tTEST <cyclically read FLASH CHIP INFO until interrupted>\n");
        exit(RETURN_FAIL);
    }
    
    /* Open any version intuition.library to support displayAlert */
    IntuitionBase = OpenLibrary("intuition.library", 0);
    
    /* Check if opened correctly, otherwise exit with message and error */
    if (NULL == IntuitionBase)
    {
        printf("Failed to open intuition.library\n");
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
        CloseLibrary(IntuitionBase);
        exit(RETURN_FAIL);
    }
    else
    /* Opened correctly, so print out the configuration details */
    {
        printf("FLASH Kickstart Hardware identified with configuration:\n");
        printf("cd_BoardAddr = 0x%06X\n", myCD->cd_BoardAddr);
        printf("cd_BoardSize = 0x%06X (%ldK)\n", myCD->cd_BoardSize,((ULONG)myCD->cd_BoardSize)/1024);
    }
    
    while ((argc > 1) && (argv[1][0] == '-'))
    {
        switch (argv[1][1])
        {
            case 'i':
            {
                UWORD flashManufactureID, flashDeviceID;
                
                if (flashOK == readManufactureID((ULONG)myCD->cd_BoardAddr, &flashManufactureID))
                {
                    printf("Manufacturing ID: High Device 0x%02X, Low Device 0x%02X\n", ((flashManufactureID & 0xFF00) >> 8), (flashManufactureID & 0xFF));
                }
                else
                {
                    printf("Failed to identify FLASH Manufacturer ID\n");
                }

                if (flashOK == readDeviceID((ULONG)myCD->cd_BoardAddr, &flashDeviceID))
                {
                    printf("Device ID: High Device 0x%02X, Low Device 0x%02X\n", ((flashDeviceID & 0xFF00) >> 8), (flashDeviceID & 0xFF));
                }
                else
                {
                    printf("Failed to identify FLASH Device ID\n");
                }
            }
            break;
            
            case 'e':
            {
                if (!DisplayAlert(RECOVERY_ALERT, eraseAlertMsg, 52))
                    break;
                
                if (flashOK == eraseCompleteFlash((ULONG)myCD->cd_BoardAddr))
                {
                    tFlashCommandStatus flashCommandStatus;
                    ULONG breakCount = 0;
                    
                    do {
                        flashCommandStatus = checkFlashStatus((ULONG)myCD->cd_BoardAddr);
                        breakCount++;
                        
                    } while ((flashCommandStatus != flashOK) && (breakCount < LOOP_TIMEOUT));
                                    
                    if (flashOK == flashCommandStatus)
                    {
                        printf("FLASH device erased OK\n");
                    }
                    else
                    {
                        printf("FLASH device erased ERROR\n");
                    }
                }
                else
                {
                    printf("FLASH device erased COMMAND NOT ACCEPTED\n");
                }
            }
            break;
            
            case 'd':
            {
                char *nextParam;
                ULONG startAddress = 0xF80000;
                UWORD length = 64;

                if ((argc > 2) && argv[2])
                {
                    startAddress = strtoul(argv[2], &nextParam, 16);
                }
                
                if ((argc > 3) && argv[3])
                {
                    length = strtoul(argv[3], &nextParam, 16);
                }
                
                hexDump("Memory DUMP", (APTR)startAddress, length);                
            }
            break;
            
            case 'p':
            {
                if ((argc > 2) && argv[2])
                {
                    APTR pBuffer;
                    ULONG fileSize;
                    
                    tReadFileHandler readFileProgram = getFileSize(argv[2], &fileSize);
                    
                    if (readFileOK == readFileProgram)
                    {
                        readFileProgram = readFileIntoMemoryHandler(argv[2], fileSize, &pBuffer);
                        
                        if (readFileOK == readFileProgram)
                        {
                            tFlashCommandStatus programFlashStatus = flashIdle;
                            ULONG baseAddress = (fileSize == KICKSTART_256K) ? ((ULONG)myCD->cd_BoardAddr + KICKSTART_256K) : (ULONG)myCD->cd_BoardAddr
                            
                            do {
                                programFlashStatus = programFlashLoop(fileSize, baseAddress, pBuffer);
                            
                            } while (programFlashStatus == flashProgramRetry);
                            
                            if (programFlashStatus == flashProgramTimeout)
                            {
                                printf("Failed to program kickstart image: Flash Program Timeout\n");
                            }

                            freeFileHandler(fileSize);                            
                        }
                        else if (readFileNotFound == readFileProgram)
                        {
                            printf("Failed to open kickstart image: %s\n", argv[2]);
                        }
                        else if (readFileNoMemoryAllocated == readFileProgram)
                        {
                            printf("Failed to allocate memory for file: %s\n", argv[2]);
                        }
                        else if (readFileGeneralError == readFileProgram)
                        {
                            printf("Failed to read into memory file: %s\n\n");    
                        }
                        else
                        {
                            printf("Unhandled error in readFileIntoMemoryHandler()\n");    
                        }
                    }
                    else
                    {
                        printf("Unable to determine file size of %s\n", argv[2]);
                    }
                }
                else
                {
                    printf("No Kickstart image specified\n");
                }
            }
            break;
            
            case 't':
            {
                UWORD flashManufactureID, flashDeviceID;
                                
                do {
                    
                    if (flashOK != readManufactureID((ULONG)myCD->cd_BoardAddr, &flashManufactureID))
                        break;
                    
                    if (flashOK != readDeviceID((ULONG)myCD->cd_BoardAddr, &flashDeviceID))
                        break;
                    
                    if (flashManufactureID != EXPECTED_MANUFAC_ID)
                        printf("Manufacturing ID Error, Expected 0x%04X, Got 0x%04X\n", EXPECTED_MANUFAC_ID, flashManufactureID);
                    
                    if (flashDeviceID != EXPECTED_DEVICE_ID)
                        printf("Device ID Error, Expected 0x%04X, Got 0x%04X\n", EXPECTED_DEVICE_ID, flashDeviceID);
                } while (1);
            }
            break;
        }
        
        ++argv;
        --argc;
    }
          
    CloseLibrary(IntuitionBase);
    CloseLibrary(ExpansionBase);
}
