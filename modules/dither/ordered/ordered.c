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

/********************************************************************** */
/*                  Ordered Dither-Routinen fÅr Smurf                   */
/*                                                                      */
/*  Furchtbar schlecht und genauso langsam.                             */
/*  Wie man es (hoffentlich) besser macht, zeigen die Routinen von      */
/*  Project-X. Z.B. V256_PP.S                                           */
/*  modr, modg und modb lassen sich wahrscheinlich zusammenfassen, da   */
/*  die Verteilung sowieso gleich ist und ich mit verschiedenen keine   */
/*  verwendbaren Ergebnisse bekomme. Damit lÑût sich wahrscheinlich     */
/*  auch eine Beschleunigung erzielen, da die Register nicht mehr so    */
/*  knapp sind.                                                         */
/********************************************************************** */

#include <tos.h>
#include <stdlib.h>
#include <vdi.h>
#include <string.h>
#include <stdio.h>
#include <ext.h>
#include "..\..\..\src\lib\demolib.h"
#include "..\..\..\src\lib\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

void f_dither_orderedcol(SMURF_PIC *pic_todit,  char* ziel, DITHER_DATA *dither);
void orderedto1(SMURF_PIC *picture, char* ziel);
void orderedto4(SMURF_PIC *picture, char* ziel);
char make_tabs(SMURF_PIC *pic_todit, char *divN, char *divN6, char *divN6_1, char *divN36, char *divN36_1, int *modr, int *modg, int *modb);
void make_pal(SMURF_PIC *picture, char *tab);

/* Get Standardpixel Assembler-Routs */
void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);

int (*set16pixels)(char *buf16, char *dest, int depth, long planelen, int howmany);

int (*busybox)(int pos);

DITHER_MOD_INFO module_info =
{
    "Ordered Dither",
    "Christian Eyrich",
    0x0100,                     /* Schlumpfine-Version */
    0,                          /* konfigurierbar? */
    FIXPAL                      /* Palettenmodus */
};



/*------------------ Ordered-Dither Matrix ---------------------*/

char *plane_table, *palette,
     skiph, correct;
unsigned int width, height, endwid, endhgt, bplanes;

unsigned long skipv, planelengthw;

typedef struct
{
    char line[16][16];
} MATRIX;

MATRIX *matrix;

static  MATRIX defmatrix = {0x00, 0xc0, 0x30, 0xf0, 0x0c, 0xcc, 0x3c, 0xfc, 0x03, 0xc3, 0x33, 0xf3, 0x0f, 0xcf, 0x3f, 0xff,
                            0x80, 0x40, 0xb0, 0x70, 0x8c, 0x4c, 0xbc, 0x7c, 0x83, 0x43, 0xb3, 0x73, 0x8f, 0x4f, 0xbf, 0x7f,
                            0x20, 0xe0, 0x10, 0xd0, 0x2c, 0xec, 0x1c, 0xdc, 0x23, 0xe3, 0x13, 0xd3, 0x2f, 0xef, 0x1f, 0xdf,
                            0xa0, 0x60, 0x90, 0x50, 0xac, 0x6c, 0x9c, 0x5c, 0xa3, 0x63, 0x93, 0x53, 0xaf, 0x6f, 0x9f, 0x5f,
                            0x08, 0xc8, 0x38, 0xf8, 0x04, 0xc4, 0x34, 0xf4, 0x0b, 0xcb, 0x3b, 0xfb, 0x07, 0xc7, 0x37, 0xf7,
                            0x88, 0x48, 0xb8, 0x78, 0x84, 0x44, 0xb4, 0x74, 0x8b, 0x4b, 0xbb, 0x7b, 0x87, 0x47, 0xb7, 0x77,
                            0x28, 0xe8, 0x18, 0xd8, 0x24, 0xe4, 0x14, 0xd4, 0x2b, 0xeb, 0x1b, 0xdb, 0x27, 0xe7, 0x17, 0xd7,
                            0xa8, 0x68, 0x98, 0x58, 0xa4, 0x64, 0x94, 0x54, 0xab, 0x6b, 0x9b, 0x5b, 0xa7, 0x67, 0x97, 0x57,
                            0x02, 0xc2, 0x32, 0xf2, 0x0e, 0xce, 0x3e, 0xfe, 0x01, 0xc1, 0x31, 0xf1, 0x0d, 0xcd, 0x3d, 0xfd,
                            0x82, 0x42, 0xb2, 0x72, 0x8e, 0x4e, 0xbe, 0x7e, 0x81, 0x41, 0xb1, 0x71, 0x8d, 0x4d, 0xbd, 0x7d,
                            0x22, 0xe2, 0x12, 0xd2, 0x2e, 0xee, 0x1e, 0xde, 0x21, 0xe1, 0x11, 0xd1, 0x2d, 0xed, 0x1d, 0xdd,
                            0xa2, 0x62, 0x92, 0x52, 0xae, 0x6e, 0x9e, 0x5e, 0xa1, 0x61, 0x91, 0x51, 0xad, 0x6d, 0x9d, 0x5d,
                            0x0a, 0xca, 0x3a, 0xfa, 0x06, 0xc6, 0x36, 0xf6, 0x09, 0xc9, 0x39, 0xf9, 0x05, 0xc5, 0x35, 0xf5,
                            0x8a, 0x4a, 0xba, 0x7a, 0x86, 0x46, 0xb6, 0x76, 0x89, 0x49, 0xb9, 0x79, 0x85, 0x45, 0xb5, 0x75,
                            0x2a, 0xea, 0x1a, 0xda, 0x26, 0xe6, 0x16, 0xd6, 0x29, 0xe9, 0x19, 0xd9, 0x25, 0xe5, 0x15, 0xd5,
                            0xaa, 0x6a, 0x9a, 0x5a, 0xa6, 0x66, 0x96, 0x56, 0xa9, 0x69, 0x99, 0x59, 0xa5, 0x65, 0x95, 0x55};

