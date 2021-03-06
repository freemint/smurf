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

/*  ----------- Softshadow-Modul V1.21 ---------------  */
/*          F�r SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include <math.h>
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


/*--------------- Infostruktur f�r Hauptprogramm -----*/
MOD_INFO    module_info=
{
    "Drop Shadow",                     /* Name des Moduls */
    0x0121,
    "J�rg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen f�r Importer */
/* 4 Slider�berschriften: max 8 */
    "St�rke",
    "Weich",
    "X-Abstand",
    "Y-Abstand",
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
    1,255,
    1,20,
    -30,30,
    -30,30,
/* min/max f�r Editobjekte */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaultwerte f�r Slider, Check und Edit */
    128,2,5,5,
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
if(SmurfMessage==MTERM)
{
    smurf_struct->module_mode=M_EXIT;
    return; 
}

} /*ende*/


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



/*--------------- Soft-Shadow ----------------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture;
    int width, height, strengh, softness;
    char *pic, *greypic, *tcyoffset, *tcoffset, *offset, *moffset, *mxoffset;
    long bpl;
    int x, y, mx, my;
    int mgx_o, mgy_o;
    int x_start, x_end, y_start, y_end;
    int mw, mh;
    long shadowfak, shadowcount;
    long color;
    char grey, mred, mgreen, mblue;
    
    int radius;
    int x_quad,y_quad;
    
    int mx_s[50];
    int mx_e[50];
    int mx_cs,mx_ce,my_cs;
    int x_o, y_o;

    /*--- Slider auslesen ---------------------- */ 
    
    strengh = (int)smurf_struct->slide1;
    if (strengh == 0) return(M_PICDONE);   /*--- Keine �nderung --> BEENDEN --------*/

    softness = (int)smurf_struct->slide2;

    x_o = (int)smurf_struct->slide3;
    y_o = (int)smurf_struct->slide4;
    
    mw = mh = softness*2 + 1;
    
    
    /*--- Radius-Maske generieren ------------------------*/
    shadowcount = 0;
    for(y=0; y<mh; y++)
    {
        y_quad = (y-softness)*(y-softness);
        x=0;
        do
        {
            x_quad = (x-softness)*(x-softness);
            radius = 0.5 + sqrt(x_quad + y_quad);
            x++;
        }while(radius > softness);
        mx_s[y] = x-1;
        
        do
        {
            x_quad = (x-softness)*(x-softness);
            radius = 0.5 + sqrt(x_quad + y_quad);
            x++;
        }while( (radius <= softness) && (x<=mw));
        mx_e[y] = x-1;
        shadowcount += mx_e[y] - mx_s[y];
    }
    
    shadowfak = 65535 / shadowcount;
    
    
    /*--- Bilddaten auslesen --------------------*/
 
    picture = smurf_struct->smurf_pic;  
    width   = picture->pic_width;
    height  = picture->pic_height; 
    pic     = picture->pic_data;
    
    bpl = width*3L;


