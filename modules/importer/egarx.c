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

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define TRUE        1
#define FALSE       0
#define DEBUG       0
#define W_PIX       255
#define B_PIX       0

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"EGA Paint/ColorRIX Bitmap",
						0x0050,
                        "Dale Russell",
                        "RIX", "SCI", "SCR", "SCP", "SCG",
                        "SCU", "", "", "", "",
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
/*      EGA Paint COLORRIX Bitmap                   */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char dummy[3], impmessag[17];
char *buffer=smurf_struct->smurf_pic->pic_data;
char *retbuf, *getbuf;
long len;
int BitsPerPixel=0, width=0, height=0, Mode=0;
int bperz,xc,ColCount=0;
char *output, *palbuf;
/*****************************************************/
/*          MAGIC Code ÅberprÅfen                    */
/*****************************************************/
if ( strncmp(buffer, "RIX3", 4)!=0 )
            return(M_INVALID);
/*****************************************************/         
/*      Kopfdaten auslesen                           */
/*****************************************************/         
width=*(buffer+4)+(*(buffer+5)<<8);
height=*(buffer+6)+(*(buffer+7)<<8);
Mode=*(buffer+8);
BitsPerPixel=*(buffer+9);

strcpy(impmessag, "EGA Paint ");
strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
strcat(impmessag, " Bit");
smurf_struct->services->reset_busybox(128, impmessag);

if (Mode==0xaf) { BitsPerPixel=8; Mode=0; }
else Mode=1;

#if DEBUG>0
    printf("\n  Width: %i",width);          
    printf("\n  Height: %i",height);            
    printf("\n  Depth: %i",BitsPerPixel);           
    getch();
#endif
/*****************************************************/                     
/*      Palette auslesen                             */
/*****************************************************/                     
output=smurf_struct->smurf_pic->palette;    
palbuf=buffer+10;
if (BitsPerPixel==8) ColCount=256;
else if (BitsPerPixel==4) ColCount=16;
else if (BitsPerPixel==1) ColCount=2;

for (xc=0; xc<ColCount; xc++)
{
    *(output++)=(*(palbuf++)<<2);       /* r */
    *(output++)=(*(palbuf++)<<2);       /* g */
    *(output++)=(*(palbuf++)<<2);       /* b */
}           
if (BitsPerPixel==8)
    len=((long)width)*((long)height);
else if (BitsPerPixel==4)
    len=(long)((width+3)/4)*((long)height);
else if (BitsPerPixel==1)
    len=(long)((width+7)/8)*((long)height);
    
strncpy(smurf_struct->smurf_pic->format_name, "EGA Paint / ColorRIX", 21);
smurf_struct->smurf_pic->format_type=Mode;
smurf_struct->smurf_pic->col_format=RGB;
retbuf=Malloc(len);
getbuf=(smurf_struct->smurf_pic->pic_data);
getbuf+=10+ColCount*3;
memcpy(retbuf, getbuf, len);
Mfree(smurf_struct->smurf_pic->pic_data);
smurf_struct->smurf_pic->pic_data=retbuf;
smurf_struct->smurf_pic->depth=(int)BitsPerPixel;
smurf_struct->smurf_pic->pic_width=(int)width;
smurf_struct->smurf_pic->pic_height=(int)height;
return(M_PICDONE);
}