/* ---------------------------------------------------------------- */
/*                          Ordered Dither                          */
/* ---------------------------------------------------------------- */
int dither_module_main(DITHER_DATA *dither)
{
    char *ziel;

    SMURF_PIC *picture;


    /*-------------------- Struktur auslesen */
    picture = dither->picture;
    ziel = dither->dest_mem;
    
    width = picture->pic_width;
    height = picture->pic_height;
    palette = picture->palette;

    endwid = width / (dither->zoom + 1);
    endhgt = height / (dither->zoom + 1);

    if(picture->depth == 24)
    {
        /* Nach jedem Pixel Åberspringen */
        skiph = picture->zoom * 3;
        skipv = (unsigned long)picture->zoom * (unsigned long)width * 3L;
        /* Unter den Tisch gefallene Bytes nachholen */
        correct = (width - endwid * (picture->zoom + 1)) * 3;
    }
    else
    {
        /* Nach jedem Pixel Åberspringen */
        skiph = picture->zoom;
        if(picture->format_type == FORM_PIXELPAK)
            skipv = (unsigned long)picture->zoom * (unsigned long)width;
        else
            skipv = (unsigned long)picture->zoom * ((unsigned long)(width + 7) / 8);
        /* Unter den Tisch gefallene Bytes nachholen */
        correct = width - endwid * (picture->zoom + 1);
    }

    bplanes = dither->dst_depth;

    plane_table = dither->planetable;

    planelengthw = dither->dest_planelength;

    busybox = dither->services->busybox;

    set16pixels = dither->set_16pixels;

    matrix = &defmatrix;

    /*-------------------- Und los gehts... */
    dither->services->reset_busybox(0, "Ordered Dither...");

    if(bplanes < 4)
        orderedto1(picture, ziel);
    else
        if(bplanes == 4)
            orderedto4(picture, ziel);
        else
            f_dither_orderedcol(picture, ziel, dither);

    return(M_PICDONE);
}



