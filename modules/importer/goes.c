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
/*   GOES - Importer                                    */
/*  File Format von der University of Wisconsin u.a     */
/*  Importer von Olaf                                   */
/*                                                      */
/*  So langsam habe ich die Schnauze voll von           */
/*  schwachsinnigen, Speicherverschwendenden            */
/*  Grafikformaten!                                     */
/*  Wenigstens wurde hier auf einen RLE ver-            */
/*  zichzet, der die Bilder wieder doppelt so           */
/*  groû und das Bildformat doppelt so kompliziert      */
/*  gemacht hÑtte.                                      */
/*                                                      */
/*  FÅr SMURF Bildkonverter,                            */
/*                                                      */
/* **************************************************** */


#include <tos.h>
#include <stdio.h>
#include <string.h>
#include <ext.h>

#include "..\import.h"
#include "..\..\src\smurfine.h"

#define GARS    1
#define MCIDAS  2



MOD_INFO module_info=
{
    "GOES-Importer",
    0x0010,
    "Olaf Piesche",
/* Extensionen */
    "GOE","","","","","","","","","",
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
char *cpic;
int *pic_data;
long width, height;
int depth;
int pictype;
long *lpic;
char *smurf_palette;
long dcount=0, PicLen, t;

picture=smurf_struct->smurf_pic;
pic_data=cpic=picture->pic_data;
lpic=cpic=pic_data;

smurf_palette=picture->palette;

/* Dateikennung prÅfen */
if( *(lpic) == 0xc8c4d940L ) pictype=GARS;
else if( *(lpic+0xd0) == 0xf5415720L ) pictype=MCIDAS;
else return(M_INVALID);



if(pictype==GARS) 
    {
    width=*(pic_data+0x1e50);       /* GARS-Image, Motorola-Format.  */
    height=*(pic_data+0x1e54);
    }
else if(pictype==MCIDAS) 
    {
    width=*(pic_data+0x20);     /* MCIDAS-Image, Intel-ASCII-Format.  */
    height=*(pic_data+0x24);
    }
else return(M_UNKNOWN_TYPE);        /* oder unbekanntes Bild. */



depth=8;    /* GrundsÑtzlich 8 Bit, 256 Graustufen. */

PicLen=(long)width*(long)height;


if(pictype==MCIDAS)
{
    memcpy(pic_data, pic_data+0x300, PicLen);
}
else if(pictype==GARS)
{
    cpic=pic_data+0x3c48;

    do
    {
        memcpy(pic_data+dcount, cpic+dcount, 0xe100);   
        dcount+=0xe100;
        cpic+=0x18; 
    } while(dcount<PicLen);


}


/* lineare Palette erzeugen */
for(t=0; t<256; t++)
{
    *(smurf_palette++)=t;
    *(smurf_palette++)=t;
    *(smurf_palette++)=t;
}


Mshrink(0, pic_data, PicLen);

strncpy(picture->format_name, "GOES Satellite Image", 21);
picture->pic_data=pic_data;
picture->depth=depth;
picture->pic_width=width;
picture->pic_height=height;
picture->col_format=RGB;

return(M_PICDONE);
}
