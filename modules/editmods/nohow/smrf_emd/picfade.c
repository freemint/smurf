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

/*  ----------------- Fade-Modul V0.5 ------------------    */
/*          FÅr SMURF Bildkonverter, 30.12.95               */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include "..\..\..\..\sym_gem.h"
#include "..\..\..\import.h"
#include "..\..\..\..\src\smurfine.h"


void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);

char p1string[20]="Fadebild 1";
char p2string[20]="Fadebild 2";


/*------ Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
    {
    "öberblenden (24Bit)",
    0x0050,
    "Olaf Piesche",
    "","","","","","","","","","",
    "StÑrke",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    0,100,
    0,128,
    0,128,
    0,128,
    0,10,
    0,10,
    0,10,
    0,10,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    2,
    p1string,
    p2string
    };


MOD_ABILITY  module_ability = {
                        24, 0, 0, 0, 0, 0, 0, 0,    /* Farbtiefen */
            /* Grafikmodi: */
                        FORM_PIXELPAK,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        0 /* Extra-Flag */ 
                        };


/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
int my_id;
SMURF_PIC *picture[3];
int width, height;
int x,y;
long slidval=0;
int r,g,b;
char *data;



/*---------- Wenn das Modul zum ersten Mal gestartet wurde, */
if(smurf_struct->module_mode==MSTART)
{
    my_id=smurf_struct->module_number;

    smurf_struct->f_module_prefs(&module_info, my_id);  /* PD aufrufen */

    smurf_struct->module_mode=M_WAITING;
    return;
}


/*--------- MPICTURE-Message (Bilder werden geliefert) */
/* Jetzt schickt Smurf der Reihe nach die einzelnen Bilder. Die
    Nummer des Bildes, das beim Eintreten der Message in der
    Gargamel hÑngt, steht in GARGAMEL -> event_par und liegt
    zwischen 1 und 6.               */
else if(smurf_struct->module_mode=MPICTURE)
{
    /* Bild holen */
    picture[smurf_struct->event_par] = smurf_struct->picture;

    /* und weiter warten */
    smurf_struct->module_mode=M_WAITING;
    return;
}



/*--------- MEXEC-Message (Los gehts!) */
else if(smurf_struct->module_mode=MEXEC)
{
    slidval=smurf_struct->slide1;       /* Slider holen */

}




/* Mterm empfangen - Speicher freigeben und beenden */
else if(smurf_struct->module_mode==MTERM)
    {
    smurf_struct->module_mode=M_EXIT;
    return;
    }


}


/*------ Previewfunktion - wird von Smurf bei Klick aufs Preview aufgerufen.------- */
/* Diese Funktion sollte ein 64*64 Pixel groûes Preview des Bildes nach             */
/* Abarbeitung der Modulfunktion erzeugen. Das Preview muû in der SMURF_PIC-        */
/* Struktur *preview abgelegt werden. Dithering und Darstellung werden von Smurf    */
/* Åbernommen.                                                                      */
/* In prev_struct->smurf_pic liegt das unbearbeitete Bild. Das Modul muû nun        */
/* ein Previewbild in *preview erzeugen. Speicher wurde hierfÅr bereits von Smurf   */
/* angefordert. Das Preview (im Smurf-Standardformat) wird dann vom Hauptprogramm   */
/* fÅr die Screen-Farbtiefe gedithert und im Einstellformular dargestellt.          */

void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview){

    return;     /* Ich mach' noch nix. */
}
