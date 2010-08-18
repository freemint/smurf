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

/*  -------------   Speed-O-Move V0.5   --------------- */
/*          FÅr SMURF Bildkonverter, 19.10.95           */

#ifdef GERMAN
#define TEXT1 "Speed-O-Move"
#define TEXT2 "StÑrke"
#define TEXT3 "Winkel"
#define TEXT4 "Motor anlassen..."
#else
#ifdef ENGLISH
#define TEXT1 "Speed-O-Move"
#define TEXT2 "Strength"
#define TEXT3 "Angle"
#define TEXT4 "Motor anlassen..."
#else
#ifdef FRENCH
#define TEXT1 "Speed-O-Move"
#define TEXT2 "Strength"
#define TEXT3 "Angle"
#define TEXT4 "Motor anlassen..."
#else
#error "Keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdi.h>
#include <aes.h>
#include <math.h>
#include <screen.h>
#include "..\..\src\lib\sym_gem.h"
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define betrag(a)   (a<0 ? -a : a)

/* Infostruktur fÅr Hauptmodul */
MOD_INFO    module_info={
                        TEXT1,
                        0x0050,
                        "Olaf Piesche",
                        "", "", "", "", "",
                        "", "", "", "", "",

                        TEXT2, TEXT3,"","",
                        "", "", "", "",
                        "", "", "", "",
                        1,100,
                        0,360,
                        0,128,
                        0,128,
                        0,10,
                        0,10,
                        0,10,
                        0,10,
                        20,0,0,0,
                        0,0,0,0,
                        0,0,0,0,
                        1
                        };


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
                        0 /* Extra Flag */ 
                        };


/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	void (*get_PD)(MOD_INFO *infostruct, int mod_id);
	int mod_id;
	int SmurfMessage;
	char *picdata, *pd;
	char *dest, *dest_data;
	char *read/*, *write*/;
	
	int width, height;
	long x, y;
	SMURF_PIC *picture;
	long sliderval/*, str*/;
	int red, green, blue;
	int strcounter, divide, divide2;
	int count_orig;
	long yoffset=0, xoffset=0, bytewidth, offset;
	long maxoffset;
	int angle;
	int ox1, ox2;
	int x1, y1, x2, y2, dx, dy, xcalc, dxabs, dyabs;
	long incx, incy, xpos, ypos, yinc10;
	long maxx, maxy;
	float dist;
	float bog;
	long *offset_buffer, *current_offset;
	
	SmurfMessage=smurf_struct->module_mode;
	
	/* Hier werden die Funktionen aus der GARGAMEL-Struktur geholt. */
	get_PD=smurf_struct->services->f_module_prefs;    /* PD-Funktion  */
	
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
	    pd=picdata=picture->pic_data;
	    sliderval=smurf_struct->slide1;         /* Sliderwert auslesen */
	    angle=(int)smurf_struct->slide2;         /* Sliderwert auslesen */
	
	    width=picture->pic_width;       /* Bildabmessungen */
	    height=picture->pic_height;
	    bytewidth=(long)width*3L;
	
	
	    /*--------------- Vorberechnungen -------------*/
	
	    smurf_struct->services->reset_busybox(0, TEXT4);
	
	    /* Start- und Anfangspunkt ermitteln */
	    bog=(angle*M_PI)/180F;
	    x1=cos(bog)*sliderval;
	    y1=sin(bog)*sliderval;
	
	    x2=0;
	    y2=0;
	
	    dx=x1-x2;
	    dy=y1-y2;
	
	    dist = sqrt( (dx*dx)+(dy*dy) );
	
	    incx = (dx*1024L)/dist;
	    incy = (dy*1024L)/dist;
	
	    ox1=x1;
	    ox2=x2;
	
	    maxoffset = (long)(height-1)*bytewidth;
	
	    dest_data=dest=Malloc((long)width*(long)height*3L);
	
	    if(sliderval < 0) sliderval=360+sliderval;
	
	    dxabs=abs(dx);
	    dyabs=abs(dy);
	
	    maxx=(width-1)*1024L;
	    maxy=(height-1)*1024L;
	
	
	    /*--------------------- Counter vorbestimmen -----------*/
	
	    count_orig=dyabs;
	    if(dxabs>dyabs)
	        divide=count_orig=dxabs;
	
	    /*--------------------- Offsettabelle anlegen -----------*/
	
	    strcounter=count_orig;
	    
	    offset_buffer=Malloc(strcounter*4L);
	
	    xpos=(long)x1;
	    ypos=(long)y1;
	
	    for(strcounter=0; strcounter<count_orig; strcounter++)          
	    {
	        yoffset=(ypos>>10)*bytewidth;
	        xcalc=xpos>>10L;
	
	        offset=yoffset+xcalc+xcalc+xcalc;
	        
	        offset_buffer[strcounter]=offset;
	
	        xpos+=incx;
	        ypos+=incy;
	
	    }
	
	    /*--------------------- Hauptschleife --------------------*/
	
	    smurf_struct->services->reset_busybox(0, "Gas geben...");
	
	    for(y=0; y<height; y++)
	    {
	        if(!(y&7)) smurf_struct->services->busybox( ((long)y<<7L) / (long)height );
	
	        for(x=0; x<width; x++)
	        {
	            red=green=blue=0;
	            
	            divide=strcounter=count_orig;
	
	            current_offset=offset_buffer/*+strcounter*/;
	            
	            while(strcounter--)
	            {
	                offset=*(current_offset++);
	
	                read=picdata+offset;
	                red += *(read++);
	                green += *(read++);
	                blue += *(read);
	            }
	
	            if(divide==0)  divide=1;
	            picdata+=3;
	
	            red/=divide;
	            green/=divide;
	            blue/=divide;
	
	            *(dest++)=(char)red;            
	            *(dest++)=(char)green;          
	            *(dest++)=(char)blue;           
	        }
	    }
	
	    Mfree(pd);
	    Mfree(offset_buffer);
	    picture->pic_data = dest_data;
	    
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
