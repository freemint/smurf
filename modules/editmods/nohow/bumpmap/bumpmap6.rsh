/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "BUMPMAP6.H"

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
{ "St\204rke",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 7, 1,
  "123",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1178, 0, -1, 4, 1,
  "H\224henwert von:",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, 0, 15, 1,
  "Rot",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 4, 1,
  "Gr\201n",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "Blau",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "Grau",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "Parallel",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 9, 1,
  "Bump-Mapping",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "R",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "St\204rke",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 7, 1,
  "H\204rte",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 6, 1,
  "G",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "B",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "123",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1178, 0, -1, 4, 1,
  "123",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1178, 0, -1, 4, 1,
  "123",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1178, 0, -1, 4, 1,
  "123",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1178, 0, -1, 4, 1,
  "123",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1178, 0, -1, 4, 1,
  "Material",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 9, 1,
  "Diffuses Licht",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 15, 1,
  "Glanzpunkt",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 11, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 BUMPMAIN ****************************************************/
        -1, DO_IT           , M_RADIO_GLANZ   , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003F, 0x0012,
         2,       -1,       -1, G_BUTTON          ,   /* Object 1 DO_IT */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Bump!",
  0x0034, 0x0010, 0x0008, 0x0001,
         3,       -1,       -1, G_IBOX            ,   /* Object 2  */
  FLAGS10, NORMAL, (LONG)0x00010100L,
  0x0121, 0x0101, 0x001C, 0x080D,
        19,        4, BM_PARALEL      , G_IBOX            ,   /* Object 3  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0021, 0x0001, 0x001C, 0x080D,
  BM_STRENGH_FHR  ,       -1,       -1, G_TEXT            ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0002, 0x0001, 0x0006, 0x0001,
  BM_STRENGH_EDIT , BM_STRENGH_SLIDE, BM_STRENGH_SLIDE, G_BOX             ,   /* Object 5 BM_STRENGH_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0004, 0x0802, 0x0401, 0x0108,
  BM_STRENGH_FHR  ,       -1,       -1, G_BOX             |0x1500,   /* Object 6 BM_STRENGH_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0B07, 0x0401, 0x0600,
         8,       -1,       -1, G_BOXTEXT         ,   /* Object 7 BM_STRENGH_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0403, 0x000B, 0x0402, 0x0800,
         9,       -1,       -1, G_TEXT            ,   /* Object 8  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x040B, 0x0001, 0x000E, 0x0001,
  HW_RADIO_RED    ,       -1,       -1, G_TEXT            ,   /* Object 9  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x000F, 0x0003, 0x0003, 0x0001,
        11,       -1,       -1, G_IBOX            |0x2B00,   /* Object 10 HW_RADIO_RED */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF2100L,
  0x040C, 0x0003, 0x0405, 0x0001,
  HW_RADIO_GREEN  ,       -1,       -1, G_TEXT            ,   /* Object 11  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x000F, 0x0804, 0x0004, 0x0001,
        13,       -1,       -1, G_IBOX            |0x2B00,   /* Object 12 HW_RADIO_GREEN */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF2100L,
  0x040C, 0x0804, 0x0406, 0x0001,
  HW_RADIO_BLUE   ,       -1,       -1, G_TEXT            ,   /* Object 13  */
  NONE, NORMAL, (LONG)&rs_tedinfo[5],
  0x000F, 0x0006, 0x0004, 0x0001,
        15,       -1,       -1, G_IBOX            |0x2B00,   /* Object 14 HW_RADIO_BLUE */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF2100L,
  0x040C, 0x0006, 0x0406, 0x0001,
  HW_RADIO_GREY   ,       -1,       -1, G_TEXT            ,   /* Object 15  */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x000F, 0x0807, 0x0004, 0x0001,
        17,       -1,       -1, G_IBOX            |0x2B00,   /* Object 16 HW_RADIO_GREY */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF2100L,
  0x040C, 0x0807, 0x0406, 0x0001,
  BM_PARALEL      ,       -1,       -1, G_TEXT            ,   /* Object 17  */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x000F, 0x080A, 0x0008, 0x0001,
         3,       -1,       -1, G_IBOX            |0x2C00,   /* Object 18 BM_PARALEL */
  SELECTABLE, NORMAL, (LONG)0x00FF2100L,
  0x040C, 0x080A, 0x040A, 0x0001,
        20,       -1,       -1, G_TEXT            ,   /* Object 19  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[8],
  0x0422, 0x0800, 0x040C, 0x0001,
        21,       -1,       -1, G_IBOX            ,   /* Object 20  */
  NONE, NORMAL, (LONG)0x00010100L,
  0x0002, 0x0001, 0x041C, 0x0010,
        22,       -1,       -1, G_IBOX            ,   /* Object 21  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0701, 0x0F00, 0x041C, 0x0010,
        23,       -1,       -1, G_TEXT            ,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0704, 0x0402, 0x0301, 0x0001,
        24,       -1,       -1, G_TEXT            ,   /* Object 23  */
  SELECTABLE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0310, 0x0402, 0x0006, 0x0001,
        25,       -1,       -1, G_TEXT            ,   /* Object 24  */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0717, 0x0402, 0x0005, 0x0001,
        26,       -1,       -1, G_TEXT            ,   /* Object 25  */
  NONE, NORMAL, (LONG)&rs_tedinfo[12],
  0x0308, 0x0402, 0x0401, 0x0001,
  M_HARD_FHR      ,       -1,       -1, G_TEXT            ,   /* Object 26  */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x070B, 0x0402, 0x0401, 0x0001,
  M_HARD_EDIT     , M_HARD_SLIDE    , M_HARD_SLIDE    , G_BOX             ,   /* Object 27 M_HARD_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0319, 0x0C03, 0x0401, 0x0108,
  M_HARD_FHR      ,       -1,       -1, G_BOX             |0x1500,   /* Object 28 M_HARD_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_STRENGH_FHR   ,       -1,       -1, G_BOXTEXT         ,   /* Object 29 M_HARD_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0718, 0x040C, 0x0402, 0x0800,
  M_STRENGH_EDIT  , M_STRENGH_SLIDE , M_STRENGH_SLIDE , G_BOX             ,   /* Object 30 M_STRENGH_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0312, 0x0C03, 0x0401, 0x0108,
  M_STRENGH_FHR   ,       -1,       -1, G_BOX             |0x1500,   /* Object 31 M_STRENGH_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_RED_FHR       ,       -1,       -1, G_BOXTEXT         ,   /* Object 32 M_STRENGH_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[15],
  0x0711, 0x040C, 0x0402, 0x0800,
  M_GREEN_FHR     , M_RED_SLIDE     , M_RED_SLIDE     , G_BOX             ,   /* Object 33 M_RED_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0704, 0x0C03, 0x0401, 0x0108,
  M_RED_FHR       ,       -1,       -1, G_BOX             |0x1500,   /* Object 34 M_RED_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1172L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_BLUE_FHR      , M_GREEN_SLIDE   , M_GREEN_SLIDE   , G_BOX             ,   /* Object 35 M_GREEN_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0308, 0x0C03, 0x0401, 0x0108,
  M_GREEN_FHR     ,       -1,       -1, G_BOX             |0x1500,   /* Object 36 M_GREEN_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1173L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_RED_EDIT      , M_BLUE_SLIDE    , M_BLUE_SLIDE    , G_BOX             ,   /* Object 37 M_BLUE_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x070B, 0x0C03, 0x0401, 0x0108,
  M_BLUE_FHR      ,       -1,       -1, G_BOX             |0x1500,   /* Object 38 M_BLUE_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1174L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_GREEN_EDIT    ,       -1,       -1, G_BOXTEXT         ,   /* Object 39 M_RED_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x0304, 0x040C, 0x0402, 0x0800,
  M_BLUE_EDIT     ,       -1,       -1, G_BOXTEXT         ,   /* Object 40 M_GREEN_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[17],
  0x0707, 0x040C, 0x0402, 0x0800,
        42,       -1,       -1, G_BOXTEXT         ,   /* Object 41 M_BLUE_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[18],
  0x030B, 0x040C, 0x0402, 0x0800,
        43,       -1,       -1, G_TEXT            ,   /* Object 42  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[19],
  0x0303, 0x0800, 0x0408, 0x0001,
  M_RADIO_DIFFUSE ,       -1,       -1, G_TEXT            ,   /* Object 43  */
  RBUTTON, NORMAL, (LONG)&rs_tedinfo[20],
  0x0007, 0x080D, 0x000E, 0x0001,
        45,       -1,       -1, G_IBOX            |0x2B00,   /* Object 44 M_RADIO_DIFFUSE */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF2100L,
  0x0404, 0x080D, 0x0410, 0x0001,
  M_RADIO_GLANZ   ,       -1,       -1, G_TEXT            ,   /* Object 45  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0007, 0x000F, 0x000A, 0x0001,
         0,       -1,       -1, G_IBOX            |0x2B00,   /* Object 46 M_RADIO_GLANZ */
  SELECTABLE|RBUTTON|LASTOB, NORMAL, (LONG)0x00FF2100L,
  0x0404, 0x000F, 0x040C, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 BUMPMAIN         */
};
