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

/*  ------------- Smooth-Modul V0.5 ------------------  */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include "..\..\sym_gem.h"
#include "..\import.h"
#include "..\..\src\smurfine.h"


/*------ Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
    {
    "Weichzeichnen",
    0x0080,
    "Olaf Piesche",
    " "," "," "," "," "," "," "," "," "," ",
    "X-Str",
    "Y-Str",
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
    2,15,
    2,15,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    3,3,0,0,
    0,0,0,0,
    0,0,0,0,
    1,
    "","","","","",""
    };


/*--------------------- Was kann ich ? ----------------------*/
MOD_ABILITY  module_ability = {
                        24, 0, 0, 0, 0, 0, 0, 0,
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



void smooth_me(long *par);


/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
SMURF_PIC *picture;
int module_id;
int width, height;
int x,y;
char *data, *datacopy, *dest_data, *dest, *read;
long red, green, blue;
long linelen;
long *offset_table, *current_offset;
long counter, curr_counter, offset, old_offset;
int matw, math;
int ypos, xpos;
long par[10];

/* Wenn das Modul aufgerufen wurde, */
if(smurf_struct->module_mode == MSTART)
{
    module_id=smurf_struct->module_number;
    smurf_struct->services->f_module_prefs(&module_info, module_id);
    smurf_struct->module_mode=M_WAITING;
    return; 
}

/* Wenn das Modul gestartet wurde */
if(smurf_struct->module_mode == MEXEC)
{
    picture=smurf_struct->smurf_pic;
    data=picture->pic_data;
    width=picture->pic_width;
    height=picture->pic_height;
    linelen=(long)width*3L;
    datacopy=data;

    dest_data=dest=Malloc((long)height*linelen);

    /*
    xm1=-(long)(smurf_struct->slide1/2L);
    xm2=(long)(smurf_struct->slide1/2L);
    ym1=-(long)(smurf_struct->slide2/2L);
    ym2=(long)(smurf_struct->slide2/2L);
    */

    /*-------------------- Offset-Table vorbereiten --------------  */
    /*--------------------- Delta-Table (schnell!)  --------------- */
    matw=smurf_struct->slide1;
    math=smurf_struct->slide2;

    counter=(long)matw*(long)math;  

    offset_table=Malloc(counter);
    curr_counter=0;
    old_offset=0;
    
    for(y=0; y<math; y++)
    {
        for(x=0; x<matw; x++)
        {
            xpos=x-(matw/2);
            ypos=y-(math/2);

            offset = ((long)ypos*linelen)+(xpos*3L);
            
            offset_table[curr_counter]=offset - old_offset;
            
            /* Deltaoffset -2 ( wegen move.b (a1)+ ) */
            old_offset=offset+2;

            curr_counter++;     
        }
    }


    smurf_struct->services->reset_busybox(0, "Weichzeichnen...");


    par[0]=counter;
    par[1]=datacopy;
    par[2]=dest;
    par[3]=offset_table;

    for(y=0; y<height; y++)
    {
        if(!(y&15)) smurf_struct->services->busybox(((long)y<<7L)/(long)height);

        for(x=0; x<width; x++)
        {
            smooth_me(par);
            par[2]+=3;
            par[1]+=3;
        }

    }


Mfree(offset_table);
Mfree(data);

smurf_struct->smurf_pic->pic_data=dest_data;
smurf_struct->module_mode=M_PICDONE;

return;
}

/* Wenn das Modul sich verpissen soll */
if(smurf_struct->module_mode==MTERM)
{
    smurf_struct->module_mode=M_EXIT;
    return; 
}

}


