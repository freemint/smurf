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
MOD_INFO module_info = {"Adex ChromaGraph Bitmap",
						0x0010,
                        "Dale Russell",
                        "IMG", "RLE", "", "", "",
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
/*  ADEX ChromaGraph Bitmap                         */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char *buffer=smurf_struct->smurf_pic->pic_data;
int BitsPerPixel=0, Planes=0, Colors=0, width=0, height=0, CodeFlag=0;
char *retbuf, *output, *buf2, *paldat;
int bperz,n,c,xc,yc, ColCount;
char dummy[3], impmessag[17];

/*****************************************************/
/*          MAGIC Code ÅberprÅfen                    */
/*****************************************************/
if ( strncmp(buffer, "PICT", 4)!=0 )
            return(M_INVALID);
/*****************************************************/         
/*      Kopfdaten auslesen                           */
/*****************************************************/         
CodeFlag=(int)*(buffer+4);
BitsPerPixel=(int)*(buffer+5);
width=*(buffer+6)+(*(buffer+7)<<8);
height=*(buffer+8)+(*(buffer+9)<<8);
Colors=*(buffer+10)+(*(buffer+11)<<8);

strcpy(impmessag, "ChromaGraph ");
strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
strcat(impmessag, " Bit");
smurf_struct->services->reset_busybox(128, impmessag);

#if DEBUG>0
    printf("\n  Width: %i",width);          
    printf("\n  Height: %i",height);            
    printf("\n  Depth: %i",BitsPerPixel);           
    printf("\n  Planes: %i",Planes);            
    printf("\n  Colors: %i",Colors);            
    getch();
#endif
/*****************************************************/                     
/*      Palette auslesen                             */
/*****************************************************/                     
if (BitsPerPixel==8) ColCount=256; 
    else ColCount=16; 
    
paldat=buffer+0x0e;
output=smurf_struct->smurf_pic->palette;    
for (xc=0; xc<ColCount; xc++)
{
    *(output++)=(*(paldat++)<<2);       /* r */
    *(output++)=(*(paldat++)<<2);       /* g */
    *(output++)=(*(paldat++)<<2);       /* b */
    
}           
bperz= ( width+( (8/BitsPerPixel)-1 ) ) / (8/BitsPerPixel);

/*****************************************************/                     
/*      Bilddaten ggf. dekodieren                    */
/*****************************************************/                     
if (CodeFlag==TRUE)
{
output=Malloc( (long)bperz*(long)height);
retbuf=output;
yc=xc=0;
while (yc++<height)
{
while (xc<bperz)
    {
        n=*(buffer++);
        c=*(buffer++);
        xc+=n;
            while(n--)
                *(output++)=c;
    }
xc-=bperz;
}
Mfree(smurf_struct->smurf_pic->pic_data);
}

else retbuf=buffer;

if (BitsPerPixel==4)
{
    buf2=Malloc( (long)bperz*(long)height*2L);
    buffer=buf2;
    output=retbuf;
    
    for (yc=0; yc<height; yc++)
    {
        for (xc=0; xc<bperz; xc++)
        {
        
            *(buf2++)=(*(retbuf) & 0xf0)>>4;
            *(buf2++)=(*(retbuf++) & 0x0f);
        }
    }
    Mfree(output);
}

else buffer=retbuf; 
strncpy(smurf_struct->smurf_pic->format_name, "Adex ChromaGraph    ", 21);
smurf_struct->smurf_pic->format_type=0;
smurf_struct->smurf_pic->pic_data=buffer;
smurf_struct->smurf_pic->depth=(int)BitsPerPixel;
smurf_struct->smurf_pic->pic_width=(int)width;
smurf_struct->smurf_pic->pic_height=(int)height;
return(M_PICDONE);
}