/*---GRAU-BILD generieren ---------------------------------*/
    mred   = *(pic);
    mgreen = *(pic+1);
    mblue  = *(pic+2);

    greypic = Malloc((long)width * height);
    if(greypic == NULL)
    {
        printf("MEMORY !!!!");
        return(M_MEMORY);
    }

    offset = pic;
    moffset = greypic;
    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            
            if( (*(offset++) != mred) | 
                    (*(offset++) != mgreen) | 
                    (*(offset++) != mblue)
                )
            {
                *(moffset++) = (char)strengh;
            }
            else
            {
                *(moffset++) = 0;
            }
        }
    }

    /*--- Pre-Calcs ---------------------------------------*/
    
    x_start = softness + x_o;
    if(x_start < 0) x_start = 0;
    x_end = width - softness + x_o;
    if(x_end > width) x_end = width;
    
    y_start = softness + y_o;
    if(y_start < 0) y_start = 0;
    y_end = height - softness + y_o;
    if(y_end > height) y_end = height;



    /*--- BusyBox Vorberechnungen ----------------------*/
    busycount = 0;
    busycall =  31;
    busymax = y_end - y_start;
    if(busymax >  32) busycall = 15;
    if(busymax > 256) busycall = 1;
    busymax = (busymax<<10) /127;


    /*--- Hauptroutine --------------------*/

    for(y=y_start; y<y_end; y++)
    {
        busycount++;
        if(!(busycount & busycall)) smurf_struct->services->busybox((int)((busycount<<10) / busymax));
        
        /*--- main-part -----------------*/
        tcyoffset = pic + (long)y*bpl;
        offset = greypic + (long)y*width + (long)x_start;
        for(x=x_start; x<x_end; x++)
        {
            if(*(offset++) == 0)
            {
                mgx_o = x-x_o-softness;
                mgy_o = y-y_o-softness;
                shadowcount = 0;
                for(my=0; my<mh; my++)
                {
                    mxoffset = greypic + (long)(mgy_o+my)*width;
                    for(mx=mx_s[my]; mx<mx_e[my]; mx++)
                    {
                        shadowcount += *(mxoffset + (long)(mgx_o+mx));
                    }
                }
            
                grey = 255 - ((shadowcount * shadowfak) >> 16);
                
                tcoffset = tcyoffset + x*3L;
                
                *(tcoffset++) = grey;   
                *(tcoffset++) = grey;   
                *(tcoffset++) = grey;                   
            
            }
        }   
        
        /*--- left-part -------------------------------------*/
        tcyoffset = pic + (long)y*bpl;
        offset = greypic + (long)y*width;
        for(x=0; x<x_start; x++)
        {
            if(*(offset++) == 0)
            {
                shadowcount = 0;
                mgx_o = x-x_o-softness;
                mgy_o = y-y_o-softness;
                for(my=0; my<mh; my++)
                {
                    if((mgx_o + mx_e[my]) > 0)
                    {
                        mx_cs = mx_s[my];
                        if((mgx_o + mx_cs) < 0) mx_cs -= (mgx_o + mx_cs);
                        
                        mxoffset = greypic + (long)(mgy_o + my)*width;
                        for(mx=mx_cs; mx<mx_e[my]; mx++)
                        {
                            shadowcount += *(mxoffset + (long)(mgx_o+mx));
                        }
                    }
                }
            
                grey = 255 - ((shadowcount * shadowfak) >> 16);
                
                tcoffset = tcyoffset + x*3L;
                
                *(tcoffset++) = grey;   
                *(tcoffset++) = grey;   
                *(tcoffset++) = grey;                   
            
            }
        }
            
        /*--- right-part ------------------------------------*/
        tcyoffset = pic + (long)y*bpl;
        offset = greypic + (long)y*width + x_end;
        for(x=x_end; x<width; x++)
        {
            if(*(offset++) == 0)
            {
                shadowcount = 0;
                mgx_o = x-x_o-softness;
                mgy_o = y-y_o-softness;
                for(my=0; my<mh; my++)
                {
                    if((mgx_o + mx_s[my]) < width)
                    {
                        mx_cs = mx_e[my];
                        if((mgx_o + mx_cs) > width) mx_cs -= (width-(mgx_o + mx_cs));
                        
                        mxoffset = greypic + (long)(mgy_o + my)*width;
                        for(mx=mx_s[my]; mx<mx_cs; mx++)
                        {
                            shadowcount += *(mxoffset + (long)(mgx_o+mx));
                        }
                    }
                }
            
                grey = 255 - ((shadowcount * shadowfak) >> 16);
                
                tcoffset = tcyoffset + x*3L;
                
                *(tcoffset++) = grey;   
                *(tcoffset++) = grey;   
                *(tcoffset++) = grey;                   
            
            }
        }
                    
    } /*--- ende Y-schleife ----------------*/


    /*--- upper-part ---------------------------------------*/
    for(y=0; y<y_start; y++)
    {
        busycount++;
        if(!(busycount & busycall)) smurf_struct->services->busybox((int)((busycount<<10) / busymax));
        
        mgy_o = y-y_o-softness;
        if((mgy_o + mh) > 0)
        {
            tcyoffset = pic + (long)y*bpl;
            offset = greypic + (long)y*width;
            for(x=0; x<width; x++)
            {
                if(*(offset++) == 0)
                {
                    shadowcount = 0;
                    mgx_o = x-x_o-softness;
                    
                    my_cs =0;
                    if((mgy_o + my_cs) < 0) my_cs -= (mgy_o + my_cs);
                    
                    for(my=my_cs; my<mh; my++)
                    {
                        if( ((mgx_o+mx_e[my])>0) || ((mgx_o+mx_s[my])<width))
                        {
                            mx_cs = mx_s[my];
                            if((mgx_o + mx_cs) < 0) mx_cs -= (mgx_o + mx_cs);
                            mx_ce = mx_e[my];
                            if((mgx_o + mx_ce) > width) mx_ce += width -(mgx_o + mx_ce);
                            
                            mxoffset = greypic + (long)(mgy_o + my)*width;
                            for(mx=mx_cs; mx<mx_ce; mx++)
                            {
                                shadowcount += *(mxoffset + (long)(mgx_o+mx));
                            }
                        }
                    }
                
                    grey = 255 - ((shadowcount * shadowfak) >> 16);
                    
                    tcoffset = tcyoffset + x*3L;
                    
                    *(tcoffset++) = grey;   
                    *(tcoffset++) = grey;   
                    *(tcoffset++) = grey;                   
                
                }
            }   
        }
        else
        {
            tcyoffset = pic + (long)y*bpl;
            offset = greypic + (long)y*width;
            for(x=0; x<width; x++)
            {
                if(*(offset++) == 0)
                {
                    *tcyoffset++ = 255;
                    *tcyoffset++ = 255;
                    *tcyoffset++ = 255;
                }
                else
                {
                 tcyoffset += 3;
                }
            }
        }
                    
    } /* Y-schleife ----------------*/



    /*--- lower-part ---------------------------------------*/
    for(y=y_end; y<height; y++)
    {
        busycount++;
        if(!(busycount & busycall)) smurf_struct->services->busybox((int)((busycount<<10) / busymax));
        
        mgy_o = y-y_o-softness;
        if(mgy_o < height )
        {
            tcyoffset = pic + (long)y*bpl;
            offset = greypic + (long)y*width;
            for(x=0; x<width; x++)
            {
                if(*(offset++) == 0)
                {
                    shadowcount = 0;
                    mgx_o = x-x_o-softness;
                    
                    my_cs = mh;
                    if((mgy_o + my_cs) > height) my_cs += height - (mgy_o + my_cs);
                    
                    for(my=0; my<my_cs; my++)
                    {
                        if( ((mgx_o+mx_e[my])>0) || ((mgx_o+mx_s[my])<width))
                        {
                            mx_cs = mx_s[my];
                            if((mgx_o + mx_cs) < 0) mx_cs -= (mgx_o + mx_cs);
                            mx_ce = mx_e[my];
                            if((mgx_o + mx_ce) > width) mx_ce += width -(mgx_o + mx_ce);
                            
                            mxoffset = greypic + (long)(mgy_o + my)*width;
                            for(mx=mx_cs; mx<mx_ce; mx++)
                            {
                                shadowcount += *(mxoffset + (long)(mgx_o+mx));
                            }
                        }
                    }
                
                    grey = 255 - ((shadowcount * shadowfak) >> 16);
                    
                    tcoffset = tcyoffset + x*3L;
                    
                    *(tcoffset++) = grey;   
                    *(tcoffset++) = grey;   
                    *(tcoffset++) = grey;                   
                
                }
            }   
        
        }
        else
        {
            tcyoffset = pic + (long)y*bpl;
            offset = greypic + (long)y*width;
            for(x=0; x<width; x++)
            {
                if(*(offset++) == 0)
                {
                    *tcyoffset++ = 255;
                    *tcyoffset++ = 255;
                    *tcyoffset++ = 255;
                }
                else
                {
                 tcyoffset += 3;
                }
            }
        }           
    } /* Y-schleife ----------------*/



    Mfree(greypic);
    
    return(M_PICDONE);
}


