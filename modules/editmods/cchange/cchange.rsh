/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "CCHANGE.H"

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
{ "\0\000\000\000",
  "___",
  "999",
  SMALL, 0, TE_RIGHT, 0x1180, 0, -1, 4, 4,
  "\0\000\000\000",
  "___",
  "999",
  SMALL, 0, TE_RIGHT, 0x1180, 0, -1, 4, 4,
  "\0\000\000\000",
  "___",
  "999",
  SMALL, 0, TE_RIGHT, 0x1180, 0, -1, 4, 4,
  "R",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 2, 1,
  "B",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 2, 1,
  "G",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 2, 1,
  "\0\000\000\000",
  "___",
  "999",
  SMALL, 0, TE_RIGHT, 0x1180, 0, -1, 4, 4,
  "\0\000\000\000",
  "___",
  "999",
  SMALL, 0, TE_RIGHT, 0x1180, 0, -1, 4, 4,
  "\0\000\000\000",
  "___",
  "999",
  SMALL, 0, TE_RIGHT, 0x1180, 0, -1, 4, 4,
  "R",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 2, 1,
  "G",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 2, 1,
  "B",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, -1, 2, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 COL_CHANGE ****************************************************/
        -1, START           , SRAHMEN         , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x001C, 0x000C,
  SRC             ,       -1,       -1, G_BUTTON          ,   /* Object 1 START */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Start",
  0x0410, 0x000A, 0x0409, 0x0001,
  DST             ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 2 SRC */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0410, 0x0002, 0x0002, 0x0001,
  DST_SMP         ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 3 DST */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF1100L,
  0x0401, 0x0002, 0x0002, 0x0001,
  SRC_SMP         ,       -1,       -1, G_BOX             ,   /* Object 4 DST_SMP */
  NONE, NORMAL, (LONG)0x00FF1170L,
  0x0404, 0x0001, 0x0006, 0x0003,
         6,       -1,       -1, G_BOX             ,   /* Object 5 SRC_SMP */
  NONE, NORMAL, (LONG)0x00FF1170L,
  0x0413, 0x0001, 0x0006, 0x0003,
  DRAHMEN         ,       -1,       -1, G_STRING          ,   /* Object 6  */
  NONE, NORMAL, (LONG)"-->",
  0x000C, 0x0002, 0x0003, 0x0001,
  SRAHMEN         , EDIT_DR         ,       13, G_IBOX            ,   /* Object 7 DRAHMEN */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0404, 0x0804, 0x0006, 0x0004,
  EDIT_DG         ,       -1,       -1, G_FTEXT           ,   /* Object 8 EDIT_DR */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[0],
  0x0402, 0x0800, 0x0202, 0x0001,
  EDIT_DB         ,       -1,       -1, G_FTEXT           ,   /* Object 9 EDIT_DG */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[1],
  0x0402, 0x0801, 0x0202, 0x0001,
        11,       -1,       -1, G_FTEXT           ,   /* Object 10 EDIT_DB */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[2],
  0x0402, 0x0802, 0x0202, 0x0001,
        12,       -1,       -1, G_TEXT            ,   /* Object 11  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[3],
  0x0001, 0x0800, 0x0600, 0x0001,
        13,       -1,       -1, G_TEXT            ,   /* Object 12  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[4],
  0x0001, 0x0802, 0x0600, 0x0001,
  DRAHMEN         ,       -1,       -1, G_TEXT            ,   /* Object 13  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[5],
  0x0001, 0x0801, 0x0600, 0x0001,
         0, EDIT_SR         ,       20, G_IBOX            ,   /* Object 14 SRAHMEN */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0413, 0x0804, 0x0006, 0x0004,
  EDIT_SG         ,       -1,       -1, G_FTEXT           ,   /* Object 15 EDIT_SR */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[6],
  0x0402, 0x0800, 0x0202, 0x0001,
  EDIT_SB         ,       -1,       -1, G_FTEXT           ,   /* Object 16 EDIT_SG */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[7],
  0x0402, 0x0801, 0x0202, 0x0001,
        18,       -1,       -1, G_FTEXT           ,   /* Object 17 EDIT_SB */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[8],
  0x0402, 0x0802, 0x0202, 0x0001,
        19,       -1,       -1, G_TEXT            ,   /* Object 18  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[9],
  0x0001, 0x0800, 0x0600, 0x0001,
        20,       -1,       -1, G_TEXT            ,   /* Object 19  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[10],
  0x0001, 0x0801, 0x0600, 0x0001,
  SRAHMEN         ,       -1,       -1, G_TEXT            ,   /* Object 20  */
  LASTOB|FLAGS10, NORMAL, (LONG)&rs_tedinfo[11],
  0x0001, 0x0802, 0x0600, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 COL_CHANGE       */
};
