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

/*  ----------------- PEPSIP-Modul V0.1 ----------------    */
/*          F�r SMURF Bildkonverter, 30.12.95               */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include "..\..\..\import.h"
#include "..\..\..\..\src\smurfine.h"

void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);

char p1string[20]="Tiefenbild";
char p2string[20]="Texturbild";


/*------ Infostruktur f�r Hauptprogramm -----*/
MOD_INFO    module_info=
    {
    "PEPSIP",
    0x0010,
    "J�rg Dittmer",
    "","","","","","","","","","",  /* 10 Extensionen f�r Importer */
/* 4 Slider�berschriften: max 8 */
    "Tiefe",
    "T-offset",
    "Auge",
    "Extrapol",
/* 4 Checkbox�berschriften: */
    "",
    "",
    "",
    "",
/* 4 Edit-Objekt-�berschriften: */
    "",
    "",
    "",
    "",
/* min/max-Werte f�r Slider */
    1,1000,
    1,200,
    1,50,
    1,32,
/* min/max f�r Editobjekte */
    0,0,
    0,0,
    0,0,
    0,0,
    /* Defaultwerte f�r Slider, Check und Edit */
    100,100,30,1,
    0,0,0,0,
    0,0,0,0,
    2, /* Anzahl der Bilder */
    p1string, /* Bezeichnung f�r Bilder */
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


SMURF_PIC *picture[3];

/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
int my_id;


 

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
    Gargamel h�ngt, steht in GARGAMEL -> event_par und liegt
    zwischen 1 und 6.               */
else if(smurf_struct->module_mode == MPICTURE)
{
    /* Bild holen */
    picture[smurf_struct->event_par] = smurf_struct->smurf_pic;

    /* und weiter warten */
    smurf_struct->module_mode=M_WAITING;
    return;
}



/*--------- MEXEC-Message (Los gehts!) */
else if(smurf_struct->module_mode == MEXEC)
{
    


    
    smurf_struct->module_mode=M_PICDONE;
    return;
}




/* Mterm empfangen - Speicher freigeben und beenden */
else if(smurf_struct->module_mode==MTERM)
{
    smurf_struct->module_mode=M_EXIT;
    return;
}


}


/*------ Previewfunktion - wird von Smurf bei Klick aufs Preview aufgerufen.------- */
/* Diese Funktion sollte ein 64*64 Pixel gro�es Preview des Bildes nach             */
/* Abarbeitung der Modulfunktion erzeugen. Das Preview mu� in der SMURF_PIC-        */
/* Struktur *preview abgelegt werden. Dithering und Darstellung werden von Smurf    */
/* �bernommen.                                                                      */
/* In prev_struct->smurf_pic liegt das unbearbeitete Bild. Das Modul mu� nun        */
/* ein Previewbild in *preview erzeugen. Speicher wurde hierf�r bereits von Smurf   */
/* angefordert. Das Preview (im Smurf-Standardformat) wird dann vom Hauptprogramm   */
/* f�r die Screen-Farbtiefe gedithert und im Einstellformular dargestellt.          */

void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview){

    return;     /* Ich mach' noch nix. */
}
