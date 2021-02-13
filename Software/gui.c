#include <proto/intuition.h>
#include <proto/dos.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <libraries/reqtools.h>
#include <clib/reqtools_protos.h>
#include <workbench/startup.h>
#include <clib/expansion_protos.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "RomInfo.h"
#include "Helpers.h"
#include "FlashDeviceSST39.h"

struct ReqToolsBase *ReqToolsBase;
struct Library *ExpansionBase = NULL;

#define LOOP_TIMEOUT        (ULONG)10000
#define KICKSTART_256K      (ULONG)(256 * 1024)

#define button1w 54
#define button1h 20

#define button2w 87
#define button2h 20

#define button3w 98
#define button3h 20

#define tbox1w 260
#define tbox1h 18

SHORT SharedBordersPairs0[] =
{
    0, 0, 0, button1h - 1, 1, button1h - 2, 1, 0, button1w - 2, 0
};
SHORT SharedBordersPairs1[] =
{
    1, button1h - 1, button1w - 2, button1h - 1, button1w - 2, 1, button1w - 1, 0, button1w - 1, button1h - 1
};

SHORT SharedBordersPairs4[] =
{
    0, 0, 0, button2h - 1, 1, button2h - 2, 1, 0, button2w - 2, 0
};
SHORT SharedBordersPairs5[] =
{
    1, button2h - 1, button2w - 2, button2h - 1, button2w - 2, 1, button2w - 1, 0, button2w - 1, button2h - 1
};

SHORT SharedBordersPairs2[] =
{
    0, 0, 0, button3h - 1, 1, button3h - 2, 1, 0, button3w - 2, 0
};
SHORT SharedBordersPairs3[] =
{
    1, button3h - 1, button3w - 2, button3h - 1, button3w - 2, 1, button3w - 1, 0, button3w - 1, button3h - 1
};


SHORT SharedBordersPairs6[] =
{
    -2, -1, -2, tbox1h - 1, -1, tbox1h - 2, -1, -1, tbox1w - 2, -1
    };
SHORT SharedBordersPairs7[] =
{
    -1, tbox1h - 1, tbox1w - 2, tbox1h - 1, tbox1w - 2, 0, tbox1w - 1, -1, tbox1w - 1, tbox1h - 1
    };


struct Border SharedBorders[] =
{
    0, 0, 2, 0, JAM2, 5, (SHORT *) &SharedBordersPairs0[0], &SharedBorders[1],
    0, 0, 1, 0, JAM2, 5, (SHORT *) &SharedBordersPairs1[0], NULL,
    0, 0, 2, 0, JAM2, 5, (SHORT *) &SharedBordersPairs2[0], &SharedBorders[3],
    0, 0, 1, 0, JAM2, 5, (SHORT *) &SharedBordersPairs3[0], NULL,
    0, 0, 2, 0, JAM2, 5, (SHORT *) &SharedBordersPairs4[0], &SharedBorders[5],
    0, 0, 1, 0, JAM2, 5, (SHORT *) &SharedBordersPairs5[0], NULL,
    0, 0, 1, 0, JAM2, 5, (SHORT *) &SharedBordersPairs6[0], &SharedBorders[7],
    0, 0, 2, 0, JAM2, 5, (SHORT *) &SharedBordersPairs7[0], NULL,
};

struct Border SharedBordersInvert[] =
{
    0, 0, 1, 0, JAM2, 5, (SHORT *) &SharedBordersPairs0[0], &SharedBordersInvert[1],
    0, 0, 2, 0, JAM2, 5, (SHORT *) &SharedBordersPairs1[0], NULL,
    0, 0, 1, 0, JAM2, 5, (SHORT *) &SharedBordersPairs2[0], &SharedBordersInvert[3],
    0, 0, 2, 0, JAM2, 5, (SHORT *) &SharedBordersPairs3[0], NULL,
    0, 0, 1, 0, JAM2, 5, (SHORT *) &SharedBordersPairs4[0], &SharedBordersInvert[5],
    0, 0, 2, 0, JAM2, 5, (SHORT *) &SharedBordersPairs5[0], NULL,
};

