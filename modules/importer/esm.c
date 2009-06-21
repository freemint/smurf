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
/*				Enhanced Simplex-Format Decoder				*/
/* Version 0.1  --  15.10.97								*/
/*	  1, 8 und 24 Bit										*/
/*	  Ein Modul aus der Reihe der 2 Minuten Decoder			*/
/* =========================================================*/

#ifdef GERMAN
#define ERROR "[1][unbekanntes ESM. Bild bitte an uns einschicken!][ OK ]"
#else
#ifdef ENGLISH
#define ERROR "[1][unbekanntes ESM. Bild bitte an uns einschicken!][ OK ]"
#else
#ifdef FRENCH
#define ERROR "[1][unbekanntes ESM. Bild bitte an uns einschicken!][ OK ]"
#else
#error "Keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"ESM Enhanced Simplex",
						0x0010,
						"Christian Eyrich",
						"ESM", "", "", "", "",
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
/*					Enhanced Simplex (ESM)			*/
/*	  1, 8 und 24 Bit, unkomprimiert				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel, *pal, *ppal,
		 BitsPerPixel;
	char dummy[3], impmessag[21];

	unsigned int width, height, i, DataOffset;

	unsigned long len;


	buffer = smurf_struct->smurf_pic->pic_data;

	if(strncmp(buffer, "TMS", 3) != 0)
		return(M_INVALID);
	else
	{
		DataOffset = *(unsigned int *)(buffer + 0x04);

		width = *(unsigned int *)(buffer + 0x06);
		height = *(unsigned int *)(buffer + 0x08);

		BitsPerPixel = *(unsigned int *)(buffer + 0x0a);

		strncpy(smurf_struct->smurf_pic->format_name, "Enhanced Simplex .ESM",21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "Enhanced Simplex ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);


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
				if(DataOffset > 768)			/* Platz fÅr Palette? */
				{
					ppal = buffer + 0x24;

					for(i = 0; i < 256; i++)
					{
						*pal++ = *ppal;
						*pal++ = *(ppal + 256);
						*pal++ = *(ppal++ + 512);
					}
				}
				else
					for(i = 0; i < 256; i++)
					{
						*pal++ = (char)i;
						*pal++ = (char)i;
						*pal++ = (char)i;
					}


		ziel = buffer;

		switch((int)BitsPerPixel)
		{
			case 1: len = (long)(width + 7) / 8 * (long)height;
					break;
			case 8: len = (long)width * (long)height;
					break;
			case 24: len = (long)width * (long)height * 3L;
					 break;
			default: form_alert(0, "[1][unbekanntes ESM. Bild bitte an uns einschicken!][ OK ]");
					 break;
		}

		memcpy(ziel, buffer + DataOffset, len);

		Mshrink(0, ziel, len);

		smurf_struct->smurf_pic->pic_data = ziel;

		if(BitsPerPixel == 1)
			smurf_struct->smurf_pic->format_type = FORM_STANDARD;
		else
			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

		if(BitsPerPixel == 8 && DataOffset < 768)				/* keine Palette -> Graustufenbild */
			smurf_struct->smurf_pic->col_format = GREY;
		else
			smurf_struct->smurf_pic->col_format = RGB;
	} /* Erkennung */

	return(M_PICDONE);
}