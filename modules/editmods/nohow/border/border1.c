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

/*  ------------- Border-Modul V0.5 ------------------  */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\..\..\sym_gem.h"
#include "..\..\..\import.h"
#include "..\..\..\..\smurf\smurfine.h"

/*-----> Modul --------*/
int do_it(GARGAMEL *smurf_struct);


/*--------------- Globale Variablen --------------------------*/
unsigned long busycount, busymax, busycall;


/*--------------- Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
{
    "Border-Me-Up..",                      /* Name des Moduls */
    0x0010,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "Red",
    "Green",
    "Blue",
    "",
/* 4 CheckboxÅberschriften: */
    "Center",
    "Use Color",
    "",
    "",
/* 4 Edit-Objekt-öberschriften: */
    "left",
    "right",
    "up",
    "down",
/* min/max-Werte fÅr Slider */
    0,255,
    0,255,
    0,255,
    0,0,
/* min/max fÅr Editobjekte */
    0,32768,
    0,32768,
    0,32768,
    0,32768,
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
                        0
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
    
    int x, y, i;
    

    /*--- Slider auslesen ---------------------- */
    
    bd_left = (int)smurf_struct->edit1;
    bd_right = (int)smurf_struct->edit2;    
    bd_up = (int)smurf_struct->edit3;
    bd_down = (int)smurf_struct->edit4;
    
    /*--- Bilddaten auslesen --------------------*/
 
    picture = smurf_struct->smurf_pic;  
    width   = picture->pic_width;
    height  = picture->pic_height; 
    pic     = picture->pic_data;
    
    bpl_a = (long)width *3L;
    
    /*--- Neues Bild festlegen ----------------*/
    n_width = bd_left + bd_right + width;
    n_height = bd_up + bd_down + height;        
    bpl_b = (long)n_width * 3L;
    
    n_pic = Malloc((long)n_width * n_height *3L);
    if(n_pic == NULL)
        return(M_MEMORY);
    
    
    if(smurf_struct->check1)  /* Zentriert */
    {
        x_pos = (n_width-width)/2;
        y_pos = (n_height-height)/2;
    }
    else
    {
        x_pos = bd_left;
        y_pos = bd_up;
    } 
    
    if(smurf_struct->check2)
    {
        bkgrd_r = smurf_struct->slide1; 
        bkgrd_g = smurf_struct->slide2; 
        bkgrd_b = smurf_struct->slide3;
    }
    else
    {
        bkgrd_r = pic[0]; 
        bkgrd_g = pic[1]; 
        bkgrd_b = pic[2]; 
    }
     
    offset_b = n_pic;
    for(y=0; y<n_height; y++)
    {
        for(x=0; x< n_width; x++)
        {
            *offset_b++ = bkgrd_r;
            *offset_b++ = bkgrd_g;
            *offset_b++ = bkgrd_b;
        }
    }
     
     
     
    /*--- BusyBox Vorberechnungen ----------------------*/
    busycount = 0;
    busycall =  1;
    busymax = height;
    if(busymax >  32) busycall =  7;
    if(busymax > 256) busycall = 31;
    busymax = (busymax<<10) /127;

    
    /*--- Hauptroutine --------------------*/

    offset_a = pic;
    for (y=0; y<height; y++)
    {
        busycount++;
        if(!(busycount & busycall)) smurf_struct->services->busybox((busycount<<10) / busymax);
            
        offset_b = n_pic + (long)(y+y_pos)*bpl_b + (long)(x_pos)*3L;
        for(x=0; x<width; x++)
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


