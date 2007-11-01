/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "MAGIC.H"

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
{ "100",
  "Maximale H\224he: __._%",
  "999",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 4, 21,
  "Berechnungsart f\201r",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 19, 1,
  "Objektmuster",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 13, 1,
  "Pixelmuster",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 1,
  "beide",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "<<<O",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "<<O>",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "<O>>",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "O>>>",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "<<<|",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "<<|>",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "<|>>",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "|>>>",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "<<<<",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  ">>>>",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 1,
  "Spiegeln",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 9, 1,
  "Vergr\224\236ern durch",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 17, 1,
  "Verschieben",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 1,
  "0100",
  "fest auf: ____",
  "9999",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 5, 15,
  "Spaltenbreite",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 14, 1,
  "= Texturbreite",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 15, 1,
  "Interpolieren",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 14, 1,
  "Speicher sparen",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 16, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 SM_MAGIC ****************************************************/
        -1, HOEHE_SW        , SPEICHER        , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0031, 0x080C,
         2,       -1,       -1, G_FTEXT           ,   /* Object 1 HOEHE_SW */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[0],
  0x0005, 0x0009, 0x0014, 0x0001,
  GO              ,        3, RI3             , G_BOX             ,   /* Object 2  */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0800, 0x001C, 0x0807,
         4,       -1,       -1, G_TEXT            ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0005, 0x0800, 0x0012, 0x0001,
         5,       -1,       -1, G_TEXT            ,   /* Object 4  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[2],
  0x0001, 0x0801, 0x000C, 0x0001,
         6,       -1,       -1, G_TEXT            ,   /* Object 5  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[3],
  0x0010, 0x0801, 0x000B, 0x0001,
         7,       -1,       -1, G_TEXT            ,   /* Object 6  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[4],
  0x000B, 0x0003, 0x0005, 0x0001,
  RI10            ,       -1,       -1, G_TEXT            ,   /* Object 7  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[5],
  0x0004, 0x0003, 0x0006, 0x0001,
         9,       -1,       -1, G_IBOX            |0x2B00,   /* Object 8 RI10 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0003, 0x0008, 0x0001,
  RI7             ,       -1,       -1, G_TEXT            ,   /* Object 9  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[6],
  0x0004, 0x0004, 0x0006, 0x0001,
        11,       -1,       -1, G_IBOX            |0x2B00,   /* Object 10 RI7 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0004, 0x0008, 0x0001,
  RI8             ,       -1,       -1, G_TEXT            ,   /* Object 11  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[7],
  0x0004, 0x0005, 0x0006, 0x0001,
        13,       -1,       -1, G_IBOX            |0x2B00,   /* Object 12 RI8 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0005, 0x0008, 0x0001,
  RI9             ,       -1,       -1, G_TEXT            ,   /* Object 13  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[8],
  0x0004, 0x0006, 0x0006, 0x0001,
        15,       -1,       -1, G_IBOX            |0x2B00,   /* Object 14 RI9 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0006, 0x0008, 0x0001,
  RI6             ,       -1,       -1, G_TEXT            ,   /* Object 15  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[9],
  0x0016, 0x0003, 0x0006, 0x0001,
        17,       -1,       -1, G_IBOX            |0x2B00,   /* Object 16 RI6 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0013, 0x0003, 0x0008, 0x0001,
  RI2             ,       -1,       -1, G_TEXT            ,   /* Object 17  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[10],
  0x0016, 0x0004, 0x0006, 0x0001,
        19,       -1,       -1, G_IBOX            |0x2B00,   /* Object 18 RI2 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0013, 0x0004, 0x0008, 0x0001,
  RI4             ,       -1,       -1, G_TEXT            ,   /* Object 19  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[11],
  0x0016, 0x0005, 0x0006, 0x0001,
        21,       -1,       -1, G_IBOX            |0x2B00,   /* Object 20 RI4 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0013, 0x0005, 0x0008, 0x0001,
  RI5             ,       -1,       -1, G_TEXT            ,   /* Object 21  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[12],
  0x0016, 0x0006, 0x0006, 0x0001,
        23,       -1,       -1, G_IBOX            |0x2B00,   /* Object 22 RI5 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0013, 0x0006, 0x0008, 0x0001,
  RI1             ,       -1,       -1, G_TEXT            ,   /* Object 23  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[13],
  0x000D, 0x0804, 0x0006, 0x0001,
        25,       -1,       -1, G_IBOX            |0x2B00,   /* Object 24 RI1 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x000A, 0x0804, 0x0008, 0x0001,
  RI3             ,       -1,       -1, G_TEXT            ,   /* Object 25  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[14],
  0x000D, 0x0805, 0x0006, 0x0001,
         2,       -1,       -1, G_IBOX            |0x2B00,   /* Object 26 RI3 */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF1100L,
  0x000A, 0x0805, 0x0008, 0x0001,
  SHORTEN         ,       -1,       -1, G_BUTTON          ,   /* Object 27 GO */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"go magic!",
  0x0025, 0x000A, 0x000A, 0x0801,
        29,       -1,       -1, G_IBOX            |0x2C00,   /* Object 28 SHORTEN */
  SELECTABLE, SELECTED, (LONG)0x00FF1100L,
  0x0002, 0x0009, 0x0011, 0x0001,
        35,       30, SPOSCH2         , G_BOX             ,   /* Object 29  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x001E, 0x0005, 0x0012, 0x0803,
  SPOSCH1         ,       -1,       -1, G_TEXT            ,   /* Object 30  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[15],
  0x0004, 0x0801, 0x000A, 0x0001,
        32,       -1,       -1, G_IBOX            |0x2B00,   /* Object 31 SPOSCH1 */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF1100L,
  0x0001, 0x0801, 0x000B, 0x0001,
        33,       -1,       -1, G_TEXT            ,   /* Object 32  */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0001, 0x0000, 0x0010, 0x0001,
  SPOSCH2         ,       -1,       -1, G_TEXT            ,   /* Object 33  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[17],
  0x0004, 0x0802, 0x000C, 0x0001,
        29,       -1,       -1, G_IBOX            |0x2B00,   /* Object 34 SPOSCH2 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0802, 0x000E, 0x0001,
        41, ANZSPALT        , STRF1           , G_BOX             ,   /* Object 35  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x001E, 0x0800, 0x0013, 0x0004,
  STRF2           ,       -1,       -1, G_FTEXT           ,   /* Object 36 ANZSPALT */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[18],
  0x0004, 0x0802, 0x000F, 0x0001,
        38,       -1,       -1, G_IBOX            |0x2B00,   /* Object 37 STRF2 */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF1100L,
  0x0001, 0x0802, 0x000D, 0x0001,
        39,       -1,       -1, G_TEXT            ,   /* Object 38  */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0001, 0x0000, 0x000D, 0x0001,
  STRF1           ,       -1,       -1, G_TEXT            ,   /* Object 39  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[20],
  0x0004, 0x0801, 0x000F, 0x0001,
        35,       -1,       -1, G_IBOX            |0x2B00,   /* Object 40 STRF1 */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF1100L,
  0x0001, 0x0801, 0x0011, 0x0001,
  INTERPOL        ,       -1,       -1, G_TEXT            ,   /* Object 41  */
  FLAGS10, DISABLED, (LONG)&rs_tedinfo[21],
  0x0005, 0x000A, 0x000D, 0x0001,
        43,       -1,       -1, G_IBOX            |0x2C00,   /* Object 42 INTERPOL */
  SELECTABLE, DISABLED, (LONG)0x00FF1100L,
  0x0002, 0x000A, 0x000E, 0x0001,
  SPEICHER        ,       -1,       -1, G_TEXT            ,   /* Object 43  */
  FLAGS10, DISABLED, (LONG)&rs_tedinfo[22],
  0x0005, 0x000B, 0x000F, 0x0001,
         0,       -1,       -1, G_IBOX            |0x2C00,   /* Object 44 SPEICHER */
  SELECTABLE|LASTOB, DISABLED, (LONG)0x00FF1100L,
  0x0002, 0x000B, 0x000E, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 SM_MAGIC         */
};
