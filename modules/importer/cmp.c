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
#include "..\..\smurf\smurfine.h"

char *change8Bit(char *buf, int xmax, int ymax);
#define NOMEM   ( (void*) 0L )
#define DEBUG           0
#define TRUE            1
#define FALSE           !TRUE

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Atari Public Painter",
						0x0050,
                        "Dale Russell",
                        "CMP", "", "", "", "",
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
/*      CMP Atari Public Painter Monochrom          */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer, *out, *buf, *oout;
int width=0, height=0, SByte;
int xc,yc, x,n,c;
char ext[4]="";
char *extend;

/***************************************************************/
/*      Kopfdaten analysieren                                  */
/***************************************************************/
buffer=smurf_struct->smurf_pic->pic_data;

/*********** ERKENNUNG .CMP *****************/
extend=smurf_struct->smurf_pic->filename;
strncpy(ext, extend+(strlen(extend)-3), 3);
if(strncmp(ext, "CMP", 3)!=0)   return(M_INVALID);

smurf_struct->services->reset_busybox(128, "Public Painter 1 Bit");

strncpy(smurf_struct->smurf_pic->format_name, "Atari Public Painter ", 21);
memset(smurf_struct->smurf_pic->palette+3, 3, 3L);
memset(smurf_struct->smurf_pic->palette, 255, 3L);
width=640;
height=400;
SByte=*(buffer);
buf=buffer+2;
out=Mxalloc(32000L, 2);

if (!out) return(M_MEMORY);

memset(out, 0, 32000L);
oout=out;
/***************************************************************/
/*      Dekodieren                                             */
/***************************************************************/
yc=0;
while (yc++<400)            /* 400 Zeilen */
{
    xc=0;
    while (xc<80)           /* 80 Byte Spalten */
    {
        x=*(buf++);
        if (x==SByte)
        {
            n=*(buf++)+1;
            c=*(buf++);
            xc+=n;
            while (n--)
                *(out++)=c;
        }
        else
        {
            *(out++)=x;
            xc++;
        }   
    
    }
}
Mfree(buffer);
        
/***************************************************************/
/*      Strukturen fllen                                      */
/***************************************************************/
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->depth=1;
smurf_struct->smurf_pic->bp_pal=24;
smurf_struct->smurf_pic->pic_data=oout;
smurf_struct->smurf_pic->format_type=1;

return (M_PICDONE);
}

