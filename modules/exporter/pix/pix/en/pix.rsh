/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "pix\en\PIX.H"

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
{ "RLE",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 4, 1,
  "none",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  " Compression ",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 14, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 PIX_EXPORT ****************************************************/
        -1, SAVE            , OK              , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x001F, 0x000A,
         2,       -1,       -1, G_BUTTON          ,   /* Object 1 SAVE */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"save",
  0x040A, 0x0008, 0x0008, 0x0001,
         3,       -1,       -1, G_IBOX            ,   /* Object 2  */
  FLAGS10, NORMAL, (LONG)0x00010100L,
  0x0102, 0x0102, 0x001B, 0x0804,
        10,        4,        9, G_IBOX            ,   /* Object 3  */
  FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0002, 0x0002, 0x001B, 0x0804,
         5,       -1,       -1, G_TEXT            ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0404, 0x0802, 0x0003, 0x0001,
  KEINE           ,       -1,       -1, G_TEXT            ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0404, 0x0001, 0x0004, 0x0001,
  RLE             ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 6 KEINE */
  SELECTABLE|RBUTTON|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0002, 0x0001, 0x0008, 0x0001,
         8,       -1,       -1, G_IBOX            |0x2B00,   /* Object 7 RLE */
  SELECTABLE|RBUTTON|FLAGS10, DISABLED, (LONG)0x00FF1100L,
  0x0002, 0x0802, 0x0008, 0x0001,
         9,       -1,       -1, G_STRING          ,   /* Object 8  */
  NONE, NORMAL, (LONG)"-> PI?",
  0x040B, 0x0001, 0x0006, 0x0001,
         3,       -1,       -1, G_STRING          ,   /* Object 9  */
  NONE, NORMAL, (LONG)"-> PC?",
  0x040B, 0x0802, 0x0006, 0x0001,
  OK              ,       -1,       -1, G_TEXT            ,   /* Object 10  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[2],
  0x0003, 0x0801, 0x000D, 0x0001,
         0,       -1,       -1, G_BUTTON          ,   /* Object 11 OK */
  SELECTABLE|DEFAULT|EXIT|LASTOB|FLAGS9|FLAGS10, NORMAL, (LONG)"OK",
  0x0414, 0x0008, 0x0008, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 PIX_EXPORT       */
};
