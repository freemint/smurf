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

/* ******************************************** */
/*                                              */
/* SPECTRUM 512 - .SPC - Importer               */
/*  Format von: keine Ahnung                    */
/*      Importer von Olaf.                      */
/*                                              */
/*  Spectrum 512-Bilder haben eigentlich 9 Bit  */
/*  mîgliche Farben. Daher werden die Bilder    */
/*  beim Import nach 16Bit gewandelt.           */
/*                                              */
/*  Fertiggeworden am                           */
/*                                              */
/* ******************************************** */


#include <tos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ext.h>

#include "..\import.h"
#include "..\..\smurf\smurfine.h"


MOD_INFO module_info=
{
    "Spectrum 512-Importer",
    0x0010,
    "Olaf Piesche",
/* Extensionen */
    "SPC","SPU","","","","","","","","",

/* Slider */
    "","","","",
/* Editfelder */
    "","","","",
/* Checkboxen */
    "","","","",

/* Minima + Maxima */
/* Slider */
    0,0,
    0,0,
    0,0,
    0,0,
/* Edits */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaults */
    0,0,0,0,
    0,0,0,0,
    0,0,0,0
};



unsigned int *palettes[200];



int FindIndex(int x, int c);
void *decompress_SPC(char *picdata, char *dest_data, long decompressed_bytes);
void get_standard_pix(void *st_pic, void *buf16, int planes, long planelen);
void decode_palettes(unsigned int *picdata);

int imp_module_main(GARGAMEL *smurf_struct)
{
char *picdata, *pdcopy;
char *dest_data, *DecodedPic, *EndPic;
int *dest_zerbastel;
int *pal;
int *CurrentPalette;
char *CompressedPalette, *DecompPalette;
long DatenOffset, PalOffset, PlaneLen, Offset;
int format, t;
int plane;
signed char pix;
unsigned int palpix;
char pixel[17];
long EinePalette=48;
unsigned int C, x,y;
unsigned int Col16, red,green,blue;
long planelen;

void (*reset_busybox)(int lft, char *txt);

    reset_busybox=smurf_struct->services->reset_busybox;

    pdcopy=picdata=smurf_struct->smurf_pic->pic_data;

/* Kennung prÅfen */
    if( strncmp(picdata, "SP\0\0", 4) == 0 )
        format=2;
    else return(M_INVALID);   /* Kenn ich nicht */

    reset_busybox(0, "Spectrum 512 C");




    /****************************************************************************/
    /*                          Bilddaten Dekomprimieren                        */
    /****************************************************************************/
    dest_data=Malloc(32000L*2L);
    DecodedPic=dest_data;
        
    picdata+=12;

    picdata=decompress_SPC(picdata, dest_data, 31840);



    /****************************************************************************/
    /* -------------------  Paletten 'dekomprimieren'   ----------------------- */
    /****************************************************************************/
    CompressedPalette=picdata;

    reset_busybox(0, "Paletten decodieren");
    decode_palettes(CompressedPalette);

    Mfree(pdcopy);          /* Ur-Bilddaten freigeben */
    
        


    picdata=DecodedPic;                             /* fÅrs Zerbasteln auf dekomprimiertes Bild legen */
    EndPic=Malloc(320L*199L*2L);        /* und Speicher fÅrs Zerbasteln anfordern */
    dest_zerbastel=EndPic;
    
    planelen=(320L*199L)/8L;

    reset_busybox(0, "SPC zerbasteln");


        for(y=0; y<199; y++)
        {
            for(x=0; x<320; x+=16)
            {
                memset(pixel, 0, 16);
                
                get_standard_pix(picdata, pixel, 4, planelen);
                
                for(t=0; t<16; t++)
                {
                    /*
                    pal=CurrentPalette+ ( EinePalette * FindIndex(x+t, C) );
                    palpix=*(pal+pixel[t]);
                    */

                    C=(unsigned int)pixel[t];
                    CurrentPalette=palettes[y];
                    palpix = *( CurrentPalette+FindIndex(x+t, C) );

                    /* und nach 16Bit umbauen */
                    blue = (palpix & 0x0003)<<2;
                    green= (palpix & 0x0030)<<3;
                    red= (palpix & 0x0300)<<2;
                    *(dest_zerbastel++)=(unsigned int)(red|green|blue);
                }

                picdata+=2;
            }
        }

    Mfree(DecodedPic);
    for(t=0; t<199; t++) free(palettes[t]);


    smurf_struct->smurf_pic->pic_data=EndPic;
    smurf_struct->smurf_pic->depth=16;  /*16*/

    strncpy(smurf_struct->smurf_pic->format_name, "Spectrum 512 SPC     ", 21);
    smurf_struct->smurf_pic->pic_width=320;
    smurf_struct->smurf_pic->pic_height=199;        
    smurf_struct->smurf_pic->format_type=FORM_PIXELPAK;     /* PP */

    return(M_PICDONE);  /* das wars. */
}




/*--------------------- Find Color Palette Index--------------*/
int FindIndex(int x, int c)
{
        int x1;

        x1 = 10 * c;

        if (1 & c)              /* If c is odd */
                x1 = x1 - 5;
        else                    /* If c is even */
                x1 = x1 + 1;

        if (x >= x1 && x < x1 + 160) 
                c = c + 16;
        else if (x >= x1 + 160) 
                c = c + 32;

        return c;
}





void *decompress_SPC(char *picdata, char *dest_data, long decompressed_bytes)
{
long count;
signed char pix;
int C, t;


        count=0;

        do
        {
            pix= *(picdata++);
            count++;
            /*------------------------ Compressed Run */
            if((int)pix>=-128 && (int)pix<=-1)
            {
                C = (-pix)+2;
                pix=*(picdata++);
                for(t=0; t<C; t++)
                {
                    *(dest_data++)=pix;
                    count++;
                }

            }
            /*------------------------ Literal Run */
            else /*if(pix>=0 && x<=127)*/
            {
                C = pix+1;
                for(t=0; t<C; t++)
                {
                    count++;
                    *(dest_data++)=*(picdata++);
                }
            }
        } while(count<decompressed_bytes);

    return(picdata);
}




void decode_palettes(unsigned int *picdata)
{
int t, bit, palette_number;
unsigned int header_word, counter;
unsigned int *Cpal;

    for(t=0; t<199; t++)
    {
        palettes[t]=malloc(97);
        counter=0;
        Cpal=palettes[t];

        for(palette_number=0; palette_number<3; palette_number++)
        {
            header_word=*(picdata++);

            bit=0;
            
            do
            {
                /* Paletteneintrag vorhanden */
                if( ( (header_word&(1<<bit))==0) || bit==15 /*|| bit==0*/)
                    *( Cpal ++ ) = 0;
                else
                    *( Cpal ++ ) = *(picdata++);

                counter++;
            } while(++bit<16);

        }


    }
    

}
