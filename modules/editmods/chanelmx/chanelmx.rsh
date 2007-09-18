/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "CHANELMX.H"

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
{ "Preview",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 8, 1,
  "Zielkanal",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 10, 1,
  "Blau",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "Gr\201n",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "Rot",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 4, 1,
  "Quellkanal",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 11, 1,
  "___",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "Blau",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "Gr\201n",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "Rot",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 4, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 CHANELMIXER ****************************************************/
        -1,        1, LOAD            , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0433, 0x000F,
         2,       -1,       -1, G_BOX             |0x2F00,   /* Object 1  */
  FLAGS9, SELECTED, (LONG)0x00FF1178L,
  0x0321, 0x0301, 0x0010, 0x0008,
  START           ,       -1,       -1, G_TEXT            ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0720, 0x0B09, 0x0007, 0x0001,
         4,       -1,       -1, G_BUTTON          ,   /* Object 3 START */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Start",
  0x0427, 0x000D, 0x0409, 0x0001,
        13,       -1,       -1, G_TEXT            ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0003, 0x0001, 0x0009, 0x0001,
  SAVE            , SRC_BOX         , SRC_BOX         , G_BOX             ,   /* Object 5  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0014, 0x0002, 0x040B, 0x0005,
         5,        7, QR              , G_IBOX            ,   /* Object 6 SRC_BOX */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0001, 0x0000, 0x0409, 0x0005,
         8,       -1,       -1, G_TEXT            ,   /* Object 7  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0403, 0x0803, 0x0004, 0x0001,
         9,       -1,       -1, G_TEXT            ,   /* Object 8  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0403, 0x0002, 0x0004, 0x0001,
  QB              ,       -1,       -1, G_TEXT            ,   /* Object 9  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0403, 0x0800, 0x0003, 0x0001,
  QG              ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 10 QB */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0803, 0x0008, 0x0001,
  QR              ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 11 QG */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0002, 0x0008, 0x0001,
  SRC_BOX         ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 12 QR */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF1100L,
  0x0400, 0x0800, 0x0008, 0x0001,
  S_F             ,       -1,       -1, G_TEXT            ,   /* Object 13  */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0415, 0x0001, 0x000A, 0x0001,
  LOAD            ,       -1,       -1, G_BUTTON          ,   /* Object 14 SAVE */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"speichern",
  0x040D, 0x000D, 0x0409, 0x0001,
         0,       -1,       -1, G_BUTTON          ,   /* Object 15 LOAD */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"laden",
  0x0402, 0x000D, 0x0409, 0x0001,
  S_E             , S_S             , S_S             , G_BOX             ,   /* Object 16 S_F */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0410, 0x0802, 0x0001, 0x0108,
  S_F             ,       -1,       -1, G_BOX             |0x1500,   /* Object 17 S_S */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        19,       -1,       -1, G_BOXTEXT         ,   /* Object 18 S_E */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[6],
  0x040F, 0x000B, 0x0702, 0x0B00,
         5, DEST_BOX        , DEST_BOX        , G_BOX             ,   /* Object 19  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0001, 0x0002, 0x000C, 0x0005,
        19,       21, ZB              , G_IBOX            ,   /* Object 20 DEST_BOX */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0401, 0x0000, 0x0009, 0x0005,
        22,       -1,       -1, G_TEXT            ,   /* Object 21  */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0004, 0x0803, 0x0004, 0x0001,
        23,       -1,       -1, G_TEXT            ,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0004, 0x0002, 0x0004, 0x0001,
  ZR              ,       -1,       -1, G_TEXT            ,   /* Object 23  */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0004, 0x0800, 0x0003, 0x0001,
  ZG              ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 24 ZR */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF1100L,
  0x0400, 0x0800, 0x0008, 0x0001,
  ZB              ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 25 ZG */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0002, 0x0008, 0x0001,
  DEST_BOX        ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 26 ZB */
  SELECTABLE|RBUTTON|LASTOB, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0803, 0x0008, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 CHANELMIXER      */
};