/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/*    Alle Farbtiefen nach 1 Bit Ordered Dither                     */
/*    Hingeschmiert von Christian Eyrich 6/96                       */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
void orderedto1(SMURF_PIC *picture, char* ziel)
{
    char *buffer, *line, *pal, *matline, *pixbuf,
         bit, v, depth;

    unsigned int *buffer16,
                 x, y, val, w;

    long planelengthr;


/* wie schnell sind wir? */
/*  init_timer(); */

    depth = picture->depth;

    if(depth == 16)
        buffer16 = (unsigned int *)picture->pic_data;
    else
        buffer = picture->pic_data;

    pixbuf = (char *)Malloc(width + 7);

/* PlanelÑnge beim Lesen */
    w = (unsigned int)((width + 7) / 8);
    planelengthr = (unsigned long)w * (unsigned long)height;    /* LÑnge einer Plane in Bytes */

    /* Padpixel um auf das Zielformataligning zu kommen */
    v = endwid%8 == 0 ? 0 : 1;
    /* Padpixel ausrechnen ... */
    if(set16pixels(pixbuf, ziel, 1, 0, 1) == 2 && ((endwid + 7) / 8 * 8)%16)
        v++;

    memset(ziel, 0x0, ((long)endwid * (long)endhgt) >> 3L);


    if(picture->format_type == FORM_STANDARD)
    {
        y = 0;
        do
        {
            matline = matrix->line[y & 15];

            memset(pixbuf, 0x0, width);
            getpix_std_line(buffer, pixbuf, depth, planelengthr, width);
            buffer += w;
            line = pixbuf;

            x = 0;
            bit = 7;
            do
            {
                val = *line++;
                pal = palette + val + val + val;
                if((((unsigned long)*pal++ * 872L
                   + (unsigned long)*pal++ * 2930L
                   + (unsigned long)*pal * 296L) >> 12) <= matline[x & 15])
                    *ziel |= 0x01 << bit;

                if(bit-- == 0)
                {
                    bit = 7;
                    ziel++;
                }
                line += skiph;
            } while(++x < endwid);
            ziel += v;

            buffer += skipv;
        } while(++y < endhgt);
    } /* STANDARD */
    else
    {
        y = 0;
        do
        {
            matline = matrix->line[y & 15];

            if(depth == 8)
            {
                x = 0;
                bit = 7;
                do
                {
                    val = *buffer++;
                    pal = palette + val + val + val;
                    if((((unsigned long)*pal++ * 872L
                       + (unsigned long)*pal++ * 2930L
                       + (unsigned long)*pal * 296L) >> 12) <= matline[x & 15])
                        *ziel |= 0x01 << bit;

                    if(bit-- == 0)
                    {
                        bit = 7;
                        ziel++;
                    }
                    buffer += skiph;
                } while(++x < endwid);

                buffer += correct;
                buffer += skipv;
            }
            else
                if(depth == 24)
                {
                    x = 0;
                    bit = 7;
                    do
                    {
                        if((((unsigned long)*buffer++ * 872L
                          + (unsigned long)*buffer++ * 2930L
                          + (unsigned long)*buffer++ * 296L) >> 12) <= matline[x & 15])
                            *ziel |= 0x01 << bit;

                        if(bit-- == 0)
                        {
                            bit = 7;
                            ziel++;
                        }
                        buffer += skiph;
                    } while(++x < endwid);

                    buffer += correct;
                    buffer += skipv;
                }
                else
                {
                    x = 0;
                    bit = 7;
                    do
                    {
                        val = *buffer16++;
                        if((((unsigned long)((val & 0xf800) >> 8) * 872L
                           + (unsigned long)((val & 0x7e0) >> 3) * 2930L
                           + (unsigned long)((val & 0x1f) << 3) * 296L) >> 12) <= matline[x & 15])
                            *ziel |= 0x01 << bit;

                        if(bit-- == 0)
                        {
                            bit = 7;
                            ziel++;
                        }
                        buffer16 += skiph;
                    } while(++x < endwid);

                    buffer16 += correct;
                    buffer16 += skipv;
            }

            ziel += v;
        } while(++y < endhgt);
    }

/* wie schnell waren wir? */
/*  printf("%lu\n", get_timer());
    getch(); */

    Mfree(pixbuf);

    return;
} /* orderedto1 */



