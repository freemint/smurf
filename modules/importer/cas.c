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
#include <stdlib.h>
#include <time.h>
#include "..\import.h"
#include "..\..\smurf\smurfine.h"

char *decodePCX(char *buffer, int BytePerLine, int height);
#define NOMEM   ( (void*) 0L )
#define DEBUG           0
#define TRUE            1
#define FALSE           !TRUE

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"DCX-CAS-Fax Importer",
						0x0050,
                        "Dale Russell",
                        "DCX", "", "", "", "",
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
/*      CAS FAX Standard                            */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer, *GetBuf, *out2;
int width=0, height=0, PicCnt,x;
long PicOffset[1024];
int lBorder,rBorder,uBorder,dBorder;
int BytePerLine, KompFlag;
/***************************************************************/
/*      Kopfdaten analysieren                                  */
/***************************************************************/
buffer=smurf_struct->smurf_pic->pic_data;

smurf_struct->services->reset_busybox(128, "CAS Fax 1 Bit");

if ( (*(buffer)!=0xb1) || (*(buffer+1)!=0x68) ||
        (*(buffer+2)!=0xde) || (*(buffer+3)!=0x3a) ) return(M_INVALID);

PicCnt=0;                       /* Bildz„hler */            
for (x=0; x<1024; x++)          /* Eintragen der Offsets */
{
    GetBuf=buffer+0x04+(x<<2);
    PicOffset[x]=(long)*(GetBuf)+(long)(*(GetBuf+1)<<8)+(long)
                (*(GetBuf+2)<<16L)+(long)(*(GetBuf+3)<<24L);
    if (PicOffset[x]!=0L) PicCnt++;     
}
if (!PicCnt)
{
        form_alert(1,"[1][Kein PCX Bild innerhalb|der CAS FAX Struktur]\[ Pech ]");
        return(M_PICERR);
}               
if (PicCnt>1)
        form_alert(1,"[1][Es kann nur das erste Bild|angezeigt werden !]\[ Leider ]");

strncpy(smurf_struct->smurf_pic->format_name, "CAS Fax Standard     ", 21);

/**********************************************************/
/*  Auslesen der PCX Struktur                             */
/**********************************************************/
GetBuf=buffer+PicOffset[0];     /* 1. Bild */
if (*(GetBuf)!=0x0a)
{
    form_alert(1,"[1][Eingebautes Bild hat|nicht das PCX Format!]\[ Pech ]");
    return(M_PICERR);
}
KompFlag=*(GetBuf+2);
lBorder=*(GetBuf+4)+(*(GetBuf+5)<<8);
uBorder=*(GetBuf+6)+(*(GetBuf+7)<<8);
rBorder=*(GetBuf+8)+(*(GetBuf+9)<<8);
dBorder=*(GetBuf+10)+(*(GetBuf+11)<<8);
BytePerLine=*(GetBuf+0x42)+(*(GetBuf+0x43)<<8);
width=(rBorder-lBorder)+1;
height=(dBorder-uBorder)+1;

if (KompFlag)
{
    out2=decodePCX(GetBuf+128, BytePerLine, height);
    if (out2==NOMEM)
    {
        return(M_MEMORY);
    }   
    else Mfree(buffer);
    buffer=out2;
}

else if (!KompFlag)     
    buffer=GetBuf+128;  /* Rohdaten */
        
/***************************************************************/
/*      Strukturen fllen                                      */
/***************************************************************/
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->depth=1;
smurf_struct->smurf_pic->bp_pal=0;
smurf_struct->smurf_pic->pic_data=buffer;
smurf_struct->smurf_pic->format_type=1;

return (M_PICDONE);
}

/*************************************************************/
/*  PCX ggf. dekodieren                                      */
/*************************************************************/
char *decodePCX(char *buffer, int BytePerLine, int height)
{
register int x,c,xc,yc;
register char *output, *RetBuf;
output=Mxalloc( (long)BytePerLine*(long)height, 2);
if (!output)
    return(NOMEM);
RetBuf=output;
yc=0;
 while(yc++<height)
 {
    xc=0;
    while(xc<BytePerLine)
    {
        x=*(buffer++);
        if (x >= 192)
        {   
            x -= 192;
            xc+=x;
            c=*(buffer++);
            while (x--)
                *(output++)=c;
        }
        else  {
            *(output++)=x; xc++; }
    } /* while xc */
 } /* while yc */   
return(RetBuf);
}
