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

/*  ------------- SoftScale-Modul V0.2 ---------------  */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\..\import.h"
#include "..\..\..\..\src\smurfine.h"

/*--------------- Funktionen -------------------*/
/*----------------------------------------------*/
/*-----> Smurf --------*/
void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);

/*-----> Modul --------*/
int do_it(GARGAMEL *smurf_struct);
void X_Scale(char *opic, char *npic, long obpl, long nbpl, int owidth, int height, int nwidth, GARGAMEL *smurf_struct);
void Y_Scale(char *opic, char *npic, long obpl, long nbpl, int width, int oheight, int nheight, GARGAMEL *smurf_struct);


/*--------------- Globale Variablen --------------------------*/
unsigned long busycount, busymax, busycall;


/*--------------- Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
{
    "SoftScale",                        /* Name des Moduls */
    0x0020,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "X-Fakt. %",
    "Y-Fakt. %",
    "",
    "",
/* 4 CheckboxÅberschriften: */
    "Use-Res",
    "",
    "",
    "",
/* 4 Edit-Objekt-öberschriften: */
    "X-Res",
    "Y-Res",
    "",
    "",
/* min/max-Werte fÅr Slider */
    50,200,
    50,200,
    0,0,
    0,0,
/* min/max fÅr Editobjekte */
    1,32768,
    1,32768,
    0,0,
    0,0,
/* Defaultwerte fÅr Slider, Check und Edit */
    100,100,0,0,
    0,0,0,0,
    640,480,0,0,
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



/*--------------- SCALING ----------------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture;
    int x_fak, y_fak, x_res, y_res;
    int width, height, n_width, n_height;
    char *pic, *n_pic, *offset, *n_offset;
    long bpl, n_bpl;
    

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
    
    /*--- Faktor bestimmen -----------------------------*/
    
    if(smurf_struct->check1 != 0)
    {
        n_width = x_res;
        n_height = y_res;
    }
    else
    {
        n_width  = (int)((long)x_fak * (long)width  / 100);
        n_height = (int)((long)y_fak * (long)height / 100);     
    }
        
    bpl = (long)width *3L;
    n_bpl = (long)n_width *3L;
    
    
    /*--- BusyBox Vorberechnungen ----------------------*/
    busycount = 0;
    busycall = 31;
    
    
    
    /*--- Keine Scalierung --> Beenden !!! ------------------*/
    if((width == n_width) && (height == n_height)) return(M_WAITING);
    
    /*--- Zu kleine Scalierung --> Beenden !!! --------------*/
    if((n_width < 8) || (n_height < 8)) return(M_WAITING);
    
    /*--- Speicher fÅr neues Bild ---------------------*/
    n_pic = Malloc((long)n_width*(long)n_height*3L);
    
    if(n_pic == NULL) return(M_MEMORY); /*  Kein Speicher !!! */
    
    
    
    /*--- Hauptroutine --------------------*/
    
    if(n_width == width)
    {                   /*--- Nur Y-Scale -------*/
        busymax = ((long)width<<10) / 127;
        Y_Scale(pic, n_pic, bpl, n_bpl, width, height, n_height, smurf_struct);
    }
    else
    {
        if(n_height == height)
        {               /*--- Nur X-Scale --------*/        
            busymax = ((long)height<<10) / 127;
            X_Scale(pic, n_pic, bpl, n_bpl, width, height, n_width, smurf_struct); 
        }
        else
        {               /*--- X- und Y-Scale -------*/
            if(n_width < width)
            {                   /*--- Zuerst X-Scale ---*/
                busymax = (((long)height + (long)n_width)<<10) / 127;
                X_Scale(pic, pic, bpl, bpl, width, height, n_width, smurf_struct);
                Y_Scale(pic, n_pic, bpl, n_bpl, n_width, height, n_height, smurf_struct);
            }
            else
            {
                if(n_height < height)
                {               /*--- Zuerst Y-Scale ---*/
                    busymax = (((long)width + (long)n_height)<<10) / 127;
                    Y_Scale(pic, pic, bpl, bpl, width, height, n_height, smurf_struct);
                    X_Scale(pic, n_pic, bpl, n_bpl, width, n_height, n_width, smurf_struct);
                }
                else
                {               /*--- Egal !!! (weil beide grîûer) ---*/
                    busymax = (((long)height + (long)n_width)<<10) / 127;
                    X_Scale(pic, n_pic, bpl, n_bpl, width, height, n_width, smurf_struct);
                    Y_Scale(n_pic, n_pic, n_bpl, n_bpl, n_width, height, n_height, smurf_struct);
                }
            }
        }
    }
        
    
    
    /*--- Neue Bildwerte in Struktur schreiben -------*/
    picture->pic_width = n_width;
    picture->pic_height = n_height;
    picture->pic_data = n_pic;
    
    Mfree(pic);

    return(M_PICDONE);
}


