#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/expansion_protos.h>

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

#define LOOP_TIMEOUT    (ULONG)10000

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

char progressIndicator[] = "-\\|//";

/*****************************************************************************/
/* Prototypes ****************************************************************/
/*****************************************************************************/

int main(int argc, char **argv);

/*****************************************************************************/
/* Code **********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Main **********************************************************************/
/*****************************************************************************/
int main(int argc, char **argv)
{
    struct ConfigDev *myCD = NULL;
    struct FileInfoBlock myFIB;
    BPTR fileHandle = 0L;

    /* Check if application has been started with correct parameters */
    if (argc <= 1)
    {
        printf("usage: FlashKickstart <option> <filename>\n");
        printf(" -i\tFLASH CHIP INFO\n");
        printf(" -k\tKICKSTART IMAGE INFO\n");
        printf(" -e\tERASE\n");
        printf(" -d\tDUMP <start address [default F80000]> <length [default 64]>\n");
        printf(" -p\tPROGRAM <filename>\n");
        printf(" -v\tVERIFY <filename>\n")
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
        printf("cd_BoardAddr = 0x%X\n", myCD->cd_BoardAddr);
        printf("cd_BoardSize = 0x%X (%ldK)\n", myCD->cd_BoardSize,((ULONG)myCD->cd_BoardSize)/1024);
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
                    printf("Manufacturing ID: High Device 0x%X, Low Device 0x%X\n", ((flashManufactureID & 0xFF00) >> 8), (flashManufactureID & 0xFF));
                }
                else
                {
                    printf("Failed to identify FLASH Manufacturer ID\n");
                }

                if (flashOK == readDeviceID((ULONG)myCD->cd_BoardAddr, &flashDeviceID))
                {
                    printf("Device ID: High Device 0x%X, Low Device 0x%X\n", ((flashDeviceID & 0xFF00) >> 8), (flashDeviceID & 0xFF));
                }
                else
                {
                    printf("Failed to identify FLASH Device ID\n");
                }
            }
            break;
            
            case 'k':
            {
                if ((argc > 2) && argv[2])
                {
                    ULONG fileSize;
                    tReadFileHandler readFileDetails;

                    readFileDetails = getFileSize(argv[2], &fileSize);
                    
                    if (readFileOK == readFileDetails)
                    {
                        printf("Kickstart image file <%s>, size <%d bytes>\n", argv[2], fileSize)
                    }
                    else if (readFileNotFound == readFileDetails)
                    {
                        printf("Failed to open kickstart image: %s\n", argv[2]);
                    }
                    else if (readFileNoFileSpecified == readFileDetails)
                    {
                        printf("Unhandled error in getFileSiye()\n");    
                    }
                }
                else
                {
                    printf("No Kickstart image specified\n");
                }
            }
            break;
            
            case 'e':
            {
                if (flashOK == eraseCompleteFlash((ULONG)myCD->cd_BoardAddr))
                {
                    tFlashCommandStatus flashCommandStatus;
                    ULONG breakCount = 0;
                    UBYTE progressIndicatorIndex = 0;
                    
                    do {
                        flashCommandStatus = checkFlashStatus((ULONG)myCD->cd_BoardAddr);
                        breakCount++;

                        printf("Erase FLASH ... %c\r", progressIndicator[progressIndicatorIndex++]);
                        
                        if (progressIndicatorIndex >= 4)
                            progressIndicatorIndex = 0;
                        
                    } while ((flashCommandStatus != flashOK) && (breakCount < LOOP_TIMEOUT));
                                    
                    if (flashOK == flashCommandStatus)
                    {
                        printf("\nFLASH device erased OK\n");
                    }
                    else
                    {
                        printf("\nFLASH device erased ERROR\n");
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

#ifndef NDEBUG
                printf("MAIN: DUMP Handler <start address [default F80000]> <length [default 64]>\n");
#endif
                if ((argc > 2) && argv[2])
                {
                    startAddress = strtoul(argv[2], &nextParam, 16);
#ifndef NDEBUG
                    printf("MAIN: DUMP Handler <start address [default F80000] 0x%X>\n", startAddress);
#endif          
                }
                
                if ((argc > 3) && argv[3])
                {
                    length = strtoul(argv[3], &nextParam, 16);
#ifndef NDEBUG
                    printf("MAIN: DUMP Handler <length [default 64] 0x%X>\n", length);
#endif          
                }
                
                hexDump("Memory DUMP", (APTR)startAddress, length);                
            }
            break;
            
            case 'p':
            {
                if ((argc > 2) && argv[2])
                {
                    /* Check file size and other credentials */
                    fileHandle = Lock(argv[2], MODE_OLDFILE);
                    
                    if (0L != fileHandle)
                    {
                        Examine(fileHandle, &myFIB);
#ifndef NDEBUG
                        printf("MAIN: FLASH program Handler: Kickstart image [%s], fileSize %d\n", argv[2], (ULONG)myFIB.fib_Size);
#endif
                        UnLock(fileHandle);   

                        fileHandle = Open(argv[2], MODE_OLDFILE);

                        if (0L != fileHandle)
                        {                        
                            APTR memoryHandle = AllocMem(0x80000, 0);

                            if (memoryHandle)
                            {
                                tFlashCommandStatus flashCommandStatus = flashIdle;
                                ULONG currentWordWritten = 0;
                                ULONG totalBytesReadFromFile = 0;
                                ULONG breakCount = 0;
                                UWORD progressIndicatorUpdateCounter = 0;
                                UBYTE progressIndicatorIndex = 0;
                                
                                totalBytesReadFromFile = Read(fileHandle, memoryHandle, 0x80000);
                                
                                // DisplayAlert(RECOVERY_ALERT, alertMsg, 52);
                                
#ifndef NDEBUG
                                printf("MAIN: FLASH program Handler: Read %d byes\n", totalBytesReadFromFile);
#endif                                                                
                                do {
                                    writeFlashWord((ULONG)myCD->cd_BoardAddr, currentWordWritten << 1, ((UWORD *)memoryHandle)[currentWordWritten]);
                                    
#ifndef NDEBUG
                                    printf("MAIN: FLASH program Handler: Current index: %d, wordToWrite: %x\n", currentWordWritten << 1, ((UWORD *)memoryHandle)[currentWordWritten]);
#endif                                
                                    breakCount = 0;
                                    
                                    do {
                                        flashCommandStatus = checkFlashStatus((ULONG)myCD->cd_BoardAddr + (currentWordWritten << 1));
                                        breakCount++;

                                    } while ((flashCommandStatus != flashOK) && (breakCount < LOOP_TIMEOUT));
                                    
                                    if (breakCount == LOOP_TIMEOUT)
                                    {
                                        FreeMem(memoryHandle, 0x80000);
                                        Close(fileHandle);
                                        printf("Failed to program word %d:%d\n", currentWordWritten, (ULONG)myFIB.fib_Size);
                                        break;
                                    }
                                                                      
                                    if (progressIndicatorUpdateCounter == 0)
                                    {
                                        printf("Programming FLASH ... %c\n", progressIndicator[progressIndicatorIndex++]);
                                        
                                        if (progressIndicatorIndex >= 4)
                                            progressIndicatorIndex = 0;
                                    }

                                    if (progressIndicatorUpdateCounter++ > 0x4000)
                                    /* Move progress indicator after each 32K is written. */
                                        progressIndicatorUpdateCounter = 0;
                                        
                                    currentWordWritten += 1;
                                } while (currentWordWritten < (totalBytesReadFromFile >> 1));

                                FreeMem(memoryHandle, 0x80000);
                                Close(fileHandle);
                                printf("FLASH programmed OK\n");                                   
                            }
                            else
                            {
                                Close(fileHandle);
                                printf("Failed to allocate memory %d bytes required\n", (ULONG)myFIB.fib_Size);                                
                            }
                        }
                    }
                    else
                    {
                        printf("Failed to open kickstart image: %s\n", argv[2]);
                    }
                }
                else
                {
                    printf("No Kickstart image specified\n");
                }
            }
            break;
            
            case 'v':
            {
                if ((argc > 2) && argv[2])
                {
                    /* Check file size and other credentials */
                    fileHandle = Lock(argv[2], MODE_OLDFILE);
                    
                    if (0L != fileHandle)
                    {
                        Examine(fileHandle, &myFIB);
#ifndef NDEBUG
                        printf("MAIN: VERIFY image Handler: Kickstart image [%s], fileSize %d\n", argv[2], (ULONG)myFIB.fib_Size);
#endif
                        UnLock(fileHandle);   

                        fileHandle = Open(argv[2], MODE_OLDFILE);

                        if (0L != fileHandle)
                        {                        
                            APTR memoryHandle = AllocMem(0x80000, 0);

                            if (memoryHandle)
                            {
                                ULONG currentWordIndex = 0;
                                ULONG totalBytesReadFromFile = 0;
                                UWORD currentMemoryWord;
                                UWORD currentFlashWord;
                                
                                totalBytesReadFromFile = Read(fileHandle, memoryHandle, 0x80000);
                                                               
                                do {
                                    currentMemoryWord = ((UWORD *)memoryHandle)[currentWordIndex];
                                    currentFlashWord = ((UWORD *)myCD->cd_BoardAddr)[currentWordIndex];
                                    
                                    if (currentMemoryWord != currentFlashWord)
                                        printf("VERIFY ERROR: Address: 0x%X, Memory: 0x%X, FLASH: 0x%X\n", currentWordIndex, currentMemoryWord, currentFlashWord);
                                    
                                    currentWordIndex += 1;

                                } while (currentWordIndex < (totalBytesReadFromFile >> 1));

                                FreeMem(memoryHandle, 0x80000);
                                Close(fileHandle);
                            }
                            else
                            {
                                Close(fileHandle);
                                printf("Failed to allocate memory %d bytes required\n", (ULONG)myFIB.fib_Size);                                
                            }
                        }
                    }
                    else
                    {
                        printf("Failed to open kickstart image: %s\n", argv[2]);
                    }
                }
                else
                {
                    printf("No Kickstart image specified\n");
                }
            }
            break;            
                
            default:
            {
                printf("Wrong argument: %s\n", argv[1]);
            }
            break;
        }
        
        ++argv;
        --argc;
    }
          
    CloseLibrary(IntuitionBase);
    CloseLibrary(ExpansionBase);
}
