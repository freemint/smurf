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
/*					s/w Schwellwert-Modul					*/
/*	Sonderversion um 16 Farben Snapshots verlustfrei nach	*/
/*	2 Farben zu bringen.									*/
/* Version 0.1  --  09.10.97								*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include <..\..\demolib.h>

char box(char *data);

MOD_INFO module_info = {"s/w Schwellwert special",
						0x0010,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"Schwellwert",
						"",
						"",
						"",
						"Modus 1",
						"Modus 2",
						"",
						"",
						"",
						"",
						"",
						"",
						0,255,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						150, 0, 0, 0,
						2, 3, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY  module_ability = {
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
/*	Sonderversion um 16 Farben Snapshots verlustfrei*/
/*	nach 2 Faren zu bringen.						*/
/*		1-8, 16 und 24 Bit							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data, *pal,
		 modus;
	
	int module_id;
	unsigned int width, height;
	
	unsigned long i, length, grenze;


	module_id = smurf_struct->module_number;

	/* Wenn das Modul zum ersten Mal gestartet wurde, */
	/* ein Einstellformular anfordern....             */
	if(smurf_struct->module_mode == MSTART)
	{
		smurf_struct->services->f_module_prefs(&module_info, module_id);
		smurf_struct->module_mode = M_WAITING;
		return;
	}
	/* Einstellformular wurde mit START verlassen - Funktion ausfhren */
	else
		if(smurf_struct->module_mode == MEXEC)
		{
/* wie schnell sind wir? */
/*	init_timer(); */

			grenze = (unsigned long)(smurf_struct->slide1 << 12);

			if(smurf_struct->check1 > smurf_struct->check2)
				modus = 1;
			else
				modus = 2;

			pal = smurf_struct->smurf_pic->palette;

			data = smurf_struct->smurf_pic->pic_data;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			length = (unsigned long)width * (unsigned long)height;

			data += width;			/* erste */
			i = width;				/* und letzte Zeile auslassen */
			if(modus == 1)
				do
				{
					if(*data == 0x0 && *(data - width) != 0x0 && *(data - width) == *(data + width) ||
					   *data == 0x0 && *(data - 1) != 0x0 && *(data - 1) == *(data + 1))
						*data++ = 0x01;
					else
						data++;
				} while(++i < length);
			else
				do
				{
					if(*data == 0x0 && *(data - width) != 0x0 && *(data + width) != 0x0 ||
					   *data == 0x0 && *(data - 1) != 0x0 && *(data + 1) != 0x0)
						*data++ = 0x01;
					else
						data++;
				} while(++i < length);

			pal = smurf_struct->smurf_pic->palette;
			data = pal;
			length = 254L;

			i = 2;					/* und die ersten beiden Eintr„ge auslassen */
			do
			{
				if(((unsigned long)*data * 872L +
				 	(unsigned long)*(data + 1) * 2930L +
				 	(unsigned long)*(data + 2) * 296L) < grenze)
				{
					*data++ = 0x0;
					*data++ = 0x0;
					*data++ = 0x0;
				}
				else
				{
					*data++ = 0xff;
					*data++ = 0xff;
					*data++ = 0xff;
				}
			} while(++i < length);

/* wie schnell waren wir? */
/*	printf("\n%lu\n", get_timer());
	getch(); */
		
			smurf_struct->module_mode = M_PICDONE;
			return;
		}
		/* Mterm empfangen - Speicher freigeben und beenden */
		else 
			if(smurf_struct->module_mode == MTERM)
			{
				smurf_struct->module_mode = M_EXIT;
				return;
			}
}