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

/*  ---------- FarbsÑttigung-Modul V0.2 --------------  */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\..\import.h"
#include "..\..\..\..\smurf\smurfine.h"

/*--------------- Funktionen -------------------*/
/*----------------------------------------------*/
/*-----> Smurf --------*/
void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);

/*-----> Modul --------*/
int do_it(GARGAMEL *smurf_struct);


/*--------------- Globale Variablen --------------------------*/
unsigned long busycount, busymax, busycall;


/*--------------- Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
{
    "Farb-SÑttigung",                       /* Name des Moduls */
    0x0020,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "F-SÑt %",
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
    0,200,
    0,0,
    0,0,
    0,0,
/* min/max fÅr Editobjekte */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaultwerte fÅr Slider, Check und Edit */
    100,0,0,0,
    0,0,0,0,
    0,0,0,0,
    1,
    "Bild 1"
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


/*-----------------------  FUNCTION MAIN --------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
SMURF_PIC *picture;
int module_id;

module_id=smurf_struct->module_number;

/* Wenn das Modul aufgerufen wurde, */
if(smurf_struct->module_mode == MSTART)
{
    smurf_struct->f_module_prefs(&module_info, module_id);
    smurf_struct->module_mode=M_WAITING;
    return; 
}

/* Wenn das Modul gestartet wurde */
if(smurf_struct->module_mode == MEXEC)
{
    smurf_struct->module_mode = do_it(smurf_struct);
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



/*--------------- Farb-SÑttigung ----------------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture;
    int width, height, colsat;
    char *pic, *cs_tab, *offset, *cs_offset;
    signed int *div_tab, *div_offset;
    long bpl, div;
    float cs_fak;
    signed int x, y;
    long red,green,blue,grey;

    /*--- Slider auslesen ---------------------- */ 
    
    colsat = (int)smurf_struct->slide1;
    if (colsat == 100) return(M_PICDONE);   /*--- Keine énderung --> BEENDEN --------*/
    
    cs_fak = colsat / 100.0;
    
    /*--- Bilddaten auslesen --------------------*/
 
    picture = smurf_struct->smurf_pic;  
    width   = picture->pic_width;
    height  = picture->pic_height; 
    pic     = picture->pic_data;
    
    
    if(colsat != 0) 
    {
        /*--- Speicher anfordern ----------------------*/
        cs_tab = Malloc(256L*256L);
        if(cs_tab == NULL) return(M_MEMORY); /*  Kein Speicher !!! */
        div_tab = Malloc(256L*2L);
        if(div_tab == NULL) 
        {
            Mfree(cs_tab);
            return(M_MEMORY); /*  Kein Speicher !!! */
        }
        /*--- Farb-SÑttigungs-Tabelle berechnen ------------*/
        
        div_offset = div_tab;
        for(x=0; x<512; x++)
        {       
            *(div_offset++) = (signed int)((x-255) * cs_fak);
        }   
        
        offset = cs_tab;
        for(y=0; y<256; y++)
        {
            for(x=0; x<256; x++)
            {
                div = y + *(div_tab + (long)((x-y) + 255));
                if(div < 0) div = 0;
                if(div > 255) div = 255;
                
                *(offset++) = (char)div;
            }
        }
    }
    
    
    /*--- BusyBox Vorberechnungen ----------------------*/
    busycount = 0;
    busycall =  3;
    busymax = height;
    if(busymax >  32) busycall = 15;
    if(busymax > 256) busycall = 63;
    busymax = (busymax<<10) /127;


    /*--- Hauptroutine --------------------*/

    if(colsat != 0)
    {                       /*--- Freie Farb-SÑttigung ----------*/         
        offset = pic;
        for(y=0; y<height; y++)
        {
            busycount++;
            if(!(busycount & busycall)) smurf_struct->busybox((int)((busycount<<10) / busymax));
            for(x=0; x<width; x++)
            {
                red   = *(offset++);
                green = *(offset++);
                blue  = *(offset);
                offset -= 2;
                
                grey = ((red + green + blue) * 21846L)>>16;
                cs_offset = cs_tab +(grey<<8);
                
                *(offset++) = *(cs_offset + red);
                *(offset++) = *(cs_offset + green);
                *(offset++) = *(cs_offset + blue); 
            }
        }
    }
    else
    {                       /*--- Farb-SÑttigung ist NULL --> Graubild -------*/
        offset = pic;
        for(y=0; y<height; y++)
        {
            busycount++;
            if(!(busycount & busycall)) smurf_struct->busybox((int)((busycount<<10) / busymax));
            for(x=0; x<width; x++)
            {
                red   = *(offset++);
                green = *(offset++);
                blue  = *(offset);
                offset -= 2;
                
                grey = ((red + green + blue) * 21846L)>>16;
                                
                *(offset++) = grey;
                *(offset++) = grey;
                *(offset++) = grey; 
            }
        }
    }

    if(colsat != 0)
    {
        Mfree(div_tab);
        Mfree(cs_tab);
    }
    return(M_PICDONE);
}


