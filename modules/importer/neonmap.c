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

/* ******************************************** */
/*                                              */
/*                                              */
/* ******************************************** */


#include <tos.h>
#include <stdio.h>
#include <string.h>
#include <ext.h>

#include "..\import.h"
#include "..\..\smurf\smurfine.h"


MOD_INFO module_info=
{
    "NeoN-Mapfile Import",
    0x0010,
    "Olaf Piesche",
/* Extensionen */
    "MAP","","","","","","","","","",

/* Slider */
    "","","","",
/* Editfelder */
    "","","","",
/* Checkboxen */
    "","","","",

/* Minima + Maxima */
/* Slider */
    0,0,
    0,0,
    0,0,
    0,0,
/* Edits */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaults */
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    
    0,
    "","","","","",""
};






int imp_module_main(GARGAMEL *smurf_struct)
{
int *picdata, *dest;
char *pd, *pal;
int width, height, t;

if(smurf_struct->module_mode==MSTART)       /* geht's los, geht's los? */
{
    pd=picdata=smurf_struct->smurf_pic->pic_data;
    pal=smurf_struct->smurf_pic->palette;

     /* Kennung prÅfen */
    if(strncmp(picdata, "Rip-Mapfile:", 12)!=0) return(M_INVALID);
        
    width=*(pd+18);
    height=*(pd+20);
    
    dest=Malloc((long)width*(long)height);
    memcpy(dest, pd+24, (long)width*(long)height);

    Mfree(picdata);
    
    for(t=0; t<256; t++)
    {
        *( pal + (t*3)) = (char)t;
        *( pal + (t*3)+1) = (char)t;
        *( pal + (t*3)+2) = (char)t;
    }



    smurf_struct->smurf_pic->pic_data=dest;
    smurf_struct->smurf_pic->depth=8;

    strncpy(smurf_struct->smurf_pic->format_name, "NeoN-Mapfile        ", 21);
    smurf_struct->smurf_pic->pic_width=width;
    smurf_struct->smurf_pic->pic_height=height;

    return(M_PICDONE);  /* das wars. */
}

}
