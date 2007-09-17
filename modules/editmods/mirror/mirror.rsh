/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "MIRROR.H"

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
{ "horizontal",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 11, 1,
  "vertikal",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 9, 1,
  "beide",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 6, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 MIRROR_POP ****************************************************/
        -1,        1, BOTH            , G_BOX             ,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00011100L,
  0x0000, 0x0000, 0x000D, 0x0105,
  HORIZONT        ,       -1,       -1, G_STRING          ,   /* Object 1  */
  FLAGS10, DISABLED, (LONG)"  spiegeln",
  0x0200, 0x0200, 0x000A, 0x0001,
  VERTIKAL        ,       -1,       -1, G_BOXTEXT         ,   /* Object 2 HORIZONT */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[0],
  0x0300, 0x0501, 0x030C, 0x0001,
  BOTH            ,       -1,       -1, G_BOXTEXT         ,   /* Object 3 VERTIKAL */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[1],
  0x0300, 0x0902, 0x030C, 0x0001,
         0,       -1,       -1, G_BOXTEXT         ,   /* Object 4 BOTH */
  SELECTABLE|LASTOB|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[2],
  0x0300, 0x0D03, 0x030C, 0x0101
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 MIRROR_POP       */
};
