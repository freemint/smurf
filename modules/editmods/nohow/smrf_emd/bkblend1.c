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

/*  ------------- Edge-O-Kill-Modul V1.1 -------------  */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\..\..\sym_gem.h"
#include "..\..\..\import.h"
#include "..\..\..\..\src\smurfine.h"

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
    "Edge-O-Kill",                     /* Name des Moduls */
    0x0110,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "X-Blend %",
    "Y-Blend %",
    "",
    "",
/* 4 CheckboxÅberschriften: */
    "Use-Res",
    "",
    "",
    "",
/* 4 Edit-Objekt-öberschriften: */
    "X-Pixel",
    "Y-Pixel",
    "",
    "",
/* min/max-Werte fÅr Slider */
    0,100,
    0,100,
    0,0,
    0,0,
/* min/max fÅr Editobjekte */
    0,32768,
    0,32768,
    0,0,
    0,0,
/* Defaultwerte fÅr Slider, Check und Edit */
    0,0,0,0,
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
int SmurfMessage;
static int module_id;

SmurfMessage = smurf_struct->module_mode; 

/* Wenn das Modul aufgerufen wurde, */
if(SmurfMessage == MSTART)
{
    module_id=smurf_struct->module_number;

    smurf_struct->services->f_module_prefs(&module_info, module_id);
    smurf_struct->module_mode=M_WAITING;
    return; 
}

/* Wenn das Modul gestartet wurde */
if(SmurfMessage == MEXEC)
{
    smurf_struct->module_mode = do_it(smurf_struct);
    return;
}

/* Wenn das Modul sich verpissen soll */
if(SmurfMessage == MTERM)
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



/*--------------- SCALING ----------------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture;
    int x_fak, y_fak, x_res, y_res, blend_x, blend_y;
    int width, height;
    char *pic, *offset_a, *offset_b;
    long red_a,green_a,blue_a, red_b,green_b,blue_b;
    long bpl;
    unsigned int step, fak_a, fak_b;
    unsigned int fak_a1,fak_a2,fak_b1,fak_b2;
    int x, y, i;

    /*--- Slider auslesen ---------------------- */
    
    x_fak = (int)smurf_struct->slide1;
    y_fak = (int)smurf_struct->slide2;
    x_res = (int)smurf_struct->edit1;
    y_res = (int)smurf_struct->edit2;   
    
    /*--- Bilddaten auslesen --------------------*/
 
    picture = smurf_struct->smurf_pic;  
    width   = picture->pic_width;
    height  = picture->pic_height; 
    pic     = picture->pic_data;
    
    bpl = (long)width *3L;
    
    /*--- öberblendbereich festlegen ----------------*/
    if(smurf_struct->check1 != 0)
    {
        blend_x = x_res;
        blend_y = y_res;
    }
    else
    {
        blend_x = (int)((long)x_fak * (long)width  / 200);
        blend_y = (int)((long)y_fak * (long)height / 200);      
    }
    
    /*--- Kanten-Bereich Clippen --------------------*/
    if(blend_x > (width >>1)) blend_x = (width >>1);
    if(blend_y > (height>>1)) blend_y = (height>>1);
    
        
     
    /*--- BusyBox Vorberechnungen ----------------------*/
    busycount = 0;
    busycall =  1;
    busymax = blend_x + blend_y;
    if(busymax >  32) busycall =  7;
    if(busymax > 256) busycall = 31;
    busymax = (busymax<<10) /127;

    
    /*--- Hauptroutine --------------------*/

    if(blend_x != 0)   /*--- X-Kanten Åberblend ----*/
    {
        step = (unsigned int)( 0x10000L / ((((long)blend_x<<1))+1) );
        fak_a = 0x8000 + (step>>1);
            
        fak_a1 = fak_a;
        fak_a2 = 0x10000 - fak_a;
        
        for (x=0; x<blend_x; x++)
        {
            busycount++;
            if(!(busycount & busycall)) smurf_struct->services->busybox((busycount<<10) / busymax);
                
            offset_a = pic + x+x+x;
            offset_b = pic + (long)(width-1-x)*3L;
            for(y=0; y<height; y++)
            {
                red_a   = *(offset_a++);
                green_a = *(offset_a++);
                blue_a  = *(offset_a);
                offset_a -= 2;
                
                red_b   = *(offset_b++);
                green_b = *(offset_b++);
                blue_b  = *(offset_b);
                offset_b -= 2;
                
                *(offset_a++) = (char)((red_a   *fak_a1 + red_b   *fak_a2)>>16);
                *(offset_a++) = (char)((green_a *fak_a1 + green_b *fak_a2)>>16);
                *(offset_a) = (char)((blue_a  *fak_a1 + blue_b  *fak_a2)>>16);
                offset_a += (bpl-2);
                
                *(offset_b++) = (char)((red_a   *fak_a2 + red_b   *fak_a1)>>16);
                *(offset_b++) = (char)((green_a *fak_a2 + green_b *fak_a1)>>16);
                *(offset_b) = (char)((blue_a  *fak_a2 + blue_b  *fak_a1)>>16);  
                offset_b += (bpl-2);

            }
            fak_a1 += step;
            fak_a2 -= step;
        }
    }
    
    if(blend_y != 0)   /*--- Y-Kanten Åberblend ----*/
    {
        step = (unsigned int)( 0x10000L / ((((long)blend_y<<1))+1) );
        fak_a = 0x8000 + (step>>1);
            
        fak_a1 = fak_a;
        fak_a2 = 0x10000 - fak_a;
        
        for (y=0; y<blend_y; y++)
        {
            busycount++;
            if(!(busycount & busycall)) smurf_struct->services->busybox((busycount<<10) / busymax);
            
            offset_a = pic + y*bpl;
            offset_b = pic + (long)(height-1-y)*bpl;
            for(x=0; x<width; x++)
            {
                red_a   = *(offset_a++);
                green_a = *(offset_a++);
                blue_a  = *(offset_a);
                offset_a -= 2;
                
                red_b   = *(offset_b++);
                green_b = *(offset_b++);
                blue_b  = *(offset_b);
                offset_b -= 2;
                
                *(offset_a++) = (char)((red_a   *fak_a1 + red_b   *fak_a2)>>16);
                *(offset_a++) = (char)((green_a *fak_a1 + green_b *fak_a2)>>16);
                *(offset_a++) = (char)((blue_a  *fak_a1 + blue_b  *fak_a2)>>16);
                
                *(offset_b++) = (char)((red_a   *fak_a2 + red_b   *fak_a1)>>16);
                *(offset_b++) = (char)((green_a *fak_a2 + green_b *fak_a1)>>16);
                *(offset_b++) = (char)((blue_a  *fak_a2 + blue_b  *fak_a1)>>16);    
            }
            fak_a1 += step;
            fak_a2 -= step;
        }
    }


    return(M_PICDONE);
}


