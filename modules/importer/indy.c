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


/* Infostruktur fr Hauptmodul */
MOD_INFO    module_info={"Indypaint-Importer",
                        0x0050,
                        "Dale Russell",
                        "TRU","","","","",
                        "","","","","",
                    /* Objekttitel */
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
                    /* Objektgrenzwerte */
                        0,128,
                        0,128,
                        0,128,
                        0,128,
                        0,10,
                        0,10,
                        0,10,
                        0,10,
                    /* Slider-Defaultwerte */
                        0,0,0,0,
                        0,0,0,0,
                        0,0,0,0,
                        };

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*      IndyPaint (by Lazer)            .tru        */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
int *buffer;
char *ziel;
int     *obuf;
unsigned int width, height;
long    len;
unsigned int x,y, r,g,b, h1,h2;

buffer=smurf_struct->smurf_pic->pic_data;
len=smurf_struct->smurf_pic->file_len;

if(strncmp(buffer, "Indy", 4)!=0) return(M_INVALID);

strncpy(smurf_struct->smurf_pic->format_name, "Indypaint I - .TRU   ", 21);
if (len<130000L) { width=320; height=200; }
else if (len<160000L) { width=320; height=240; }
else if (len<190000L) { width=384; height=240; }
else if (len<260000L) { width=640; height=200; }
else { width=640; height=400; }

obuf=buffer;
buffer+=128;
ziel=Mxalloc((long)width*(long)height*3L,3);
if (!ziel) return(M_MEMORY);
else {
smurf_struct->smurf_pic->pic_data=ziel;
for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
            h1=*(buffer++);
            r=(h1 & 0xf800)>>8;
            g=(h1 & 0x07e0)>>3;
            b=(h1 & 0x001f)<<3;
            *(ziel++)=b;
            *(ziel++)=g;
            *(ziel++)=r;
        }
}
}
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->bp_pal=0;
smurf_struct->smurf_pic->depth=24;
Mfree(obuf);
return(M_PICDONE);
}
