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

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/*  Therapy Graphix Export Modul fr Smurf v1.0                     */
/*  Written by Dale Russell, April->August                          */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */

#define TRUE    1
#define FALSE   0

#include <tos.h>
#include <ext.h>
#include <aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <portab.h>
#include "..\..\import.h"
#include "..\..\..\smurf\smurfine.h"
#include "trp_gfx.h"
#include "trp_gfx.rh"
#include "trp_gfx.rsh"
 
BITBLK *prev(GARGAMEL *prev_struct);

/* Infostruktur fr Hauptmodul */
MOD_INFO    
module_info=
{
            "Therapy Grafix (TRP)",
            0x0020,
            "Dale Russell",
            "TRP","","","","",
            "","","","","",
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
            0,0,0,0
};

MOD_ABILITY  module_ability = {
                        8, 24, 4, 1, 2, 16, 15, 32,
                        FORM_PIXELPAK,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
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

void header_fuellen(trp_header *header);
void bitmapstruktur_fuellen(trp_header *header, bitmap_struct *bitmap, 
                                GARGAMEL *smurf_struct);
long encoding8Bit_trp(bitmap_struct *bitmap,GARGAMEL *smurf_struct,
                    char *expdata, int quality);                                
long encoding24Bit_trp(bitmap_struct *bitmap,GARGAMEL *smurf_struct,
                    char *expdata, int quality);
/***********************************************************/
/*  Exporter MAIN                                          */
/***********************************************************/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
EXPORT_PIC *exp_pic;
trp_header *header;
bitmap_struct *bitmap;
char *expdata;      /* Output Arrays */
long org_length;
long datalength;    /* L„nge der Rckgabedaten in Bytes */                                          
int quality;
int module_id, t, button;

    org_length=(long)smurf_struct->smurf_pic->pic_width * (long)
            smurf_struct->smurf_pic->pic_height * (long) 
            (smurf_struct->smurf_pic->depth / 8);
    exp_pic=Malloc(sizeof(EXPORT_PIC));
    header=Malloc (sizeof(trp_header));
    bitmap=Malloc (sizeof(bitmap_struct));
    expdata=Malloc (org_length+16000l);
    if ( (!exp_pic) || (!header) || (!bitmap) || (!expdata) )
        {   Mfree(header); Mfree(bitmap); Mfree(expdata);
            exp_pic->backval=M_MEMORY;
            smurf_struct->module_mode=M_MEMORY; 
            return(exp_pic); }

    /***************************************************/
    /*  Header in Output Array kopieren                */
    /***************************************************/
    header_fuellen(header);
    printf("\n  Header OK!");
    memcpy( expdata, header, sizeof(trp_header) );
    /***************************************************/
    /*  Bildstruktur in Output Array kopieren          */
    /***************************************************/
    bitmapstruktur_fuellen(header, bitmap, smurf_struct);
    printf("\n  Bitmap-Strukt OK!");
    memcpy( expdata+sizeof(trp_header), bitmap, sizeof(bitmap_struct) );
    /***************************************************/
    /*  Farbpalette kopieren (bitmap->coltable==1)     */
    /***************************************************/
    if ( (bitmap->coltable) == 1)
        memcpy (expdata+(bitmap->tableoffset),smurf_struct->smurf_pic->palette, 
                    (long)((bitmap->data)-(bitmap->tableoffset)) );
    /***************************************************/
    /*  8Bit D-RLE  Kodierung                          */
    /***************************************************/
    if (bitmap->depth==8)
    {
        printf("\nQualit„t 8 Bit: "); 
        scanf("%i",&quality);
        datalength=
            encoding8Bit_trp(bitmap,smurf_struct,expdata,quality);
    }   
    else if (bitmap->depth==24)
    {
        printf("\nQualit„t 24 Bit: "); 
        scanf("%i",&quality);
        datalength=
            encoding24Bit_trp(bitmap,smurf_struct,expdata,quality);
    }       
        datalength+=(bitmap->data);             
    /***************************************************/
    /*  smurf-struct fllen                            */
    /***************************************************/
    exp_pic->pic_data=expdata;
    exp_pic->f_len=datalength;
    exp_pic->backval=M_DONEEXIT;
    smurf_struct->module_mode=M_DONEEXIT;
    
    Mfree(header);
    Mfree(bitmap);
    return(exp_pic);
}




