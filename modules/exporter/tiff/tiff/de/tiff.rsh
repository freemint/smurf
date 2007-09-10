/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "tiff\de\TIFF.H"

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
{ "PackBits 32773",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 15, 1,
  "keine",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 6, 1,
  "Huffman",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 8, 1,
  "LZW",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 4, 1,
  " Kompression ",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 14, 1,
  "Intel",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 6, 1,
  "Motorola",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 9, 1,
  " Format ",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 9, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 TIFF_EXPORT ****************************************************/
        -1, OK              , SAVE            , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x041E, 0x0013,
         2,       -1,       -1, G_BUTTON          ,   /* Object 1 OK */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"OK",
  0x0014, 0x0011, 0x0008, 0x0001,
         3,       -1,       -1, G_IBOX            ,   /* Object 2  */
  FLAGS10, NORMAL, (LONG)0x00010100L,
  0x0102, 0x0102, 0x041A, 0x0807,
        12,        4, LZW             , G_IBOX            ,   /* Object 3  */
  FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0002, 0x0002, 0x041A, 0x0807,
         5,       -1,       -1, G_TEXT            ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0404, 0x0802, 0x000E, 0x0001,
  KEINE           ,       -1,       -1, G_TEXT            ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0404, 0x0001, 0x0005, 0x0001,
  RLE             ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 6 KEINE */
  SELECTABLE|RBUTTON|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0002, 0x0001, 0x0008, 0x0001,
         8,       -1,       -1, G_IBOX            |0x2B00,   /* Object 7 RLE */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0802, 0x0011, 0x0001,
  HUFFMAN         ,       -1,       -1, G_TEXT            ,   /* Object 8  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0404, 0x0004, 0x0007, 0x0001,
        10,       -1,       -1, G_IBOX            |0x2B00,   /* Object 9 HUFFMAN */
  SELECTABLE|RBUTTON|FLAGS10, DISABLED, (LONG)0x00FF1100L,
  0x0002, 0x0004, 0x000A, 0x0001,
  LZW             ,       -1,       -1, G_TEXT            ,   /* Object 10  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0404, 0x0805, 0x0003, 0x0001,
         3,       -1,       -1, G_IBOX            |0x2B00,   /* Object 11 LZW */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0805, 0x0006, 0x0001,
        13,       -1,       -1, G_TEXT            ,   /* Object 12  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[4],
  0x0003, 0x0801, 0x000D, 0x0001,
        14,       -1,       -1, G_IBOX            ,   /* Object 13  */
  FLAGS10, NORMAL, (LONG)0x00010100L,
  0x0102, 0x010B, 0x041A, 0x0804,
        19,       15, INTEL           , G_IBOX            ,   /* Object 14  */
  FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0002, 0x000B, 0x041A, 0x0804,
        16,       -1,       -1, G_TEXT            ,   /* Object 15  */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x0404, 0x0802, 0x0005, 0x0001,
  MOTO            ,       -1,       -1, G_TEXT            ,   /* Object 16  */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0404, 0x0001, 0x0008, 0x0001,
  INTEL           ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 17 MOTO */
  SELECTABLE|RBUTTON|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0002, 0x0001, 0x000B, 0x0001,
        14,       -1,       -1, G_IBOX            |0x2B00,   /* Object 18 INTEL */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0002, 0x0802, 0x0008, 0x0001,
  SAVE            ,       -1,       -1, G_TEXT            ,   /* Object 19  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[7],
  0x0003, 0x080A, 0x0008, 0x0001,
         0,       -1,       -1, G_BUTTON          ,   /* Object 20 SAVE */
  SELECTABLE|EXIT|LASTOB|FLAGS9|FLAGS10, NORMAL, (LONG)"sichern",
  0x000A, 0x0011, 0x0008, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 TIFF_EXPORT      */
};
