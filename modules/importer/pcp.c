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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define TRUE        1
#define FALSE       0
#define DEBUG       0
#define W_PIX       255
#define B_PIX       0

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Atari Grafik Format",
                        0x0010,
                        "Dale Russell",
                        "PCP", "", "", "", "",
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
/*  Atari Bitmap Format                             */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char *buffer=smurf_struct->smurf_pic->pic_data;
int width=0, height=0;
long len=0;
char *filenam, *retbuf;
char extend[4];

/*****************************************************/
/*          MAGIC Code berprfen                    */
/*****************************************************/
filenam=smurf_struct->smurf_pic->filename;
strncpy(extend, filenam+(strlen(filenam)-3), 3);
if(strncmp(extend, "PCP", 3)!=0) 
    return M_INVALID;

smurf_struct->services->reset_busybox(128, "Atari Grafik 1 Bit");

width=(*(buffer)<<8)+*(buffer+1);
height=(*(buffer+2)<<8)+*(buffer+3);
len=(long)((width+7)/8)*(long)height;
/*****************************************************/
/*  Smurf-Strukturen fllen                          */
/*****************************************************/
strncpy(smurf_struct->smurf_pic->format_name, "Atari Grafik .PCP    ", 21);
smurf_struct->smurf_pic->format_type=FORM_STANDARD;
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->depth=1;
smurf_struct->smurf_pic->col_format=WURSCHT;
retbuf=Malloc(len);
memcpy(retbuf, buffer+6, len);
Mfree(smurf_struct->smurf_pic->pic_data);
smurf_struct->smurf_pic->pic_data=retbuf;

return(M_PICDONE);
}
