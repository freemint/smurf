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

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Neochrome-Importer",
                        0x0090,
                        "Dale Russell",
                        "NEO", "", "", "", "",
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
/*      Neochrome Dekomprimierer (neo)              */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer, *smpal, *paddr, *opaddr;
char mdummy[3], impmessag[21];
int     *ibuf, *palet;
unsigned int    e,f,v1, r,g,b, width, height, BitsPerPixel,x ,y;
unsigned int    mask_bytes[18]={0x8000, 0x4000, 0x2000, 0x1000, 
                        0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 
                        0x0040, 0x0020, 0x0010, 0x0008, 0x0004,
                        0x0002,0x0001};
unsigned int    dummy;
unsigned    int merk_bytes1, merk_bytes2, merk_bytes3, merk_bytes4;
char extend[4];
char *filenam;

filenam=smurf_struct->smurf_pic->filename;
strncpy(extend, filenam+(strlen(filenam)-3), 3);
buffer=smurf_struct->smurf_pic->pic_data;
if ( strncmp(extend, "NEO", 3) )
            return(M_INVALID);

strncpy(smurf_struct->smurf_pic->format_name, "Neochrome .NEO      ", 21);
v1=*(buffer+2)*256+*(buffer+3);             /* Bilddimensionen */
if (v1==0)  { width=320; height=200; BitsPerPixel=4; }
else if (v1==1) { width=640; height=200; BitsPerPixel=2;}
else { width=640; height=400; BitsPerPixel=1;}
palet=(int *)(buffer+4);

strcpy(impmessag, "Neochrome ");
strcat(impmessag, itoa(BitsPerPixel, mdummy, 10));
strcat(impmessag, " Bit");
smurf_struct->services->reset_busybox(128, impmessag);

paddr=Malloc((long)width*(long)height);
if (!paddr) return(M_MEMORY);
smpal=smurf_struct->smurf_pic->palette;

for (e=0; e<16; e++)
{
    v1=*(palet++);

    r=v1 & 0x0f00;
    g=v1 & 0x00f0;
    b=v1 & 0x000f;
 
    *(smpal++)=(r>>4);
    *(smpal++)=g;
    *(smpal++)=(b<<4);
}

opaddr=paddr;
ibuf=(int *)(buffer+128);

for (y=0; y<height; y++) {
    for (x=0; x<20; x++) {
            merk_bytes1=*(ibuf++);
            merk_bytes2=*(ibuf++);
            merk_bytes3=*(ibuf++);
            merk_bytes4=*(ibuf++);

        for (f=0; f<16; f++)
        {
        dummy=0;
        if (merk_bytes1 & mask_bytes[f]) dummy+=1;
        if (merk_bytes2 & mask_bytes[f]) dummy+=2;
        if (merk_bytes3 & mask_bytes[f]) dummy+=4;
        if (merk_bytes4 & mask_bytes[f]) dummy+=8;
        *(paddr++)=dummy;
        }
    }
}   
Mfree(smurf_struct->smurf_pic->pic_data);
smurf_struct->smurf_pic->pic_data=opaddr;
smurf_struct->smurf_pic->col_format=RGB;
smurf_struct->smurf_pic->format_type=0;
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->bp_pal=24;
smurf_struct->smurf_pic->depth=BitsPerPixel;
return(M_PICDONE);
}
