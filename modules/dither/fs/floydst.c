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
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

/* Set 16 Pixel (Standard Format) Assembler-Rout */
    int (*set_16_pixels)(char *source, char *dest, int depth, long planelen, int howmany);          
/* Get 16 Pixel (Standard Format) Assembler-Rout */
    void get_standard_pix(void *st_pic, void *buf16, int planes, long planelen, int zoom);


int floyd_steinberg68000(SMURF_PIC *picture, DITHER_DATA *dither, char *output);
int floyd_steinberg68030(SMURF_PIC *picture, DITHER_DATA *dither, char *output);

SERVICE_FUNCTIONS *service;



DITHER_MOD_INFO module_info =
{
    "Floyd-Steinberg",
    "Olaf Piesche\0",
    0x0100,                 /* Schlumpfine-Version */
    0,                      /* Konfigurierbar? */
    ALLPAL                  /* Palettenmodi */
};

SERVICE_FUNCTIONS *service;

int (*seek_nearest_col)(long *par, int maxcol);
int not_in_nct;


/*  --------------------------------------------------------------  */
/*  --------------------- 8-24-Bit Bild -------------------------   */
/*  --------------------------------------------------------------  */
int dither_module_main(DITHER_DATA *dither)
{
    int back;

    service=dither->services;
    
    set_16_pixels = dither->set_16pixels;
    not_in_nct = dither->not_in_nct;

    seek_nearest_col = service->seek_nearest_col;

    back = floyd_steinberg68000(dither->picture, dither, dither->dest_mem);
    
    return back;
}
