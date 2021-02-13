#include <intuition/intuition.h>
#include <libraries/reqtools.h>
#include <clib/reqtools_protos.h>

void
rtUnlockWindow (struct Window * win, APTR winlock)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct Window * a0 __asm("a0") = win;
  register APTR a1 __asm("a1") = winlock;
  __asm __volatile ("jsr a6@(-162)"
  : /* no output */
  : "r" (a6), "r" (a0), "r" (a1)
  : "a0", "a1",  "memory");
}
APTR
rtLockWindow (struct Window * win)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register APTR _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct Window * a0 __asm("a0") = win;
  __asm __volatile ("jsr a6@(-156)"
  : "=r" (_res)
  : "r" (a6), "r" (a0)
  : "a0",  "memory");
  return _res;
}
void
rtCloseWindowSafely (struct Window * win)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct Window * a0 __asm("a0") = win;
  __asm __volatile ("jsr a6@(-150)"
  : /* no output */
  : "r" (a6), "r" (a0)
  : "a0",  "memory");
}
ULONG
rtScreenModeRequestA (struct rtScreenModeRequester * screenmodereq, char * title, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register ULONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct rtScreenModeRequester * a1 __asm("a1") = screenmodereq;
  register char * a3 __asm("a3") = title;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-144)"
  : "=r" (_res)
  : "r" (a6), "r" (a1), "r" (a3), "r" (a0)
  : "a0", "a1", "a3",  "memory");
  return _res;
}
void
rtScreenToFrontSafely (struct Screen * screen)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct Screen * a0 __asm("a0") = screen;
  __asm __volatile ("jsr a6@(-138)"
  : /* no output */
  : "r" (a6), "r" (a0)
  : "a0",  "memory");
}
void
rtSpread (ULONG * posarray, ULONG * sizearray, ULONG length, ULONG min, ULONG max, ULONG num)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register ULONG * a0 __asm("a0") = posarray;
  register ULONG * a1 __asm("a1") = sizearray;
  register ULONG d0 __asm("d0") = length;
  register ULONG d1 __asm("d1") = min;
  register ULONG d2 __asm("d2") = max;
  register ULONG d3 __asm("d3") = num;
  __asm __volatile ("jsr a6@(-132)"
  : /* no output */
  : "r" (a6), "r" (a0), "r" (a1), "r" (d0), "r" (d1), "r" (d2), "r" (d3)
  : "a0", "a1", "d0", "d1", "d2", "d3",  "memory");
}
void
rtSetReqPosition (ULONG reqpos, struct NewWindow * newwindow, struct Screen * screen, struct Window * window)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register ULONG d0 __asm("d0") = reqpos;
  register struct NewWindow * a0 __asm("a0") = newwindow;
  register struct Screen * a1 __asm("a1") = screen;
  register struct Window * a2 __asm("a2") = window;
  __asm __volatile ("jsr a6@(-126)"
  : /* no output */
  : "r" (a6), "r" (d0), "r" (a0), "r" (a1), "r" (a2)
  : "a0", "a1", "a2", "d0",  "memory");
}
ULONG
rtGetVScreenSize (struct Screen * screen, ULONG * widthptr, ULONG * heightptr)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register ULONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct Screen * a0 __asm("a0") = screen;
  register ULONG * a1 __asm("a1") = widthptr;
  register ULONG * a2 __asm("a2") = heightptr;
  __asm __volatile ("jsr a6@(-120)"
  : "=r" (_res)
  : "r" (a6), "r" (a0), "r" (a1), "r" (a2)
  : "a0", "a1", "a2",  "memory");
  return _res;
}
void
rtSetWaitPointer (struct Window * window)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct Window * a0 __asm("a0") = window;
  __asm __volatile ("jsr a6@(-114)"
  : /* no output */
  : "r" (a6), "r" (a0)
  : "a0",  "memory");
}
ULONG
rtReqHandlerA (struct rtHandlerInfo * handlerinfo, ULONG sigs, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register ULONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct rtHandlerInfo * a1 __asm("a1") = handlerinfo;
  register ULONG d0 __asm("d0") = sigs;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-108)"
  : "=r" (_res)
  : "r" (a6), "r" (a1), "r" (d0), "r" (a0)
  : "a0", "a1",  "memory");
  return _res;
}
LONG
rtPaletteRequestA (char * title, struct rtReqInfo * reqinfo, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register LONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register char * a2 __asm("a2") = title;
  register struct rtReqInfo * a3 __asm("a3") = reqinfo;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-102)"
  : "=r" (_res)
  : "r" (a6), "r" (a2), "r" (a3), "r" (a0)
  : "a0", "a2", "a3",  "memory");
  return _res;
}
ULONG
rtFontRequestA (struct rtFontRequester * fontreq, char * title, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register ULONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct rtFontRequester * a1 __asm("a1") = fontreq;
  register char * a3 __asm("a3") = title;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-96)"
  : "=r" (_res)
  : "r" (a6), "r" (a1), "r" (a3), "r" (a0)
  : "a0", "a1", "a3",  "memory");
  return _res;
}
ULONG
rtGetLongA (ULONG * longptr, char * title, struct rtReqInfo * reqinfo, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register ULONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register ULONG * a1 __asm("a1") = longptr;
  register char * a2 __asm("a2") = title;
  register struct rtReqInfo * a3 __asm("a3") = reqinfo;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-78)"
  : "=r" (_res)
  : "r" (a6), "r" (a1), "r" (a2), "r" (a3), "r" (a0)
  : "a0", "a1", "a2", "a3",  "memory");
  return _res;
}
ULONG
rtGetStringA (UBYTE * buffer, ULONG maxchars, char * title, struct rtReqInfo * reqinfo, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register ULONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register UBYTE * a1 __asm("a1") = buffer;
  register ULONG d0 __asm("d0") = maxchars;
  register char * a2 __asm("a2") = title;
  register struct rtReqInfo * a3 __asm("a3") = reqinfo;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-72)"
  : "=r" (_res)
  : "r" (a6), "r" (a1), "r" (d0), "r" (a2), "r" (a3), "r" (a0)
  : "a0", "a1", "a2", "a3", "memory");
  return _res;
}
ULONG
rtEZRequestA (char * bodyfmt, char * gadfmt, struct rtReqInfo * reqinfo, APTR argarray, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register ULONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register char * a1 __asm("a1") = bodyfmt;
  register char * a2 __asm("a2") = gadfmt;
  register struct rtReqInfo * a3 __asm("a3") = reqinfo;
  register APTR a4 __asm("a4") = argarray;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-66)"
  : "=r" (_res)
  : "r" (a6), "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a0)
  : "a0", "a1", "a2", "a3", "a4",  "memory");
  return _res;
}
void
rtFreeFileList (struct rtFileList * filelist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct rtFileList * a0 __asm("a0") = filelist;
  __asm __volatile ("jsr a6@(-60)"
  : /* no output */
  : "r" (a6), "r" (a0)
  : "a0",  "memory");
}
APTR
rtFileRequestA (struct rtFileRequester * filereq, char * file, char * title, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register APTR _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register struct rtFileRequester * a1 __asm("a1") = filereq;
  register char * a2 __asm("a2") = file;
  register char * a3 __asm("a3") = title;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-54)"
  : "=r" (_res)
  : "r" (a6), "r" (a1), "r" (a2), "r" (a3), "r" (a0)
  : "a0", "a1", "a2", "a3",  "memory");
  return _res;
}
LONG
rtChangeReqAttrA (APTR req, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register LONG _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register APTR a1 __asm("a1") = req;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-48)"
  : "=r" (_res)
  : "r" (a6), "r" (a1), "r" (a0)
  : "a0", "a1",  "memory");
  return _res;
}
void
rtFreeReqBuffer (APTR req)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register APTR a1 __asm("a1") = req;
  __asm __volatile ("jsr a6@(-42)"
  : /* no output */
  : "r" (a6), "r" (a1)
  : "a1",  "memory");
}
void
rtFreeRequest (APTR req)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register APTR a1 __asm("a1") = req;
  __asm __volatile ("jsr a6@(-36)"
  : /* no output */
  : "r" (a6), "r" (a1)
  : "a1",  "memory");
}
APTR
rtAllocRequestA (ULONG type, struct TagItem * taglist)
{
  extern struct ReqToolsBase *  ReqToolsBase;
  register APTR _res  __asm("d0");
  register struct ReqToolsBase * a6 __asm("a6") = ReqToolsBase;
  register ULONG d0 __asm("d0") = type;
  register struct TagItem * a0 __asm("a0") = taglist;
  __asm __volatile ("jsr a6@(-30)"
  : "=r" (_res)
  : "r" (a6), "r" (d0), "r" (a0)
  : "a0",  "memory");
  return _res;
}