/************* Header-Struktur fllen ****************/

void header_fuellen(trp_header *header)
{
    strncpy ( (header->magic), "TRP_GFX!", 8);  /* MagicCode */
    header->type=1;                             /* Bitmap */
    header->bitmap=sizeof(trp_header);              /* Offset auf Bitmap-Struktur */
    header->animation=0;                        /* Keine Animation */
    header->vector=0;                           /* Keine Vektorgraphik */
}

/************* Bitmap-Struktur fllen ****************/

void bitmapstruktur_fuellen(trp_header *header, bitmap_struct *bitmap, 
                                GARGAMEL *smurf_struct)
{
if (smurf_struct->smurf_pic->bp_pal > 0)
{
    bitmap->colors = 256;
    bitmap->data=(header->bitmap)+sizeof(bitmap_struct)+
        (long)(bitmap->colors)*(long)(smurf_struct->smurf_pic->bp_pal / 8); 
    bitmap->tableoffset=(header->bitmap)+sizeof(bitmap_struct); /* Offset auf Palettendaten */
    bitmap->coltable=1; /* Farbpalette ist vorhanden */
    bitmap->table_depth=(char)smurf_struct->smurf_pic->bp_pal;  /* Palettenfarbtiefe in Bits */
}
else        
{
    bitmap->data=(header->bitmap)+sizeof(bitmap_struct);
    bitmap->tableoffset=0;          /* Offset auf Palettendaten */
    bitmap->colors=0;
    bitmap->coltable=0; /* Keine Farbpalette vorhanden */
    bitmap->table_depth=0;  /* Palettenfarbtiefe in Bits */
}                       
bitmap->width =(long)smurf_struct->smurf_pic->pic_width;    /* Bildbreite */
bitmap->height=(long)smurf_struct->smurf_pic->pic_height;   /* Bildbreite */
bitmap->depth=(char)smurf_struct->smurf_pic->depth; /* Farbtiefe */
bitmap->planes=1;   /* Farbebenen */
bitmap->mode=1;     /* Kodierung: Dynamic RLE */
bitmap->incoming=0; /* current Bitstream */
bitmap->colormodel=0;   /* RGB-Farbmodell */
strcpy(bitmap->text,"This bitmap was saved with Smurf\0");
}                               

