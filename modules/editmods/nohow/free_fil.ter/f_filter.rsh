/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "F_FILTER.H"

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
{ "Noname\000\000\000\000\000\000\000\000\000\000\000\000\000",
  "___________________",
  "XXXXXXXXXXXXXXXXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, 0, 20, 20,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, 0, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+01",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1A80, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+00",
  "___",
  "X99",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 4, 4,
  "+1\000\000",
  "Div.: ____",
  "X999",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 5, 11,
  "+0\000\000",
  "Bias: ____",
  "X999",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 5, 11,
  "Clip",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 5, 1,
  "Invert",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 7, 1,
  "St\204rke",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1100, 0, -1, 7, 1,
  "123",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x0100, 0, 0, 4, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 FREE_FILTER ****************************************************/
        -1, FILTER_NAME     ,       52, G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0024, 0x0811,
         2,       -1,       -1, G_FTEXT           ,   /* Object 1 FILTER_NAME */
  EDITABLE|FLAGS9|FLAGS10, SELECTED, (LONG)&rs_tedinfo[0],
  0x0602, 0x0101, 0x0713, 0x0001,
  DO_IT           ,        3,        3, G_IBOX            ,   /* Object 2  */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0002, 0x0002, 0x0015, 0x0808,
         2,        4,       32, G_BOX             ,   /* Object 3  */
  FLAGS9|FLAGS10, SELECTED, (LONG)0x00001100L,
  0x0600, 0x0600, 0x0713, 0x0D07,
         5,       -1,       -1, G_IBOX            ,   /* Object 4  */
  NONE, NORMAL, (LONG)0x00010100L,
  0x0200, 0x0200, 0x0413, 0x0A07,
         6,       -1,       -1, G_IBOX            ,   /* Object 5  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0100, 0x0A01, 0x0413, 0x0901,
         7,       -1,       -1, G_IBOX            ,   /* Object 6  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0100, 0x0203, 0x0413, 0x0901,
         8,       -1,       -1, G_IBOX            ,   /* Object 7  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0100, 0x0A04, 0x0413, 0x0901,
        14, MATRIX_00       , MATRIX_04       , G_IBOX            ,   /* Object 8  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0100, 0x0100, 0x0004, 0x0A07,
  MATRIX_01       ,       -1,       -1, G_FTEXT           ,   /* Object 9 MATRIX_00 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[1],
  0x0400, 0x0600, 0x0003, 0x0001,
  MATRIX_02       ,       -1,       -1, G_FTEXT           ,   /* Object 10 MATRIX_01 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[2],
  0x0400, 0x0E01, 0x0003, 0x0001,
  MATRIX_03       ,       -1,       -1, G_FTEXT           ,   /* Object 11 MATRIX_02 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[3],
  0x0400, 0x0603, 0x0003, 0x0001,
  MATRIX_04       ,       -1,       -1, G_FTEXT           ,   /* Object 12 MATRIX_03 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[4],
  0x0400, 0x0E04, 0x0003, 0x0001,
         8,       -1,       -1, G_FTEXT           ,   /* Object 13 MATRIX_04 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[5],
  0x0400, 0x0606, 0x0003, 0x0001,
        20, MATRIX_10       , MATRIX_14       , G_IBOX            ,   /* Object 14  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0004, 0x0100, 0x0004, 0x0A07,
  MATRIX_11       ,       -1,       -1, G_FTEXT           ,   /* Object 15 MATRIX_10 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[6],
  0x0400, 0x0600, 0x0003, 0x0001,
  MATRIX_12       ,       -1,       -1, G_FTEXT           ,   /* Object 16 MATRIX_11 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[7],
  0x0400, 0x0E01, 0x0003, 0x0001,
  MATRIX_13       ,       -1,       -1, G_FTEXT           ,   /* Object 17 MATRIX_12 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[8],
  0x0400, 0x0603, 0x0003, 0x0001,
  MATRIX_14       ,       -1,       -1, G_FTEXT           ,   /* Object 18 MATRIX_13 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[9],
  0x0400, 0x0E04, 0x0003, 0x0001,
        14,       -1,       -1, G_FTEXT           ,   /* Object 19 MATRIX_14 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[10],
  0x0400, 0x0606, 0x0003, 0x0001,
        26, MATRIX_20       , MATRIX_24       , G_IBOX            ,   /* Object 20  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x0707, 0x0100, 0x0004, 0x0A07,
  MATRIX_21       ,       -1,       -1, G_FTEXT           ,   /* Object 21 MATRIX_20 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[11],
  0x0400, 0x0600, 0x0003, 0x0001,
  MATRIX_22       ,       -1,       -1, G_FTEXT           ,   /* Object 22 MATRIX_21 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[12],
  0x0400, 0x0E01, 0x0003, 0x0001,
  MATRIX_23       ,       -1,       -1, G_FTEXT           ,   /* Object 23 MATRIX_22 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[13],
  0x0400, 0x0603, 0x0003, 0x0001,
  MATRIX_24       ,       -1,       -1, G_FTEXT           ,   /* Object 24 MATRIX_23 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[14],
  0x0400, 0x0E04, 0x0003, 0x0001,
        20,       -1,       -1, G_FTEXT           ,   /* Object 25 MATRIX_24 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[15],
  0x0400, 0x0606, 0x0003, 0x0001,
        32, MATRIX_30       , MATRIX_34       , G_IBOX            ,   /* Object 26  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x060B, 0x0100, 0x0004, 0x0A07,
  MATRIX_31       ,       -1,       -1, G_FTEXT           ,   /* Object 27 MATRIX_30 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[16],
  0x0400, 0x0600, 0x0003, 0x0001,
  MATRIX_32       ,       -1,       -1, G_FTEXT           ,   /* Object 28 MATRIX_31 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[17],
  0x0400, 0x0E01, 0x0003, 0x0001,
  MATRIX_33       ,       -1,       -1, G_FTEXT           ,   /* Object 29 MATRIX_32 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[18],
  0x0400, 0x0603, 0x0003, 0x0001,
  MATRIX_34       ,       -1,       -1, G_FTEXT           ,   /* Object 30 MATRIX_33 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[19],
  0x0400, 0x0E04, 0x0003, 0x0001,
        26,       -1,       -1, G_FTEXT           ,   /* Object 31 MATRIX_34 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[20],
  0x0400, 0x0606, 0x0003, 0x0001,
         3, MATRIX_40       , MATRIX_44       , G_IBOX            ,   /* Object 32  */
  NONE, NORMAL, (LONG)0x00011100L,
  0x050F, 0x0100, 0x0004, 0x0A07,
  MATRIX_41       ,       -1,       -1, G_FTEXT           ,   /* Object 33 MATRIX_40 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[21],
  0x0400, 0x0600, 0x0003, 0x0001,
  MATRIX_42       ,       -1,       -1, G_FTEXT           ,   /* Object 34 MATRIX_41 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[22],
  0x0400, 0x0E01, 0x0003, 0x0001,
  MATRIX_43       ,       -1,       -1, G_FTEXT           ,   /* Object 35 MATRIX_42 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[23],
  0x0400, 0x0603, 0x0003, 0x0001,
  MATRIX_44       ,       -1,       -1, G_FTEXT           ,   /* Object 36 MATRIX_43 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[24],
  0x0400, 0x0E04, 0x0003, 0x0001,
        32,       -1,       -1, G_FTEXT           ,   /* Object 37 MATRIX_44 */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[25],
  0x0400, 0x0606, 0x0003, 0x0001,
  DIV             ,       -1,       -1, G_BUTTON          ,   /* Object 38 DO_IT */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"OK",
  0x001A, 0x080F, 0x0008, 0x0001,
  BIAS            ,       -1,       -1, G_FTEXT           ,   /* Object 39 DIV */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[26],
  0x0402, 0x000B, 0x000A, 0x0001,
  LOAD_FILTER     ,       -1,       -1, G_FTEXT           ,   /* Object 40 BIAS */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[27],
  0x0402, 0x080C, 0x000A, 0x0001,
  SAVE_FILTER     ,       -1,       -1, G_BUTTON          ,   /* Object 41 LOAD_FILTER */
  SELECTABLE|FLAGS9|FLAGS10, NORMAL, (LONG)"Laden",
  0x0003, 0x080F, 0x0008, 0x0001,
        43,       -1,       -1, G_BUTTON          ,   /* Object 42 SAVE_FILTER */
  SELECTABLE|FLAGS9|FLAGS10, NORMAL, (LONG)"Sichern",
  0x000E, 0x080F, 0x0008, 0x0001,
        44,       -1,       -1, G_TEXT            ,   /* Object 43  */
  NONE, NORMAL, (LONG)&rs_tedinfo[28],
  0x0412, 0x000B, 0x0004, 0x0001,
  CLIP            ,       -1,       -1, G_TEXT            ,   /* Object 44  */
  NONE, NORMAL, (LONG)&rs_tedinfo[29],
  0x0412, 0x080C, 0x0006, 0x0001,
  INVERT          ,       -1,       -1, G_IBOX            |0x2C00,   /* Object 45 CLIP */
  SELECTABLE, SELECTED, (LONG)0x00FF2100L,
  0x040F, 0x000B, 0x0007, 0x0001,
        47,       -1,       -1, G_IBOX            |0x2C00,   /* Object 46 INVERT */
  SELECTABLE, NORMAL, (LONG)0x00FF2100L,
  0x040F, 0x080C, 0x0409, 0x0001,
  STRENGTH_FHR    ,       -1,       -1, G_TEXT            ,   /* Object 47  */
  NONE, NORMAL, (LONG)&rs_tedinfo[30],
  0x071B, 0x0101, 0x0006, 0x0001,
        50, STRENGTH_SLIDE  , STRENGTH_SLIDE  , G_BOX             ,   /* Object 48 STRENGTH_FHR */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x071D, 0x0902, 0x0401, 0x0108,
  STRENGTH_FHR    ,       -1,       -1, G_BOX             |0x1500,   /* Object 49 STRENGTH_SLIDE */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0B07, 0x0401, 0x0600,
        51,       -1,       -1, G_BOX             ,   /* Object 50  */
  NONE, NORMAL, (LONG)0x00009179L,
  0x0202, 0x040E, 0x071F, 0x0100,
        52,       -1,       -1, G_BOX             ,   /* Object 51  */
  NONE, NORMAL, (LONG)0x00009170L,
  0x0302, 0x050E, 0x071F, 0x0100,
         0, STRENGTH_EDIT   , STRENGTH_EDIT   , G_BOX             ,   /* Object 52  */
  FLAGS9|FLAGS10, SELECTED, (LONG)0x00001100L,
  0x061C, 0x030B, 0x0403, 0x0001,
        52,       -1,       -1, G_TEXT            ,   /* Object 53 STRENGTH_EDIT */
  LASTOB|FLAGS10, NORMAL, (LONG)&rs_tedinfo[31],
  0x0000, 0x0000, 0x0403, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 FREE_FILTER      */
};
