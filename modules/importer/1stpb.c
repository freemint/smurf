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

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\smurf\smurfine.h"

#define TRUE        1
#define FALSE       0
#define DEBUG       0
#define W_PIX       255
#define B_PIX       0

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"First Publisher",
						0x0050,
                        "Dale Russell",
                        "ART", "", "", "", "",
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
/*  First Publisher Bitmap Format                   */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char *buffer=smurf_struct->smurf_pic->pic_data;
char *retbuf;
int width, height;
long len;
/*****************************************************/
/*          MAGIC Code ÅberprÅfen                    */
/*****************************************************/
width=*(buffer+2)+(*(buffer+3)<<8);
height=*(buffer+6)+(*(buffer+7)<<8);
len=(long)((width+7)/8) * (long)height;
if ( (len+8) !=smurf_struct->smurf_pic->file_len ) return (M_INVALID);

strncpy(smurf_struct->smurf_pic->format_name, "First Publisher .ART", 21);
smurf_struct->smurf_pic->format_type=1;
smurf_struct->smurf_pic->depth=1;
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;

smurf_struct->services->reset_busybox(128, "First Publisher 1Bit");

retbuf=Malloc(len);
memcpy(retbuf, buffer+8, len); 
Mfree(buffer);
smurf_struct->smurf_pic->pic_data=retbuf;
return(M_PICDONE);
}
