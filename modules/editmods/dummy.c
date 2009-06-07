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

MOD_INFO module_info = {"Dummy",
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
						2,
						"Zielbild",
						"Quellbild"
						};


MOD_ABILITY module_ability = {
						8, 0, 0, 0, 0,
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
/*						Dummy						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	int module_id;


	module_id = smurf_struct->module_number;

	switch(smurf_struct->module_mode)
	{
		case MSTART:	smurf_struct->services->f_module_prefs(&module_info, module_id);
						smurf_struct->module_mode = M_WAITING;
						break;
		case MPICS:		if(smurf_struct->event_par[0] == 0)
						{
							smurf_struct->event_par[0] = smurf_struct->smurf_pic->depth;
							smurf_struct->event_par[1] = smurf_struct->smurf_pic->format_type;
							smurf_struct->event_par[2] = smurf_struct->smurf_pic->col_format;
						}
						else
						{
							smurf_struct->event_par[0] = 8;
							smurf_struct->event_par[1] = FORM_PIXELPAK;
							smurf_struct->event_par[2] = GREY;
						}
						smurf_struct->module_mode = M_PICTURE;
						break;
		case MEXEC:		smurf_struct->event_par[0] = 0;
						smurf_struct->module_mode = M_PICDONE;
						break;
		case MTERM:		smurf_struct->module_mode = M_EXIT;
						break;
	}

	return;
}