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

/*  ----------- Interpolieren-Modul V0.2 -------------  */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include "..\..\..\import.h"
#include "..\..\..\..\smurf\smurfine.h"


void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);


/*------ Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
    {
    "Interpolieren x2",                        /* Name des Moduls */
    0x0010,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "",
    "",
    "",
    "",
/* 4 CheckboxÅberschriften: */
    "",
    "",
    "",
    "",
/* 4 Edit-Objekt-öberschriften: */
    "",
    "",
    "",
    "",
/* min/max-Werte fÅr Slider */
    0,0,
    0,0,
    0,0,
    0,0,
/* min/max fÅr Editobjekte */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaultwerte fÅr Slider, Check und Edit */
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    };


/*--------------------- Was kann ich ? ----------------------*/
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
SMURF_PIC *picture;
int module_id;
int width, height, n_width, n_height;
int x,y;
char *pic,*n_pic,*offset,*noffset;
unsigned int red, green, blue;
long bpl, nbpl;

module_id=smurf_struct->module_number;

/* Wenn das Modul aufgerufen wurde, */
if(smurf_struct->module_mode == MSTART)
{
    picture=smurf_struct->smurf_pic;
    pic=picture->pic_data;
    width=picture->pic_width;
    height=picture->pic_height;
    bpl = (long)width*3L;
    
    n_width = width*2L - 1;
    n_height = height*2L - 1;
    nbpl = (long)n_width * 3L;
    
    if((n_pic = Malloc((long)n_width*(long)n_height*3L)) == NULL) 
    {
        smurf_struct->module_mode=M_MEMORY;
        return;
    }
    
    for(y=0;y<n_height;y++)
    {
        noffset = n_pic + y*nbpl;
        for(x=0;x<n_width;x++)
        {
            *(noffset++) = 0;
            *(noffset++) = 0;
            *(noffset++) = 0;
        }
    }
    
/*---Orginalbild kopieren------------------------------*/
    for (y=0;y<height;y++)
    {
        offset = pic + (long)y*bpl;
        noffset = n_pic + (long)y*2L*nbpl;
        for(x=0;x<width;x++)
        {
            *(noffset++) = *(offset++);
            *(noffset++) = *(offset++);
            *(noffset++) = *(offset++);
            noffset += 3;
        }
    }


/*---In X-Richtung skalieren---------------------------------*/
    for (x=1;x<n_width;x+=2)
    {
        noffset = n_pic + (long)x*3L;
        for(y=0;y<n_height;y++)
        {
            red   = *(noffset - 3L) + *(noffset + 3L);
            green = *(noffset - 2L) + *(noffset + 4L);
            blue  = *(noffset - 1L) + *(noffset + 5L);
            
            *(noffset     ) = (char)(red  >>1);
            *(noffset + 1L) = (char)(green>>1);
            *(noffset + 2L) = (char)(blue >>1);
        
            noffset += nbpl;
        }
    }

/*---In Y-Richtung skalieren---------------------------------*/
    for (y=1;y<n_height;y+=2)
    {
        noffset = n_pic + (long)y*nbpl;
        for(x=0;x<n_width;x++)
        {
            red   = *(noffset - nbpl     ) + *(noffset + nbpl);
            green = *(noffset - nbpl +1L ) + *(noffset + nbpl + 1L);
            blue  = *(noffset - nbpl +2L ) + *(noffset + nbpl + 2L);
            
            *(noffset     ) = (char)(red  >>1);
            *(noffset + 1L) = (char)(green>>1);
            *(noffset + 2L) = (char)(blue >>1);
        
            noffset += 3;
        }
    }   

    
    picture->pic_width  = n_width;
    picture->pic_height = n_height;
    picture->pic_data   = n_pic;
    
    Mfree(pic);

smurf_struct->module_mode=M_DONEEXIT;
return;
}

/* Wenn das Modul sich verpissen soll */
if(smurf_struct->module_mode==MTERM)
    {
    smurf_struct->module_mode=M_EXIT;
    return; 
    }

} /*ende*/


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
