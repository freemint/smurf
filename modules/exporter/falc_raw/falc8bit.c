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

/* =========================================================*/
/*                  Falcon-Raw-Format Encoder               */
/* Version 0.1  --  25.05.96                                */
/*  Speichert ein 256-Farben-Bild im Falcon-Screenformat ab */
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

/* Infostruktur fr Hauptmodul */
MOD_INFO    module_info = {"8Bit PP Raw",
                        0x0010,
                        "Olaf Piesche",
                        "RA8", "", "", "", "",
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
                        0,0,0,0
                        };


MOD_ABILITY  module_ability = {
                        8, 0, 0, 0, 0,
                        0, 0, 0,
                        FORM_BOTH,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        0
                        };


/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*              8 Bit, unkomprimiert                */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
    EXPORT_PIC *exp_pic;
    char *palette;
    char *buffer, *ziel;
    int *intbuf;
    long f_len;
    int width, height, depth;
    long headsize;
    int plane, x,y, pix;
    int pixel[16], onepixel, xpix;
    int andval;
    
    buffer  =   smurf_struct->smurf_pic->pic_data;
    width   =   smurf_struct->smurf_pic->pic_width;
    height  =   smurf_struct->smurf_pic->pic_height;
    depth   =   smurf_struct->smurf_pic->depth;
    palette =   smurf_struct->smurf_pic->palette;
    
    exp_pic=Malloc(sizeof(EXPORT_PIC));

    ziel=Malloc((long)width*(long)height + 256L*3L);
    if( ! ziel)
    {
        smurf_struct->module_mode=M_MEMORY; 
        return(exp_pic);
    }

/*-------------------------- Encoding -----------------------*/

        memcpy(ziel, palette, 256L*3L);
        memcpy(ziel+256L*3L, buffer, (long)width*(long)height);

        exp_pic->pic_data=ziel;
        exp_pic->f_len=(long)width*(long)height+256L*3L;

        smurf_struct->module_mode=M_DONEEXIT;
        return(exp_pic);

}
