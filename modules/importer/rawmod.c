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


/* Infostruktur fr Hauptmodul */
MOD_INFO    module_info={".RAW Modul",
                        0x0010,
                        "Dale Russell",
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
                        0,10
                        };



/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*      raw hsi Dekomprimierer                      */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer, *Daten;
int width=0, height=0, BitsPerPixel, colors;
buffer=smurf_struct->smurf_pic->pic_data;
if (strncmp(buffer,"mhwanh",6)!=0)
    form_alert(1,"[1][ Kein RAW Format ]\[ Stop ]");
else {
    width=(*(buffer+8)<<8)+*(buffer+9);
    height=(*(buffer+8)<<10)+*(buffer+11);
    colors=(*(buffer+12)<<10)+*(buffer+13);
    if (colors==0 || colors>255) 
    {   BitsPerPixel=24; Daten=buffer+0x20; }
    else {  BitsPerPixel=8;
    smurf_struct->smurf_pic->palette=buffer+0x20;
    smurf_struct->smurf_pic->bp_pal=24;
    Daten=buffer+0x20+(colors+1)*3; }
    smurf_struct->smurf_pic->pic_width=width;
    smurf_struct->smurf_pic->pic_height=height;
    smurf_struct->smurf_pic->pic_data=Daten;
    smurf_struct->smurf_pic->depth=BitsPerPixel;
    strncpy(smurf_struct->smurf_pic->format_name, "RAW-Hsi File Format  ", 21);
}
}
