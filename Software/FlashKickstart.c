/*
    This file is part of FLASH_KICKSTART originally designed by
    Paul Raspa.

    Modifications by Andrew (LinuxJedi) Hutchings.

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
#include "RomInfo.h"

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

// Compiler complains if we don't do this, can't see why
struct ConfigDev *FindConfigDev( CONST struct ConfigDev *oldConfigDev, LONG manufacturer, LONG product );

/*****************************************************************************/
/* Local Code ****************************************************************/
/*****************************************************************************/
static void eraseFlash(struct ConfigDev *myCD)
{
    if (!DisplayAlert(RECOVERY_ALERT, (CONST_STRPTR)eraseAlertMsg, 52))
        return;

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

static void getDeviceID(UBYTE manufacturer, UBYTE model)
{
    if (manufacturer == 0xBF)
    {
        switch (model)
        {
            case 0xB5:
                printf("SST39SF010A (128KB)\n");
                break;
            case 0xB6:
                printf("SST39SF020A (256KB)\n");
                break;
            case 0xB7:
                printf("SST39SF040 (512KB)\n");
                break;
            default:
                printf("Unknown SST\n");
                break;
        }
    }
    else
    {
        printf("Unknown\n");
    }
}

tFlashCommandStatus programFlashLoop(ULONG fileSize, ULONG baseAddress, APTR * pBuffer)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;
    ULONG currentWordIndex = 0;
    ULONG breakCount = 0;

    do {
        writeFlashWord(baseAddress, currentWordIndex << 1, ((UWORD *)pBuffer)[currentWordIndex]);

        breakCount = 0;

        do {
            flashCommandStatus = checkFlashStatus(baseAddress + (currentWordIndex << 1));
            breakCount++;

        } while ((flashCommandStatus != flashOK) && (breakCount < LOOP_TIMEOUT));

        if (((UWORD *)pBuffer)[currentWordIndex] != (((UWORD *)baseAddress)[currentWordIndex]))
        {
            printf("Failed at ADDR: 0x%06X, SRC: 0x%04X, DEST 0x%04X\n", (unsigned)(currentWordIndex << 1), ((UWORD *)pBuffer)[currentWordIndex], ((UWORD *)baseAddress)[currentWordIndex]);
            return flashProgramError;
        }

        if (breakCount == LOOP_TIMEOUT)
        {
            flashCommandStatus = flashProgramTimeout;
            printf("Flash device timeout hit\n");
            break;
        }

        currentWordIndex += 1;
    } while (currentWordIndex < (fileSize >> 1));

    return (flashCommandStatus);
}

