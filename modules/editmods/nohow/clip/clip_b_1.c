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

/*  ------------- Clip Border-Modul V0.5 ------------------ */
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


/*-----> Modul --------*/
int do_it(GARGAMEL *smurf_struct);


/*--------------- Globale Variablen --------------------------*/
unsigned long busycount, busymax, busycall;


/*--------------- Infostruktur fÅr Hauptprogramm -----*/
#ifdef ENGLISH
MOD_INFO    module_info=
{
    "Clip Border",                     /* Name des Moduls */
    0x0050,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "",
    "",
    "",
    "",
/* 4 CheckboxÅberschriften: */
    "Center",
    "",
    "",
    "",
/* 4 Edit-Objekt-öberschriften: */
    "",
    "t",
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
    1,
    "Bild 1"
};
#else
MOD_INFO    module_info=
{
    "Clip Border",                     /* Name des Moduls */
    0x0050,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "",
    "",
    "",
    "",
/* 4 CheckboxÅberschriften: */
    "Zentriert",
    "",
    "",
    "",
/* 4 Edit-Objekt-öberschriften: */
    "",
    "t",
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
    1,
    "Bild 1"
};
#endif

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
                        0 /* Preview Flag */ 
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

    smurf_struct->module_mode=M_STARTED;
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


/*--------------- SCALING ----------------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture;
    int width, height;
    char *pic, *n_pic, *offset_a, *offset_b;
    long bpl_a, bpl_b;
    char bkgrd_r, bkgrd_g, bkgrd_b; 
    int bd_left, bd_right, bd_up, bd_down;
    int n_width, n_height, x_pos, y_pos;
    
    int x, y, hit;
    

    /*--- Bilddaten auslesen --------------------*/
 
    picture = smurf_struct->smurf_pic;  
    width   = picture->pic_width;
    height  = picture->pic_height; 
    pic     = picture->pic_data;
    
    bpl_a = (long)width *3L;
    
    bkgrd_r = pic[0];
    bkgrd_g = pic[1];
    bkgrd_b = pic[2];
    
    /*--- Scan Borders --------------------------------*/
    y = 0;
    hit = 0;
    do {
        offset_a = pic + (long)y*bpl_a;
        x = 0;
        do {
            if( (bkgrd_r != *offset_a++) ||
                    (bkgrd_g != *offset_a++) ||
                    (bkgrd_b != *offset_a++) )
            {
                hit = 1;
            }
        } while(++x<width && !hit);
    
    } while(++y<height && !hit);
    bd_up = y-1;
    
    y = height-1;
    hit = 0;
    do {
        offset_a = pic + (long)y*bpl_a;
        x = 0;
        do {
            if( (bkgrd_r != *offset_a++) ||
                    (bkgrd_g != *offset_a++) ||
                    (bkgrd_b != *offset_a++)  )
            {
                hit = 1;
            }
        } while(++x<width && !hit);
    
    } while(y-->0 && !hit);
    bd_down = y+1;
     
    x = 0;
    hit = 0;
    do {
        offset_a = pic + (long)x+x+x;
        y = 0;
        do {
            if( (bkgrd_r != *offset_a++) ||
                    (bkgrd_g != *offset_a++) ||
                    (bkgrd_b != *offset_a) )
            {
                hit = 1;
            }
            
            offset_a += bpl_a-2L;
            
        } while(++y<height && !hit);
    
    } while(++x<width && !hit);
    bd_left = x-1; 
    
    x = width-1;
    hit = 0;
    do {
        offset_a = pic + (long)x+x+x;
        y = 0;
        do {
            if( (bkgrd_r != *offset_a++) ||
                    (bkgrd_g != *offset_a++) ||
                    (bkgrd_b != *offset_a)  )
            {
                hit = 1;
            }
            
            offset_a += bpl_a-2L;
            
        } while(++y<height && !hit);
    
    } while(x-->0 && !hit);
    bd_right = x+1;  
     
    n_width = bd_right - bd_left +1; 
    n_height = bd_down - bd_up +1; 
    bpl_b = (long)n_width*3L; 
    
    x_pos = bd_left;
    y_pos = bd_up;
    
    
    if( (n_width<=0) || (n_width>width) || 
            (n_height<=0) || (n_height>height) )
    {
        return(M_WAITING);
    }
    
    
    
    n_pic = Malloc((long)n_width*n_height*3L);
    if(n_pic == NULL)
    {
        return(M_MEMORY);
    }
     
    /*--- BusyBox Vorberechnungen ----------------------*/
    busycount = 0;
    busycall =  1;
    busymax = n_height;
    if(busymax >  32) busycall =  7;
    if(busymax > 256) busycall = 31;
    busymax = (busymax<<10) /127;

    
    /*--- Hauptroutine --------------------*/

    offset_b = n_pic;
    for (y=0; y<n_height; y++)
    {
        busycount++;
        if(!(busycount & busycall)) smurf_struct->services->busybox((busycount<<10) / busymax);
            
        offset_a = pic + ((long)y+y_pos)* bpl_a + (long)x_pos*3L; 
        for(x=0; x<n_width; x++)
        {
            *offset_b++ = *offset_a++;
            *offset_b++ = *offset_a++;
            *offset_b++ = *offset_a++;
        }
    }
    
    picture->pic_width = n_width;
    picture->pic_height = n_height; 
    picture->pic_data = n_pic;
    
    Mfree(pic);
    
    smurf_struct->event_par[0] = 0;
    smurf_struct->event_par[1] = -1;

    return(M_PICDONE);
}