/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
/*    Alle Farbtiefen nach 4 Bit Ordered Dither                     */
/*    Hingeschmiert von Christian Eyrich 4/97                       */
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
void orderedto4(SMURF_PIC *picture, char* ziel)
{
    char *buffer, *line, *pal, *matline, *pixbuf,
         matpix, ind, convtab[8], depth;

    unsigned int *buffer16,
                 x, y, w, val, bh, bl;

    unsigned long planelengthr;


/* wie schnell sind wir? */
/*  init_timer(); */

/* Palette mit festen FarbabstÑnden berechnen, und Bildpalette darauf setzen */
    make_pal(picture, convtab);

    depth = picture->depth;

    if(depth == 16)
        buffer16 = (unsigned int *)picture->pic_data;
    else
        buffer = picture->pic_data;

/* PlanelÑnge beim Lesen */
    w = (unsigned int)((width + 7) / 8);
    planelengthr = (unsigned long)w * (unsigned long)height;   /* LÑnge einer Plane in Bytes */

    bh = endhgt / 10;               /* busy-height */
    bl = 0;                         /* busy-length */

    pixbuf = (char *)Malloc(width + 7);

    y = 0;
    do
    {
        if(!(y%bh))
        {
            busybox(bl);
            bl += 12;
        }

        matline = matrix->line[y & 15];

        if(depth <= 8)
        {
            if(picture->format_type == FORM_STANDARD)
            {
                memset(pixbuf, 0x0, width);
                getpix_std_line(buffer, pixbuf, depth, planelengthr, width);
                buffer += w;
                line = pixbuf;
            }
            else
            {
                line = buffer;
                buffer += width;
            }

            x = 0;
            do
            {
                val = *line++;
                pal = palette + val + val + val;

                ind = 0;
                matpix = matline[x & 15];               

                if(*pal++ > matpix)
                    ind |= 0x04;
                if(*pal++ > matpix)
                    ind |= 0x02;
                if(*pal > matpix)
                    ind |= 0x01;

                pixbuf[x] = plane_table[convtab[ind]];

                line += skiph;
            } while(++x < endwid);

            buffer += skipv;
        }
        else
            if(depth == 24)
            {
                x = 0;
                do
                {
                    ind = 0;
                    matpix = matline[x & 15];               

                    if(*buffer++ > matpix)
                        ind |= 0x04;
                    if(*buffer++ > matpix)
                        ind |= 0x02;
                    if(*buffer++ > matpix)
                        ind |= 0x01;

                    pixbuf[x] = plane_table[convtab[ind]];

                    buffer += skiph;
                } while(++x < endwid);

                buffer += correct;
                buffer += skipv;
            }
            else
            {
                x = 0;
                do
                {
                    ind = 0;
                    matpix = matline[x & 15];

                    val = *buffer16++;
                    if(((val & 0xf800) >> 8) > matpix)
                        ind |= 0x04;
                    if(((val & 0x7e0) >> 3) > matpix)
                        ind |= 0x02;
                    if(((val & 0x1f) << 3) > matpix)
                        ind |= 0x01;

                    pixbuf[x] = plane_table[convtab[ind]];

                    buffer16 += skiph;
                } while(++x < endwid);

                buffer16 += correct;
                buffer16 += skipv;
            }

        ziel += set16pixels(pixbuf, ziel, bplanes, planelengthw, endwid);   /* Pixel setzen und ziel weiter */
    } while(++y < endhgt);

/* wie schnell waren wir? */
/*  printf("%lu\n", get_timer());
    getch(); */

    Mfree(pixbuf);

    return;
} /* orderedto4 */



