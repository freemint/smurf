#ifndef _globdefs_h_
#define _globdefs_h_

/*
 * ***** BEGIN LICENSE BLOCK *****
 *
 * The contents of this file are subject to the GNU General Public License
 * Version 2 (the "License"); you may not use this file except in compliance
 * with the GPL. You may obtain a copy of the License at
 * http://www.gnu.org/copyleft/gpl.html or the file GPL.TXT from the program
 * or source code package.
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
 * the specific language governing rights and limitations under the GPL.
 *
 * The Original Code is Therapy Seriouz Software code.
 *
 * The Initial Developer of the Original Code are
 * Olaf Piesche, Christian Eyrich, Dale Russell and J”rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */
/*                          GLOBDEFS.H                          */
/*  Enth„lt globale defines, die in allen Modulen verwendet     */
/*  werden k”nnen.                                              */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */

#define SMURF_VERSION   0x0106

#define LONG2_2INT(val) (int)((long)val>>16),(int)val
#define INT2_2_LONG(hiword,loword) (long)(((long)hiword<<16)|loword)


/*----- fest definierte Timerevents */
    #define STG_TIMER   19

#if MILANHOME
    #define CR_FILEPAL  9497
#endif

/*----- 3D-Objekt - Flags */
    #define FL3DIND     0x0200
    #define FL3DBAK     0x0400
    #define FL3DACT     0x0600

/*----- Max/Min-Makros - gr”žere oder kleinere aus 2 Variablen */
    #define min(a,b)             ((a) < (b) ? (a) : (b))
    #define max(a,b)             ((a) > (b) ? (a) : (b))

/* Neue AES-Messages (ab 3.2 aufw„rts) */
    #define WM_BOTTOMED 33          /* WM_BOTTOMED Message-ID */
    #define AP_DRAGDROP 63          /* Drag&Drop   Message-ID */

/* AES 4.0-3D-Flags */
    #define INDICATOR   0x0200;
    #define BACKGRND    0x0400;
    #define ACTIVATOR   0x0600;
    

/* WINX-Messages */
    #define WM_SHADED   22360       /* Window Shadowed */
    #define WM_UNSHADED 22361       /* Window UnShadowed */

/* Iconified - Value fr Shaded-flag */
    #define ICONIFIED   1
    #define SHADED      2

/* Image-Type Defines - LONG Bitvektor */
    #define BITMAP  1
    #define VECTOR  2
    #define ANIMATION   4
    #define TEXT    8
    

    #define PRG_CLOSED  -2

    #define FSBOX   1
    #define FSBOX2  2
    #define ARGS    3
    #define START   4
    #define VA      5
    #define PATH    6

    #define AV_IMAGE    0
    #define AV_BLOCK    1

/* a bunch of MiNT defines put in here by Baldrick */
/* proper use of newer GEMLIB would get rid of these
 * But that caused a bunch of other routines  to have 
 * problems.  And I'm trying to get this compiled at
 */
#ifndef EINVFN
#define EINVFN 	-32
#endif

#ifndef FEATURES
#define FEATURES 0x0007
#endif

#ifndef GETCOOKIE
#define GETCOOKIE 0x0008
#endif

#ifndef SHW_BROADCAST
#define SHW_BROADCAST 	7
#endif

#ifndef SH_WDRAW
#define SH_WDRAW	72
#endif

#ifndef SC_CHANGED
#define SC_CHANGED	80
#endif

#ifndef EACCDN
#define EACCDN	-36
#endif

#ifndef ENMFIL
#define ENMFIL	-47
#endif


/* end of Baldricks insertion */


#define UDO     21

#define SEL     1
#define UNSEL   0
#define ENABLED 2

#define REDRAW 99

#define RADIOBUTTON     43
#define CHECKBOX        44
#define CYCLEBUTTON     45
#define CORNER_GOODIE   46
#define PREVIEW_ACTION  47
#define MPIC_INFORMATION    48
#define NO_DESELECTION  49
#define CANCELBUTTON    50