struct IntuiText QuitButton_text =
{
    1, 0, JAM2, 12, 6, NULL, (UBYTE *)"Quit", NULL
};

#define GADQUIT 7

struct Gadget QuitButton =
{
    NULL, 438, 102, button1w, button1h,
    GADGHIMAGE,
    RELVERIFY,
    BOOLGADGET,
    (APTR) &SharedBorders[0], (APTR) &SharedBordersInvert[0],
    &QuitButton_text, NULL, NULL, GADQUIT, NULL
};

struct IntuiText AboutButton_text =
{
    1, 0, JAM2, 8, 6, NULL, (UBYTE *)"About", NULL
};

#define GADABOUT 6

struct Gadget AboutButton =
{
    &QuitButton, 356, 102, button1w, button1h,
    GADGHIMAGE,
    RELVERIFY,
    BOOLGADGET,
    (APTR) &SharedBorders[0], (APTR) &SharedBordersInvert[0],
    &AboutButton_text, NULL, NULL, GADABOUT, NULL
};

struct IntuiText EraseButton_text =
{
    1, 0, JAM2, 8, 6, NULL, (UBYTE *)"Erase ROMs", NULL
};

#define GADERASE 5

struct Gadget EraseButton =
{
    &AboutButton, 20, 102, button3w, button3h,
    GADGHIMAGE,
    RELVERIFY,
    BOOLGADGET,
    (APTR) &SharedBorders[2], (APTR) &SharedBordersInvert[2],
    &EraseButton_text, NULL, NULL, GADERASE, NULL
};

struct IntuiText LoadFile_text =
{
    1, 0, JAM2, 8, 6, NULL, (UBYTE *)"Load File", NULL
};

#define GADFILE2 4

struct Gadget LoadFile2 =
{
    &EraseButton, 402, 69, button2w, button2h,
    GADGHIMAGE,
    RELVERIFY,
    BOOLGADGET,
    (APTR) &SharedBorders[4], (APTR) &SharedBordersInvert[4],
    &LoadFile_text, NULL, NULL, GADFILE2, NULL
};

#define GADFILE1 3

struct Gadget LoadFile1 =
{
    &LoadFile2, 402, 45, button2w, button2h,
    GADGHIMAGE,
    RELVERIFY,
    BOOLGADGET,
    (APTR) &SharedBorders[4], (APTR) &SharedBordersInvert[4],
    &LoadFile_text, NULL, NULL, GADFILE1, NULL
};

UBYTE FlashROM2_buf[64];

struct IntuiText FlashROM2_text[] =
{
    1, 0, JAM2, -100, 4, NULL, (UBYTE *)"FLASH ROM 2:", &FlashROM2_text[1],
    1, 0, JAM2, 4, 4, NULL, (UBYTE *)FlashROM2_buf, NULL
};

#define GADFLASH2 2

struct Gadget FlashROM2 =
{
    &LoadFile1, 130, 70, tbox1w, tbox1h,
    GADGHIMAGE,
    0,
    BOOLGADGET,
    (APTR) &SharedBorders[6], NULL,
    &FlashROM2_text, NULL, NULL, GADFLASH2, NULL
};

UBYTE FlashROM1_buf[64];

struct IntuiText FlashROM1_text[] =
{
    1, 0, JAM2, -100, 4, NULL, (UBYTE *)"FLASH ROM 1:", &FlashROM1_text[1],
    1, 0, JAM2, 4, 4, NULL, (UBYTE *)FlashROM1_buf, NULL
};

#define GADFLASH1 1

struct Gadget FlashROM1 =
{
    &FlashROM2, 130, 46, tbox1w, tbox1h,
    GADGHIMAGE,
    0,
    BOOLGADGET,
    (APTR) &SharedBorders[6], NULL,
    &FlashROM1_text, NULL, NULL, GADFLASH1, NULL
};

UBYTE Motherboard_buf[64];

struct IntuiText Motherboard_text[] =
{
    1, 0, JAM2, -108, 4, NULL, (UBYTE *)"Internal ROM:", &Motherboard_text[1],
    1, 0, JAM2, 4, 4, NULL, (UBYTE *)Motherboard_buf, NULL
};

