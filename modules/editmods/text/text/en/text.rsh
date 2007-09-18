/* GEM Resource C Source */

#include <portab.h>
#include <aes.h>
#include "text\en\TEXT.H"

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
{ "Testtext_____________________________________",
  "_____________________________________________",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  IBM  , 0, TE_CNTR , 0x1180, 0, -1, 46, 46,
  "Preview:",
  "\0",
  "\0",
  SMALL, 0, TE_LEFT , 0x1100, 0, -1, 9, 1,
  "Antialiasing",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 13, 1,
  "00036",
  "_____ Point",
  "99999",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 12,
  "Font:",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "Size:",
  "\0",
  "\0",
  IBM  , 0, TE_RIGHT, 0x1180, 0, -1, 6, 1,
  "12345678901234567890123456789012345678901234567890",
  "\0",
  "\0",
  IBM  , 0, TE_CNTR , 0x1180, 0, -2, 51, 1,
  "Text:",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 6, 1,
  "The text module needs NVDI 2.5 or higher to run (EdDI 1.0)!",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 60, 1,
  "No xFSL- or MagiC-Fontselector installed!",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 42, 1,
  "Error calling the xFSL-Fontselectors!",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 38, 1,
  "Error calling the MagiC-Fontselector!",
  "\0",
  "\0",
  IBM  , 0, TE_LEFT , 0x1180, 0, -1, 38, 1
};

OBJECT rs_object[] =
{ 
  /******** Tree 0 TEXT_MAIN ****************************************************/
        -1, PREVIEW         ,       11, G_BOX             ,   /* Object 0  */
  FLAGS10, NORMAL, (LONG)0x00001100L,
  0x0000, 0x0000, 0x0334, 0x0D0E,
  TEXT_OB         ,       -1,       -1, G_BOX             ,   /* Object 1 PREVIEW */
  FLAGS9, SELECTED, (LONG)0x00011101L,
  0x0001, 0x0807, 0x0432, 0x0804,
         3,       -1,       -1, G_FTEXT           ,   /* Object 2 TEXT_OB */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[0],
  0x0406, 0x0005, 0x002D, 0x0001,
  TEXT_START      ,       -1,       -1, G_TEXT            ,   /* Object 3  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[1],
  0x0001, 0x0D06, 0x0409, 0x0800,
         5,       -1,       -1, G_BUTTON          ,   /* Object 4 TEXT_START */
  SELECTABLE|DEFAULT|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)"generate text",
  0x0023, 0x000D, 0x040F, 0x0001,
  ANTIALIAS       ,       -1,       -1, G_TEXT            ,   /* Object 5  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[2],
  0x040F, 0x000D, 0x040E, 0x0001,
  FONTSIZE        ,       -1,       -1, G_IBOX            |0x2C00,   /* Object 6 ANTIALIAS */
  SELECTABLE, DISABLED, (LONG)0x00FF1100L,
  0x000D, 0x000D, 0x000F, 0x0001,
         8,       -1,       -1, G_FTEXT           ,   /* Object 7 FONTSIZE */
  EDITABLE|FLAGS10, NORMAL, (LONG)&rs_tedinfo[3],
  0x0007, 0x0803, 0x000B, 0x0001,
         9,       -1,       -1, G_TEXT            ,   /* Object 8  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[4],
  0x0700, 0x0D00, 0x0005, 0x0001,
  FONTNAME        ,       -1,       -1, G_TEXT            ,   /* Object 9  */
  FLAGS10, NORMAL, (LONG)&rs_tedinfo[5],
  0x0001, 0x0803, 0x0005, 0x0001,
        11,       -1,       -1, G_BOXTEXT         ,   /* Object 10 FONTNAME */
  SELECTABLE|EXIT|FLAGS9|FLAGS10, NORMAL, (LONG)&rs_tedinfo[6],
  0x0001, 0x0002, 0x0432, 0x0001,
         0,       -1,       -1, G_TEXT            ,   /* Object 11  */
  LASTOB|FLAGS10, NORMAL, (LONG)&rs_tedinfo[7],
  0x0001, 0x0005, 0x0005, 0x0001,
  
  /******** Tree 1 ALERT_STRINGS ****************************************************/
        -1, NO_NVDI         , ERROR_MAGIC_FSEL, G_BOX             ,   /* Object 0  */
  NONE, SHADOWED, (LONG)0x00011100L,
  0x0000, 0x0000, 0x0445, 0x0009,
  NO_FSEL         ,       -1,       -1, G_TEXT            ,   /* Object 1 NO_NVDI */
  NONE, NORMAL, (LONG)&rs_tedinfo[8],
  0x0001, 0x0800, 0x003B, 0x0001,
  ERROR_XFSL_FSEL ,       -1,       -1, G_TEXT            ,   /* Object 2 NO_FSEL */
  NONE, NORMAL, (LONG)&rs_tedinfo[9],
  0x0001, 0x0002, 0x0029, 0x0001,
  ERROR_MAGIC_FSEL,       -1,       -1, G_TEXT            ,   /* Object 3 ERROR_XFSL_FSEL */
  NONE, NORMAL, (LONG)&rs_tedinfo[10],
  0x0001, 0x0803, 0x0025, 0x0001,
         0,       -1,       -1, G_TEXT            ,   /* Object 4 ERROR_MAGIC_FSEL */
  LASTOB, NORMAL, (LONG)&rs_tedinfo[11],
  0x0001, 0x0005, 0x0025, 0x0001
};

OBJECT *rs_trindex[] =
{ &rs_object[0],   /* Tree  0 TEXT_MAIN        */
  &rs_object[12]    /* Tree  1 ALERT_STRINGS    */
};
