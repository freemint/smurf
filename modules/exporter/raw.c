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
/*							RAW Exporter					*/
/* Version 0.1  --  2.10.98									*/
/*	  1, 8 und 24 Bit										*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include <..\..\demolib.h>

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"RAW",
						0x0010,
						"Christian Eyrich",
						"RAW","","","","",
						"","","","","",
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


MOD_ABILITY  module_ability = {
						1, 8, 24, 0, 0,
						0, 0, 0,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0
						};


/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					RAW Exporter (RAW)				*/
/*	  1, 8 und 24 Bit, unkomprimiert				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *obuffer, *ziel, *oziel, *pal,
		 BitsPerPixel;

	unsigned int x, y, width, height, i, pallen;

	unsigned long w, f_len;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			smurf_struct->module_mode = M_WAITING;	/* Ich warte... */
			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */

			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			buffer = smurf_struct->smurf_pic->pic_data;
			obuffer = buffer;

			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			if(BitsPerPixel == 1)
				w = (unsigned long)(width + 7) / 8;
			else
				if(BitsPerPixel == 8)
					w = (unsigned long)width;
				else
					w = (unsigned long)width * 3L;

			f_len = w * (unsigned long)height;
			if(BitsPerPixel == 8)
				pallen = 768;
			else
				pallen = 0;

			if((ziel = (char *)SMalloc(pallen + f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				oziel = ziel;

				if(pallen)
				{
					pal = smurf_struct->smurf_pic->palette;

					for(i = 0; i < 256; i++)
					{
						*ziel++ = *pal++;
						*ziel++ = *pal++;
						*ziel++ = *pal++;
					}
				}

				y = 0;
				do
				{
					x = 0;
					do
					{
						*ziel++ = *buffer++;
					} while(++x < width);					
				} while(++y < height);

				buffer = obuffer;
				ziel = oziel;

				exp_pic->pic_data = ziel;
				exp_pic->f_len = pallen + f_len;
			} /* Malloc */

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */

			smurf_struct->module_mode = M_DONEEXIT;
			return(exp_pic);

/* Mterm empfangen - Speicher freigeben und beenden */
		case MTERM:
			SMfree(exp_pic->pic_data);
			SMfree((char *)exp_pic);
			smurf_struct->module_mode = M_EXIT;
			break;

		default:
			smurf_struct->module_mode = M_WAITING;
			break;
	} /* switch */

	return(NULL);
}