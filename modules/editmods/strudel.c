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

/*  -------------   Strudel V0.5    --------------- */
/*      F�r SMURF Bildkonverter, 19.10.95           */

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdi.h>
#include <aes.h>
#include <math.h>
#include <screen.h>
#include "..\..\sym_gem.h"
#include "..\import.h"
#include "..\..\src\smurfine.h"


#define betrag(a)   (a<0 ? -a : a)


BITBLK *prev(GARGAMEL *prev_struct);        /* Previewfunktion */
void f_doit(GARGAMEL *smurfstruct);


/* Infostruktur f�r Hauptmodul */
MOD_INFO    module_info={
                        "Strudel",
                        0x0050,
                        "Olaf Piesche",
                        "", "", "", "", "",
                        "", "", "", "", "",

                        "Winkel", "","","",
                        "Von au�en", "Tunnel",  "", "",
                        "", "", "", "",
                        0,360,
                        0,128,
                        0,128,
                        0,128,
                        0,10,
                        0,10,
                        0,10,
                        0,10,
                        90,0,0,0,
                        0,0,0,0,
                        0,0,0,0,
                        1
                        };



MOD_ABILITY module_ability=
{
/* Farbtiefen, die vom Modul unterst�tzt werden:            */
    24,0,0,0,0,0,0,0,
/*  Dazugeh�rige Datenformate (FORM_PIXELPAK/FORM_STANDARD/FORM_BOTH) */
    FORM_PIXELPAK,1,1,1,1,1,1,1,
    0   /* Wird ein Preview unterst�tzt (Edit)? 1=ja, 0=nein */
};



/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
void (*get_PD)(MOD_INFO *infostruct, int mod_id);
int mod_id, t;
int SmurfMessage;
int umgekehrt;
char *picdata, *pdat;
char *destpic, *dest;

int width, height, xmax, ymax;
long x,y, phi, xsrc, ysrc, clpdiv;
long xmitte, ymitte;
long xb, yb;
long offset, srcoffset;
long bytewidth;
long cosval, sinval;
float bog;
/*float abstand, divi;*/
long abstand, divi;
SMURF_PIC *picture;
long sliderval;
char red, green, blue;
long tr, tg, tb;

long yoffset=0;
long ym_qu, xm_qu;
int tunnel;

int Cos[370];
int Sin[370];


SmurfMessage=smurf_struct->module_mode;

/* Hier werden die Funktionen aus der GARGAMEL-Struktur geholt. */
get_PD=smurf_struct->f_module_prefs;    /* PD-Funktion  */



/*--------------------- Wenn das Modul zum ersten Mal gestartet wurde */
if(SmurfMessage==MSTART)
{
    mod_id=smurf_struct->module_number;
    get_PD(&module_info, mod_id);           /* Gib mir 'n PD! */

    smurf_struct->module_mode=M_WAITING;        /* doch? Ich warte... */
    return;
}





/*------------------------------------- Modul soll loslegen */
else if(SmurfMessage==MEXEC)
{

    picture=smurf_struct->smurf_pic;                /* Bild         */
    picdata=picture->pic_data;
    sliderval=smurf_struct->slide1;         /* Sliderwert auslesen */

    umgekehrt=smurf_struct->check1;         /* Checkboxen auslesen */
    tunnel=smurf_struct->check2;

    width=picture->pic_width;       /* Bildabmessungen */
    height=picture->pic_height;
    bytewidth=(long)width*3L;

    xmax=width-1;
    ymax=height-1;


    destpic=Malloc((long)width*(long)height*3L);

    xmitte=width/2;                 /* Mittelpunkt */
    ymitte=height/2;

    xm_qu=xmitte*xmitte;
    ym_qu=ymitte*ymitte;

    if(sliderval < 0) sliderval=360+sliderval;

    
    smurf_struct->reset_busybox(0, "Tabellen anlegen...");


    for(t=0; t<370; t++)
    {
        if(umgekehrt)   bog=((360-t)*M_PI)/180F;
        else    bog=(t*M_PI)/180F;
        Sin[t]=sin(bog)*1024F;  
        Cos[t]=cos(bog)*1024F;  
    }


    smurf_struct->reset_busybox(0, "Verstrudeln...");


    for(y=0; y<height; y++)
    {
        if(!(y&7)) smurf_struct->busybox( ((long)y<<7L) / (long)height );

        
        for(x=0; x<width; x++)
        {
            xb=x-xmitte;
            yb=y-ymitte;

            abstand = yb*yb+xb*xb;                      /* die sqrt wird unten durch das quadrieren von ymitte gespart */

            if(height<width)
                divi = 1024-((abstand<<10)/ym_qu);      /* Divisor f�r skalierung des Winkels nach Abstand */
            else
                divi = 1024-((abstand<<10)/xm_qu);      /* Divisor f�r skalierung des Winkels nach Abstand */

            phi= (sliderval * divi)>>10;

            if(phi<0)
            {
                if(!umgekehrt) phi=0;
                else    phi+=(float)phi/((float)phi/360F);
            }
            else if(phi>360)
            {
                if(!umgekehrt) phi=360;
                else    phi-=(float)phi/((float)phi/360F);
            }

            if(phi !=0 && phi!=360)
            {

                /* Punkt rotieren */
                sinval = Sin[phi];
                cosval = Cos[phi];
                xsrc = ((long)(xb*cosval - yb*sinval)>>10L) + xmitte;
                ysrc = ((long)(xb*sinval + yb*cosval)>>10L) + ymitte;
                
                if(xsrc<0)
                    xsrc=0;
                else
                    if(xsrc>xmax)
                        xsrc-=width;
                
                if(ysrc<0)
                    ysrc=0;
                else
                    if(ysrc>ymax)
                        ysrc-=height;

                offset = (yoffset)+(x+x+x);
                srcoffset = (ysrc*bytewidth)+(xsrc+xsrc+xsrc);
            }   
            else
            {
                offset = (yoffset)+(x+x+x);
                srcoffset = (yoffset)+(x+x+x);
            }               


            dest=destpic+offset;
            pdat=picdata+srcoffset;
            
            red=*(pdat++);
            green=*(pdat++);
            blue=*(pdat++);

            
            if(tunnel && phi && divi<=1024 && divi>=0)
            {
                tr = red * (1024-divi);
                tg = green * (1024-divi);
                tb = blue * (1024-divi);
                red =   tr>>10;
                green = tg>>10;
                blue =  tb>>10;
            }
            
            *(dest++) = red;
            *(dest++) = green;
            *(dest++) = blue;

        }
        
        yoffset+=bytewidth;     /* Wieder ne Multiplikation weniger */
    }


    Mfree(picdata);
    picture->pic_data = destpic;
    
    smurf_struct->module_mode=M_PICDONE;
    return;
}






/* --------------------------------------Mterm empfangen - Speicher freigeben und beenden */
else if(SmurfMessage==MTERM)
{
    smurf_struct->module_mode=M_EXIT;
    return;
}



}







BITBLK *prev(GARGAMEL *prev_struct)
{
    return(0);
}
