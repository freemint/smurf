/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "TEXTURE.H"

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
{ "XXXXXXXX.MAP",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 13, 1,
  "Map",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 4, 1,
  "Map",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 4, 1,
  "00000",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "11111",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "22222",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "XXXXXXXX.SMT",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 13, 1,
  "Textur",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 7, 1,
  "Textur",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 7, 1,
  "55555",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Turb.",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 6, 1,
  "Turb.",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 6, 1,
  "44444",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "Y-Scale",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 8, 1,
  "Y-Scale",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 8, 1,
  "00",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 3, 1,
  "Farben",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 7, 1,
  "Farben",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 7, 1,
  " scharf ",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 9, 1,
  "Map",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 4, 1,
  "Map",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 4, 1,
  "R",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 2, 1,
  "R",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 2, 1,
  "G",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 2, 1,
  "G",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 2, 1,
  "B",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 2, 1,
  "B",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 2, 1,
  " keine ",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 8, 1,
  "Turbulenz",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 10, 1,
  "Turbulenz",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 10, 1,
  "33333",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 6, 1,
  "X-Scale",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 8, 1,
  "X-Scale",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 8, 1,
  " normal ",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 9, 1,
  "Modus",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1000, 0, -1, 6, 1,
  "Modus",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 6, 1,
  "keine",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 6, 1,
  "Zoom",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 1,
  "Sinus",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 6, 1,
  "Rotate",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1,
  "16 Bit",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1,
  "24 Bit",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1,
  "scharf",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1,
  "linear",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1,
  "soft",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 5, 1,
  "Xtrasoft",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 9, 1,
  "normal",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1,
  "Holz x",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1,
  "Holz y",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1,
  "Marmor",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, 0, 7, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 TEX_MAIN ****************************************************/
        -1,        1,       62, G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0000, 0x0000, 0x0438, 0x0019,
  LOADMAP         ,       -1,       -1, G_BOX             ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00FF1141L,
  0x031E, 0x0F11, 0x040C, 0x0406,
  MAPNAME         ,       -1,       -1, G_BUTTON          ,   /* Object 2 LOADMAP */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Laden",
  0x0610, 0x0112, 0x0008, 0x0001,
         4,       -1,       -1, G_TEXT            ,   /* Object 3 MAPNAME */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0602, 0x0112, 0x000C, 0x0001,
         5,       -1,       -1, G_TEXT            ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0602, 0x0911, 0x0202, 0x0800,
  RS_PAR          ,       -1,       -1, G_TEXT            ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0502, 0x0811, 0x0202, 0x0800,
  RS_A            , RED_SLIDER      , RED_SLIDER      , G_BOX             ,   /* Object 6 RS_PAR */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0703, 0x0502, 0x0001, 0x0108,
  RS_PAR          ,       -1,       -1, G_BOX             |0x1500,   /* Object 7 RED_SLIDER */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1172L,
  0x0000, 0x0907, 0x0001, 0x0800,
  GS_PAR          ,       -1,       -1, G_BOXTEXT         ,   /* Object 8 RS_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[3],
  0x0402, 0x0F0A, 0x0603, 0x0D00,
  GS_A            , GREEN_SLIDER    , GREEN_SLIDER    , G_BOX             ,   /* Object 9 GS_PAR */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0408, 0x0502, 0x0001, 0x0108,
  GS_PAR          ,       -1,       -1, G_BOX             |0x1500,   /* Object 10 GREEN_SLIDER */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1173L,
  0x0000, 0x0907, 0x0001, 0x0800,
  BS_PAR          ,       -1,       -1, G_BOXTEXT         ,   /* Object 11 GS_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[4],
  0x0107, 0x0F0A, 0x0603, 0x0D00,
  BS_A            , BLUE_SLIDER     , BLUE_SLIDER     , G_BOX             ,   /* Object 12 BS_PAR */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x000D, 0x0502, 0x0001, 0x0108,
  BS_PAR          ,       -1,       -1, G_BOX             |0x1500,   /* Object 13 BLUE_SLIDER */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1174L,
  0x0000, 0x0907, 0x0001, 0x0800,
  LOADTEX         ,       -1,       -1, G_BOXTEXT         ,   /* Object 14 BS_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[5],
  0x050B, 0x0F0A, 0x0603, 0x0D00,
  TEXNAME         ,       -1,       -1, G_BUTTON          ,   /* Object 15 LOADTEX */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Laden",
  0x0510, 0x0F14, 0x0008, 0x0001,
        17,       -1,       -1, G_TEXT            ,   /* Object 16 TEXNAME */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0502, 0x0F14, 0x000C, 0x0001,
        18,       -1,       -1, G_TEXT            ,   /* Object 17  */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0502, 0x0714, 0x0404, 0x0800,
  SAVETEX         ,       -1,       -1, G_TEXT            ,   /* Object 18  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0402, 0x0614, 0x0404, 0x0800,
  START           ,       -1,       -1, G_BUTTON          ,   /* Object 19 SAVETEX */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Sichern",
  0x0510, 0x0E16, 0x0008, 0x0001,
  TS_PAR          ,       -1,       -1, G_BUTTON          ,   /* Object 20 START */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Start",
  0x002E, 0x0017, 0x0008, 0x0001,
  TS_A            , TURB_SLIDER     , TURB_SLIDER     , G_BOX             ,   /* Object 21 TS_PAR */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0233, 0x0902, 0x0001, 0x0108,
  TS_PAR          ,       -1,       -1, G_BOX             |0x1500,   /* Object 22 TURB_SLIDER */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        24,       -1,       -1, G_BOXTEXT         ,   /* Object 23 TS_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[9],
  0x0731, 0x010B, 0x0603, 0x0D00,
        25,       -1,       -1, G_TEXT            ,   /* Object 24  */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0032, 0x0A01, 0x0603, 0x0B00,
  YS_PAR          ,       -1,       -1, G_TEXT            ,   /* Object 25  */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0731, 0x0901, 0x0603, 0x0B00,
  YS_A            , YS_SLIDER       , YS_SLIDER       , G_BOX             ,   /* Object 26 YS_PAR */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x002D, 0x0902, 0x0001, 0x0108,
  YS_PAR          ,       -1,       -1, G_BOX             |0x1500,   /* Object 27 YS_SLIDER */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        29,       -1,       -1, G_BOXTEXT         ,   /* Object 28 YS_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[12],
  0x052B, 0x010B, 0x0603, 0x0D00,
        30,       -1,       -1, G_TEXT            ,   /* Object 29  */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x002B, 0x0A01, 0x0205, 0x0B00,
        31,       -1,       -1, G_TEXT            ,   /* Object 30  */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x072A, 0x0901, 0x0205, 0x0B00,
  COL_UP          ,       -1,       -1, G_BOX             ,   /* Object 31  */
  NONE, NORMAL, (LONG)0x00001141L,
  0x0215, 0x0504, 0x0004, 0x0002,
  COL_DN          ,       -1,       -1, G_BOXCHAR         |0x1500,   /* Object 32 COL_UP */
  FLAGS9|FLAGS10, NORMAL, (LONG) ((LONG)'\001' << 24)|0x00001100L,
  0x0215, 0x0503, 0x0004, 0x0001,
  COL_NUMBER      ,       -1,       -1, G_BOXCHAR         |0x1500,   /* Object 33 COL_DN */
  FLAGS9|FLAGS10, NORMAL, (LONG) ((LONG)'\002' << 24)|0x00001100L,
  0x0215, 0x0506, 0x0004, 0x0001,
        35,       -1,       -1, G_TEXT            ,   /* Object 34 COL_NUMBER */
  NONE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0212, 0x0D04, 0x0002, 0x0001,
        36,       -1,       -1, G_TEXT            ,   /* Object 35  */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0412, 0x0502, 0x0404, 0x0B00,
  MODE_PB         ,       -1,       -1, G_TEXT            ,   /* Object 36  */
  NONE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0312, 0x0402, 0x0404, 0x0B00,
  MODE_CB         ,       -1,       -1, G_BOXTEXT         ,   /* Object 37 MODE_PB */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, SHADOWED, (LONG)&rs_tedinfo[18],
  0x0629, 0x0F0D, 0x030A, 0x0001,
        39,       -1,       -1, G_IBOX            |0x2D00,   /* Object 38 MODE_CB */
  SELECTABLE, NORMAL, (LONG)0x00012100L,
  0x0434, 0x0C0D, 0x0003, 0x0801,
        40,       -1,       -1, G_TEXT            ,   /* Object 39  */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x0329, 0x000D, 0x0202, 0x0B00,
        41,       -1,       -1, G_TEXT            ,   /* Object 40  */
  NONE, NORMAL, (LONG)&rs_tedinfo[20],
  0x0229, 0x0F0C, 0x0202, 0x0B00,
        42,       -1,       -1, G_TEXT            ,   /* Object 41  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0004, 0x0601, 0x0600, 0x0B00,
        43,       -1,       -1, G_TEXT            ,   /* Object 42  */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0703, 0x0501, 0x0600, 0x0B00,
        44,       -1,       -1, G_TEXT            ,   /* Object 43  */
  NONE, NORMAL, (LONG)&rs_tedinfo[23],
  0x0608, 0x0601, 0x0600, 0x0B00,
        45,       -1,       -1, G_TEXT            ,   /* Object 44  */
  NONE, NORMAL, (LONG)&rs_tedinfo[24],
  0x0508, 0x0501, 0x0600, 0x0B00,
        46,       -1,       -1, G_TEXT            ,   /* Object 45  */
  NONE, NORMAL, (LONG)&rs_tedinfo[25],
  0x020D, 0x0601, 0x0600, 0x0B00,
  LOAD_COLS       ,       -1,       -1, G_TEXT            ,   /* Object 46  */
  NONE, NORMAL, (LONG)&rs_tedinfo[26],
  0x010D, 0x0501, 0x0600, 0x0B00,
  SAVE_COLS       ,       -1,       -1, G_BUTTON          ,   /* Object 47 LOAD_COLS */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Laden",
  0x0312, 0x0508, 0x000A, 0x0001,
        49,       -1,       -1, G_BUTTON          ,   /* Object 48 SAVE_COLS */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Sichern",
  0x0312, 0x050A, 0x000A, 0x0001,
  TURB_PB         ,       -1,       -1, G_BOX             ,   /* Object 49  */
  FLAGS9, SELECTED, (LONG)0x00000100L,
  0x0202, 0x0F10, 0x0334, 0x0100,
  TURB_CB         ,       -1,       -1, G_BOXTEXT         ,   /* Object 50 TURB_PB */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, SHADOWED, (LONG)&rs_tedinfo[27],
  0x011A, 0x0F0D, 0x030A, 0x0001,
        52,       -1,       -1, G_IBOX            |0x2D00,   /* Object 51 TURB_CB */
  SELECTABLE, NORMAL, (LONG)0x00012100L,
  0x0724, 0x0C0D, 0x0003, 0x0801,
        53,       -1,       -1, G_TEXT            ,   /* Object 52  */
  NONE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0519, 0x000D, 0x0606, 0x0B00,
  XS_PAR          ,       -1,       -1, G_TEXT            ,   /* Object 53  */
  NONE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0419, 0x0F0C, 0x0606, 0x0B00,
  XS_A            , XS_SLIDER       , XS_SLIDER       , G_BOX             ,   /* Object 54 XS_PAR */
  FLAGS9, SELECTED, (LONG)0x00FE8178L,
  0x0327, 0x0902, 0x0001, 0x0108,
  XS_PAR          ,       -1,       -1, G_BOX             |0x1500,   /* Object 55 XS_SLIDER */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
        57,       -1,       -1, G_BOXTEXT         ,   /* Object 56 XS_A */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[30],
  0x0026, 0x010B, 0x0603, 0x0D00,
        58,       -1,       -1, G_TEXT            ,   /* Object 57  */
  NONE, NORMAL, (LONG)&rs_tedinfo[31],
  0x0125, 0x0A01, 0x0205, 0x0B00,
        59,       -1,       -1, G_TEXT            ,   /* Object 58  */
  NONE, NORMAL, (LONG)&rs_tedinfo[32],
  0x0025, 0x0901, 0x0205, 0x0B00,
        60,       -1,       -1, G_BOXTEXT         ,   /* Object 59  */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, SHADOWED, (LONG)&rs_tedinfo[33],
  0x010A, 0x000E, 0x030A, 0x0001,
        61,       -1,       -1, G_IBOX            |0x2D00,   /* Object 60  */
  SELECTABLE, NORMAL, (LONG)0x00012100L,
  0x0714, 0x0D0D, 0x0003, 0x0801,
        62,       -1,       -1, G_TEXT            ,   /* Object 61  */
  NONE, NORMAL, (LONG)&rs_tedinfo[34],
  0x0509, 0x010D, 0x0603, 0x0B00,
         0,       -1,       -1, G_TEXT            ,   /* Object 62  */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[35],
  0x0409, 0x000D, 0x0603, 0x0B00,
  
  /******** Tree 1 TURB_POPUP ****************************************************/
        -1, TURB_NONE       , TURB_Y          , G_BOX             ,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00011100L,
  0x0000, 0x0000, 0x000B, 0x0A07,
  TURB_ZOOM       ,       -1,       -1, G_BOXTEXT         ,   /* Object 1 TURB_NONE */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[36],
  0x0300, 0x0300, 0x030A, 0x0001,
  TURB_SIN        ,       -1,       -1, G_BOXTEXT         ,   /* Object 2 TURB_ZOOM */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[37],
  0x0300, 0x0701, 0x030A, 0x0001,
  TURB_ROTATE     ,       -1,       -1, G_BOXTEXT         ,   /* Object 3 TURB_SIN */
  FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[38],
  0x0300, 0x0B02, 0x030A, 0x0001,
  TURB_X          ,       -1,       -1, G_BOXTEXT         ,   /* Object 4 TURB_ROTATE */
  FLAGS9|FLAGS10, NORMAL, (LONG)&rs_tedinfo[39],
  0x0300, 0x0F03, 0x030A, 0x0001,
  TURB_Y          ,       -1,       -1, G_BOXTEXT         ,   /* Object 5 TURB_X */
  FLAGS9|FLAGS10, NORMAL, (LONG)&rs_tedinfo[40],
  0x0300, 0x0305, 0x030A, 0x0001,
         0,       -1,       -1, G_BOXTEXT         ,   /* Object 6 TURB_Y */
  LASTOB|FLAGS9|FLAGS10, NORMAL, (LONG)&rs_tedinfo[41],
  0x0300, 0x0706, 0x030A, 0x0001,
  
  /******** Tree 2 MAP_POPUP ****************************************************/
        -1, MAP_SHARP       , MAP_XSOFT       , G_BOX             ,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00011100L,
  0x0000, 0x0000, 0x010B, 0x0205,
  MAP_LIN         ,       -1,       -1, G_BOXTEXT         ,   /* Object 1 MAP_SHARP */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[42],
  0x0300, 0x0300, 0x030A, 0x0001,
  MAP_SOFT        ,       -1,       -1, G_BOXTEXT         ,   /* Object 2 MAP_LIN */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[43],
  0x0300, 0x0701, 0x030A, 0x0001,
  MAP_XSOFT       ,       -1,       -1, G_BOXTEXT         ,   /* Object 3 MAP_SOFT */
  FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[44],
  0x0300, 0x0B02, 0x030A, 0x0001,
         0,       -1,       -1, G_BOXTEXT         ,   /* Object 4 MAP_XSOFT */
  LASTOB|FLAGS9|FLAGS10, NORMAL, (LONG)&rs_tedinfo[45],
  0x0300, 0x0F03, 0x030A, 0x0001,
  
  /******** Tree 3 MODE_POPUP ****************************************************/
        -1,        1,        4, G_BOX             ,   /* Object 0  */
  FLAGS10, SHADOWED, (LONG)0x00011100L,
  0x0000, 0x0000, 0x020B, 0x0B0A,
         2,       -1,       -1, G_BOXTEXT         ,   /* Object 1  */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[46],
  0x0300, 0x0300, 0x030A, 0x0001,
         3,       -1,       -1, G_BOXTEXT         ,   /* Object 2  */
  SELECTABLE|FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[47],
  0x0300, 0x0701, 0x030A, 0x0001,
         4,       -1,       -1, G_BOXTEXT         ,   /* Object 3  */
  FLAGS9|FLAGS10, DRAW3D, (LONG)&rs_tedinfo[48],
  0x0300, 0x0B02, 0x030A, 0x0001,
         0,       -1,       -1, G_BOXTEXT         ,   /* Object 4  */
  LASTOB|FLAGS9|FLAGS10, NORMAL, (LONG)&rs_tedinfo[49],
  0x0300, 0x0F03, 0x030A, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 TEX_MAIN         */
  &rs_object[63],   /* Tree  1 TURB_POPUP       */
  &rs_object[70],   /* Tree  2 MAP_POPUP        */
  &rs_object[75]    /* Tree  3 MODE_POPUP       */
};
