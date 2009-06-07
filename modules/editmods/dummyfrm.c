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
 * The Initial Developer of the Original Code is
 * Christian Eyrich
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* =========================================================*/
/*						Dummy-Modul							*/
/* Version 0.1  --  irgendwann 1996							*/
/*	  welche Farbtiefe gerade gebraucht wird				*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\import.h"
#include "..\..\..\src\smurfine.h"
#include "..\..\..\src\lib\demolib.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

MOD_INFO module_info = {"Dummy mit Formular",
						0x0010,
						"Christian Eyrich",
						"", "", "", "", "",
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
						0,64,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						24, 0, 0, 0, 0,
						0, 0, 0,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*			Dummy mit Einstellformular				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *ziel, *oziel, *buffer, *obuffer;

	int module_id;
	unsigned int x, y, width, height, nwidth, nheight;

	long v;


	module_id = smurf_struct->module_number;

	switch(smurf_struct->module_mode)
	{
		case MSTART:	smurf_struct->services->f_module_prefs(&module_info, module_id);
						smurf_struct->module_mode = M_WAITING;
						break;
		case MEXEC:		SMalloc = smurf_struct->services->SMalloc;
						SMfree = smurf_struct->services->SMfree;

						width = smurf_struct->smurf_pic->pic_width;
						height = smurf_struct->smurf_pic->pic_height;
						obuffer = buffer = smurf_struct->smurf_pic->pic_data;

						nwidth = width * 2;
						nheight = height * 2;

						oziel = ziel = (char *)SMalloc((long)nwidth * (long)nheight * 3L);

						v = (long)(nwidth - width) * 3L;

						y = 0;
						do
						{
							x = 0;
							do
							{
								*ziel++ = *buffer++;
								*ziel++ = *buffer++;
								*ziel++ = *buffer++;
							} while(++x < width);

							ziel += v;
						} while(++y < height);

						SMfree(obuffer);
						smurf_struct->smurf_pic->pic_data = oziel;

						smurf_struct->smurf_pic->pic_width = nwidth;
						smurf_struct->smurf_pic->pic_height = nheight;

						smurf_struct->module_mode = M_PICDONE;
						break;
		case MTERM:		smurf_struct->module_mode = M_EXIT;
						break;
	}

	return;
}