/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
/*                  Ordered ColorDither                             */
/*          Hingeschmiert von Christian Eyrich 9/96                 */
/*  Eingebaute Dithermatrix entspricht ORDERCOL.DIT aus GEM-View    */
/*  Standardbilder und Pixelpacked als Input                        */
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
void f_dither_orderedcol(SMURF_PIC *picture, char* ziel, DITHER_DATA *dither)
{
    char *buffer, *line, *pal, *matline, *pixbuf,
         r, g, b, offset, matpix, depth,
         divN[256], divN6[256], divN6_1[256],
         divN36[256], divN36_1[256];

    int modr[256], modg[256], modb[256];
    unsigned int *buffer16,
                 x, y, ind, bh, bl, val, w;
    
    unsigned long planelengthr, skipBytes;


    depth = picture->depth;

    if(depth == 16)
        buffer16 = (unsigned int *)picture->pic_data;
    else
        buffer = picture->pic_data;

    offset = make_tabs(picture, divN, divN6, divN6_1, divN36, divN36_1, modr, modg, modb);

/* PlanelÑnge beim Lesen */
    w = (unsigned int)((width + 7) / 8);
    planelengthr = (unsigned long)w * (unsigned long)height;   /* LÑnge einer Plane in Bytes */

    bh = endhgt / 10;               /* busy-height */
    bl = 0;                         /* busy-length */

    pixbuf = (char *)Malloc(width + 7);

/* wie schnell sind wir? */
/*  init_timer(); */



    /*  Source-Skipbytes (fÅr Ausschnitte) berechnen
     */
    if(dither->w==0)    skipBytes=0;
    else
    {
        if(picture->format_type==FORM_PIXELPAK)
            skipBytes = (width-dither->w)*(depth/8);
        else skipBytes = 0;
    }

    /*  Source-Zeiger auf den Ausschnittsbeginn ausrichten
     */
    if(picture->format_type==FORM_PIXELPAK)
    {
        buffer += ((long)dither->x*(long)(depth/8))+((long)dither->y*(long)picture->pic_width*(long)(depth/8));
        buffer16 += ((long)dither->x*(long)(depth/8))+((long)dither->y*(long)picture->pic_width*(long)(depth/8));
    }
    else
        buffer += (dither->x/8)+((long)dither->y*(long)((width+7)/8));

    if(dither->w!=0)
    {
        width = dither->w;
        height = dither->h;
        endwid = dither->w / (picture->zoom+1);
        endhgt = dither->h / (picture->zoom+1);
    }


    y = 0;
    do
    {
        if(!(y%bh))
        {
            busybox(bl);
            bl += 12;
        }

        matline = matrix->line[y & 15];

        if(depth <= 8)
        {
            if(picture->format_type == FORM_STANDARD)
            {
                memset(pixbuf, 0x0, width);
                getpix_std_line(buffer, pixbuf, depth, planelengthr, width);
                buffer += w;
                line = pixbuf;
            }
            else
            {
                line = buffer;
                buffer += width;
            }

            x = 0;
            do
            {
                val = *line++;
                pal = palette + val + val + val;
                matpix = matline[x & 15];
                r = *pal++;
                ind = offset + (modr[r] > matpix ? divN36_1[r] : divN36[r]);
                g = *pal++;
                ind += modg[g] > matpix ? divN6_1[g] : divN6[g];
                b = *pal;
                ind += modb[b] > matpix ? divN[b] + 1 : divN[b];
                pixbuf[x] = plane_table[ind];

                line += skiph;
            } while(++x < endwid);

            buffer += skipv;
            buffer += skipBytes;
        }
        else
            if(depth == 24)
            {
                x = 0;
                do
                {
                    matpix = matline[x & 15];
                    r = *buffer++;
                    ind = offset + (modr[r] > matpix ? divN36_1[r] : divN36[r]);
                    g = *buffer++;
                    ind += modg[g] > matpix ? divN6_1[g] : divN6[g];
                    b = *buffer++;
                    ind += modb[b] > matpix ? divN[b] + 1 : divN[b];

                    pixbuf[x] = plane_table[ind];

                    buffer += skiph;
                } while(++x < endwid);

                buffer += correct;
                buffer += skipv;
                buffer += skipBytes;
            }
            else
            {
                x = 0;
                do
                {
                    val = *buffer16++;
                    matpix = matline[x & 15];
                    r = (val & 0xf800) >> 8;
                    ind = offset + (modr[r] > matpix ? divN36_1[r] : divN36[r]);
                    g = (val & 0x7e0) >> 3;
                    ind += modg[g] > matpix ? divN6_1[g] : divN6[g];
                    b = (val & 0x1f) << 3;
                    ind += modb[b] > matpix ? divN[b] + 1 : divN[b];
                    pixbuf[x] = plane_table[ind];

                    buffer16 += skiph;
                } while(++x < endwid);

                buffer16 += correct;
                buffer16 += skipv;
                buffer16 += skipBytes;
            }

            ziel += set16pixels(pixbuf, ziel, bplanes, planelengthw, endwid);   /* Pixel setzen und ziel weiter */
    } while(++y < endhgt);

/* wie schnell waren wir? */
/*  printf("%lu\n", get_timer());
    getch(); */

    Mfree(pixbuf);

    return;
} /* f_dither_orderedcol */