/******************************************************************/
/*      8Bit "Dynamic-RLE"-Kodierung                              */
/******************************************************************/
long encoding8Bit_trp(bitmap_struct *bitmap,GARGAMEL *smurf_struct,
                    char *expdata, int quality)
{
char ID_Byte_Run, *output, *input, *cptr, *scan1, *scan2;
int pixel,counter,xgo,same,typ,MaxRun,diff;
long x,y,gox,goy,length;
long id_case[8]={0,0,0,0,0,0,0,0};
char *pal, *getpal;
int w,r1,g1,b1,r2,g2,b2,gquality;

/* ------------- */
/* Quantisierung */
/* ------------- */
gox=bitmap->width;
goy=bitmap->height;
scan1=smurf_struct->smurf_pic->pic_data;
scan2=scan1+1;
pal=smurf_struct->smurf_pic->palette;
if (quality>0)
{
gquality=quality-1;
y=0;    
while (y++<goy)
{
x=0;
    while( x++<gox)
    {
        w=*(scan1);
        getpal=pal+w*3;
        r1=*(getpal++);
        g1=*(getpal++);
        b1=*(getpal++);
        w=*(scan2);
        getpal=pal+w*3;
        r2=*(getpal++);
        g2=*(getpal++);
        b2=*(getpal++); 
        
        if (abs(r1-r2)<quality)
            if (abs(g1-g2)<gquality)
                if (abs(b1-b2)<quality)
                    *(scan2)=*(scan1); 

        scan1++;
        scan2++;                        
    }
}
}

/* ---------------- */
/*  Byte Run suchen */
/* ---------------- */
scan1=smurf_struct->smurf_pic->pic_data;
x=y=0;  
while (y<goy)
{
xgo=TRUE;
    while( xgo==TRUE)
    {
        pixel=*(scan1++);
        cptr=scan1;
        counter=0x00;
        same=TRUE;
        x++;
            for (;;)
            {
                if (same==FALSE || counter>=0x7f) 
                    break; 
                counter++;
                if (*(cptr++)!=pixel)
                    same=FALSE;
            }

    
        typ=0;
        if (counter>=2 && counter<16) typ=1;
        else if (counter>=16 && counter<32) typ=2;
        else if (counter>=32 && counter<64) typ=3;
        else if (counter>=64 && counter<128) typ=4;
        id_case[typ]+=counter;  
        scan1+=(counter-1); 
        x+=(counter-1);     
        if (x>=gox) 
        {
            x-=gox;
            y++;
            if (y>=goy)
                xgo=FALSE;
        }
    }
}   
x=id_case[1];
counter=1;
for (xgo=2; xgo<=4; xgo++)
{
    if (id_case[xgo]>=x)
    {
        x=id_case[xgo];
        counter=xgo;
    }
}
switch(counter)
{
    case    0x01:   ID_Byte_Run=0xf0; break;
    case    0x02:   ID_Byte_Run=0xe0; break;
    case    0x03:   ID_Byte_Run=0xc0; break;
    case    0x04:   ID_Byte_Run=0x80; break;
}           

/* ------------------ */
/*  Kodierung         */
/* ------------------ */
MaxRun=0xff-ID_Byte_Run;
input=smurf_struct->smurf_pic->pic_data;
output=expdata+(bitmap->data);
*(output++)=ID_Byte_Run;
length=1;   
x=y=0;  

while (y<goy)
{
xgo=TRUE;
    while( xgo==TRUE)
    {
        pixel=*(input++);
        cptr=input;
        counter=0x00;
        same=TRUE;
        x++;
        
            for (;;)
            {
                if (same==FALSE || counter>=MaxRun) 
                    break; 
                counter++;
                if (*(cptr++)!=pixel)
                    same=FALSE;
            }
            
            
            
            if (counter<=0x01)
            {       
                    if (pixel>=ID_Byte_Run)
                    {       
                        *(output++)=ID_Byte_Run+1; 
                        *(output++)=pixel;
                        length+=2;
                    }
                    else        
                    {
                        *(output++)=pixel;  
                        length++;
                    }   
            }   

            else
            {
                *(output++)=(unsigned char)(counter+ID_Byte_Run);
                *(output++)=pixel;
                x+=(counter-1);
                input+=(counter-1);
                length+=2;
            }
            
        if (x>=gox) 
        {
            x-=gox;
            y++;
            if (y>=goy)
                xgo=FALSE;
        }
    }   
}   
return(length);
}


/******************************************************************/
/*      24Bit "Smurfy's Law"-Kodierung                            */
/******************************************************************/
long encoding24Bit_trp(bitmap_struct *bitmap,GARGAMEL *smurf_struct,
                    char *expdata, int quality)
{
long xc,yc;
long xmax,ymax;
char *getdata, *getred, *getgreen, *getblue;
char *putdata, *puty, *putcb, *putcr;
char y,cr,r,g,b;
char cb;

long calc,len;
xmax=bitmap->width;
ymax=bitmap->height;
getdata=smurf_struct->smurf_pic->pic_data;
putdata=expdata+(bitmap->data);
yc=len=0;

    while(yc++ < xmax)
    {
    xc=0;
        while (xc++<ymax)
        {
            r=(unsigned char)*(getdata++);
            g=(unsigned char)*(getdata++);
            b=(unsigned char)*(getdata++);
            *(putdata++)=r;
            *(putdata++)=g;
            *(putdata++)=b;
            len+=3;
        }
    }   

return(len);
}                   