#define PM_VIEWCALL 1

#define SAME    1
#define NEW     2

/* Defines fr Formularwindows */
/* Die Nummern 20-120 sind fr Bilder reserviert! */
#define WIND_DOPT       2
#define WIND_PIC        3
#define WIND_PICINFO    4
#define WIND_OPTIONS    5
#define WIND_ALERT      6
#define WIND_MODFORM    7
#define WIND_BUSY       8
#define WIND_MODULES    9
#define WIND_INFO       10
#define WIND_NEWPIC     11
#define WIND_BTYPEIN    12
#define WIND_EXPORT     13
#define WIND_PICMAN     14
#define FORM_EXPORT     15
#define WIND_TRANSFORM  16
#define WIND_PICORDER   17
#define WIND_BLOCKMODE  18
#define FIRST_PIC       20
#define LAST_PIC        120


extern  int TOOLBAR_HEIGHT;

#define MAX_PIC 25

#define MAIN_FUNCTION_OFFSET    4   /* war garnicht */
#define MAGIC_OFFSET    8           /* war 4 */
#define MOD_INFO_OFFSET 12          /* war 8 */
#define MOD_ABS_OFFSET  16          /* war 12 */

#define FILESELECT_ERROR ( (void*) -1L )

#define DIALOG_EXIT 255

#define KEY_SHIFT 0x003         /* rechtes SHIFT ist 1, linkes SHIFT ist 2 */
#define KEY_CTRL 0x004
#define KEY_ALT 0x008
#define KEY_RETURN 13
#define SCAN_RETURN 28
#define SCAN_ENTER 114

/* Cursortasten */
    #define KEY_UP      72
    #define KEY_DOWN    80
    #define KEY_ESCAPE  1
    #define KEY_BACKSPACE   14
    #define KEY_DELETE      83
    #define KEY_HOME    71
    #define KEY_UNDO    97
    #define KEY_HELP    98


/* window-Redraw-Flags */
    #define DRAWNOPICTURE   1
    #define DRAWNOTREE      2
    #define BLOCK_ONLY      4
    #define NOBLOCKBOX      8
    #define DRAWNOBLOCK     16

/* Defines fr Dither-Modi */
#define STAND8  1               /* 8Bit-Standardformat (f. Export und Destr.) */
#define PLANTAB 2               /* Planetable verwenden */
#define LNCT    4               /* Lokale NCT verwenden */
#define SCALEPAL    8           /* es kommt eine 24Bit-Palette, bitte auf 15 Bit skalieren */


/* Objektmakros */
#define TextCast    ob_spec.tedinfo->te_ptext
#define IsDisabled(object) ( ((object.ob_state) & DISABLED)? 1 : 0)
#define IsSelected(object) ( ((object.ob_state) & SELECTED)? 1 : 0)
#define IsSelectable(object) ( ((object.ob_flags) & SELECTABLE)? 1 : 0)


/* Funktionsmakros */
#define P_LINELEN8(w) ( (int)((w+7)>>3) )
#define PLANELEN8(w, h) ( ((long)(w+7)>>3)*(long)h )
#define PLANELEN16(w, h) ( ((long)(w+15)>>4)*(long)h )



/* Blind-Funktionsdeklarationen */
#define VOID_FUNCTION   void(*)()
#define VOIDP_FUNCTION  void(*)()   
#define INT_FUNCTION    int(*)()
#define INTP_FUNCTION   int(*)()        


#define DD_OK         0    /* Ok, - weitermachen                */
#define DD_NAK        1    /* Drag&Drop abbrechen               */
#define DD_EXT        2    /* Datenformat wird nicht akzeptiert */
#define DD_LEN        3    /* Wunsch nach weniger Daten         */
#define DD_TRASH      4    /* Ziel ist ein Papierkorb-Icon      */
#define DD_PRINTER    5    /* Ziel ist ein Drucker-Icon         */
#define DD_CLIPBOARD  6    /* Ziel ist ein Klemmbrett-Icon      */

#endif