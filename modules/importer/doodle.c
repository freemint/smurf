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

#define DEBUG           0
#define TRUE            1
#define FALSE           !TRUE

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Doodle-Importer",
						0x0050,
                        "Dale Russell",
                        "DOO", "", "", "", "",
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
/*      Atari Doodle                                */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer;
int width=0, height=0;
char ext[4]="";
char *extend;

/***************************************************************/
/*      Kopfdaten analysieren                                  */
/***************************************************************/
buffer=smurf_struct->smurf_pic->pic_data;

/*********** ERKENNUNG .DOO *****************/
extend=smurf_struct->smurf_pic->filename;
strncpy(ext, extend+(strlen(extend)-3), 3);
if(strncmp(ext, "DOO", 3)!=0)   return(M_INVALID);

smurf_struct->services->reset_busybox(128, "Doodle 1 Bit");

strncpy(smurf_struct->smurf_pic->format_name, "Atari Doodle Format  ", 21);
width=640;
height=400;

/***************************************************************/
/*      Strukturen fllen                                      */
/***************************************************************/
smurf_struct->smurf_pic->pic_width=width;
smurf_struct->smurf_pic->pic_height=height;
smurf_struct->smurf_pic->depth=1;
smurf_struct->smurf_pic->bp_pal=0;
smurf_struct->smurf_pic->pic_data=buffer;
smurf_struct->smurf_pic->format_type=1;
return(M_PICDONE);
}

