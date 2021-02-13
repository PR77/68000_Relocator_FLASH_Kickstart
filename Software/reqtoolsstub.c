#include <intuition/intuition.h>
#include <libraries/reqtools.h>
#include <clib/reqtools_protos.h>

APTR rtAllocRequest (ULONG type, ULONG Tag, ...)
{
  return rtAllocRequestA (type, (struct TagItem *) (&type + 1));
}
LONG rtChangeReqAttr (APTR req, ULONG Tag, ...)
{
  return rtChangeReqAttrA (req, (struct TagItem *) (&req + 1));
}

APTR rtFileRequest (struct rtFileRequester * filereq, char * file, char * title, ULONG Tag, ...)
{
  return rtFileRequestA (filereq, file, title, (struct TagItem *) (&title + 1));
}

ULONG rtEZRequestTags (char * bodyfmt, char * gadfmt, struct rtReqInfo * reqinfo, APTR argarray, ULONG Tag, ...)
{
  return rtEZRequestA (bodyfmt, gadfmt, reqinfo, argarray, (struct TagItem *) (&argarray + 1));
}

ULONG rtEZRequest (char * bodyfmt, char * gadfmt, struct rtReqInfo * reqinfo, struct TagItem * tagitem, ...)
{
  return rtEZRequestA (bodyfmt, gadfmt, reqinfo, (APTR) (&tagitem + 1), tagitem);
}

ULONG rtGetString (UBYTE * buffer, ULONG maxchars, char * title, struct rtReqInfo * reqinfo, ULONG Tag, ...)
{
  return rtGetStringA (buffer, maxchars, title, reqinfo, (struct TagItem *) (&reqinfo + 1));
}

ULONG rtGetLong (ULONG * longptr, char * title, struct rtReqInfo * reqinfo, ULONG Tag, ...)
{
  return rtGetLongA (longptr, title, reqinfo, (struct TagItem *) (&reqinfo + 1));
}

ULONG rtFontRequest (struct rtFontRequester * fontreq, char * title, ULONG Tag, ...)
{
  return rtFontRequestA (fontreq, title, (struct TagItem *) (&title + 1));
}

LONG rtPaletteRequest (char * title, struct rtReqInfo * reqinfo, ULONG Tag, ...)
{
  return rtPaletteRequestA (title, reqinfo, (struct TagItem *) (&reqinfo + 1));
}

ULONG rtReqHandler (struct rtHandlerInfo * handlerinfo, ULONG sigs, ULONG Tag, ...)
{
  return rtReqHandlerA (handlerinfo, sigs, (struct TagItem *) (&sigs + 1));
}

ULONG rtScreenModeRequest (struct rtScreenModeRequester * screenmodereq, char * title, ULONG Tag, ...)
{
  return rtScreenModeRequestA (screenmodereq, title, (struct TagItem *) (&title + 1));
}
