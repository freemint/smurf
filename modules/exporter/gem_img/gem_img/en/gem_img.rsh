/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "gem_img\en\GEM_IMG.H"

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
{ "Pixart",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 7, 1,
  "GEM-View",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 9, 1,
  " TC-Organization ",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 18, 1,
  "extended format",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1100, 0, -1, 16, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 XIMG_EXPORT ****************************************************/
        -1, OK              , SAVE            , G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x001F, 0x080B,
         2,       -1,       -1, G_BUTTON          ,   /* Object 1 OK */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"OK",
  0x0414, 0x0809, 0x0008, 0x0001,
         3,       -1,       -1, G_IBOX            ,   /* Object 2  */
  FLAGS10, NORMAL, (LONG)0x00010100L,
  0x0102, 0x0903, 0x001B, 0x0804,
         8,        4, PIXA            , G_IBOX            ,   /* Object 3  */
  FLAGS10, NORMAL, (LONG)0x00011100L,
  0x0002, 0x0803, 0x001B, 0x0804,
         5,       -1,       -1, G_TEXT            ,   /* Object 4  */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0404, 0x0802, 0x0006, 0x0001,
  GVW             ,       -1,       -1, G_TEXT            ,   /* Object 5  */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0404, 0x0001, 0x0008, 0x0001,
  PIXA            ,       -1,       -1, G_IBOX            |0x2B00,   /* Object 6 GVW */
  SELECTABLE|RBUTTON|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0002, 0x0001, 0x000B, 0x0001,
         3,       -1,       -1, G_IBOX            |0x2B00,   /* Object 7 PIXA */
  SELECTABLE|RBUTTON|FLAGS10, DISABLED, (LONG)0x00FF1100L,
  0x0002, 0x0802, 0x0009, 0x0001,
         9,       -1,       -1, G_TEXT            ,   /* Object 8  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[2],
  0x0003, 0x0003, 0x0011, 0x0001,
  FORMAT          ,       -1,       -1, G_TEXT            ,   /* Object 9  */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0404, 0x0001, 0x000F, 0x0001,
  SAVE            ,       -1,       -1, G_IBOX            |0x2C00,   /* Object 10 FORMAT */
  SELECTABLE|FLAGS10, SELECTED, (LONG)0x00FF1100L,
  0x0002, 0x0001, 0x0015, 0x0001,
         0,       -1,       -1, G_BUTTON          ,   /* Object 11 SAVE */
  SELECTABLE|EXIT|LASTOB|FLAGS9|FLAGS10, NORMAL, (LONG)"save",
  0x040A, 0x0809, 0x0008, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 XIMG_EXPORT      */
};
