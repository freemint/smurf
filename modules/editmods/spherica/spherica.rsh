/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "SPHERICA.H"

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
{ "r",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "g",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "b",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  " Textur",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, 1, 8, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "xScale %.",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 10, 1,
  "yscale %.",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 10, 1,
  "xOff Pixel",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 11, 1,
  "yOff Pixel",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 11, 1,
  "001000",
  "______",
  "999999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 7,
  "001000",
  "______",
  "999999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 7,
  "00000",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "00000",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "->Bump",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 1,
  " rauh",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, 1, 6, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  " Transparenz",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, 1, 13, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "1000",
  "_.___",
  "9999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 6,
  "Idx",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 4, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "xScale %.",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 10, 1,
  "yscale %.",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 10, 1,
  "xOff Pixel",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 11, 1,
  "yOff Pixel",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 11, 1,
  "000400",
  "______",
  "999999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 7,
  "000400",
  "______",
  "999999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 7, 7,
  "00000",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "00000",
  "_____",
  "99999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 6, 6,
  "Soft (1-3)",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 11, 1,
  "1",
  "_",
  "9",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 2, 2,
  "Persp.",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 7, 1,
  " Bumpmapping",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, 1, 13, 1,
  "0030",
  "X: ____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 8,
  "0030",
  "Y: ____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 8,
  "0030",
  "Z: ____",
  "9999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 8,
  " Glanzpunkt ",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, 1, 13, 1,
  " Ambient",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, 1, 9, 1,
  "An",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1180, 0, -1, 3, 1,
  "Bumpmapping",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 1,
  "Textur",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 7, 1,
  "Roughness",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 10, 1,
  "Transparenz",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 12, 1,
  "Parameter",
  "\0",
  "\0",
  SMALL, 0, TE_RIGHT, 0x1180, 0, -1, 10, 1,
  "r",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "g",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "b",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Str",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 4, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Pow",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 4, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Filt",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 5, 1,
  "_____",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  " Kugel",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, 1, 7, 1,
  "Radius",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1180, 0, 1, 7, 1,
  "Preview",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, 1, 8, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 SPH_MAIN ****************************************************/
        -1,        1,      110, G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0044, 0x0818,
  TEXTURE_BOX     , _TIT            , KB_A            , G_BOX             ,   /* Object 1  */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0402, 0x0001, 0x0010, 0x000B,
  KR_F            ,       -1,       -1, G_TEXT            ,   /* Object 2 _TIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0402, 0x0800, 0x0600, 0x0800,
  KR_A            , KR_R            , KR_R            , G_BOX             |0x1500,   /* Object 3 KR_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0402, 0x0001, 0x0001, 0x0108,
  KR_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 4 KR_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
         6,       -1,       -1, G_BOXTEXT         ,   /* Object 5 KR_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[1],
  0x0001, 0x0809, 0x0603, 0x0D00,
  KG_F            ,       -1,       -1, G_TEXT            ,   /* Object 6  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0407, 0x0800, 0x0600, 0x0800,
  KG_A            , KG_R            , KG_R            , G_BOX             |0x1500,   /* Object 7 KG_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0407, 0x0001, 0x0001, 0x0108,
  KG_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 8 KG_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        10,       -1,       -1, G_BOXTEXT         ,   /* Object 9 KG_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[3],
  0x0006, 0x0809, 0x0603, 0x0D00,
  KB_F            ,       -1,       -1, G_TEXT            ,   /* Object 10  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x040C, 0x0800, 0x0600, 0x0800,
  KB_A            , KB_R            , KB_R            , G_BOX             |0x1500,   /* Object 11 KB_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x040C, 0x0001, 0x0001, 0x0108,
  KB_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 12 KB_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
         1,       -1,       -1, G_BOXTEXT         ,   /* Object 13 KB_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[5],
  0x000B, 0x0809, 0x0603, 0x0D00,
  ROUGH_BOX       ,       15,       28, G_BOX             ,   /* Object 14 TEXTURE_BOX */
  HIDETREE|FLAGS9|FLAGS10, NORMAL, (LONG)0x00011100L,
  0x041C, 0x0800, 0x0010, 0x080B,
  TEXT_A          ,       -1,       -1, G_BOXTEXT         ,   /* Object 15  */
  FLAGS9, NORMAL, (LONG)&rs_tedinfo[6],
  0x0000, 0x0000, 0x0010, 0x0800,
  TEXT_F          ,       -1,       -1, G_BOXTEXT         ,   /* Object 16 TEXT_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[7],
  0x0001, 0x000A, 0x0603, 0x0D00,
        19, TEXT_R          , TEXT_R          , G_BOX             ,   /* Object 17 TEXT_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0402, 0x0801, 0x0001, 0x0108,
  TEXT_F          ,       -1,       -1, G_BOX             |0x1500,   /* Object 18 TEXT_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        20,       -1,       -1, G_TEXT            ,   /* Object 19  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0006, 0x0801, 0x0606, 0x0800,
        21,       -1,       -1, G_TEXT            ,   /* Object 20  */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0406, 0x0003, 0x0606, 0x0800,
        22,       -1,       -1, G_TEXT            ,   /* Object 21  */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0006, 0x0005, 0x0407, 0x0800,
  TEX_XSC         ,       -1,       -1, G_TEXT            ,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0006, 0x0806, 0x0407, 0x0800,
  TEX_YSC         ,       -1,       -1, G_FTEXT           ,   /* Object 23 TEX_XSC */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[12],
  0x0008, 0x0002, 0x0006, 0x0001,
  TEX_XOFF        ,       -1,       -1, G_FTEXT           ,   /* Object 24 TEX_YSC */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[13],
  0x0008, 0x0803, 0x0006, 0x0001,
  TEX_YOFF        ,       -1,       -1, G_FTEXT           ,   /* Object 25 TEX_XOFF */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[14],
  0x0009, 0x0805, 0x0005, 0x0001,
        27,       -1,       -1, G_FTEXT           ,   /* Object 26 TEX_YOFF */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[15],
  0x0009, 0x0007, 0x0005, 0x0001,
        28,       -1,       -1, G_TEXT            ,   /* Object 27  */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[16],
  0x0009, 0x0808, 0x0006, 0x0001,
  TEXTURE_BOX     ,       -1,       -1, G_IBOX            |0x2C00,   /* Object 28  */
  SELECTABLE, NORMAL, (LONG)0x00FF1100L,
  0x0406, 0x0808, 0x0009, 0x0001,
  TRANSP_BOX      ,       30, ROUGH_F         , G_BOX             ,   /* Object 29 ROUGH_BOX */
  HIDETREE|FLAGS9|FLAGS10, NORMAL, (LONG)0x00011100L,
  0x041C, 0x0800, 0x0010, 0x080B,
  ROUGH_A         ,       -1,       -1, G_BOXTEXT         ,   /* Object 30  */
  FLAGS9, NORMAL, (LONG)&rs_tedinfo[17],
  0x0000, 0x0000, 0x0010, 0x0800,
  ROUGH_F         ,       -1,       -1, G_BOXTEXT         ,   /* Object 31 ROUGH_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[18],
  0x0001, 0x000A, 0x0603, 0x0D00,
  ROUGH_BOX       , ROUGH_R         , ROUGH_R         , G_BOX             ,   /* Object 32 ROUGH_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0402, 0x0801, 0x0001, 0x0108,
  ROUGH_F         ,       -1,       -1, G_BOX             |0x1500,   /* Object 33 ROUGH_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
  BUMP_BOX        ,       35,       40, G_BOX             ,   /* Object 34 TRANSP_BOX */
  HIDETREE|FLAGS9|FLAGS10, NORMAL, (LONG)0x00011100L,
  0x041C, 0x0800, 0x0010, 0x080B,
  TRANSP_A        ,       -1,       -1, G_BOXTEXT         ,   /* Object 35  */
  FLAGS9, NORMAL, (LONG)&rs_tedinfo[19],
  0x0000, 0x0000, 0x0010, 0x0800,
  TRANSP_F        ,       -1,       -1, G_BOXTEXT         ,   /* Object 36 TRANSP_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[20],
  0x0001, 0x000A, 0x0603, 0x0D00,
  TRAN_IDX        , TRANSP_R        , TRANSP_R        , G_BOX             ,   /* Object 37 TRANSP_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0402, 0x0801, 0x0001, 0x0108,
  TRANSP_F        ,       -1,       -1, G_BOX             |0x1500,   /* Object 38 TRANSP_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        40,       -1,       -1, G_FTEXT           ,   /* Object 39 TRAN_IDX */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[21],
  0x0408, 0x0002, 0x0005, 0x0001,
  TRANSP_BOX      ,       -1,       -1, G_TEXT            ,   /* Object 40  */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0007, 0x0801, 0x0202, 0x0800,
        58, BUMP_A          ,       57, G_BOX             ,   /* Object 41 BUMP_BOX */
  FLAGS10, NORMAL, (LONG)0x00011100L,
  0x041C, 0x0800, 0x0010, 0x080B,
  BUMP_F          ,       -1,       -1, G_BOXTEXT         ,   /* Object 42 BUMP_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[23],
  0x0001, 0x000A, 0x0603, 0x0D00,
        45, BUMP_R          , BUMP_R          , G_BOX             |0x1500,   /* Object 43 BUMP_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0402, 0x0801, 0x0001, 0x0108,
  BUMP_F          ,       -1,       -1, G_BOX             |0x1500,   /* Object 44 BUMP_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        46,       -1,       -1, G_TEXT            ,   /* Object 45  */
  NONE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0405, 0x0801, 0x0606, 0x0800,
        47,       -1,       -1, G_TEXT            ,   /* Object 46  */
  NONE, NORMAL, (LONG)&rs_tedinfo[25],
  0x0405, 0x0003, 0x0606, 0x0800,
        48,       -1,       -1, G_TEXT            ,   /* Object 47  */
  NONE, NORMAL, (LONG)&rs_tedinfo[26],
  0x0405, 0x0005, 0x0407, 0x0800,
  BUMP_XSC        ,       -1,       -1, G_TEXT            ,   /* Object 48  */
  NONE, NORMAL, (LONG)&rs_tedinfo[27],
  0x0405, 0x0806, 0x0407, 0x0800,
  BUMP_YSC        ,       -1,       -1, G_FTEXT           ,   /* Object 49 BUMP_XSC */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[28],
  0x0407, 0x0002, 0x0006, 0x0001,
  BUMP_XOFF       ,       -1,       -1, G_FTEXT           ,   /* Object 50 BUMP_YSC */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[29],
  0x0407, 0x0803, 0x0006, 0x0001,
  BUMP_YOFF       ,       -1,       -1, G_FTEXT           ,   /* Object 51 BUMP_XOFF */
  EDITABLE|FLAGS10, DISABLED, (LONG)&rs_tedinfo[30],
  0x0408, 0x0805, 0x0005, 0x0001,
        53,       -1,       -1, G_FTEXT           ,   /* Object 52 BUMP_YOFF */
  EDITABLE|FLAGS10, DISABLED, (LONG)&rs_tedinfo[31],
  0x0408, 0x0007, 0x0005, 0x0001,
        54,       -1,       -1, G_TEXT            ,   /* Object 53  */
  NONE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0006, 0x0808, 0x0407, 0x0800,
        55,       -1,       -1, G_FTEXT           ,   /* Object 54  */
  EDITABLE|FLAGS10, DISABLED, (LONG)&rs_tedinfo[33],
  0x040C, 0x0009, 0x0001, 0x0001,
  BUMP_PERSP      ,       -1,       -1, G_TEXT            ,   /* Object 55  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[34],
  0x0409, 0x000A, 0x0006, 0x0001,
        57,       -1,       -1, G_IBOX            |0x2C00,   /* Object 56 BUMP_PERSP */
  SELECTABLE|FLAGS10, SELECTED|DISABLED, (LONG)0x00FF1100L,
  0x0007, 0x000A, 0x0408, 0x0001,
  BUMP_BOX        ,       -1,       -1, G_BOXTEXT         ,   /* Object 57  */
  FLAGS9, NORMAL, (LONG)&rs_tedinfo[35],
  0x0000, 0x0000, 0x0010, 0x0800,
        62, XRAD            ,       61, G_BOX             ,   /* Object 58  */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0412, 0x0001, 0x000A, 0x000B,
        60,       -1,       -1, G_FTEXT           ,   /* Object 59 XRAD */
  EDITABLE|FLAGS10, DISABLED, (LONG)&rs_tedinfo[36],
  0x0401, 0x0001, 0x0007, 0x0001,
        61,       -1,       -1, G_FTEXT           ,   /* Object 60  */
  EDITABLE|FLAGS10, DISABLED, (LONG)&rs_tedinfo[37],
  0x0401, 0x0802, 0x0007, 0x0001,
        58,       -1,       -1, G_FTEXT           ,   /* Object 61  */
  EDITABLE|FLAGS10, DISABLED, (LONG)&rs_tedinfo[38],
  0x0401, 0x0004, 0x0007, 0x0001,
  LOS             ,       -1,       -1, G_BOXTEXT         ,   /* Object 62  */
  FLAGS9, NORMAL, (LONG)&rs_tedinfo[39],
  0x0402, 0x080C, 0x0010, 0x0800,
        64,       -1,       -1, G_BUTTON          ,   /* Object 63 LOS */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Los!",
  0x0039, 0x0816, 0x0007, 0x0001,
  PREV_BOX        ,       -1,       -1, G_BOXTEXT         ,   /* Object 64  */
  FLAGS9, NORMAL, (LONG)&rs_tedinfo[40],
  0x0412, 0x080C, 0x0010, 0x0800,
        66,       -1,       -1, G_BOX             ,   /* Object 65 PREV_BOX */
  FLAGS9|FLAGS10, SELECTED, (LONG)0x00011101L,
  0x0024, 0x000D, 0x000C, 0x0006,
        67,       -1,       -1, G_BUTTON          ,   /* Object 66  */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, DISABLED, (LONG)"laden",
  0x0424, 0x0014, 0x040B, 0x0001,
        68,       -1,       -1, G_BUTTON          ,   /* Object 67  */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, DISABLED, (LONG)"sichern",
  0x0424, 0x0016, 0x040B, 0x0001,
        69,       -1,       -1, G_TEXT            ,   /* Object 68  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[41],
  0x0131, 0x0D00, 0x0401, 0x0800,
  BUMP_RADIO      ,       -1,       -1, G_TEXT            ,   /* Object 69  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[42],
  0x0436, 0x0801, 0x000B, 0x0001,
        71,       -1,       -1, G_IBOX            |0x2B00,   /* Object 70 BUMP_RADIO */
  SELECTABLE|RBUTTON|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0034, 0x0801, 0x000E, 0x0001,
        72,       -1,       -1, G_TEXT            ,   /* Object 71  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[43],
  0x0436, 0x0003, 0x0006, 0x0001,
        73,       -1,       -1, G_TEXT            ,   /* Object 72  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[44],
  0x0436, 0x0804, 0x0009, 0x0001,
  TEXT_RADIO      ,       -1,       -1, G_TEXT            ,   /* Object 73  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[45],
  0x0436, 0x0006, 0x000B, 0x0001,
  ROUGH_RADIO     ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 74 TEXT_RADIO */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0034, 0x0003, 0x0408, 0x0001,
  TRANSP_RADIO    ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 75 ROUGH_RADIO */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0034, 0x0804, 0x040B, 0x0001,
  BUMP_CHECK      ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 76 TRANSP_RADIO */
  SELECTABLE|RBUTTON|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0034, 0x0006, 0x000E, 0x0001,
  TEXT_CHECK      ,       -1,       -1, G_IBOX            |0x2C00,   /* Object 77 BUMP_CHECK */
  SELECTABLE|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0031, 0x0801, 0x0002, 0x0001,
  ROUGH_CHECK     ,       -1,       -1, G_IBOX            |0x2C00,   /* Object 78 TEXT_CHECK */
  SELECTABLE|FLAGS10, DISABLED, (LONG)0x00FF1100L,
  0x0031, 0x0003, 0x0002, 0x0001,
  TRANS_CHECK     ,       -1,       -1, G_IBOX            |0x2C00,   /* Object 79 ROUGH_CHECK */
  SELECTABLE|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0031, 0x0804, 0x0002, 0x0001,
        81,       -1,       -1, G_IBOX            |0x2C00,   /* Object 80 TRANS_CHECK */
  SELECTABLE|FLAGS10, NORMAL, (LONG)0x00FF1100L,
  0x0031, 0x0006, 0x0002, 0x0001,
        82,       -1,       -1, G_TEXT            ,   /* Object 81  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[46],
  0x0034, 0x0D00, 0x0606, 0x0800,
        95,       83, AB_A            , G_BOX             ,   /* Object 82  */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0412, 0x000D, 0x0010, 0x000B,
  AR_F            ,       -1,       -1, G_TEXT            ,   /* Object 83  */
  NONE, NORMAL, (LONG)&rs_tedinfo[47],
  0x0402, 0x0800, 0x0600, 0x0800,
  AR_A            , AR_R            , AR_R            , G_BOX             |0x1500,   /* Object 84 AR_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0402, 0x0001, 0x0001, 0x0108,
  AR_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 85 AR_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        87,       -1,       -1, G_BOXTEXT         ,   /* Object 86 AR_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[48],
  0x0001, 0x0809, 0x0603, 0x0D00,
  AG_F            ,       -1,       -1, G_TEXT            ,   /* Object 87  */
  NONE, NORMAL, (LONG)&rs_tedinfo[49],
  0x0407, 0x0800, 0x0600, 0x0600,
  AG_A            , AG_R            , AG_R            , G_BOX             |0x1500,   /* Object 88 AG_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0407, 0x0001, 0x0001, 0x0108,
  AG_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 89 AG_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        91,       -1,       -1, G_BOXTEXT         ,   /* Object 90 AG_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[50],
  0x0006, 0x0809, 0x0603, 0x0D00,
  AB_F            ,       -1,       -1, G_TEXT            ,   /* Object 91  */
  NONE, NORMAL, (LONG)&rs_tedinfo[51],
  0x040C, 0x0800, 0x0600, 0x0800,
  AB_A            , AB_R            , AB_R            , G_BOX             |0x1500,   /* Object 92 AB_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x040C, 0x0001, 0x0001, 0x0108,
  AB_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 93 AB_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        82,       -1,       -1, G_BOXTEXT         ,   /* Object 94 AB_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[52],
  0x000B, 0x0809, 0x0603, 0x0D00,
       108,       96, SFLT_A          , G_BOX             ,   /* Object 95  */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0402, 0x000D, 0x0010, 0x000B,
  SSTR_F          ,       -1,       -1, G_TEXT            ,   /* Object 96  */
  NONE, NORMAL, (LONG)&rs_tedinfo[53],
  0x0002, 0x0800, 0x0202, 0x0600,
  SSTR_A          , SSTR_R          , SSTR_R          , G_BOX             |0x1500,   /* Object 97 SSTR_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0402, 0x0001, 0x0001, 0x0108,
  SSTR_F          ,       -1,       -1, G_BOX             |0x1500,   /* Object 98 SSTR_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
       100,       -1,       -1, G_BOXTEXT         ,   /* Object 99 SSTR_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[54],
  0x0001, 0x0809, 0x0603, 0x0D00,
  SPOW_F          ,       -1,       -1, G_TEXT            ,   /* Object 100  */
  NONE, NORMAL, (LONG)&rs_tedinfo[55],
  0x0007, 0x0800, 0x0202, 0x0800,
  SPOW_A          , SPOW_R          , SPOW_R          , G_BOX             |0x1500,   /* Object 101 SPOW_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0407, 0x0001, 0x0001, 0x0108,
  SPOW_F          ,       -1,       -1, G_BOX             |0x1500,   /* Object 102 SPOW_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
       104,       -1,       -1, G_BOXTEXT         ,   /* Object 103 SPOW_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[56],
  0x0006, 0x0809, 0x0603, 0x0D00,
  SFLT_F          ,       -1,       -1, G_TEXT            ,   /* Object 104  */
  NONE, NORMAL, (LONG)&rs_tedinfo[57],
  0x040B, 0x0800, 0x0003, 0x0800,
  SFLT_A          , SFLT_R          , SFLT_R          , G_BOX             |0x1500,   /* Object 105 SFLT_F */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x040C, 0x0001, 0x0001, 0x0108,
  SFLT_F          ,       -1,       -1, G_BOX             |0x1500,   /* Object 106 SFLT_R */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        95,       -1,       -1, G_BOXTEXT         ,   /* Object 107 SFLT_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[58],
  0x000B, 0x0809, 0x0603, 0x0D00,
       109,       -1,       -1, G_BOXTEXT         ,   /* Object 108  */
  FLAGS9, NORMAL, (LONG)&rs_tedinfo[59],
  0x0402, 0x0800, 0x0010, 0x0800,
       110,       -1,       -1, G_BOXTEXT         ,   /* Object 109  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[60],
  0x0412, 0x0800, 0x000A, 0x0800,
         0,       -1,       -1, G_BOXTEXT         ,   /* Object 110  */
  LASTOB|FLAGS9, NORMAL, (LONG)&rs_tedinfo[61],
  0x0024, 0x080C, 0x000C, 0x0800
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 SPH_MAIN         */
};
