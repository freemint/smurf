/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "bmp\en\BMP.H"

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
{ "none",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "RLE",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 4, 1,
  "OS/2 1.x",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 9, 1,
  "Windows 1.x/2.x",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 16, 1,
  "Windows 3.x",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 12, 1,
  "OS/2 2.x",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 9, 1,
  "OS/2 2.x",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 9, 1,
  "standalone Bitmap",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 18, 1,
  "embedded Bitmap",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 16, 1,
  " Filetype ",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 11, 1,
  " Compression ",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 14, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 BMP_EXPORT ****************************************************/
        -1, OK              , SAVE            , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x001F, 0x0815,
         2,       -1,       -1, G_BUTTON          ,   /* Object 1 OK */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"OK",
  0x0414, 0x0813, 0x0008, 0x0001,
         3,       -1,       -1, G_IBOX            ,   /* Object 2  */
  FLAGS10, NORMAL, (LONG)0x00010100L,
  0x0102, 0x0102, 0x001B, 0x0804,
         9, COMP_BOX        , COMP_BOX        , G_IBOX            ,   /* Object 3  */
  FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0002, 0x0002, 0x001B, 0x0804,
         3,        5, RLE             , G_IBOX            ,   /* Object 4 COMP_BOX */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0001, 0x0800, 0x000A, 0x0803,
         6,       -1,       -1, G_TEXT            ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0403, 0x0800, 0x0004, 0x0001,
  KEINE           ,       -1,       -1, G_TEXT            ,   /* Object 6  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0403, 0x0002, 0x0003, 0x0001,
  RLE             ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 7 KEINE */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0800, 0x0008, 0x0001,
  COMP_BOX        ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 8 RLE */
  SELECTABLE|RBUTTON|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0001, 0x0002, 0x0008, 0x0001,
        10,       -1,       -1, G_IBOX            ,   /* Object 9  */
  FLAGS10, NORMAL, (LONG)0x00010100L,
  0x0102, 0x0108, 0x001B, 0x000A,
        23,       11,       22, G_IBOX            ,   /* Object 10  */
  FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0002, 0x0008, 0x001B, 0x000A,
        12,       -1,       -1, G_TEXT            ,   /* Object 11  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0404, 0x0004, 0x0008, 0x0001,
  WIN12           ,       -1,       -1, G_TEXT            ,   /* Object 12  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0404, 0x0001, 0x000F, 0x0001,
        14,       -1,       -1, G_IBOX            |0x2B00,   /* Object 13 WIN12 */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0001, 0x0012, 0x0001,
        15,       -1,       -1, G_TEXT            ,   /* Object 14  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0404, 0x0802, 0x000B, 0x0001,
  OS2_2T1         ,       -1,       -1, G_TEXT            ,   /* Object 15  */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0404, 0x0805, 0x0008, 0x0001,
  WIN3            ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 16 OS2_2T1 */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0805, 0x000B, 0x0001,
  OS2_1           ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 17 WIN3 */
  SELECTABLE|RBUTTON|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0002, 0x0802, 0x000E, 0x0001,
        19,       -1,       -1, G_IBOX            |0x2B00,   /* Object 18 OS2_1 */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0004, 0x000B, 0x0001,
  OS2_2T2         ,       -1,       -1, G_TEXT            ,   /* Object 19  */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0404, 0x0807, 0x0008, 0x0001,
        21,       -1,       -1, G_IBOX            |0x2B00,   /* Object 20 OS2_2T2 */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0807, 0x000B, 0x0001,
        22,       -1,       -1, G_TEXT            ,   /* Object 21  */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0404, 0x0806, 0x060C, 0x0001,
        10,       -1,       -1, G_TEXT            ,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0404, 0x0808, 0x020B, 0x0001,
        24,       -1,       -1, G_TEXT            ,   /* Object 23  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[9],
  0x0003, 0x0807, 0x000A, 0x0001,
  SAVE            ,       -1,       -1, G_TEXT            ,   /* Object 24  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[10],
  0x0003, 0x0801, 0x000D, 0x0001,
         0,       -1,       -1, G_BUTTON          ,   /* Object 25 SAVE */
  SELECTABLE|EXIT|LASTOB|FLAGS9|FLAGS10, NORMAL, (LONG)"save",
  0x000A, 0x0813, 0x0008, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 BMP_EXPORT       */
};
