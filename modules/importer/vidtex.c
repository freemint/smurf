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
#include "..\..\smurf\smurfine.h"

#define TRUE        1
#define FALSE       0
#define DEBUG       0
#define W_PIX       255
#define B_PIX       0

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Compuserve Vidtex RLE",
                        0x0050,
                        "Dale Russell",
                        "RLE", "", "", "", "",
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
/*  COMPUSERVE VIDTEX RLE Bitmap                    */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer=smurf_struct->smurf_pic->pic_data;
int     mag1, mag2,mag3;
int     width, height, xc, yc, c;
char    WhitePair, BlackPair, WhiteRun, BlackRun;
char    *get=buffer+3;
char    *retbuf, *output;
long    len;
int     Mask[]={0,0x0080,0x0040, 0x0020, 0x0010, 0x0008,0x0004,0x0002,0x0001};

/*****************************************************/
/*          MAGIC Code berprfen                    */
/*****************************************************/
mag1=*(buffer);
mag2=*(buffer+1);
mag3=*(buffer+2);
if ( !((mag1==0x1b && mag2==0x47 && (mag3==0x48 || mag3==0x4d)) ))  
            return(M_INVALID);

smurf_struct->services->reset_busybox(128, "CI$ Vidtex 1 Bit");

switch(mag3)
{
    case 0x4d:  width=128;
                height=96; break;
    case 0x48:  width=256;
                height=192; break;
}

len=(long)((width+7)/8) * (long)height;
output=Malloc( len );
if (!output) return(M_MEMORY);
memset(output, 0, len);
retbuf=output;
yc=xc=0;
c=0;                
while (yc<height)
{                           
WhitePair=*(get++);
BlackPair=*(get++);
WhiteRun=WhitePair-0x20;
BlackRun=BlackPair-0x20;
xc+=(WhiteRun+BlackRun);
if (xc>width) { xc-=width; yc++; }

    while(WhiteRun--)   
    {
        c++;
        if (c > 8) { output++; c-=8; }
        *(output)+=Mask[c];
    }
    while(BlackRun--)   
    {
        c++;
        if ( c > 8) { output++; c-=8; }
    }
}
Mfree(buffer);
strncpy(smurf_struct->smurf_pic->format_name, "Compuserve VIDTEX  ", 21);
smurf_struct->smurf_pic->format_type=FORM_STANDARD;
smurf_struct->smurf_pic->pic_data=retbuf;
smurf_struct->smurf_pic->depth=1;
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
return(M_PICDONE);
}