/*****************************************************************************/
/* Main Code *****************************************************************/
/*****************************************************************************/
int main(int argc, char **argv)
{
    struct romInfo rInfo;
    struct ConfigDev *myCD = NULL;
    //struct FileInfoBlock myFIB;
    //BPTR fileHandle = 0L;

    /* Check if application has been started with correct parameters */
    if (argc <= 1)
    {
        printf("FlashKickstart v3.1\n");
        printf("usage: FlashKickstart <option> <filename>\n");
        printf(" -i\tFLASH CHIP INFO\n");
        printf(" -e\tERASE\n");
        printf(" -d\tDUMP <start address [default F80000]> <length [default 64]>\n");
        printf(" -p\tPROGRAM <filename> <1/2>\n");

        exit(RETURN_FAIL);
    }

    /* Open any version intuition.library to support displayAlert */
    IntuitionBase = OpenLibrary((CONST_STRPTR)"intuition.library", 0);

    /* Check if opened correctly, otherwise exit with message and error */
    if (NULL == IntuitionBase)
    {
        printf("Failed to open intuition.library\n");
        exit(RETURN_FAIL);
    }

    /* Open any version expansion.library to read in ConfigDevs */
    ExpansionBase = OpenLibrary((CONST_STRPTR)"expansion.library", 0L);

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
    myCD = FindConfigDev(NULL, 1977L, 104L);

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
        printf("Board address: 0x%06X\n", (unsigned)myCD->cd_BoardAddr);
        printf("Flash size: %ldK\n", ((ULONG)myCD->cd_BoardSize)/1024);
    }

    while ((argc > 1) && (argv[1][0] == '-'))
    {
        switch (argv[1][1])
        {
            case 'i':
            {
                UWORD flashManufactureID, flashDeviceID;

                if (!(flashOK == readManufactureID((ULONG)myCD->cd_BoardAddr, &flashManufactureID)))
                {
                    printf("Failed to identify FLASH Manufacturer ID\n");
                }

                if (!(flashOK == readDeviceID((ULONG)myCD->cd_BoardAddr, &flashDeviceID)))
                {
                    printf("Failed to identify FLASH Device ID\n");
                }
                printf("High flash chip: ");
                getDeviceID(((flashManufactureID & 0xFF00) >> 8), ((flashDeviceID & 0xFF00) >> 8));
                printf("Low flash chip: ");
                getDeviceID((flashManufactureID & 0xFF), (flashDeviceID & 0xFF));

                if (getRomInfo((UBYTE*)0xF80000, &rInfo))
                {
                    printf("Failed to get Kickstart ROM info\n");
                }
                else
                {
                    printf("Motherboard ROM: ");
                    displayRomInfo(&rInfo, NULL);
                }
                if (getRomInfo((UBYTE*)myCD->cd_BoardAddr, &rInfo))
                {
                    printf("Failed to get Flash ROM 1 info\n");
                }
                else
                {
                    printf("Flash ROM 1: ");
                    displayRomInfo(&rInfo, NULL);
                }
                if (myCD->cd_BoardSize > 512*1024)
                {
                    if (getRomInfo((UBYTE*)(myCD->cd_BoardAddr + (512 * 1024)), &rInfo))
                    {
                        printf("Failed to get Flash ROM 2 info\n");
                    }
                    else
                    {
                        printf("Flash ROM 2: ");
                        displayRomInfo(&rInfo, NULL);
                    }
                }
            }
            break;

            case 'e':
            {
                eraseFlash(myCD);
                break;
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
                if ((argc > 3) && argv[2] && argv[3])
                {
                    APTR pBuffer;
                    ULONG fileSize;

                    tReadFileHandler readFileProgram = getFileSize(argv[2], &fileSize);

                    if (readFileOK == readFileProgram)
                    {
                        readFileProgram = readFileIntoMemoryHandler(argv[2], fileSize, &pBuffer);

                        if (readFileOK == readFileProgram)
                        {
                            if (getRomInfo((UBYTE*)0xF80000, &rInfo))
                            {
                                printf("Failed to get Kickstart ROM info\n");
                            }
                            else
                            {
                                printf("Motherboard ROM: ");
                                displayRomInfo(&rInfo, NULL);
                            }
                            if (getRomInfo((UBYTE*)myCD->cd_BoardAddr, &rInfo))
                            {
                                printf("Failed to get Flash ROM 1 info\n");
                            }
                            else
                            {
                                printf("Flash ROM 1: ");
                                displayRomInfo(&rInfo, NULL);
                            }
                            if (myCD->cd_BoardSize > 512*1024)
                            {
                                if (getRomInfo((UBYTE*)(myCD->cd_BoardAddr + (512 * 1024)), &rInfo))
                                {
                                    printf("Failed to get Flash ROM 2 info\n");
                                }
                                else
                                {
                                    printf("Flash ROM 2: ");
                                    displayRomInfo(&rInfo, NULL);
                                }
                            }

                            if (getRomInfo((UBYTE*)pBuffer, &rInfo))
                            {
                                char ch;
                                printf("Failed to get File ROM info\n");
                                printf("WARNING: This could mean the ROM is byte swapped which will not work\n");
                                printf("Continue? ");
                                scanf(" %c", &ch);
                                if (ch != 'y' && ch != 'Y')
                                {
                                    return 1;
                                }
                            }
                            else
                            {
                                printf("File ROM: ");
                                displayRomInfo(&rInfo, NULL);
                            }
                            tFlashCommandStatus programFlashStatus = flashIdle;
                            ULONG baseAddress = (fileSize == KICKSTART_256K) ? ((ULONG)myCD->cd_BoardAddr + KICKSTART_256K) : (ULONG)myCD->cd_BoardAddr;

                            if (strcmp(argv[3], "2") == 0)
                            {
                                baseAddress = baseAddress + (512*1024);
                            }
                            else if (strcmp(argv[3], "1"))
                            {
                                printf("Flash ROM location '1' or '2' not specified\n");
                                return 1;
                            }

                            programFlashStatus = programFlashLoop(fileSize, baseAddress, pBuffer);

                            if (programFlashStatus != flashOK)
                            {
                                printf("Failed to program kickstart image\n");
                            }
                            else
                            {
                                printf("FLASH device programmed OK\n");
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
                            printf("Failed to read into memory file: %s\n\n", argv[2]);
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
                    printf("Kickstart image and position 1 or 2 required\n");
                }
            }
            break;
        }

        ++argv;
        --argc;
    }

    CloseLibrary(IntuitionBase);
    CloseLibrary(ExpansionBase);
}
