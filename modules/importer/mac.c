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
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define DEBUG           0

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"MacPaint-Importer",
                        0x0050,
                        "Dale Russell",
                        "MAC", "PNT", "", "", "",
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
/*      Mac Mac-Paint Format                        */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char *extend, ext[4];
char    *buffer;
int width=0, height=0;
int xc,yc, x,c,DatenOffset=0;
char *buf, *out, *oout;
buffer=smurf_struct->smurf_pic->pic_data;

extend=smurf_struct->smurf_pic->filename;
strncpy(ext, extend+(strlen(extend)-3), 3);
if(strncmp(ext, "MAC", 3)!=0 && strncmp(ext, "PNT", 3)!=0)
    return(M_INVALID);
strncpy(smurf_struct->smurf_pic->format_name, "Mac-Paint Bitmap   ", 21);
width=576;
height=720;
for (x=0x80; x<0x86; x++)
    if ( *(buffer+x)!=0 ) DatenOffset=0x280;
else DatenOffset=0x200;

smurf_struct->services->reset_busybox(128, "Mac Paint 1 Bit");

/* RLE dekodieren */
buf=buffer+DatenOffset;
out=Mxalloc( 51840L, 3);        /* Maximal 576*720 Pixel (8 Pixel pro Byte) */
if(!out) return(M_MEMORY);
oout=out;
yc=0;
while(yc++<height)
{
xc=0;
    while (xc++<72)             /* Maximal 576/8 Spalten */
    {
        x=*(buf++);
        if (x & 0x80)
        {
            x=0x101-x;
            c=*(buf++);
            while(x--)
                *(out++)=c;
        }
        else 
        {
            x++; 
            while (x--)
                *(out++)=*(buf++);
        }
    }   /* x-loop */
}   /* y- loop */
Mfree(smurf_struct->smurf_pic->pic_data);
smurf_struct->smurf_pic->pic_data=oout;
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->depth=1;
smurf_struct->smurf_pic->bp_pal=0;
smurf_struct->smurf_pic->format_type=1;
smurf_struct->smurf_pic->col_format=WURSCHT;

return(M_PICDONE);
}

