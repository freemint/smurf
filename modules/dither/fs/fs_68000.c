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

/*------------------------------------------------------------- */
/*                          FS_68000.C                          */
/*  EnthÑlt die Floyd-Steinberg-C-Routinen fÅr 68000-Prozessor  */
/*------------------------------------------------------------- */
#include <tos.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\..\..\src\lib\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

extern int (*seek_nearest_col)(long *par, int maxcol);
void (*read_pixel)(void);

int floyd_steinberg68000(SMURF_PIC *picture, DITHER_DATA *dither, 
            char *output);

void read_24_68000(void);
void read_16_68000(void);
void read_8_68000(void);

extern int (*set_16_pixels)(char *source, char *dest, int depth, long planelen, int howmany);           
extern void get_standard_pix(void *st_pic, void *buf16, int planes, long planelen, int zoom);


int r, g, b;
char *input, *picdat, *pic_pal, picpixel, *nc_table, *plane_table;

extern int not_in_nct;

extern SERVICE_FUNCTIONS *service;

int st_zoom;            /* wird in Standardformat-Assemblercode importiert! */



/************************************************************/
/*      Hier haben wir ein sehr seltenes Exemplar:          */
/*      Einen Floyd-Steinberg Ditherus-totalus              */
/************************************************************/
int floyd_steinberg68000(SMURF_PIC *picture, DITHER_DATA *dither, char *output)
{
    int *fehler, *fehler_u_ru;
    char *f_u, *f_ru;

    register int *fehler_r;
    register int *fru_acc;
    signed char *fehlerline1, *fehlerline2;
    signed char *f1,*f2;
    register signed char *read_f1, *read_f2;
    register int *write_f1;

    register signed char rot_rechts, grn_rechts, blu_rechts;
    signed char *swap;

    register int nct_index;

    int *ClipTable;
    register int *xpalette, *destpal;
    char *actual_pic;
    int *clipaccess;
    int nidx;
    int x, y, t;
    int red_diff, grn_diff, blu_diff;
    unsigned int dest_planes;
    unsigned int width, height, owidth, destwidth, destheight;
    long planelen;
    long doublineoff, linelen;
    unsigned long planelen8;
    int zoom, bpp;
    long xscale, yscale;
    long par[10];
    char *line, *line_ptr;

    /*
     * Strukturen extrahieren 
     */
    owidth = width = picture->pic_width;
    height = picture->pic_height;
    picdat=input = picture->pic_data;

    nc_table = dither->nc_tab;
    plane_table = dither->planetable;
    dest_planes = dither->dst_depth;
    zoom = dither->zoom;
    
    if(dither->w==0) destwidth = width;
    else destwidth = dither->w;
    if(dither->h==0) destheight = height;
    else destheight = dither->h;
    
    /* Quellzeiger auf Ausschnitt ausrichten
     */
    if(picture->format_type==FORM_PIXELPAK)
        picdat += ((long)dither->x*(long)(picture->depth/8))+((long)dither->y*(long)width*(long)(picture->depth/8));
    else
        picdat += (dither->x/8)+(dither->y*((width+7)/8));

    /*
     * Palette mit 15 Bit fÅr die Farbsuchroutine
     */
    par[0]=(long)dither->red;
    par[1]=(long)dither->green;
    par[2]=(long)dither->blue;
    xpalette = malloc(768L*2L);
    for(t=0; t<256; t++)
    {
        xpalette[t]=(int)(dither->blue24[t]);
        xpalette[t+256]=(int)(dither->green24[t]);
        xpalette[t+512]=(int)(dither->red24[t]);
    }

    pic_pal = picture->palette;
    actual_pic = picture->pic_data;
    planelen8 = (long)((long)((width + 7) / 8) * (long)height); /* LÑnge einer Plane in Bytes (Bytegrenzen)*/

    planelen = dither->dest_planelength;

    if(picture->depth>=8)
        bpp = picture->depth>>3;

    if(picture->format_type==FORM_PIXELPAK) 
        linelen = (width*picture->depth)>>3;
    else
        linelen = (width+7)/8;


    /*
     * Reservieren und Initialisieren der Fehlerpuffer 
     *  Es existieren 2 Fehlerpuffer zum Schreiben und auslesen, die nach jeder Zeile
     *  getauscht werden. Geschrieben wird immer Wordweise, und zwar in der Form 
     *  Ru-Rru-Gu-Gru-Bu-Bru... (unten-rechtsunten-...). Das Auslesen erfolgt Byteweise.
     */
    doublineoff = (long)width*3L * 3L;
    f1= Malloc(doublineoff);
    f2= Malloc(doublineoff); 
    
    if(f1==NULL) 
        return M_MEMORY;
    if(f2==NULL)
    { 
        Mfree(f1); 
        return M_MEMORY; 
    }
    
    memset(f1, 0, doublineoff);
    memset(f2, 0, doublineoff); 

    fehlerline1=f1+16;
    fehlerline2=f2+16;


    /*
     *  Clip-Tabelle
     */
    ClipTable = malloc(2048L*2L);
    for (t=0; t<256; t++)   ClipTable[t] = 0;
    for (t=0; t<256; t++)   ClipTable[t+256] = (int)t;
    for (t=0; t<256; t++)   ClipTable[t+256+256] = 255;
    clipaccess = ClipTable+256;


    /*
     * Fehlertabellen initialisieren
     *  fehler_u_ru enthÑlt die Fehlerskalierung fÅr die Fehler unten und rechts unten
     *  im Format Ru-Rru-Gu-Gru-Bu-Bru... (je ein Byte), so daû Int-weise aus fehler_u_ru in
     *  den Fehlerpuffer geschrieben werden kann.
     */
    if( (fehler=Malloc(1600*2)) == NULL )
        return M_MEMORY;

    for(t=0; t<512; t++)
        fehler[t] = (t-256)*7 >>4;
    fehler_r = fehler+256;

    fehler_u_ru = malloc(1200);
    f_u = (char*)fehler_u_ru;
    f_ru = ((char*)fehler_u_ru+1);

    for(t=0; t<512; t++)
    {
        *(f_u++) = (t-256)*5 >>4;
        *(f_ru++) = (t-256)*3 >>4;
        f_u++;
        f_ru++;
    }
    fru_acc = fehler_u_ru+256;


    /*
     * Scaling
     */
    yscale = (long)zoom*(long)width*(long)bpp;
    xscale = (long)zoom*(long)bpp;

    if(picture->format_type==FORM_STANDARD)
    {
        xscale = zoom;
        yscale = (long)zoom * (long)((width+7)/8);
    }

    destheight/=(zoom+1);
    destwidth/=(zoom+1);
    
    /* 
     * Read routine einhÑngen 
     */
    if(picture->format_type == FORM_STANDARD)   
        read_pixel = read_8_68000;
    else
    {
        if(picture->depth == 24)        read_pixel = read_24_68000;
        else if(picture->depth == 16)   read_pixel = read_16_68000;
        else if(picture->depth == 8)    read_pixel = read_8_68000;
    }

    line = malloc(width+32);
    service->reset_busybox(0, "Floyd Steinberg");

    y = 0;
    do
    {
        if(!(y&15))service->busybox((int)(((long)y << 7L) / (long)destheight));

        rot_rechts = grn_rechts = blu_rechts = 0;
        input=picdat;
        actual_pic=input;

        /*
         *  Standardformat? -> Zeile auslesen!
         */
        if(picture->format_type == FORM_STANDARD)
        {
            for(x=0; x<(owidth+15)/16; x++)
            {
                get_standard_pix(actual_pic, line+x*16, picture->depth, planelen8, 0);
                actual_pic +=2;
            }

            input = line;
        }

        read_f1 = fehlerline2;
        write_f1 = (int*)fehlerline1;
        line_ptr = line;
        
        x = 0;
        do
        {
            read_pixel();

            b = clipaccess[b + blu_rechts + *(read_f1) + *(read_f1+7)];
            g = clipaccess[g + grn_rechts + *(read_f1+2) + *(read_f1+2+7)];
            r = clipaccess[r + rot_rechts + *(read_f1+4) + *(read_f1+4+7)];
            read_f1+=6;
        
            /*
             * Farbindex ermitteln
             */
            nct_index = ((r>>3) << 10) | ((g>>3) << 5) | (b>>3);
            nidx = nc_table[nct_index];
            if(nidx==not_in_nct)
            {
                par[3] = (r>>3);
                par[4] = (g>>3);
                par[5] = (b>>3);
                nidx = seek_nearest_col(par, 1<<dest_planes);
                nc_table[nct_index] = nidx;
            }
            *line_ptr++ = plane_table[nidx];

            /*
             * Farbdifferenzen ermitteln + Fehler merken
             */
            destpal = xpalette+nidx;
            
            blu_diff = b - *destpal;
            blu_rechts = fehler_r[blu_diff];
            *(write_f1++) = fru_acc[blu_diff];
            
            grn_diff = g - *(destpal+256);
            grn_rechts = fehler_r[grn_diff];
            *(write_f1++) = fru_acc[grn_diff];
            
            red_diff = r - *(destpal+512);
            rot_rechts = fehler_r[red_diff];
            *(write_f1++) = fru_acc[red_diff];

            input+=xscale;
        } while(++x < destwidth);

        /*
         *  und jetzt die Zeile ins Standardformat wandeln
         */
        output += set_16_pixels(line, output, dest_planes, planelen, destwidth);

        swap = fehlerline1;
        fehlerline1 = fehlerline2;
        fehlerline2 = swap;

        picdat+=yscale;
        picdat+=linelen;
    } while(++y < destheight);

    free(ClipTable);
    free(xpalette);
    free(line);
    Mfree(f1);
    Mfree(f2);
    Mfree(fehler_u_ru);
    return M_PICDONE;
} /* floyd_steinberg24_4 */



void read_24_68000(void)
{
    r = *input++;
    g = *input++;
    b = *input++;
}

void read_8_68000(void)
{
    register char picpixel;
    register char *picpal;

    picpixel = *input++;
    picpal = pic_pal + (picpixel + picpixel + picpixel);
    r = picpal[0];
    g = picpal[1];
    b = picpal[2];
}

void read_16_68000(void)
{
    unsigned register int pixel16;

    pixel16 = *((unsigned int *)input)++;
    r = (pixel16 >> 11)<<3;
    g = ((pixel16 & 0x7c0) >> 6)<<3;
    b = (pixel16 & 0x1f)<<3;
}
