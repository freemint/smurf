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
/*                  Edsung Labs-Format						*/
/* Version 0.1  --  28.02.96                                */
/*	  xxx													*/
/*==========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Edsun Labs-Format",
						0x0010,
						"Christian Eyrich",
						"CEG", "", "", "", "",
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
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				Edsun Labs-Format (CEG)				*/
/*		24 Bit										*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel,
		 BitsPerPixel, DatenOffset;
	
	unsigned int x, y, width, height;

	unsigned long src_pos = 0, dst_pos = 0;


/* wie schnell sind wir? */
/*	init_timer(); */

	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;

	if(strcmp(buffer + 1, "EDSUN") != 0)
		return(M_INVALID);
	else
	{
		BitsPerPixel = (*buffer + 0x0b) * 3;

		width = *(buffer + 0x67) + (*(buffer + 0x68) << 8);
		height = *(buffer + 0x69) + (*(buffer + 0x70) << 8);

		strncpy(smurf_struct->smurf_pic->format_name, "Edsun Labs .CEG", 21);
		strncpy(smurf_struct->smurf_pic->infotext, buffer + 0x15, 80);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		smurf_struct->services->reset_busybox(128, "Edsun Labs 24 Bit");

		if((ziel = SMalloc((long)width * (long)height * 3)) == 0)
			return(M_MEMORY);
		else
		{
			DatenOffset = 0x3f4;
			src_pos = DatenOffset;

			y = 0;
			do /* height */
			{
				x = 0;
				do /* width */
				{
					ziel[dst_pos++] = buffer[src_pos++];
					ziel[dst_pos++] = buffer[src_pos++];
					ziel[dst_pos++] = buffer[src_pos++];
					x++;
				} while(x < width); /* x */
				y++;
			} while(y < height); /* y */

			smurf_struct->smurf_pic->pic_data = ziel;
			smurf_struct->smurf_pic->format_type = 0;

		} /* Malloc */
	} /* Erkennung */

/* wie schnell waren wir? */
/*	printf("%lu", get_timer);
	getch(); */

	SMfree(buffer);
	return(M_PICDONE);
}