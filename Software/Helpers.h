#ifndef __HELPERS__
#define __HELPERS__

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/expansion_protos.h>
#include <proto/dos.h>

/*****************************************************************************/
/* Macros ********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Defines *******************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Types *********************************************************************/
/*****************************************************************************/

typedef enum {
    
    readFileIdle = 0,
    readFileNoFileSpecified,
    readFileNotFound,
    readFileNoMemoryAllocated,
    readFileGeneralError,
    readFileOK
    
} tReadFileHandler;

/*****************************************************************************/
/* Prototypes ****************************************************************/
/*****************************************************************************/

tReadFileHandler readFileIntoMemoryHandler(char *fileName, ULONG bufferSize, APTR * pMemoryBase);
extern tReadFileHandler freeFileHandler(ULONG bufferSize);
extern tReadFileHandler getFileSize(char *fileName, ULONG * pFileSize);
extern void hexDump(char *desc, void *addr, int len);

#endif /* __HELPERS__ */