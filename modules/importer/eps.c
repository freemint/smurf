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

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"EPS-Importer",
						0x0050,
                        "Dale Russell",
                        "EPS", "", "", "", "",
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
/*      Encapsulated Post Script .EPS               */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer, *obuf, *ziel, *oziel, *pal, *opal;
int     width, height, test;
char    *pos[5];
size_t  l=3;
unsigned long magic, c, filelen, offset;
unsigned int x,y, m, wert, h1,h2;
char    *bf[5];


buffer=smurf_struct->smurf_pic->pic_data;
filelen=smurf_struct->smurf_pic->file_len;
if (*(buffer)!=0xc5 || *(buffer+1)!=0xd0 || *(buffer+2)!=0xd3 || 
        *(buffer+3)!=0xc6) 
        { 
            for (x=0; x<5; x++)
            {   Mfree(bf[x]); Mfree(pos[x]); }
        return(M_INVALID); 
        }

test=FALSE;
c=0L;

while(c<filelen && test!=TRUE)
{
    if (strncmp((buffer+c),"image",5)==0) test=TRUE;
    c++;
}
offset=c+6; /* Start of Postscript Teil */
obuf=buffer+offset;
c=0L;
test=FALSE;
while(c<filelen && test!=TRUE)
{
    if (strncmp((buffer+c),"translate",9)==0) test=TRUE;
    c++;    
}
strncpy(bf, buffer+c+10L,4);
width=atoi(bf);
strncpy(bf, buffer+c+14L,4);
height=atoi(bf);
oziel=ziel=Malloc( (long)width *(long)height);
if (!ziel) return(M_MEMORY);

for (y=0; y<height; y++)
{
    for (x=0; x<width; x++)
    {
        h1=*(obuf++);   
        h2=*(obuf++);   
        if ( h1!=0x0d || h2!=0x0a)  /* Carriage Return */
        {
        if (h1=='A') h1=10;
        else if (h1=='B') h1=11;
        else if (h1=='C') h1=12;
        else if (h1=='D') h1=13;
        else if (h1=='E') h1=14;
        else if (h1=='F') h1=15;
        else h1-=48;
        
        if (h2=='A') h2=10;
        else if (h2=='B') h2=11;
        else if (h2=='C') h2=12;
        else if (h2=='D') h2=13;
        else if (h2=='E') h2=14;
        else if (h2=='F') h2=15;
        else h2-=48;
        
        wert=(h1<<4)+h2;            /* daraus 1 Byte machen */
        *(ziel++)=wert; 
    }
}
}

pal=smurf_struct->smurf_pic->palette;   /* Graustufen einsetzen */
for (x=0; x<256; x++)
{
    *(pal++)=x;
    *(pal++)=x;
    *(pal++)=x;
}

Mfree(buffer);
smurf_struct->smurf_pic->depth=8;
smurf_struct->smurf_pic->bp_pal=24;
smurf_struct->smurf_pic->pic_data=oziel;
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
strncpy(smurf_struct->smurf_pic->format_name, "Encapsul. Postscript ", 21);
return(M_PICDONE);
}