#define GADMOTHER 0

struct Gadget Motherboard =
{
    &FlashROM1, 130, 22, tbox1w, tbox1h,
    GADGHIMAGE,
    0,
    BOOLGADGET,
    (APTR) &SharedBorders[6], NULL,
    &Motherboard_text, NULL, NULL, GADMOTHER, NULL
};


static void WriteRomText(const char *text, UBYTE *buffer, struct Window *window,
                         struct Gadget *gadget)
{
    ULONG newlen = strlen(text);
    ULONG oldlen = strlen((char *)buffer);

    if (newlen < oldlen)
    {
        snprintf((char *)buffer, 64, "%s%*.*s", text, (int)(oldlen - newlen),
                 (int)(oldlen - newlen), " ");
    }
    else
    {
        strncpy((char *)buffer, text, 64);
    }

    RefreshGadgets(gadget, window, NULL);
}

static char *GetFile()
{
    struct rtFileRequester *filereq;
    char filename[34];
    char *fullpath = malloc(256 * sizeof(char));
    FILE *romFile;
    struct romInfo romInfo;

    if ((filereq = rtAllocRequestA (RT_FILEREQ, NULL)))
    {
        filename[0] = 0;

        if (!rtFileRequest(filereq, filename, "Pick a ROM file",
                           TAG_END))
        {
            free(fullpath);
            return NULL;
        }

    }
    else
    {
        rtEZRequest("Out of memory!", "Oh no!", NULL, NULL);
    }

    snprintf(fullpath, 255, "%s/%s", filereq->Dir, filename);
    romFile = fopen(fullpath, "r");

    if (!romFile)
    {
        free(fullpath);
        rtEZRequest("Could no open ROM file", "OK", NULL, NULL);
        return NULL;
    }

    UBYTE *buf = (UBYTE *)malloc(1024 * sizeof(UBYTE));
    fread(buf, 1, 1023, romFile);

    if (getRomInfo(buf, &romInfo))
    {
        int res = rtEZRequest("ROM file does not appear to be valid.\nIt could be byte swapped which will not work.\nContinue anyway?",
                              "Yes|No", NULL, NULL);

        if (!res)
        {
            free(fullpath);
            return NULL;
        }
    }

    fclose(romFile);
    return fullpath;
}

static void getRom(int romID, struct ConfigDev *myCD, struct Window *myWindow)
{
    struct romInfo rInfo;
    char *rtext = NULL;

    if (romID == 1)
    {
        getRomInfo((UBYTE *)myCD->cd_BoardAddr, &rInfo);
    }
    else
    {
        getRomInfo((UBYTE *)(myCD->cd_BoardAddr + (512 * 1024)), &rInfo);
    }

    displayRomInfo(&rInfo, &rtext);

    if (romID == 1)
    {
        WriteRomText(rtext, FlashROM1_buf, myWindow, &FlashROM1);
    }
    else
    {
        WriteRomText(rtext, FlashROM2_buf, myWindow, &FlashROM2);
    }

    free(rtext);
}

static void eraseFlash(struct ConfigDev *myCD)
{
    if (flashOK == eraseCompleteFlash((ULONG)myCD->cd_BoardAddr))
    {
        tFlashCommandStatus flashCommandStatus;
        ULONG breakCount = 0;

        do
        {
            flashCommandStatus = checkFlashStatus((ULONG)myCD->cd_BoardAddr);
            breakCount++;

        }
        while ((flashCommandStatus != flashOK) && (breakCount < LOOP_TIMEOUT));

        if (flashOK != flashCommandStatus)
        {
            rtEZRequest("An error occurred erasing the flash chips.", "OK", NULL, NULL);
        }
    }
    else
    {
        rtEZRequest("Flash chips did not accept the erase command.", "OK", NULL, NULL);
    }
}
static tFlashCommandStatus programFlashLoop(ULONG fileSize, ULONG baseAddress,
        APTR *pBuffer)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;
    ULONG currentWordIndex = 0;
    ULONG breakCount = 0;

    do
    {
        writeFlashWord(baseAddress, currentWordIndex << 1,
                       ((UWORD *)pBuffer)[currentWordIndex]);

        breakCount = 0;

        do
        {
            flashCommandStatus = checkFlashStatus(baseAddress + (currentWordIndex << 1));
            breakCount++;

        }
        while ((flashCommandStatus != flashOK) && (breakCount < LOOP_TIMEOUT));

        if (((UWORD *)pBuffer)[currentWordIndex] != (((UWORD *)
                baseAddress)[currentWordIndex]))
        {
            return flashProgramError;
        }

        if (breakCount == LOOP_TIMEOUT)
        {
            flashCommandStatus = flashProgramTimeout;
            break;
        }

        currentWordIndex += 1;
    }
    while (currentWordIndex < (fileSize >> 1));

    return (flashCommandStatus);
}

