/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "BUMPMAPO.H"

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
  "R",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "Diffuses Licht",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 15, 1,
  "Glanzpunkt",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 11, 1,
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
  "St\204rke",
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
  "Bump-Mapping",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 13, 1,
  "Lichtposition bitte",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 20, 1,
  "im Eingabebild ein-",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 20, 1,
  "stellen !!!",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 12, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 BUMPMAIN ****************************************************/
        -1,        1,       47, G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x003F, 0x0012,
         2,       -1,       -1, G_IBOX            ,   /* Object 1  */
  NONE, NORMAL, (LONG)0x00010100L,
  0x0102, 0x0101, 0x001D, 0x0010,
        27,        3, M_BLUE_EDIT     , G_IBOX            ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0002, 0x0001, 0x001D, 0x0010,
         4,       -1,       -1, G_TEXT            ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x040E, 0x0001, 0x0006, 0x0001,
         5,       -1,       -1, G_TEXT            ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0016, 0x0001, 0x0005, 0x0001,
         6,       -1,       -1, G_TEXT            ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0406, 0x0001, 0x0401, 0x0001,
         7,       -1,       -1, G_TEXT            ,   /* Object 6  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x000A, 0x0001, 0x0401, 0x0001,
         8,       -1,       -1, G_TEXT            ,   /* Object 7  */
  NONE, NORMAL, (LONG)&rs_tedinfo[4],
  0x0003, 0x0001, 0x0401, 0x0001,
  M_RADIO_DIFFUSE ,       -1,       -1, G_TEXT            ,   /* Object 8  */
  RBUTTON, NORMAL, (LONG)&rs_tedinfo[5],
  0x0006, 0x080C, 0x000E, 0x0001,
        10,       -1,       -1, G_IBOX            |0x1200,   /* Object 9 M_RADIO_DIFFUSE */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF2100L,
  0x0403, 0x080C, 0x0410, 0x0001,
  M_RADIO_GLANZ   ,       -1,       -1, G_TEXT            ,   /* Object 10  */
  NONE, NORMAL, (LONG)&rs_tedinfo[6],
  0x0006, 0x000E, 0x000A, 0x0001,
  M_HARD_FHR      ,       -1,       -1, G_IBOX            |0x1200,   /* Object 11 M_RADIO_GLANZ */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF2100L,
  0x0403, 0x000E, 0x040C, 0x0001,
  M_HARD_EDIT     , M_HARD_SLIDE    , M_HARD_SLIDE    , G_BOX             ,   /* Object 12 M_HARD_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0417, 0x0802, 0x0401, 0x0108,
  M_HARD_FHR      ,       -1,       -1, G_BOX             |0x1500,   /* Object 13 M_HARD_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_STRENGH_FHR   ,       -1,       -1, G_BOXTEXT         ,   /* Object 14 M_HARD_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0017, 0x000B, 0x0402, 0x0800,
  M_STRENGH_EDIT  , M_STRENGH_SLIDE , M_STRENGH_SLIDE , G_BOX             ,   /* Object 15 M_STRENGH_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0410, 0x0802, 0x0401, 0x0108,
  M_STRENGH_FHR   ,       -1,       -1, G_BOX             |0x1500,   /* Object 16 M_STRENGH_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_RED_FHR       ,       -1,       -1, G_BOXTEXT         ,   /* Object 17 M_STRENGH_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0010, 0x000B, 0x0402, 0x0800,
  M_GREEN_FHR     , M_RED_SLIDE     , M_RED_SLIDE     , G_BOX             ,   /* Object 18 M_RED_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0003, 0x0802, 0x0401, 0x0108,
  M_RED_FHR       ,       -1,       -1, G_BOX             |0x1500,   /* Object 19 M_RED_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1172L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_BLUE_FHR      , M_GREEN_SLIDE   , M_GREEN_SLIDE   , G_BOX             ,   /* Object 20 M_GREEN_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0406, 0x0802, 0x0401, 0x0108,
  M_GREEN_FHR     ,       -1,       -1, G_BOX             |0x1500,   /* Object 21 M_GREEN_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1173L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_RED_EDIT      , M_BLUE_SLIDE    , M_BLUE_SLIDE    , G_BOX             ,   /* Object 22 M_BLUE_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x000A, 0x0802, 0x0401, 0x0108,
  M_BLUE_FHR      ,       -1,       -1, G_BOX             |0x1500,   /* Object 23 M_BLUE_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1174L,
  0x0000, 0x0B07, 0x0401, 0x0600,
  M_GREEN_EDIT    ,       -1,       -1, G_BOXTEXT         ,   /* Object 24 M_RED_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0402, 0x000B, 0x0402, 0x0800,
  M_BLUE_EDIT     ,       -1,       -1, G_BOXTEXT         ,   /* Object 25 M_GREEN_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0006, 0x000B, 0x0402, 0x0800,
         2,       -1,       -1, G_BOXTEXT         ,   /* Object 26 M_BLUE_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[11],
  0x0409, 0x000B, 0x0402, 0x0800,
  DO_IT           ,       -1,       -1, G_TEXT            ,   /* Object 27  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[12],
  0x0403, 0x0800, 0x0408, 0x0001,
        29,       -1,       -1, G_BUTTON          ,   /* Object 28 DO_IT */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Bump!",
  0x0034, 0x0010, 0x0008, 0x0001,
        30,       -1,       -1, G_IBOX            ,   /* Object 29  */
  FLAGS10, NORMAL, (LONG)0x00010100L,
  0x0121, 0x0101, 0x001C, 0x080D,
        44,       31, HW_RADIO_GREY   , G_IBOX            ,   /* Object 30  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0021, 0x0001, 0x001C, 0x080D,
  BM_STRENGH_FHR  ,       -1,       -1, G_TEXT            ,   /* Object 31  */
  NONE, NORMAL, (LONG)&rs_tedinfo[13],
  0x0002, 0x0001, 0x0006, 0x0001,
  BM_STRENGH_EDIT , BM_STRENGH_SLIDE, BM_STRENGH_SLIDE, G_BOX             ,   /* Object 32 BM_STRENGH_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0004, 0x0802, 0x0401, 0x0108,
  BM_STRENGH_FHR  ,       -1,       -1, G_BOX             |0x1500,   /* Object 33 BM_STRENGH_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0B07, 0x0401, 0x0600,
        35,       -1,       -1, G_BOXTEXT         ,   /* Object 34 BM_STRENGH_EDIT */
  NONE, NORMAL, (LONG)&rs_tedinfo[14],
  0x0403, 0x000B, 0x0402, 0x0800,
        36,       -1,       -1, G_TEXT            ,   /* Object 35  */
  NONE, NORMAL, (LONG)&rs_tedinfo[15],
  0x040B, 0x0003, 0x000E, 0x0001,
        37,       -1,       -1, G_TEXT            ,   /* Object 36  */
  NONE, NORMAL, (LONG)&rs_tedinfo[16],
  0x000F, 0x0005, 0x0003, 0x0001,
        38,       -1,       -1, G_TEXT            ,   /* Object 37  */
  NONE, NORMAL, (LONG)&rs_tedinfo[17],
  0x000F, 0x0806, 0x0004, 0x0001,
        39,       -1,       -1, G_TEXT            ,   /* Object 38  */
  NONE, NORMAL, (LONG)&rs_tedinfo[18],
  0x000F, 0x0008, 0x0004, 0x0001,
  HW_RADIO_RED    ,       -1,       -1, G_TEXT            ,   /* Object 39  */
  NONE, NORMAL, (LONG)&rs_tedinfo[19],
  0x000F, 0x0809, 0x0004, 0x0001,
  HW_RADIO_GREEN  ,       -1,       -1, G_IBOX            |0x1200,   /* Object 40 HW_RADIO_RED */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF2100L,
  0x040C, 0x0005, 0x0405, 0x0001,
  HW_RADIO_BLUE   ,       -1,       -1, G_IBOX            |0x1200,   /* Object 41 HW_RADIO_GREEN */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF2100L,
  0x040C, 0x0806, 0x0406, 0x0001,
  HW_RADIO_GREY   ,       -1,       -1, G_IBOX            |0x1200,   /* Object 42 HW_RADIO_BLUE */
  SELECTABLE|RBUTTON, NORMAL, (LONG)0x00FF2100L,
  0x040C, 0x0008, 0x0406, 0x0001,
        30,       -1,       -1, G_IBOX            |0x1200,   /* Object 43 HW_RADIO_GREY */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF2100L,
  0x040C, 0x0809, 0x0406, 0x0001,
        45,       -1,       -1, G_TEXT            ,   /* Object 44  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[20],
  0x0422, 0x0800, 0x040C, 0x0001,
        46,       -1,       -1, G_TEXT            ,   /* Object 45  */
  NONE, NORMAL, (LONG)&rs_tedinfo[21],
  0x0022, 0x000F, 0x020E, 0x0800,
        47,       -1,       -1, G_TEXT            ,   /* Object 46  */
  NONE, NORMAL, (LONG)&rs_tedinfo[22],
  0x0022, 0x080F, 0x020E, 0x0800,
         0,       -1,       -1, G_TEXT            ,   /* Object 47  */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[23],
  0x0022, 0x0010, 0x0208, 0x0800
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 BUMPMAIN         */
};
