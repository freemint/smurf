/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "hsv\en\HSV.H"

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
  "___",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "H",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, -1, 2, 1,
  "S",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "V",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1100, 0, 0, 2, 1,
  "___",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "___",
  "\0",
  "\0",
  SMALL, 0, TE_CNTR , 0x1180, 0, -1, 4, 1,
  "HSV",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 4, 1,
  "HLS",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 4, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 HSVS_MIR ****************************************************/
        -1, START           , SYSTEM_BOX      , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0424, 0x0811,
  PREV_BOX        ,       -1,       -1, G_BUTTON          ,   /* Object 1 START */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Start",
  0x0418, 0x080F, 0x0409, 0x0001,
         3,       -1,       -1, G_BOX             |0x2F00,   /* Object 2 PREV_BOX */
  FLAGS9, SELECTED, (LONG)0x00FF1178L,
  0x0412, 0x0001, 0x0010, 0x0008,
  S4_E            ,       -1,       -1, G_TEXT            ,   /* Object 3  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0012, 0x0809, 0x0007, 0x0001,
  S4_F            ,       -1,       -1, G_BOXTEXT         ,   /* Object 4 S4_E */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[1],
  0x0002, 0x080A, 0x0702, 0x0B00,
  COMPONENT1      , S4_S            , S4_S            , G_BOX             ,   /* Object 5 S4_F */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0003, 0x0002, 0x0001, 0x0108,
  S4_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 6 S4_S */
  FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
  COMPONENT2      ,       -1,       -1, G_TEXT            ,   /* Object 7 COMPONENT1 */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0003, 0x0001, 0x0600, 0x0800,
  COMPONENT3      ,       -1,       -1, G_BOXTEXT         ,   /* Object 8 COMPONENT2 */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[3],
  0x0407, 0x0001, 0x0600, 0x0800,
  S5_F            ,       -1,       -1, G_BOXTEXT         ,   /* Object 9 COMPONENT3 */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[4],
  0x000C, 0x0001, 0x0600, 0x0800,
  S6_F            , S5_S            , S5_S            , G_BOX             ,   /* Object 10 S5_F */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x0407, 0x0002, 0x0001, 0x0108,
  S5_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 11 S5_S */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
  S5_E            , S6_S            , S6_S            , G_BOX             ,   /* Object 12 S6_F */
  FLAGS9, SELECTED, (LONG)0x00FE9178L,
  0x000C, 0x0002, 0x0001, 0x0108,
  S6_F            ,       -1,       -1, G_BOX             |0x1500,   /* Object 13 S6_S */
  TOUCHEXIT|FLAGS9|FLAGS10, NORMAL, (LONG)0x00FF1178L,
  0x0000, 0x0907, 0x0001, 0x0800,
  S6_E            ,       -1,       -1, G_BOXTEXT         ,   /* Object 14 S5_E */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[5],
  0x0406, 0x080A, 0x0702, 0x0B00,
  CHECK_S         ,       -1,       -1, G_BOXTEXT         ,   /* Object 15 S6_E */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[6],
  0x000B, 0x080A, 0x0702, 0x0B00,
  CHECK_V         ,       -1,       -1, G_BOXCHAR         ,   /* Object 16 CHECK_S */
  TOUCHEXIT|FLAGS10, NORMAL, (LONG) ((LONG)'\0' << 24)|0x00FF1100L,
  0x0007, 0x080B, 0x0002, 0x0001,
  CHECK_H         ,       -1,       -1, G_BOXCHAR         ,   /* Object 17 CHECK_V */
  TOUCHEXIT|FLAGS10, NORMAL, (LONG) ((LONG)'\0' << 24)|0x00FF1100L,
  0x040B, 0x080B, 0x0002, 0x0001,
  MCONF_SAVE      ,       -1,       -1, G_BOXCHAR         ,   /* Object 18 CHECK_H */
  TOUCHEXIT|FLAGS10, NORMAL, (LONG) ((LONG)'\0' << 24)|0x00FF1100L,
  0x0402, 0x080B, 0x0002, 0x0001,
  MCONF_LOAD      ,       -1,       -1, G_BUTTON          ,   /* Object 19 MCONF_SAVE */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Save",
  0x000D, 0x080F, 0x0409, 0x0001,
  SYSTEM_BOX      ,       -1,       -1, G_BUTTON          ,   /* Object 20 MCONF_LOAD */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"Load",
  0x0002, 0x080F, 0x0409, 0x0001,
         0,       22, _HLS            , G_IBOX            ,   /* Object 21 SYSTEM_BOX */
  NONE, NORMAL, (LONG)0x00001100L,
  0x0002, 0x000D, 0x040F, 0x0002,
  _HSV            ,       -1,       -1, G_TEXT            ,   /* Object 22  */
  NONE, NORMAL, (LONG)&rs_tedinfo[7],
  0x0003, 0x0800, 0x0003, 0x0001,
        24,       -1,       -1, G_IBOX            |0x2B00,   /* Object 23 _HSV */
  SELECTABLE|RBUTTON, SELECTED, (LONG)0x00FF1100L,
  0x0400, 0x0800, 0x0006, 0x0001,
  _HLS            ,       -1,       -1, G_TEXT            ,   /* Object 24  */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x000B, 0x0800, 0x0003, 0x0001,
  SYSTEM_BOX      ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 25 _HLS */
  SELECTABLE|RBUTTON|LASTOB, NORMAL, (LONG)0x00FF1100L,
  0x0408, 0x0800, 0x0006, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 HSVS_MIR         */
};