static void flashRom(int romID, struct ConfigDev *myCD, struct Window *myWindow)
{
    APTR pBuffer = NULL;
    ULONG fileSize;
    char *romFile = GetFile();

    if (romFile)
    {
        tReadFileHandler readFileProgram = getFileSize(romFile, &fileSize);

        if (readFileOK == readFileProgram)
        {
            readFileProgram = readFileIntoMemoryHandler(romFile, fileSize, &pBuffer);

            if (readFileOK == readFileProgram)
            {
                tFlashCommandStatus programFlashStatus = flashIdle;
                ULONG baseAddress = (fileSize == KICKSTART_256K) ? ((ULONG)myCD->cd_BoardAddr +
                                    KICKSTART_256K) : (ULONG)myCD->cd_BoardAddr;

                if (romID == 2)
                {
                    baseAddress = baseAddress + (512 * 1024);
                    WriteRomText("Flashing...", FlashROM2_buf, myWindow, &FlashROM2);
                }
                else
                {
                    WriteRomText("Flashing...", FlashROM1_buf, myWindow, &FlashROM1);
                }

                programFlashStatus = programFlashLoop(fileSize, baseAddress, pBuffer);

                if (programFlashStatus != flashOK)
                {
                    rtEZRequest("An error occurred flashing the ROM.\nDid you erase first?", "OK",
                                NULL, NULL);
                }

                freeFileHandler(fileSize);
            }
            else if (readFileNotFound == readFileProgram)
            {
                rtEZRequest("Could not read the ROM file.", "OK", NULL, NULL);
            }
            else if (readFileNoMemoryAllocated == readFileProgram)
            {
                rtEZRequest("Not enough memory to load the ROM file to flash.", "OK", NULL,
                            NULL);
            }
            else if (readFileGeneralError == readFileProgram)
            {
                rtEZRequest("An error occurred reading the ROM file", "OK", NULL, NULL);
            }
            else
            {
                rtEZRequest("Unknown error occurred, sorry!", "OK", NULL, NULL);
            }
        }
        else
        {
            rtEZRequest("Could not get the file size for the ROM.", "OK", NULL, NULL);
        }

        free(romFile);
        free(pBuffer);
        getRom(romID, myCD, myWindow);
    }

}

void myputs (char *str)
{
    Write (Output(), str, strlen(str));
}

struct NewWindow winlayout =
{
    20, 20,
    512, 128,
    -1, -1,
    IDCMP_CLOSEWINDOW | GADGETUP | GADGETDOWN,
    WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET |    WFLG_CLOSEGADGET | WFLG_ACTIVATE,
    &Motherboard, NULL,
    (STRPTR)"Flash Kickstart Programmer",
    NULL, NULL,
    0, 0,
    600, 400,
    WBENCHSCREEN
};

