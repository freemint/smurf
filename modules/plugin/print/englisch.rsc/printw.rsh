/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "PRINTW.H"

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
{ "Needed program WDIALOG not installed.",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 38, 1,
  "No image loaded!",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 17, 1,
  "Smurf can\047t open the printer!",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 30, 1,
  "Not enough memory for printing!",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 32, 1,
  "Not enough memory to prepare the image data.",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 45, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 ALERT_STRINGS ****************************************************/
        -1, NO_WDIALOG      , NO_PREP_MEM     , G_BOX             ,   /* Object 0  */
  NONE, SHADOWED, (LONG)0x00011100L,
  0x0000, 0x0000, 0x003B, 0x0808,
  NO_PIC          ,       -1,       -1, G_TEXT            ,   /* Object 1 NO_WDIALOG */
  NONE, NORMAL, (LONG)&rs_tedinfo[0],
  0x0001, 0x0800, 0x0025, 0x0001,
  NO_OPEN         ,       -1,       -1, G_TEXT            ,   /* Object 2 NO_PIC */
  NONE, NORMAL, (LONG)&rs_tedinfo[1],
  0x0001, 0x0002, 0x0010, 0x0001,
  NO_PRT_MEM      ,       -1,       -1, G_TEXT            ,   /* Object 3 NO_OPEN */
  NONE, NORMAL, (LONG)&rs_tedinfo[2],
  0x0001, 0x0803, 0x001D, 0x0001,
  NO_PREP_MEM     ,       -1,       -1, G_TEXT            ,   /* Object 4 NO_PRT_MEM */
  NONE, NORMAL, (LONG)&rs_tedinfo[3],
  0x0001, 0x0005, 0x001F, 0x0001,
         0,       -1,       -1, G_TEXT            ,   /* Object 5 NO_PREP_MEM */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[4],
  0x0001, 0x0806, 0x002C, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0]    /* Tree  0 ALERT_STRINGS    */
};
