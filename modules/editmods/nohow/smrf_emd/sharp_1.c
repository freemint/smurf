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

/*  ------------- Sharpen-Modul V0.2 -----------------  */
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
    "Sharpen",                      /* Name des Moduls */
    0x0020,
    "Jîrg Dittmer/Dale Russell",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "StÑrke in %",
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
    0,100,
    0,0,
    0,0,
    0,0,
/* min/max fÅr Editobjekte */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaultwerte fÅr Slider, Check und Edit */
    50,0,0,0,
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

    smurf_struct->f_module_prefs(&module_info, module_id);
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
if(SmurfMessage ==MTERM)
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



/*--------------- Sharp(en)ing ----------------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture;
    int width, height;
    char *pic, *offset;
    int strength;
    char *oben,*unten,*links,*rechts,*mitte;
    char *buf_eins, *buf_zwei, *buffer_eins, *buffer_zwei;
    long r,g,b;
    int xmax, ymax,x,y;
    long yoffset,bpl,gesamt;
    long *faktor_1, *faktor_2;
    long *mul5;
    long busycount, busymax, busycall;  
        
    /*--- Slider auslesen ---------------------- */
    
    strength = (int)smurf_struct->slide1;
    faktor_1 = Malloc (256L*4L);
    faktor_2 = Malloc (256L*4L);
    mul5=Malloc(256L*4L);
    
    for (r=0; r<256; r++)
    {
        faktor_1[r] = ((r * (long)strength)<<8L) / 100L;
        faktor_2[r] = ((r * (100L-(long)strength))<<8L) / 100L;
        mul5[r] = r*3L;
    }
    
    /*--- Bilddaten auslesen --------------------*/
 
    picture = smurf_struct->smurf_pic;  
    width   = picture->pic_width;
    height  = picture->pic_height; 
    pic     = picture->pic_data;
    
    bpl = width*3L;
    
    
    /*--- BusyBox Vorberechnungen ----------------------*/
    busycount = 0;
    busycall =  3;
    busymax = height;
    if(busymax >  32) busycall = 15;
    if(busymax > 256) busycall = 63;
    busymax = (busymax<<10) /127;
    
    
    
    
    /*--------- Bild ohne RÑnder --------------------*/
    
    ymax = height-2;
    xmax = width-1;
    yoffset = bpl;
    buf_eins = Malloc(bpl);
    buf_zwei = Malloc(bpl);
    memset(buf_eins, 0, bpl);
    memset(buf_zwei, 0, bpl);
    
    y=0;
    for (;;)
    {       
        busycount++;
        if(!(busycount & busycall)) smurf_struct->busybox((int)((busycount<<10) / busymax));
        
        /* 1. Zeile */
        
        buffer_eins=buf_eins;
        mitte = pic + yoffset + 3;  /* +3 weil erst ab 2.Pixel! */

/*      oben = mitte-bpl;
        unten = mitte+bpl;
        links = mitte-3;
        rechts = mitte+3; */

        rechts = mitte - 3 - bpl;
        links = mitte + 3 + bpl;
        
        for (x=1; x<xmax; x++)
        {
                r = mul5[mitte[0]];
                g = mul5[mitte[1]];
                b = mul5[mitte[2]];
                
                r -= rechts[0];
                g -= rechts[1];
                b -= rechts[2];
                
                r -= links[0];
                g -= links[1];
                b -= links[2];
                
                /* r -= oben[0];
                g -= oben[1];
                b -= oben[2];
                
                r -= unten[0];
                g -= unten[1];
                b -= unten[2]; */
                
                if (r<0) r=0;
                else if (r>255) r=255;
                if (g<0) g=0;
                else if (g>255) g=255;
                if (b<0) b=0;
                else if (b>255) b=255;

            
                *buffer_eins++ = (faktor_1[r] + faktor_2[mitte[0]]) >>8L;               
                *buffer_eins++ = (faktor_1[g] + faktor_2[mitte[1]]) >>8L;               
                *buffer_eins++ = (faktor_1[b] + faktor_2[mitte[2]]) >>8L;               
                
                mitte+=3;
                oben+=3;
                unten+=3;
                links+=3;
                rechts+=3;
        }
        y++; 
        if (y>=ymax) 
        {
            /* Die 1. Zeile zurÅckschreiben */
    
            mitte = pic + yoffset + 3;  
            memcpy(mitte, buf_eins, bpl-6L);
            break;
        }
        yoffset+=bpl;   
        
        /* 2. Zeile */
    
        busycount++;
        if(!(busycount & busycall)) smurf_struct->busybox((int)((busycount<<10) / busymax));

        buffer_zwei=buf_zwei;
        mitte = pic + yoffset + 3;  /* +3 weil erst ab 2.Pixel! */
    
    /*  oben = mitte-bpl;
        unten = mitte+bpl;
        links = mitte-3;
        rechts = mitte+3; */
    
        rechts = mitte - 3 - bpl;
        links = mitte + 3 + bpl;
            
        for (x=1; x<xmax; x++)
        {
                r = mul5[mitte[0]];
                g = mul5[mitte[1]];
                b = mul5[mitte[2]];
                r -= rechts[0];
                g -= rechts[1];
                b -= rechts[2];
                r -= links[0];
                g -= links[1];
                b -= links[2];
                /* r -= oben[0];
                g -= oben[1];
                b -= oben[2];
                r -= unten[0];
                g -= unten[1];
                b -= unten[2];*/
                if (r<0) r=0;
                else if (r>255) r=255;
                if (g<0) g=0;
                else if (g>255) g=255;
                if (b<0) b=0;
                else if (b>255) b=255;
                *buffer_zwei++ = (faktor_1[r] + faktor_2[mitte[0]]) >> 8L;              
                *buffer_zwei++ = (faktor_1[g] + faktor_2[mitte[1]]) >> 8L;              
                *buffer_zwei++ = (faktor_1[b] + faktor_2[mitte[2]]) >> 8L;              
                mitte+=3;
                oben+=3;
                unten+=3;
                links+=3;
                rechts+=3;
        }

        /* Die 2 Zeilen zurÅckschreiben */

        mitte = pic + yoffset + 3 - bpl;    
        memcpy(mitte, buf_eins, bpl-6L);
        mitte = pic + yoffset + 3;  
        memcpy(mitte, buf_zwei, bpl-6L);
        y++; if (y>=ymax) break;
        yoffset+=bpl; 
    }

    Mfree(mul5);
    Mfree(buf_eins); 
    Mfree(buf_zwei); 
    Mfree(faktor_1); 
    Mfree(faktor_2);
    /* Mfree(pic); */
    return(M_PICDONE);
}

