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
#include <stddef.h>
#include <vdi.h>
#include <string.h>
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\smurf\smurfine.h"

void    floyd1624(long *par);               /* 24&16-Bit " " mit SysPal */
int (*set_16pixels)(char *buf16, char *dest, int depth, long planelen, int howmany);
int (*seek_nearest_col)(long *par, int maxcol);


DITHER_MOD_INFO module_info =
{
    "Fast Diffusion\0",
    "Olaf Piesche\0      ",
     0x0100,                    /* Schlumpfine-Version */
    0,                          /* Konfigurierbar? */
    ALLPAL                      /* Palettenmodi */
};



/*  --------------------------------------------------------------  */
/*  --------------------- 8-24-Bit Bild -------------------------   */
/*  --------------------------------------------------------------  */
int dither_module_main(DITHER_DATA *dither)
{
char    *pix, *pic, *nct;
int     rgbtab[2048];
long    par[23];
long    planelen, src_planelen;
int     width, height;
int     endwid, endhgt;
int     src_depth, bplanes, t, idx;
char    *plantab, *where_to;
int     cliptable[100]=
        {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31
        };

int     *red, *grn, *blu;
int not_in_nct;
SMURF_PIC *picture;
int destwid, desthgt;


    picture=dither->picture;
    
    width=picture->pic_width;
    height=picture->pic_height;
    src_depth=picture->depth;
    bplanes=dither->dst_depth;
    red=dither->red;
    grn=dither->green;
    blu=dither->blue;
    plantab=dither->planetable;
    nct=dither->nc_tab;
    where_to=dither->dest_mem;
    
    set_16pixels=dither->set_16pixels;

    seek_nearest_col = dither->services->seek_nearest_col;
    not_in_nct = dither->not_in_nct;
    
    /*
    *   Farbtabelle(15 Bit) initialisieren 
    */
    idx=0;
    for(t=0; t<256; t++)
    {
        rgbtab[idx] = blu[t];
        rgbtab[idx+1] = grn[t];
        rgbtab[idx+2] = red[t];
        idx+=4;
    }
    
    /*
    *   Variablenvorbereitung
    */
    pix = malloc(width+16);
    if(pix==NULL) 
        return(M_MEMORY);
    
    endwid = width/(dither->zoom+1);
    endhgt = height/(dither->zoom+1);
    pic = picture->pic_data;
    planelen=dither->dest_planelength;

    if(dither->w==0) destwid = width;
    else destwid = dither->w;
    if(dither->h==0) desthgt = height;
    else desthgt = dither->h;

    /*  Source-Zeiger auf den Ausschnittsbeginn ausrichten
     */
    if(picture->format_type==FORM_PIXELPAK)
        pic += ((long)dither->x*(long)(src_depth/8))+((long)dither->y*(long)width*(long)(src_depth/8));
    else
        pic += (dither->x/8)+((long)dither->y*(long)((width+7)/8));

    src_planelen=0L;
    if(picture->format_type==FORM_STANDARD)
        src_planelen=(long)(((long)(width+7)>>3)*(long)height);

        par[0] = (long)&cliptable[31];
        par[1] = (long)nct;
        par[2] = (long)plantab;
        par[3] = (long)pix;
        par[4] = (long)rgbtab;
        par[5] = (long)planelen;
        par[6] = (long)where_to;
        par[7] = (long)width;
        par[8] = (long)height;
        par[9] = (long)bplanes;
        par[10] = (long)dither->services->busybox;
        par[11] = (long)src_depth;
        par[12] = (long)picture->palette;
        par[13] = (long)src_planelen;
        par[14] = (long)picture->zoom;
        par[15] = (long)set_16pixels;
        par[16] = (long)not_in_nct;
        par[17] = (long)red;
        par[18] = (long)grn;
        par[19] = (long)blu;
        par[20] = (long)destwid;
        par[21] = (long)desthgt;
        par[22] = (long)pic;

        dither->services->reset_busybox(0, "FDiff Dither");
        floyd1624(par);

        free(pix);

        return(M_PICDONE);
}
