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
 * Olaf Piesche, Christian Eyrich, Dale Russell and J�rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* ----- Daten�bergabe-Struktur f�r SMURF-Dithermodule (.SDM) ------ */

typedef struct
{
    int dst_depth;                  /* Farbtiefe, auf die gedithert werden soll */
    int *red, *blue, *green;        /* Palette, auf die Gedithert werden soll (15 Bit!) */
    char *nc_tab;                   /* Nearest-Color-Table */
    char *planetable;               /* Plane-Table f�r VDI-Indizes */
    
    SMURF_PIC   *picture;           /* das zu dithernde Bild */

    int x,y,w,h;                    /* der zu dithernder Ausschnitt des Bildes */
                            /* Ausschnittdithern folgenderma�en: dest_mem wird auf einen Puffer, der
                                w*h Gr��e hat, ausgerichtet. Das Modul dithert diesen aus dem Bild mit
                                <x/y/w/h> voll, unter Ber�cksichtigung des Zoomfaktors!
                                Smurf transferiert ihn dann Zeile f�r Zeile aus dem Standardformat 
                                ins Ger�teabh�ngige, direkt auf den Screen. Dann wird   der Buffer 
                                freigegeben und es geht mit dem n�xten Ausschnitt weiter. 
                                Um mit Standardformatbildern richtig zurechtzukommen wird x immer auf 
                                eine 16er-Grenze ausgerichtet. So kommen wir mit recht wenig 
                                Modifikationen in den Dithermodulen aus.
                                Wenn der Speicher zum Dithern dann nicht reicht, um nach dem Laden eines
                                Bildes den gesamten sichtbaren Bereich auf einmal zu dithern k�nnen wir
                                auch Blockweise (32*32 oder so) vorgehen, dann sieht der User auch schneller
                                was. Interessant wird noch, wie es mit den Fehlerdiffusionsalgorithmen
                                aussieht, da hier an den R�ndern von Bildausschnitten Fehler entstehen werden.
                                Dadurch, da� wir aber immer auf eine 16er-Grenze ausrichten, werden sich diese
                                hoffentlich in Grenzen halten, da die Ausschnitte wohl selten genau auf 16er-
                                Grenzen anfangen werden. Vielleicht lassen sich die Fehler auch dadurch 
                                einschr�nken, da� man links und rechts vom zu Dithernden Ausschnitt je 8 
                                oder 16 Pixel mitdithert, um zu vermeiden, da� zwei Ausschnittr�nder direkt 
                                aneinander liegen.
                                */
    int zoom;
    char        *dest_mem;          /* Zielspeicher, in den gedithert werden soll */

    int message;                    /* Median CUT (1/3) oder Syspal (0/2) */

    SERVICE_FUNCTIONS   *services;
    int (*set_16pixels)(char *buf16, char *dest, int depth, long planelen, int howmany);
    long    dest_planelength;           /* Planel�nge f�r Ziel */
    int *red24, *blue24, *green24;      /* 24Bit-Palette, auf die Gedithert werden soll */

    int not_in_nct;                     /* Kennung in der NCT f�r nicht gesuchte Farben */
} DITHER_DATA;


typedef struct
{
    char algo_name[15];         /* Name f�rs Popup */
    char autor[20];             /* Autor f�r Info */

    long smurf_version;         /* Schlumpfine-Version */
    char conf_able;             /* Konfigurierbar? */
    int pal_mode;               /* Palettenmodus (MCUT|SYSPAL || FIXPAL) */
} DITHER_MOD_INFO;
