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
/*						Invertier-Modul						*/
/* Version 0.1  --  30.12.95								*/
/*	  24 Bit												*/
/* Version 0.2  --  07.03.96								*/
/*	  Palettenbilder										*/
/* Version 0.3  --  16.03.96								*/
/*	  16 Bit												*/
/* Version 0.4  --  18.05.96								*/
/*	  Nun auch Invertierung von sw-Bildern (die haben		*/
/*	  n„mlich keine Palette mužt Du wissen)					*/
/* Version 0.5  --  18.05.96								*/
/*	  Tja, da hatte ich wohl beim Einbau des tollen			*/
/*	  Features in die 0.4 einen bl”den Fehler gemacht. Es	*/
/*	  wurden keine Palettenbilder mehr invertiert.			*/
/* =========================================================*/

#ifdef GERMAN
#define TEXT1 "Invertieren"
#else
#ifdef ENGLISH
#define TEXT1 "Invertion"
#else
#ifdef FRENCH
#define TEXT1 "Invertion"
#else
#error "Keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

MOD_INFO module_info = {TEXT1,
						0x0050,
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
						1, 2, 4, 7, 8,
						16, 24, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Invertierer						*/
/*		1-8, 16 und 24 Bit 							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 BitsPerPixel;
	
	unsigned int *data16,
				 width, height;
	
	unsigned long length;
	

/* wie schnell sind wir? */
/*	init_timer(); */

/* Wenn das Modul zum ersten Mal gestartet wurde */
	if(smurf_struct->module_mode == MSTART)
	{
		smurf_struct->module_mode = M_STARTED;
		return;
	}
	else
	if(smurf_struct->module_mode == MEXEC)
	{
		BitsPerPixel = smurf_struct->smurf_pic->depth;

		if(BitsPerPixel != 16)
		{
			if(BitsPerPixel == 24 || BitsPerPixel == 1)
			{
				data = smurf_struct->smurf_pic->pic_data;

				width = smurf_struct->smurf_pic->pic_width;
				height = smurf_struct->smurf_pic->pic_height;
			}
			else
				data = smurf_struct->smurf_pic->palette;

			if(BitsPerPixel == 1)
			{
				length = (unsigned long)((width + 7) / 8) * (unsigned long)height;

				while(length--)
					*data++ = ~*data;
			}
			else
			{
				if(BitsPerPixel == 24)
					length = (unsigned long)width * (unsigned long)height;
				else
					length = 256L;

				while(length--)
				{
					*data++ = ~*data;
					*data++ = ~*data;
					*data++ = ~*data;
				}
			}
		}
		else
			if(BitsPerPixel == 16)
			{
				data16 = smurf_struct->smurf_pic->pic_data;

				width = smurf_struct->smurf_pic->pic_width;
				height = smurf_struct->smurf_pic->pic_height;

				length = (unsigned long)width * (unsigned long)height;

				while(length--)
					*data16++ = ~*data16;
			}
	
/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */
			
			smurf_struct->module_mode = M_DONEEXIT;
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