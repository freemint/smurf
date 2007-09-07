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
/*                  Vivid Ray-Tracer-Format                 */
/* Version 0.1  --  12.01.96                                */
/*	  xxx													*/
/* Version 0.1  --  19.10.97                                */
/*	  Ab jetzt wird direkt addressiert und nicht mehr per	*/
/*	  Index.												*/
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
MOD_INFO module_info = {"Vivid-Format",
						0x0020,
						"Christian Eyrich",
						"IMG", "", "", "", "",
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
/*			Vivid Raytracer-Format (IMG)			*/
/*		24 Bit, RLE									*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel,
		 n, v1, v2, v3, BitsPerPixel, DatenOffset;
	
	unsigned int x, y, width, height;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	if(*(unsigned int *)(buffer + 0x08) != 0x0018)
		return(M_INVALID);
	else
	{
		BitsPerPixel = *(unsigned int *)(buffer + 0x08);

		width = *(unsigned int *)buffer; 
		height = *(unsigned int *)(buffer + 0x02);

		strncpy(smurf_struct->smurf_pic->format_name, "Vivid Raytracer .IMG", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		smurf_struct->services->reset_busybox(128, "Vivid Raytracer 24 Bit");

		if((ziel = SMalloc((long)width * (long)height * 3)) == 0)
			return(M_MEMORY);
		else
		{
			oziel = ziel;

			DatenOffset = 0x0a;
			buffer += DatenOffset;

			y = 0;
			do
			{
				x = 0;
				do
				{
					n = *buffer++;
					v1 = *buffer++;
					v2 = *buffer++;
					v3 = *buffer++;

					x += n;

					while(n--)
					{
						*ziel++ = v1;
						*ziel++ = v2;
						*ziel++ = v3;
					}
				} while(x < width);
			} while(++y < height);

			buffer = obuffer;
			ziel = oziel;

			smurf_struct->smurf_pic->pic_data = ziel;

			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);

	return(M_PICDONE);
}