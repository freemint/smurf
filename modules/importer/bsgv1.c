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
#include "..\..\smurf\smurfine.h"

#define NOMEM   ( (void*) 0L )
#define DEBUG           0
#define TRUE            1
#define FALSE           !TRUE

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"BSG-Importer",
						0x0010,
                        "Dale Russell",
                        "BSG", "", "", "", "",
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
/*      BSG Fontasy Monochrom Format                */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer, *retbuf;
int width=0, height=0, bperz=0;
long check,len;

/***************************************************************/
/*      Kopfdaten analysieren                                  */
/***************************************************************/
buffer=smurf_struct->smurf_pic->pic_data;
check=smurf_struct->smurf_pic->file_len;
check-=4;
width=*(buffer)+(*(buffer+1)<<8);
height=*(buffer+2)+(*(buffer+3)<<8);
bperz=(width+7)/8;
if ( (long)bperz*(long)height != check ) return(M_INVALID);

smurf_struct->services->reset_busybox(128, "BSG Fontasy 1 Bit");

len=(long)((width+7)/8)*(long)height;
strncpy(smurf_struct->smurf_pic->format_name, "Fontasy Bitmap (BSG) ", 21);
/***************************************************************/
/*      Strukturen fllen                                      */
/***************************************************************/
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->depth=1;
smurf_struct->smurf_pic->bp_pal=0;
retbuf=Malloc(len);
memcpy(retbuf, buffer+4, len);
Mfree(buffer);
smurf_struct->smurf_pic->pic_data=retbuf;
smurf_struct->smurf_pic->format_type=FORM_STANDARD;
smurf_struct->smurf_pic->col_format=WURSCHT;
return(M_PICDONE);      /* Alles Klar. */
}

