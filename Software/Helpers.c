#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/expansion_protos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Helpers.h"

/*****************************************************************************/
/* Macros ********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Defines *******************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Types *********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Globals *******************************************************************/
/*****************************************************************************/

static BPTR fileHandle;
static APTR memoryHandle;
static ULONG allocatedMemorySize;

/*****************************************************************************/
/* Prototypes ****************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Code **********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Function:    readFileIntoMemoryHandler()                                  */
/* Returns:     tReadFileHandler                                             */
/* Parameters:  None                                                         */
/* Description: Open selected file if exists and reads into allocated memory */
/*****************************************************************************/
tReadFileHandler readFileIntoMemoryHandler(char *fileName)
{
    
}

/*****************************************************************************/
/* Function:    freeFileHandler()                                            */
/* Returns:     tReadFileHandler                                             */
/* Parameters:  None                                                         */
/* Description: Closes open file handle and deallocates storage memory       */
/*****************************************************************************/
tReadFileHandler freeFileHandler(void)
{
    tReadFileHandler readFileHandler = readFileIdle;
    
#ifndef NDEBUG
    printf("ENTRY: freeFileHandler(void)\n");
#endif    
    
    if (0L != fileHandle)
    {
#ifndef NDEBUG
        printf("FLOW: freeFileHandler: File Open(fileHandle 0x%X)\n", fileHandle);
#endif
        Close(fileHandle);
#ifndef NDEBUG
        printf("FLOW: freeFileHandler: File Closed(fileHandle 0x%X)\n", fileHandle);
#endif
    }
   
    if (0L != memoryHandle)
    {
#ifndef NDEBUG
        printf("FLOW: freeFileHandler: Memory Allocated(memoryHandle 0x%X)\n", memoryHandle);
#endif
        Close(fileHandle);
#ifndef NDEBUG
        printf("FLOW: freeFileHandler: Memory Deallocated(memoryHandle 0x%X)\n", memoryHandle);
#endif
        FreeMem(memoryHandle, allocatedMemorySize);
    }

    readFileHandler = readFileOK;
#ifndef NDEBUG
    printf("EXIT: freeFileHandler(readFileHandler 0x%X)\n", readFileHandler);
#endif    
    return (readFileHandler);
}

/*****************************************************************************/
/* Function:    getFileSize()                                                */
/* Returns:     tReadFileHandler                                             */
/* Parameters:  char *fileName, ULONG * pFileSize                            */
/* Description: Gets the size, in bytes, of the filename passed              */
/*****************************************************************************/
tReadFileHandler getFileSize(char *fileName, ULONG * pFileSize)
{
    struct FileInfoBlock FIB;
    tReadFileHandler readFileHandler = readFileIdle;

#ifndef NDEBUG
    printf("ENTRY: getFileSize(char *fileName %s, ULONG * pFileSize 0x%X)\n", fileName, pFileSize);
#endif

    if (0L != fileName)
    {
         fileHandle = Lock(fileName, MODE_OLDFILE);
#ifndef NDEBUG
        printf("FLOW: getFileSize: Locking fileHandle 0x%X\n", fileHandle);
#endif 

        if (0L != fileHandle)
        {
            Examine(fileHandle, &FIB);

            UnLock(fileHandle);

#ifndef NDEBUG
            printf("FLOW: getFileSize: Unlocking fileHandle 0x%X\n", fileHandle);
#endif
            readFileHandler = readFileOK;
        }
        else
        {
            readFileHandler = readFileNotFound;
        }
    }
    else
    {
        readFileHandler = readFileNoFileSpecified;
    }
    
    if (readFileOK == readFileHandler)
        *pFileSize = (ULONG)FIB.fib_Size;
    else
        *pFileSize = 0;
        
#ifndef NDEBUG
    printf("EXIT: freeFileHandler(readFileHandler 0x%X)\n", readFileHandler);
#endif    
    return (readFileHandler);
}

/*****************************************************************************/
/* Function:    hexDump()                                                    */
/* Returns:     Prints directly to console                                   */
/* Parameters:  char *desc, void *addr, int len                              */
/* Description: Dumps directly to console human readable memory              */
/*****************************************************************************/
void hexDump (char *desc, void *addr, int len)
{
    
    // https://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
    
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s [0x%X]:\n", desc, addr);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}
