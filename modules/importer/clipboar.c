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
MOD_INFO    module_info={"WinClip-Importer",
						0x0050,
                        "Dale Russell",
                        "CLP","","","","","","","","","",
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
/*      Windows Clipboard Dekomprimierer (CLP)      */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void imp_module_main(GARGAMEL *smurf_struct)
{
char    *buffer;
unsigned int v1,v2;
unsigned long v3, DatenOffset;

buffer=smurf_struct->smurf_pic->pic_data;
if (*(buffer)!=0x50 || *(buffer+1)!=0xc3)
        return(M_INVALID);
        
else {
    v1=*(buffer+2)+(*(buffer+3)<<8);
        /*printf("  Anzahl Datenbereiche...%u\n",v1);*/
    v1=*(buffer+4)+(*(buffer+5)<<8);
        /*printf("  1.Formaterkennung...%u\n",v1);*/
    v3=*(buffer+6)+(*(buffer+7)<<8)+(*(buffer+8)<<16)
                    +(*(buffer+9)<<24);
    /*printf("  L„nge des Bereiches...%lu\n",v3);*/
    DatenOffset=*(buffer+10)+(*(buffer+11)<<8)+(*(buffer+12)<<16)
                    +(*(buffer+13)<<24);
    /*printf("  Offset zum n„chsten Bereiches...%lu\n",DatenOffset);
    getch();*/
    
    strncpy(smurf_struct->smurf_pic->format_name, "Windows Clipboard CLP", 21);
    smurf_struct->smurf_pic->pic_width=/*width*/320;
    smurf_struct->smurf_pic->pic_height=/*height*/200;
    smurf_struct->smurf_pic->depth=8;
    smurf_struct->smurf_pic->bp_pal=24;
    smurf_struct->smurf_pic->pic_data=buffer+DatenOffset;
    smurf_struct->smurf_pic->palette=buffer;
    return(M_PICDONE);
    }

}
