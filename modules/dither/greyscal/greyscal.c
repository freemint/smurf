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
#include <stdlib.h>
#include <stdio.h>
#include <vdi.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\smurf\smurfine.h"

void convert_to_grey(long *par);


/* Set 16 Pixel (Standard Format) Assembler-Rout */
void setpix162(char *source, char *paddr, int num_plane, long bitplanelen);         
/* Get 16 Pixel (Standard Format) Assembler-Rout */
void get_standard_pix(void *st_pic, void *buf16, int planes, long planelen);
void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);

int (*set16pixels)(char *buf16, char *dest, int depth, long planelen, int howmany);

int (*busybox)(int pos);

long planelengthw;

DITHER_MOD_INFO module_info =
{
    "Graustufen",
    "Christian Eyrich",
    0x0100,                     /* Schlumpfine-Version */
    0,                          /* konfigurierbar? */
    FIXPAL                      /* Palettenmodus */
};


/*--------------------------------------------------*/
/*--------------------------------------------------*/
/*              Graustufenanzeige                   */
/*      Hingeschmiert von Christian Eyrich 10/96    */
/*--------------------------------------------------*/
/*--------------------------------------------------*/
int dither_module_main(DITHER_DATA *dither)
{
    char *ziel; 
    char *plane_table, *palette;
    char *buffer, *pal,
         *pixbuf, out, stufen, grenze, greytab[256], stepwidth,
         bplanes, i, j, depth, skiph, correct;
    
    int *red, *green, *blue;
    unsigned int *buffer16,
                 x, y, width, height, k, bh, bl, val, zoom, endwid, endhgt;
    
    long rval, gval, bval, skipBytes;
    unsigned long w, planelengthr, skipv;
    
    SMURF_PIC *pic_todit;



    set16pixels = dither->set_16pixels;

    planelengthw = dither->dest_planelength;

    busybox = dither->services->busybox;
    
    dither->services->reset_busybox(0, "Graustufen...");

/* Struktur auslesen */
    pic_todit = dither->picture;
    ziel = dither->dest_mem;
    bplanes = (char)dither->dst_depth;
    plane_table = dither->planetable;
    palette = pic_todit->palette;
    zoom = dither->zoom;

/* Palette anpassen */
    red = pic_todit->red;
    green = pic_todit->grn;
    blue = pic_todit->blu;

    if(bplanes == 8)
    {
        k = 0;
        do
        {
            red[k] = green[k] = blue[k] = (char)k;
        } while(++k < 256);
    }
    else
    {
        stufen = 1 << bplanes;
        stepwidth = 256 / (stufen - 1);

        j = 0;
        i = 0;
        do
        {
            red[i] = green[i] = blue[i] = j;
            j += stepwidth;
        } while(++i < stufen - 1);
        do
        {
            red[i] = green[i] = blue[i] = 255;
        } while(++i < stufen);
    }

/* Bild anpassen */
    if(bplanes == 8)
    {
        k = 0;
        do
        {
            greytab[k] = plane_table[k];
        } while(++k < 256);
    }
    else
    {
        stufen = 1 << bplanes;
        stepwidth = 256 / (stufen - 1);
        grenze = 256 / stufen;

        k = 0;
        i = 0;
        do
        {
            j = 0;
            do
            {
                greytab[k++] = plane_table[i];
            } while(++j < grenze);
        } while(++i < stufen);
    }


    width = pic_todit->pic_width;
    height = pic_todit->pic_height;
    
    depth = pic_todit->depth;

    /*
     * Zoom-inx ausrechnen 
     */
    endwid = width / (zoom + 1);
    endhgt = height / (zoom + 1);

    pixbuf = malloc(width+128);

    if(depth == 24)
    {
        /* Nach jedem Pixel berspringen */
        skiph = zoom * 3;
        skipv = (unsigned long)zoom * (unsigned long)pic_todit->pic_width * 3L;
        /* Unter den Tisch gefallene Bytes nachholen */
        correct = (width - endwid * (zoom + 1)) * 3;
    }
    else
    {
        /* nach jedem Pixel berspringen */
        skiph = zoom;
        if(pic_todit->format_type == FORM_PIXELPAK)
            skipv = (unsigned long)zoom * (unsigned long)pic_todit->pic_width;
        else
            skipv = (unsigned long)zoom * (unsigned long)((pic_todit->pic_width+7)>>3);

        /* unter den Tisch gefallene Bytes nachholen */
        correct = width - endwid * (zoom + 1);
    }


    buffer16 = (unsigned int *)pic_todit->pic_data;
    buffer = pic_todit->pic_data;


    /*  Source-Skipbytes (fr Ausschnitte) berechnen
     */
    if(dither->w==0)
        skipBytes=0;
    else
    {
        if(pic_todit->format_type==FORM_PIXELPAK)
            skipBytes = (width-dither->w)*(depth/8);
        else
            skipBytes = ((width-dither->w+7)>>3);
    }

    /*  Source-Zeiger auf den Ausschnittsbeginn ausrichten
     */
    if(pic_todit->format_type==FORM_PIXELPAK)
    {
        buffer += ((long)dither->x*(long)(depth/8))+((long)dither->y*(long)pic_todit->pic_width*(long)(depth/8));
        buffer16 += ((long)dither->x*(long)(depth/8))+((long)dither->y*(long)pic_todit->pic_width*(long)(depth/8));
    }
    else
        buffer += (dither->x/8)+((long)dither->y*(long)((width+7)/8));

    if(dither->w!=0)
    {
        endwid = dither->w / (zoom+1);
        endhgt = dither->h / (zoom+1);
    }

/* Planel„nge beim Lesen */
    w = ((width + 7) / 8);
    planelengthr = w * (unsigned long)pic_todit->pic_height;   /* L„nge einer Plane in Bytes */

    bh = pic_todit->pic_height / 10;               /* busy-height */
    bl = 0;                         /* busy-length */

    if(pic_todit->format_type == FORM_PIXELPAK)
    {
    /*---- pixel packed Bilder ----*/
        y = 0;
        do
        {
            if(!(y%bh))
            {
                busybox(bl);
                bl += 12;
            }
    
            x = 0;
            do
            {
                if(depth == 24)
                {
                    out = 0;
                    do
                    {
                        rval = *buffer++ * 871L;
                        gval = *buffer++ * 2929L;
                        bval = *buffer++ * 295L;
                        pixbuf[out] = greytab[(rval + gval + bval) >> 12];
                        x++;

                        buffer += skiph;
                    } while(++out < 16 && x < endwid);
                }
                else
                    if(depth == 16)
                    {
                        out = 0;
                        do
                        {
                            val = *buffer16++;
                            rval = ((val & 0xf800) >> 8) * 871L;
                            gval = ((val & 0x7e0) >> 3) * 2929L;
                            bval = ((val & 0x1f) << 3) * 295L;
                            pixbuf[out] = greytab[(rval + gval + bval) >> 12];
                            x++;

                            buffer16 += skiph;
                        } while(++out < 16 && x < endwid);
                    }
                    else
                    {
                        out = 0;
                        do
                        {
                            val = *buffer++;
                            pal = palette + val + val + val;
                            rval = *pal++ * 871L;
                            gval = *pal++ * 2929L;
                            bval = *pal++ * 295L;
                            pixbuf[out] = greytab[(rval + gval + bval) >> 12];
                            x++;

                            buffer += skiph;
                        } while(++out < 16 && x < endwid);
                    }

                out = set16pixels(pixbuf, ziel, bplanes, planelengthw, out);    /* Pixel setzen ... */
                ziel += out;                                            /* ziel weiter */
            } while(x < endwid);
            buffer += correct;
            buffer += skipBytes;
            buffer += skipv;
            buffer16 += correct;
            buffer16 += skipv;
            buffer16 += skipBytes;
        } while(++y < endhgt);
    }
    else
    {
    /*---- Standardformatbilder ----*/
        y = 0;
        do
        {
            if(!(y%bh)){
                busybox(bl);
                bl += 12;
            }
            memset(pixbuf, 0x0, width);
            getpix_std_line(buffer, pixbuf, depth, planelengthr, width);
            
            out = 0;
            for(x=0; x<endwid; x++)
            {
                val = pixbuf[out];
                pal = palette + val + val + val;
                pixbuf[x] = greytab[(((long)*pal * 871L)
                              + ((long)*(pal + 1) * 2929L)
                              + ((long)*(pal + 2) * 295L)) >> 12];
                out += skiph+1; /**/
            }
            
            out = set16pixels(pixbuf, ziel, bplanes, planelengthw, endwid);     /* Pixel setzen ... */
            ziel += /*out*/endwid;                                                  /* ziel weiter */
            
            buffer += (width + 7) >> 3;                                     /* buffer weiter setzen */
            buffer += skipBytes;
            buffer += skipv;
        } while(++y < height);
    } /* Standardformat */

    free(pixbuf);

    return(M_PICDONE);
} /* f_dither_greyscale */