/* Erstellen der Tabellen */
char make_tabs(SMURF_PIC *picture, char *divN, char *divN6, char *divN6_1, char *divN36, char *divN36_1, int *modr, int *modg, int *modb)
{
    char r, g, b, rdiv, gdiv, bdiv, levels, offset;

    unsigned int i, index;


    switch(bplanes)
    {
        case 4: rdiv = 2; gdiv = 2; bdiv = 2; levels = 2; offset = 1; break;
        case 5: rdiv = 3; gdiv = 3; bdiv = 3; levels = 4; offset = 1; break;
        case 6: rdiv = 4; gdiv = 4; bdiv = 4; levels = 4; offset = 0; break;
        case 7: rdiv = 5; gdiv = 5; bdiv = 5; levels = 5; offset = 1; break;
        case 8: rdiv = 6; gdiv = 6; bdiv = 6; levels = 6; offset = 16; break;
    }

/* Palette mit festen FarbabstÑnden berechnen, und Bildpalette darauf setzen */
/* Anzahl: rdiv * gdiv * bdiv */
    for(r = 0; r < rdiv; r++)
        for(g = 0; g < gdiv; g++)
            for(b = 0; b < bdiv; b++)
            {
                index = offset + (r * levels * levels + g * levels + b);
                picture->red[index] = (char)((long)r * 255L / (rdiv - 1));
                picture->grn[index] = (char)((long)g * 255L / (gdiv - 1));
                picture->blu[index] = (char)((long)b * 255L / (bdiv - 1));
            }

    for(i = 0; i < 256; i++)
    {
    /*  Initialisieren Divisionstabellen */
    /*  divN[i] = i / (255 / (Ndiv - 1)) * Ndiv; */
        divN36[i] = (i * (rdiv - 1)) / 255 * levels * levels;
        divN36_1[i] = ((i * (rdiv - 1)) / 255 + 1) * levels * levels;
        divN6[i] = (i * (gdiv - 1)) / 255 * levels;
        divN6_1[i] = ((i * (gdiv - 1)) / 255 + 1) * levels;
        divN[i] = (i * (bdiv - 1)) / 255;
    /*  Initialisieren Modulotabellen */
    /*  modN[i] = i % (255 / (Ndiv - 1)) * (Ndiv - 1); */
        modr[i] = (i * (rdiv - 1)) % 255;
        modg[i] = (i * (gdiv - 1)) % 255;
        modb[i] = (i * (bdiv - 1)) % 255;
    }

    return(offset);
} /* make_tabs */

/*
                    if(*buffer++ > matpix)
                        ind |= 0x04;
                    if(*buffer++ > matpix)
                        ind |= 0x02;
                    if(*buffer++ > matpix)
                        ind |= 0x01;
                    pixbuf[out] = plane_table[convtab[ind]];
*/

/* Erstellen der Paletten fÅr 1 und 4 Bit */
/* und Umrechnungstabelle */
void make_pal(SMURF_PIC *picture, char *convtab)
{
    picture->red[0] = 255;
    picture->grn[0] = 255;
    picture->blu[0] = 255;
    convtab[7] = 0;

    picture->red[1] = 0;
    picture->grn[1] = 0;
    picture->blu[1] = 0;
    convtab[0] = 1;

    picture->red[2] = 255;
    picture->grn[2] = 255;
    picture->blu[2] = 0;
    convtab[6] = 2;

    picture->red[3] = 255;
    picture->grn[3] = 0;
    picture->blu[3] = 255;
    convtab[5] = 3;

    picture->red[4] = 0;
    picture->grn[4] = 255;
    picture->blu[4] = 255;
    convtab[3] = 4;

    picture->red[5] = 0;
    picture->grn[5] = 0;
    picture->blu[5] = 255;
    convtab[1] = 5;

    picture->red[6] = 255;
    picture->grn[6] = 0;
    picture->blu[6] = 0;
    convtab[4] = 6;

    picture->red[7] = 0;
    picture->grn[7] = 255;
    picture->blu[7] = 0;
    convtab[2] = 7;

    return;
} /* make_pal */
