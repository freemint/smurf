/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "chanel5\en\CHANEL5.H"

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
  "Format",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 7, 1,
  "->RBG",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 6, 1,
  "->GRB",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 6, 1,
  "->GBR",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 6, 1,
  "->BRG",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 6, 1,
  "->BGR",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 6, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 CHANEL5 ****************************************************/
        -1,        1, MCONF_LOAD      , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0423, 0x000E,
         2,       -1,       -1, G_BOX             |0x2F00,   /* Object 1  */
  FLAGS9, SELECTED, (LONG)0x00FF1178L,
  0x0411, 0x0001, 0x0010, 0x0008,
  START           ,       -1,       -1, G_TEXT            ,   /* Object 2  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0011, 0x0809, 0x0007, 0x0001,
         4,       -1,       -1, G_BUTTON          ,   /* Object 3 START */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Start",
  0x0018, 0x000C, 0x0409, 0x0001,
         5,       -1,       -1, G_TEXT            ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0002, 0x0001, 0x0006, 0x0001,
         6,       -1,       -1, G_BOX             ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00001170L,
  0x0002, 0x0702, 0x0100, 0x0407,
  FORMAT_BOX      ,       -1,       -1, G_BOX             ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00001171L,
  0x0701, 0x0602, 0x0100, 0x0407,
  MCONF_SAVE      ,        8, _BGR            , G_IBOX            ,   /* Object 7 FORMAT_BOX */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0003, 0x0002, 0x0009, 0x0008,
  _RBG            ,       -1,       -1, G_TEXT            ,   /* Object 8  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0003, 0x0800, 0x0005, 0x0001,
        10,       -1,       -1, G_IBOX            |0x2B00,   /* Object 9 _RBG */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF1100L,
  0x0400, 0x0800, 0x0008, 0x0001,
  _GRB            ,       -1,       -1, G_TEXT            ,   /* Object 10  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0003, 0x0002, 0x0005, 0x0001,
        12,       -1,       -1, G_IBOX            |0x2B00,   /* Object 11 _GRB */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0002, 0x0008, 0x0001,
  _GBR            ,       -1,       -1, G_TEXT            ,   /* Object 12  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0003, 0x0803, 0x0005, 0x0001,
        14,       -1,       -1, G_IBOX            |0x2B00,   /* Object 13 _GBR */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0803, 0x0008, 0x0001,
  _BRG            ,       -1,       -1, G_TEXT            ,   /* Object 14  */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0003, 0x0005, 0x0005, 0x0001,
        16,       -1,       -1, G_IBOX            |0x2B00,   /* Object 15 _BRG */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0005, 0x0008, 0x0001,
  _BGR            ,       -1,       -1, G_TEXT            ,   /* Object 16  */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0003, 0x0806, 0x0005, 0x0001,
  FORMAT_BOX      ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 17 _BGR */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0400, 0x0806, 0x0008, 0x0001,
  MCONF_LOAD      ,       -1,       -1, G_BUTTON          ,   /* Object 18 MCONF_SAVE */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Save",
  0x040C, 0x000C, 0x0409, 0x0001,
         0,       -1,       -1, G_BUTTON          ,   /* Object 19 MCONF_LOAD */
  SELECTABLE|EXIT|LASTOB|FLAGS9|FLAGS10, NORMAL, (LONG)"Load",
  0x0401, 0x000C, 0x0409, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 CHANEL5          */
};
