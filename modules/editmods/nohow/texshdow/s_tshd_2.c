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

/*
      /---------------- Soft-Tex-Shadow-Modul V1.1 ---------------\
        |                                                                                   |
        |               FÅr SMURF Bildkonverter, 26.04.96                       |    
        |                                                                                   |
        |     Wirft einen weichen Schatten auf ein beliebiges           |
        |       Hintergrundbild (wenn zu klein wird es gekachelt).      |                                                                                   |
        |       last change: 3.04.97                                                                  |
      \___________________________________________________________/
*/


#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include <math.h>

#include "..\..\..\..\sym_gem.h"
#include "..\..\..\import.h"
#include "..\..\..\..\smurf\smurfine.h"

/*------------------------ Funktionen -----------------------*/
/*-----------------------------------------------------------*/
/*-----> Smurf                            */
void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);

/*-----> Modul                            */
int do_it(GARGAMEL *smurf_struct);


/*--------------- Globale Variablen --------------------------*/
/*               -------------------                                         */
unsigned long busycount, busymax, busycall;

SMURF_PIC *input_pictures[3];

char p1string[20] = "Shadowpic";
char p2string[20] = "Background";

/*--------------- Infostruktur fÅr Hauptprogramm --------------*/
/*               --------------------------------                */
MOD_INFO    module_info=
{
    "Drop Tex-Shadow",                       /* Name des Moduls */
    0x0110,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "StÑrke",
    "Weich",
    "X-Abstand",
    "Y-Abstand",
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
    1,255,
    1,20,
    -30,30,
    -30,30,
/* min/max fÅr Editobjekte */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaultwerte fÅr Slider, Check und Edit */
    128,5,10,10,
    0,0,0,0,
    0,0,0,0,
    2,
    p1string,
    p2string
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

if(SmurfMessage == MPICTURE)
{
    /* Bild holen */
    input_pictures[smurf_struct->event_par[0]] = smurf_struct->smurf_pic;

    /* und weiter warten */
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



/*--------------- Soft-Shadow ----------------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture,*texture;
    int width, height, strengh, softness;
    char *pic, *greypic, *tcyoffset, *tcoffset, *offset, *moffset, *mxoffset;
    int twidth,theight;
    char *texpic;
    long bpl,tbpl;
    int x, y, mx, my, tx, ty;
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
    if (strengh == 0) return(M_PICDONE);   /*--- Keine énderung --> BEENDEN --------*/

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

    texture = input_pictures[1];
    twidth = texture->pic_width;
    theight = texture->pic_height;
    texpic = texture->pic_data;
        
    tbpl = twidth * 3L; 
    
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

/*--- Texture auf Schattenbild mappen -----------------------*/

    for(y=0; y<height; y++)
    {
        ty = y % theight;
        offset = pic + bpl*y;
        tcoffset = texpic + tbpl*ty;
        moffset = greypic + (long)y * width;
        tx = 0;
        for(x=0; x<width; x++)
        {
            if(tx == twidth) 
            {
                tx = 0;
                tcoffset = texpic + tbpl*ty;
            }
            
            if( *(moffset++) == 0)
            {
                *(offset++) = *(tcoffset++);
                *(offset++) = *(tcoffset++);
                *(offset++) = *(tcoffset++);
            }
            else
            {
                offset += 3;
                tcoffset += 3;
            }
            
                        
            tx++;
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
    busycall =  1;
    busymax = height;
    if(busymax >  32) busycall = 3;
    if(busymax > 256) busycall = 7;
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
                
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);
            
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
                
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);

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
            
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);
            
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
                    
                    *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                    *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                    *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);
        
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
                    
                    *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                    *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);    
                    *(tcoffset++) = (char)(((unsigned int)*(tcoffset) * grey) >> 8);
        
                }
            }   
        
        }
                    
    } /* Y-schleife ----------------*/



    Mfree(greypic);
    
    smurf_struct->event_par[0] = 0;
    smurf_struct->event_par[1] = -1;
    
    return(M_PICDONE);
}


  
