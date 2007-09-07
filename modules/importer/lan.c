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
/*					Erdas Image-Format						*/
/* Version 0.1  --  17.01.96								*/
/*	  8 und 24 Bit											*/
/* Version 0.2  --  19.10.97								*/
/*	  Runderneuerung, Bugfixes und in place-Bearbeitung		*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

/* Dies bastelt direkt ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Erdas Image-Format",
						0x0020,
						"Christian Eyrich",
						"LAN", "GIS", "", "", "",
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
/*              Erdas Image-Format (LAN)            */
/*      8 und 24 Bit, unkomprimiert                 */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *pal,
		 Planes, BitsPerPixel, DatenOffset;
	char dummy[3], impmessag[21];

	unsigned int x, y, width, height, i;

	unsigned long len;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	if(*(unsigned long *)buffer != 0x48454144L || *(unsigned int *)(buffer + 4) != 0x3734)
		return(M_INVALID);
	else
	{

		/* Liegt zwar als DWord vor, ich lese aber nur die hintersten beiden Bytes */
		Planes = (char)swap_word(*(unsigned int *)(buffer + 0x0a));
		if(Planes == 1)
			BitsPerPixel = 8;
		else
			BitsPerPixel = 24;

		/* Liegen zwar als DWord vor, ich lese aber nur die hintersten beiden Bytes */
		width = swap_word(*(unsigned int *)(buffer + 0x12));
		height = swap_word(*(unsigned int *)(buffer + 0x16));

		DatenOffset = 0x80;

		strncpy(smurf_struct->smurf_pic->format_name, "Erdas Image-Format .LAN, .SUN", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "LAN Erdas ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		ziel = buffer;

		len = (((long)width * (long)height) * BitsPerPixel) >> 3;

		if(BitsPerPixel == 8)
			memcpy(ziel, buffer + DatenOffset, len);
		else
		{
			buffer += DatenOffset;

			y = 0;
			do
			{
				x = 0;
				do
				{
					*ziel++ = *buffer;
					*ziel++ = *(buffer + width);
					*ziel++ = *(buffer++ + (width << 1));
				} while(++x < width);
				buffer += (width << 1);
			} while(++y < height);
		}

		buffer = obuffer;
		ziel = buffer;

		Mshrink(0, ziel, len);

		smurf_struct->smurf_pic->pic_data = ziel;

		smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

		if(BitsPerPixel == 8)
		{
			pal = smurf_struct->smurf_pic->palette;

			for(i = 0; i < 256; i++)
			{
				*pal++ = (char)i;
				*pal++ = (char)i;
				*pal++ = (char)i;
			}
		}

		smurf_struct->smurf_pic->col_format = RGB;
	} /* Erkennung */

	return(M_PICDONE);
}