int main(int argc, char **argv)
{
    struct Window *myWindow;
    char librarypath[128];

    if (argc > 1)
    {
        strcpy(librarypath, argv[1]);
    }

    if (!(ReqToolsBase = (struct ReqToolsBase *)
                         OpenLibrary (REQTOOLSNAME, REQTOOLSVERSION)))
    {
        if ((argc == 0) || !(ReqToolsBase = (struct ReqToolsBase *)
                                            OpenLibrary (librarypath, REQTOOLSVERSION)))
        {

            myputs ("You need reqtools.library V38 or higher!\n"
                    "Please install it in your Libs: drirectory.\n");
            exit (RETURN_FAIL);
        }
    }

    /* Open any version expansion.library to read in ConfigDevs */
    ExpansionBase = OpenLibrary((CONST_STRPTR)"expansion.library", 0L);

    /* Check if opened correctly, otherwise exit with message and error */
    if (NULL == ExpansionBase)
    {
        myputs("Failed to open expansion.library\n");
        exit(RETURN_FAIL);
    }


    myWindow = OpenWindow(&winlayout);
    struct ConfigDev *myCD = FindConfigDev(NULL, 1977L, 104L);

    if (NULL == myCD)
    {
        rtEZRequest("Flash Kickstart board not found.\nYou might be booting from one of its ROMs.\nWhich means accessing ROMs here is disabled.\n\nPlease reboot into the Motherboard ROM and try again.",
                    "OK", NULL, NULL);
        OffGadget(&EraseButton, myWindow, NULL);
        OffGadget(&LoadFile1, myWindow, NULL);
        OffGadget(&LoadFile2, myWindow, NULL);
        OffGadget(&FlashROM1, myWindow, NULL);
        OffGadget(&FlashROM2, myWindow, NULL);
    }
    else
    {
        getRom(1, myCD, myWindow);

        if (myCD->cd_BoardSize <= 512 * 1024)
        {
            // 020 chips in use, no ROM2
            OffGadget(&FlashROM2, myWindow, NULL);
            OffGadget(&LoadFile2, myWindow, NULL);
        }
        else
        {
            getRom(2, myCD, myWindow);
        }
    }

    struct romInfo mbInfo;

    char *mbtext = NULL;

    if (!getRomInfo((UBYTE *)0xF80000, &mbInfo))
    {
        displayRomInfo(&mbInfo, &mbtext);
        WriteRomText(mbtext, Motherboard_buf, myWindow, &Motherboard);
        free(mbtext);
    }

    FOREVER
    {
        bool closewin = false;
        struct IntuiMessage *message;
        Wait(1 << myWindow->UserPort->mp_SigBit);

        while ((message = GetMsg(myWindow->UserPort)))
        {
            ULONG class = message->Class;
            //code = message->Code;
            struct Gadget *address = message->IAddress;
            //x = message->MouseX;
            //y = message- >MouseY;
            ReplyMsg(message);

            if (class == IDCMP_CLOSEWINDOW)
            {
                closewin = true;
            }
            else if (class == GADGETUP)
            {
                switch (address->GadgetID)
                {
                    case GADFILE1:
                    {
                        flashRom(1, myCD, myWindow);
                        break;
                    }

                    case GADFILE2:
                    {
                        flashRom(2, myCD, myWindow);
                        break;
                    }

                    case GADERASE:
                    {
                        int res = rtEZRequest("This action will erase both ROM slots 1 and 2\nContinue anyway?",
                                              "Yes|No", NULL, NULL);

                        if (!res)
                        {
                            break;
                        }

                        WriteRomText("Erasing...", FlashROM1_buf, myWindow, &FlashROM1);
                        WriteRomText("Erasing...", FlashROM2_buf, myWindow, &FlashROM2);
                        eraseFlash(myCD);
                        getRom(1, myCD, myWindow);
                        getRom(2, myCD, myWindow);
                        break;
                    }

                    case GADABOUT:
                        rtEZRequest("Flash Kickstart Programmer v1.0\nCreated by Andrew (LinuxJedi) Hutchings\nandrew@linuxjedi.co.uk\nThis software is released under a GPLv3 license\n\nBased on work by Paul Raspa.",
                                    "Cool!", NULL, NULL);
                        break;

                    case GADQUIT:
                        closewin = true;
                        break;
                }
            }

            if (closewin)
                break;
        }

        if (closewin)
            break;
    };

    if (myWindow) CloseWindow(myWindow);

    CloseLibrary(ExpansionBase);
    return (0);
}
