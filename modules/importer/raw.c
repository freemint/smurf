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
/*							RAW Importer					*/
/* Version 0.1  --  3.10.98									*/
/*	  1, 8 und 24 Bit										*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"RAW",
						0x0010,
						"Christian Eyrich",
						"RAW", "", "", "", "",
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
/*					RAW (RAW)						*/
/*	  1, 8 und 24 Bit, unkomprimiert				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel, *pal,
		 BitsPerPixel;

	unsigned int x, y, width, height, i;


	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	width = 320;
	height = 200;

	BitsPerPixel = 8;

	if(smurf_struct->smurf_pic->file_len < 64000L)
		return(M_INVALID);

	strncpy(smurf_struct->smurf_pic->format_name, "RAW Binary Files",21);
	smurf_struct->smurf_pic->pic_width = width;
	smurf_struct->smurf_pic->pic_height = height;
	smurf_struct->smurf_pic->depth = BitsPerPixel;

	pal = smurf_struct->smurf_pic->palette;

	if(BitsPerPixel == 1)
	{
		pal[0] = 255;
		pal[1] = 255;
		pal[2] = 255;
		pal[3] = 0;
		pal[4] = 0;
		pal[5] = 0;
	}
	else
		if(BitsPerPixel == 8)
		{
			for(i = 0; i < 256; i++)
			{
				*pal++ = *buffer++;
				*pal++ = *buffer++;
				*pal++ = *buffer++;
			}


		ziel = (char *)Malloc((long)width * (long)height);
		memset(ziel, 0x0, (long)width * (long)height);

		oziel = ziel;

		y = 0;
		do
		{
			x = 0;
			do
			{
				*ziel++ = *buffer++;
			} while(++x < width);					
		} while(++y < height);

		ziel = oziel;
		buffer = obuffer;

		Mfree(buffer);

		smurf_struct->smurf_pic->pic_data = ziel;

		if(BitsPerPixel == 1)
			smurf_struct->smurf_pic->format_type = FORM_STANDARD;
		else
			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

		smurf_struct->smurf_pic->col_format = RGB;
	} /* Erkennung */

	return(M_PICDONE);
}