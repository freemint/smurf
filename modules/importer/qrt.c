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
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"QRT .RAW Modul",
                        0x0010,
                        "Dale Russell",
                        "RAW", "", "", "", "",
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
/*      raw qrt Modul                               */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char *extend, ext[4], *retbuf;
char    *buffer;
int width=0, height=0, BitsPerPixel, v1;
long filelen=smurf_struct->smurf_pic->file_len;
long vergleich, len;

extend=smurf_struct->smurf_pic->filename;
strncpy(ext, extend+(strlen(extend)-3), 3);

buffer=smurf_struct->smurf_pic->pic_data;
width=*(buffer)+(*(buffer+1)<<8);
height=*(buffer+2)+(*(buffer+3)<<8);
len= (long)width *(long)height *3l;

smurf_struct->services->reset_busybox(128, "QRT Raw 24 Bit");

if(strncmp(ext, "RAW", 3)==0)
{
    if ( len != (filelen-4) )
        return(M_INVALID);
}
else        return(M_INVALID);

retbuf=Malloc( len );
memcpy(retbuf, buffer+4, len);
Mfree(buffer);
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->pic_data=retbuf;
smurf_struct->smurf_pic->depth=24;
smurf_struct->smurf_pic->format_type=0;
smurf_struct->smurf_pic->col_format=RGB;
strncpy(smurf_struct->smurf_pic->format_name, "QRT Ray Tracer    ", 21);
return(M_PICDONE);
}
