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
 * Olaf Piesche, Christian Eyrich, Dale Russell and J”rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define TRUE    1
#define FALSE   0
#define DEBUGMODE   0

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"ESM Bitmap Modul",
						0x0050,
                        "Dale Russell",
                        "ESM", "", "", "", "",
                        "", "", "", "", "",
                        "Slider 1",
                        "Slider 2",
                        "Slider 3",
                        "Slider 4",
                        "Checkbox 1",
                        "Checkbox 2",
                        "Checkbox 3",
                        "Checkbox 4",
                        "Edit 1",
                        "Edit 2",
                        "Edit 3",
                        "Edit 4",
                        0,128,
                        0,128,
                        0,128,
                        0,128,
                        0,10,
                        0,10,
                        0,10,
                        0,10,
                        0,0,0,0,
                        0,0,0,0,
                        0,0,0,0,
                        0
                        };

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*      ESM                                         */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char *buffer, *retbuf;
unsigned int width, height;
unsigned int BitsPerPixel;
unsigned int colors;
long len,Offset;
char *map, *get;
char dummy[3], impmessag[21];

buffer=smurf_struct->smurf_pic->pic_data;
if (strncmp(buffer,"TMS",3)!=0)
        return(M_INVALID);
        
Offset=(*(buffer+4)<<8)+*(buffer+5);
width=(*(buffer+6)<<8)+*(buffer+7);
height=(*(buffer+8)<<8)+*(buffer+9);
BitsPerPixel=(*(buffer+10)<<8)+*(buffer+11);

strcpy(impmessag, "ESM ");
strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
strcat(impmessag, " Bit");
smurf_struct->services->reset_busybox(128, impmessag);

len=(long)width* (long)height* (long)(BitsPerPixel>>3);
if (BitsPerPixel==8)
{
    map=smurf_struct->smurf_pic->palette;
    get=buffer+12l;
    for (colors=0; colors<256; colors++)
    {
        *(map++)=colors;        /* R */
        *(map++)=colors;        /* G */
        *(map++)=colors;        /* B */
    }       
    smurf_struct->smurf_pic->bp_pal=24;
}
else    smurf_struct->smurf_pic->bp_pal=0;

retbuf=Malloc(len);
memcpy(retbuf, buffer+Offset, len);
Mfree(smurf_struct->smurf_pic->pic_data);
smurf_struct->smurf_pic->depth=BitsPerPixel;
smurf_struct->smurf_pic->pic_data=retbuf;
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->col_format=RGB;
smurf_struct->smurf_pic->format_type=0;
strncpy(smurf_struct->smurf_pic->format_name, "Enhanced Simplex    ",21);
return(M_PICDONE);
}
