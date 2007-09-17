/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "SPIN90.H"

#if !defined(WHITEBAK)
#define WHITEBAK    0x0040
#endif
#if !defined(DRAW3D)
#define DRAW3D      0x0080
#endif

#define FLAGS9  0x0200
#define FLAGS10 0x0400
#define FLAGS11 0x0800
#define FLAGS12 0x1000
#define FLAGS13 0x2000
#define FLAGS14 0x4000
#define FLAGS15 0x8000
#define STATE8  0x0100
#define STATE9  0x0200
#define STATE10 0x0400
#define STATE11 0x0800
#define STATE12 0x1000
#define STATE13 0x2000
#define STATE14 0x4000
#define STATE15 0x8000

TEDINFO rs_tedinfo[] =
{ "links 90\370",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 10, 1,
  "rechts 90\370",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 11, 1,
  "180\370",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 5, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 SPIN90 ****************************************************/
        -1,        1, TWO             , G_BOX             ,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00011100L,
  0x0000, 0x0000, 0x020D, 0x0105,
  LEFT            ,       -1,       -1, G_STRING          ,   /* Object 1  */
  FLAGS10, DISABLED, (LONG)"    drehen   ",
  0x0100, 0x0200, 0x000D, 0x0001,
  RIGHT           ,       -1,       -1, G_BOXTEXT         ,   /* Object 2 LEFT */
  FLAGS9|FLAGS10, NORMAL, (LONG)&rs_tedinfo[0],
  0x0300, 0x0501, 0x040C, 0x0001,
  TWO             ,       -1,       -1, G_BOXTEXT         ,   /* Object 3 RIGHT */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[1],
  0x0300, 0x0902, 0x040C, 0x0001,
         0,       -1,       -1, G_BOXTEXT         ,   /* Object 4 TWO */
  SELECTABLE|LASTOB|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[2],
  0x0300, 0x0D03, 0x040C, 0x0101
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 SPIN90           */
};
