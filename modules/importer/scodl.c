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

/* **************************************************** */
/*   SCODL - Importer                                   */
/*  File Format by Agfa Corp.,                          */
/*  Importer by Olaf                                    */
/*                                                      */
/*  Wieder ein Format mit einer Expansions-             */
/*  komprimierung par excellence, bei der die           */
/*  Bilder nach der Komprimierung dreimal so            */
/*  groû sind, wie vorher...                            */
/*                                                      */
/*  FÅr SMURF Bildkonverter,                            */
/*                                                      */
/*  Im Unterricht geschrieben und dabei stÑndig         */
/*  vom Dozenten unterbrochen worden...                 */
/* **************************************************** */


#include <tos.h>
#include <stdio.h>
#include <string.h>
#include <ext.h>

#include "..\import.h"
#include "..\..\smurf\smurfine.h"


MOD_INFO module_info=
{
    "SCODL-Importer",
    0x0010,
    "Olaf Piesche",
/* Extensionen */
    "SCD","","","","","","","","","",
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






int imp_module_main(GARGAMEL *smurf_struct)
{
SMURF_PIC *picture;
char *cpic, *bitmap_data, *decoded_pic, *decpic_copy, PicByte;
int *pic_data;
char *redpal, *greenpal, *bluepal;
int width, height, depth;
int pictype, pal_offs, memtype;
long decode, PicLen, DecodeLen=0, t, dect, PlaneLen;
char red, green, blue;
char *smurf_palette, *smurf_palcopy;


picture=smurf_struct->smurf_pic;
pic_data=cpic=picture->pic_data;
smurf_palette=picture->palette;
smurf_palcopy=smurf_palette;


/* Dateikennung prÅfen */
if( *(pic_data) != 0xe001L ) return(M_INVALID);



pictype= *(pic_data+1);     /* Bildtype */

if(pictype==0xf800L) 
    {
    depth=8;        /* 8Bit Palettenbild,     */
    pal_offs=3*256;
    }
else if(pictype==0xdc00L) 
    {
    depth=24;   /* 24Bit TC-Bild,         */
    pal_offs=0;
    }
else return(M_UNKNOWN_TYPE);        /* oder unbekanntes Bild. */




redpal=cpic+0x04;       /* Palettenwerte ROT ab Byte 4 */
greenpal=cpic+0x104;        /* Palettenwerte GRöN */
bluepal=cpic+0x204;     /* Palettenwerte BLAU */

memtype=*(cpic+pal_offs);   /* Scaleable - Non Scaleable (???) */

width= (int) *(cpic+pal_offs+1);
height= (int) *(cpic+pal_offs+3);





PicLen=(long)width*(long)height*3L;
decoded_pic=Malloc(PicLen);
decpic_copy=decoded_pic;

bitmap_data=*(cpic+0x0d+pal_offs);




/* Expansions-Decode (RLE=RunLengthEncoding oder: Reine LÑngen-Expansion...) */
    do{
        decode=*(bitmap_data++);        /* RLE-Length */    
        PicByte=*(bitmap_data++);   /* RLE-Value  */

        for(dect=0; dect<decode; dect++)
        {
            *(decpic_copy++)=PicByte;
            DecodeLen++;
        }

    } while(DecodeLen<PicLen);





/******* 24BIT-IMAGE *********/
if(depth==24)
{
    bitmap_data=decoded_pic;
    PlaneLen=width*height;

    Mfree(pic_data);
    pic_data=Malloc(PicLen);
    cpic=pic_data;

    /* Format-Decode */
    for(t=0; t<PlaneLen; t++)
    {
        red=*(bitmap_data+t);
        green=*(bitmap_data+t+PlaneLen);
        blue=*(bitmap_data+t+PlaneLen+PlaneLen);
        *(cpic++)=red;
        *(cpic++)=green;
        *(cpic++)=blue;
    }

    Mfree(decoded_pic);
}


/******* 8BIT-IMAGE *********/
else if(depth==8)
{
    /* den Speicherblock verkÅrzen... */
    PlaneLen=(long)width*(long)height;
    Mshrink(0, decoded_pic, PlaneLen);

    Mfree(pic_data);
    pic_data=decoded_pic;


    /* Palette eintragen */
    for(t=0; t<256; t++)
    {
        *(smurf_palcopy++)=*(redpal)++;
        *(smurf_palcopy++)=*(greenpal)++;
        *(smurf_palcopy++)=*(bluepal)++;
    }

}


strncpy(picture->format_name, "Agfa SCODL-File (.SCD)", 21);
picture->pic_data=pic_data;
picture->depth=depth;
picture->pic_width=width;
picture->pic_height=height;
picture->col_format=RGB;

return(M_PICDONE);
}
