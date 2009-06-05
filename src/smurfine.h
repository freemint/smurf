#ifndef _smurfine_h_
#define _smurfine_h_

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
 * Olaf Piesche, Christian Eyrich, Dale Russell and Jîrg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* HEADERFILE fÅr Modulschnittstelle SCHLUMPFINE V0.2       */

/* has to be absolute because with relative addressing */
/* PURE-C would use the start path of the file that */
/* includes SMURFINE.H */
#include "H:\SRC\SMURF\SMURF\MODULES\DITHER\dit_mod.h"


int start_imp_module(char *modpath, SMURF_PIC *imp_pic);
EXPORT_PIC *start_exp_module(char *modpath, int message, SMURF_PIC *pic_to_export, BASPAG *exbase, GARGAMEL *sm_struct, int mod_id);
BASPAG *start_edit_module(char *modpath, BASPAG *edit_basepage, int mode, int mod_anz, GARGAMEL *smurf_struct);
BASPAG *start_dither_module(int mode, int mod_id, DITHER_DATA *ditherdata);


/*
 * Module Call Messages -  fÅr alle Module 
 */
#define MSTART           0          /* Module first START */
#define MTERM           -1          /* Module TERMinate  - Raus mit Dir! */
#define MWRED            1          /* Module Window REDraw - Redraw im Modulfenster notwendig */
#define MBEVT            2          /* Module Button EVenT - Buttonevent im Modulfenster */
#define MEXEC            3          /* Module EXECute - nach Einstellformular*/
#define MWMOVED          4          /* Module Window MOVED (Modulfenster wurde bewegt) */
#define MNEXTPIC         5          /* Smurf schickt nÑchstes Bild */
#define MQUERY           6          /* Zum abfragen der Info- und Ability-Strukturen */
#define MPICTURE         7          /* Bild kommt! */
#define MMORE            8          /* "Mehr"-Dialog aufmachen! */
#define MEXTEND          9          /* Man gebe mir eine Extension! */
#define MMORECANC       10          /* Exporter: Optionen canceln! */
#define MCOLSYS         11          /* Exporter: was fÅr ein Farbsystem willst Du */
#define MCROSSHAIR      12          /* Fadenkreuz-Positionierung vonnîten? */
#define MCH_COORDS      13          /* Fadenkreuz-Koordinaten schicken */
#define MPICS           14          /* Jetzt mîchte Smurf die Bilder Åbergeben */
#define MCH_DEFCOO      15          /* Defaultkoordinaten erfragen */
#define MDITHER_READY   16          /* Bild im Modulfenster fertig gedietert */
#define MPIC_UPDATE     17          /* Neues Bild wurde aktiviert oder aktives verÑndert */
#define MKEVT           18          /* Keyboardevent im Modulfenster */
#define DONE            19
#define GETCONFIG       20
#define CONFIG_TRANSMIT 21

#define AES_MESSAGE     255         /* weitergeleitete AES-Message */

/* Module Return Messages */
#define M_INVALID       -1      /* Bildformat ist nicht fÅr mich - I   */
#define M_PICDONE        0      /* Bild verarbeitet, alles OK - IOE */
#define M_WAITING        1      /* Modul wartet auf Abschluû des Einstellformulars -   E */
#define M_MODERR        -2      /* allgemeiner Modulfehler    - IOE */
#define M_MEMORY        -3      /* Nicht genÅgend RAM         - IOE */
#define M_IOERROR       -4      /* Harddisk/Floppy-Fehler     - IO  */
#define M_EXIT          -5      /* Modul hat sich beendet     - IOE */
#define M_DONEEXIT      -6      /* Modul fertig und beendet   - IOE */
#define M_PALERR        -7      /* Modul konnte keine Palette Ermitteln - I */
#define M_PICERR        -8      /* Fehler im Bild - I */
#define M_UNKNOWN_TYPE  -9      /* Format richtig, aber Type nicht unterstÅtzt */
#define M_MODPIC        -10     /* Bild in der Modulwindow-struktur hat sich geÑndert */
#define M_STARTED       -11     /* Startbutton im Modulfenster gedrÅckt */
#define M_EXTEND        -12     /* Smurfanfrage fÅr Extender verstanden */
#define M_MOREOK        -13     /* Moreformularquittierung quittiert */
#define M_COLSYS        -14     /* Exporter: benîtigtes Farbsystem */
#define M_CROSSHAIR     -15     /* Fadenkreuz-Positionierung vonnîten! */
#define M_CHDEFCOO      -16     /* Fadenkreuz-Defaultkoordinaten schicken */
#define M_PICTURE       -17     /* Bild ist angekommen */
#define M_CH_COORDS     -18     /* Koordinaten sind angekommen. */
#define M_CH_OFF        -19     /* Fadenkreuz wieder ausschalten! */

#define WINDEVENTS      -20     /* Modul will Windowevents durchgereicht haben */
#define ALL_MSGS        -21
#define M_TERMINATED    -22
#define M_CONFIG        -23     /* Modulkonfiguration wurde Åbermittelt */
#define M_SILENT_ERR    -24     /* Fehler aufgetreten, aber Smurf soll keine eigene Meldung bringen */
#define M_CONFSAVE      -25     /* Smurf soll anbei Åbermittelte Modulkonfiguration abspeichern */

#define M_STARTERR      -255

/* Farbsysteme */
#define RGB     0
#define BGR     1
#define CMY     2
#define YCBCR   3
#define YIQ     4
#define GREY    5
#define WURSCHT 42              /* alle Farbsysteme (f. MOD_ABILITY) */

/* Palettenmodi der Dithermodule */
#define ALLPAL  1
#define FIXPAL  0

/* Interne Formate */
#define FORM_PIXELPAK   0       /* Pixelpacked-Format */
#define FORM_STANDARD   1       /* Standard - Format */
#define FORM_BOTH       2       /* Beide (f. MOD_ABILITY) */

#endif