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
#include "..\import.h"
#include "..\..\smurf\smurfine.h"

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"SEF-Importer",
                        0x0050,
                        "Dale Russell",
                        "SEF", "", "", "", "",
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
/*      SEF-Importer                                */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer, *Out, *Merk, *buf, *m1, *m2;
int     x,y, width, height;
long    off;
char *filenam;
char extend[4];

buffer=smurf_struct->smurf_pic->pic_data;

/*****************************************************/
/*          MAGIC Code berprfen                    */
/*****************************************************/
filenam=smurf_struct->smurf_pic->filename;
strncpy(extend, filenam+(strlen(filenam)-3), 3);
if(strncmp(extend, "SEF", 3)!=0) 
    return M_INVALID;

smurf_struct->services->reset_busybox(128, "SEF 24 Bit");

width=(*(buffer+286)<<8)+(*(buffer+287));
height=(*(buffer+290)<<8)+(*(buffer+291));

Out=Mxalloc((long)width*(long)height*3L,2);
if(!Out) return(M_MEMORY);
Merk=Out;
buf=buffer+2258;
off=(long)width*(long)height;
m1=buf+off;
m2=m1+off;

/********* Make Interleaved Bitplanes for 24Bit mode ********/
for (y=0; y<height; y++)
{
    for (x=0; x<width; x++)
    {
        *(Out++)=*(m2++);
        *(Out++)=*(m1++);       
        *(Out++)=*(buf++);
    }   
}
Mfree(buffer);
strncpy(smurf_struct->smurf_pic->format_name, ".SEF (?)              ", 21);
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->depth=24;
smurf_struct->smurf_pic->bp_pal=0;
smurf_struct->smurf_pic->pic_data=Merk;
smurf_struct->smurf_pic->format_type=0;
smurf_struct->smurf_pic->col_format=BGR;
return(M_PICDONE);
}
