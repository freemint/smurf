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

/* --------------------------------------------------*/
/* --------------------------------------------------*/
/* --------------------------------------------------*/
/* --------------------------------------------------*/
#include<tos.h>
#include<ext.h>
#include<screen.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define NOMEM   (void*)0L
#define TRUE            1
#define FALSE           0

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"TRP Graphix-Importer v0.1",
                        0x0010,
                        "Dale Russell",
                        "TRP", "", "", "", "",
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

/***********************************************************/
/*  Header                                                 */
/***********************************************************/
typedef struct 
{
    char magic[8];          /* Erkennung: 'TRP_GFX!' */
    char type;              /* Typ(Bitfield; Gesetzt: vorhanden;
                                Gel”scht: nicht vorhanden) 
                                Bit 0: Bitmap enthalten
                                Bit 1: Animation enthalten
                                Bit 2: Vektorgraphik enthalten  */
    long bitmap;            /* Offset auf Bildstruktur */
    long animation;         /* Offset auf Animationsstruktur */
    long vector;            /* Offset auf Vektorstruktur */
} trp_header;

/***********************************************************/
/*  Bildbeschreibungsstruktur                              */
/***********************************************************/
typedef struct
{
    long data;              /* Offset auf Bilddaten,
                                 relativ zum Dateianfang */
    long width;             /* Bildbreite in Pixel */
    long height;            /* Bildh”he in Pixel */
    char depth;             /* Farbtiefe in Bits */
    char planes;            /* Anzahl der Farbebenen */
    char mode;              /* Kompressionsart */
    char incoming;          /* current Bitstream(0) oder 
                                interlaced(1) */
    char coltable;          /* Farbpalette (0=Nein; 1=Ja; 
                                2=Graufstufen linear, 
                                    nicht enthalten) */
    char table_depth;       /* Palettenfarbtiefe */
    int  colors;            /* Anzahl der Farben in der Palette */
    long tableoffset;       /* Offset der Palettendaten,
                                relativ zum Dateianfang */                          
    char colormodel;        /* Farbmodell: (0=RGB; 1=YCbCr;
                                2=CMYK; 3=YUV; 4=YIQ) */
    char text[64];          /* Text, z.B. zu Beschreibung ect. */                                                       
} bitmap_struct;    

int imp_module_main(GARGAMEL *smurf_struct)
{
char *input, *output, *retbuf, *GetBuf;
long xc,yc,len,gox,goy,calc;
int x,c, ID_Run, MaxRun;
char r,g,b,y,cr;
char cb;

trp_header *header;
bitmap_struct *bitmap;

input=smurf_struct->smurf_pic->pic_data;

if ( strncmp(input, "TRP_GFX!", 8) ) return(M_INVALID);
header=Malloc( sizeof(trp_header) );
bitmap=Malloc( sizeof(bitmap_struct) );
memcpy( header, input, sizeof(trp_header) );
memcpy( bitmap, input+sizeof(trp_header), sizeof(bitmap_struct) );
if ( bitmap->coltable )
{
    memcpy( smurf_struct->smurf_pic->palette,
        input+(bitmap->tableoffset),
            (long)(bitmap->colors)*(long)
                (bitmap->table_depth / 8 ) );
}               
if (bitmap->colormodel==0)
    smurf_struct->smurf_pic->col_format=RGB;
if (bitmap->depth >=8 )
    smurf_struct->smurf_pic->format_type=FORM_PIXELPAK;

len= bitmap->width * bitmap->height * (long)(bitmap->depth / 8);
output=retbuf=Malloc(len);
if (!retbuf) { Mfree(bitmap); Mfree(header); return(M_MEMORY); }
GetBuf=input+(bitmap->data);
goy=bitmap->height;
gox=bitmap->width;

/* Dynamic RLE (c)Coke of Therapy */

if (bitmap->mode==1 && bitmap->depth==8)
{
xc=0;
yc=0;
ID_Run=*(GetBuf++);
MaxRun=0xff-ID_Run;
while(yc < goy)
{
    for (;;)
    {
        x=*(GetBuf++);
        if  ( x>=ID_Run)
        {   
            x &= MaxRun;
            xc+=x;
            c=*(GetBuf++);
            while (x--)
                *(output++)=c;
        }
        else  
        { 
            xc++;
            *(output++)=x; 
        }
        
        if (xc>=gox) 
        {   xc-=gox; yc++; break; }
    } /* for (;;) */
 } /* while yc */   

Mfree(input);
smurf_struct->smurf_pic->pic_data=retbuf;
}
/****************************************************************/
/*      24 Bit Y,Cb,Cr RLE                                      */
/****************************************************************/
else if (bitmap->depth==24)
{
yc=0;
while (yc++ < goy)
{
    xc=0;
    while(xc++ < gox)
    {
        y=(unsigned char)*(GetBuf++);
        cb=(unsigned char)*(GetBuf++);
        cr=(unsigned char)*(GetBuf++);
        cb=cr=0;
        calc = (long)((long)cr * 1402l) / 1000 + (long)y;
        r = (unsigned char)calc;
        calc = (long)((long)cb * 1772l ) / 1000 + (long)y;
        b = (unsigned char)calc;
        calc = (long)(1000l*(long)y - 114l*(long)b - 299000l*(long)r) / 587;
        g = (unsigned char)calc;
        *(output++)=r;
        *(output++)=g;
        *(output++)=b;
    }
}   
Mfree(input);
smurf_struct->smurf_pic->pic_data=retbuf;
}
strncpy(smurf_struct->smurf_pic->format_name, "Therapy Graphix(TRP) ",21);
smurf_struct->smurf_pic->pic_width=(int)bitmap->width;
smurf_struct->smurf_pic->pic_height=(int)bitmap->height;
smurf_struct->smurf_pic->depth=bitmap->depth;

Mfree(bitmap);
Mfree(header);
return(M_PICDONE);
}
