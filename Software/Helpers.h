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

#ifndef __HELPERS__
#define __HELPERS__

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