void X_Scale(char *opic, char *npic, long obpl, long nbpl, int owidth, int height, int nwidth, GARGAMEL *smurf_struct)
{
    char *o_offset, *n_offset;
    unsigned int base, rest, step;
    unsigned long count;
    long red,green,blue;
    long sred,sgreen,sblue, ered,egreen,eblue;
    unsigned int x, y, i, p;
    char *checkaddress;
    
    if(nwidth < owidth)
    {               /* -Extrapolieren */
        base = (unsigned int)(owidth / nwidth);
        rest = owidth - (base * nwidth);
        step = (unsigned int)((long)rest * 0xFFFF / nwidth);
        checkaddress = npic + (long)height*nbpl;
        
        for(y=0; y<height; y++)
        {
            n_offset = npic + y*nbpl;
            o_offset = opic + y*obpl;
            count = 0;
            for(x=0; x<nwidth; x++)
            {
                p = base;
                red=green=blue = 0;
                for (i=0; i<base; i++)
                {
                    red   += *(o_offset++);
                    green += *(o_offset++);
                    blue  += *(o_offset++);
                }
                
                count += step;
                if(count >= 0xffff)
                {
                    count -= 0xffff;
                    p += 1;
                    red   += *(o_offset++);
                    green += *(o_offset++);
                    blue  += *(o_offset++);
                }
                
                *(n_offset++) = (char)(red   / p);
                *(n_offset++) = (char)(green / p);
                *(n_offset++) = (char)(blue  / p);              
            }
        }
    }
    else
    {               /* -Interpolieren */
        step = (((long)(owidth-1) * 0xffff)  / (nwidth-1));
        
        for(y=0; y<height; y++)
        {
            n_offset = npic + y*nbpl + (long)(nwidth-1)*3L;
            o_offset = opic + y*obpl + (long)(owidth-2)*3L;
            count = 0;
            busycount++;
            if(!(busycount & busycall)) smurf_struct->busybox((int)((busycount<<10) / busymax));
            
            ered   = *(o_offset++);
            egreen = *(o_offset++);
            eblue  = *(o_offset++);
            sred   = *(o_offset++);
            sgreen = *(o_offset++);
            sblue  = *(o_offset);
            o_offset -= 8;

            for(x=0; x<nwidth; x++)
            {
                *(n_offset++) = (char)(( (sred  *(0xffff-count)) + (ered  *count) )>>16);
                *(n_offset++) = (char)(( (sgreen*(0xffff-count)) + (egreen*count) )>>16);
                *(n_offset) = (char)(( (sblue *(0xffff-count)) + (eblue *count) )>>16);
                n_offset -= 5;
                
                count += step;
                if(count >= 0xffff)
                {
                    count -= 0xffff;
                    sred   = ered;
                    sgreen = egreen;
                    sblue  = eblue;
                    ered   = *(o_offset++);
                    egreen = *(o_offset++);
                    eblue  = *(o_offset);
                    o_offset -= 5;  
                }
            }
        }
    }
}

void Y_Scale(char *opic, char *npic, long obpl, long nbpl, int width, int oheight, int nheight, GARGAMEL *smurf_struct)
{
    char *o_offset, *n_offset;
    unsigned int base, rest, step;
    unsigned long count;
    long red,green,blue;
    long sred,sgreen,sblue, ered,egreen,eblue;
    unsigned int x, y, i, p;
    
    if(nheight < oheight)
    {               /* -Extrapolieren */
        base = (unsigned int)(oheight / nheight);
        rest = oheight - (base * nheight);
        step = (unsigned int)((long)rest * 0xFFFF / nheight);
    
        
        for(x=0; x<width; x++)
        {
            n_offset = npic + (long)x*3L;
            o_offset = opic + (long)x*3L;
            count = 0;
            busycount++;
            if(!(busycount & busycall)) smurf_struct->busybox((busycount<<10) / busymax);
            
            for(y=0; y<nheight; y++)
            {
                p = base;
                red=green=blue = 0;
                for (i=0; i<base; i++)
                {
                    red   += *(o_offset++);
                    green += *(o_offset++);
                    blue  += *(o_offset);
                    o_offset += (obpl -2L);
                }
                
                count += step;
                if(count >= 0xffff)
                {
                    count -= 0xffff;
                    p += 1;
                    red   += *(o_offset++);
                    green += *(o_offset++);
                    blue  += *(o_offset);
                    o_offset += (obpl -2L);
                }
                
                *(n_offset++) = (char)(red   / p);
                *(n_offset++) = (char)(green / p);
                *(n_offset) = (char)(blue  / p);
                n_offset += (nbpl - 2L);
            }
        }
    }
    else
    {               /* -Interpolieren */
        step = (((long)(oheight-1) * 0xffff)  / (nheight-1));
        
        for(x=0; x<width; x++)
        {
            n_offset = npic + (long)(nheight-1)*nbpl + (long)x*3L;
            o_offset = opic + (long)(oheight-2)*obpl + (long)x*3L;
            count = 0;
            busycount++;
            if(!(busycount & busycall)) smurf_struct->busybox((busycount<<10) / busymax);
            
            ered   = *(o_offset++);
            egreen = *(o_offset++);
            eblue  = *(o_offset);
            o_offset += (obpl -2L);
            sred   = *(o_offset++);
            sgreen = *(o_offset++);
            sblue  = *(o_offset);
            o_offset -= (obpl*2 + 2L);

            for(y=0; y<nheight; y++)
            {
                *(n_offset++) = (char)(( (sred  *(0xffff-count)) + (ered  *count) )>>16);
                *(n_offset++) = (char)(( (sgreen*(0xffff-count)) + (egreen*count) )>>16);
                *(n_offset)   = (char)(( (sblue *(0xffff-count)) + (eblue *count) )>>16);
                n_offset -= (nbpl + 2L);
                
                count += step;
                if(count >= 0xffff)
                {
                    count -= 0xffff;
                    sred   = ered;
                    sgreen = egreen;
                    sblue  = eblue;
                    ered   = *(o_offset++);
                    egreen = *(o_offset++);
                    eblue  = *(o_offset);
                    o_offset -= (obpl + 2L);    
                }
            }
        }
    }
}
