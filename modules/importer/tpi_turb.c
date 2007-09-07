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
/*					Turbo Pascal-Image Decoder				*/
/* Version 0.1  --  27.01.96								*/
/*	  4 Bit, die Bilder sind etwas verschoben				*/
/*	  Schwierigkeiten bei der Erkennung "/ 2"!				*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Dies bastelt direct ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Turbo Pascal Grafik Format",
						0x0010,
						"Christian Eyrich",
						"TPI", "", "", "", "",
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
/*				Turbo Pascal Format .TPI			*/
/*		4 Bit, unkomprimiert						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel, *pal,
		 i, p;

	unsigned int x, y, width, height, DatenOffset;

	unsigned long dst_pos = 0, src_pos = 0, pal_pos = 0, w, pla, plh, plo;

	struct pal
	{
		char r;
		char g;
		char b;
	} stdpal[] = {
				{0x00, 0x00, 0x00},
				{0x00, 0x00, 0xa1},
				{0x00, 0xa1, 0x00},
				{0x00, 0xa1, 0xa1},
				{0xa1, 0x00, 0x00},
				{0xa1, 0x00, 0xa1},
				{0xa1, 0x55, 0x00},
				{0xa1, 0xa1, 0xa1},
				{0x55, 0x55, 0x55},
				{0x55, 0x55, 0xff},
				{0x55, 0xff, 0x55},
				{0x55, 0xff, 0xff},
				{0xff, 0x55, 0x55},
				{0xff, 0x55, 0xff},
				{0xff, 0xff, 0x55},
				{0xff, 0xff, 0xff},
				 };
	
	struct pal *ppal;

	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;

	/* Header Check */  /* (((Word0 + 1) * (Word1 + 1)) / 2) + 6 */
	if(smurf_struct->smurf_pic->file_len !=
       ((swap_word(*(unsigned int *)buffer) + 1) / 2)
	   *
	   (swap_word(*(unsigned int *)(buffer + 2)) + 1) + 6)
		return(M_INVALID);
	else
	{
		width = swap_word(*(unsigned int *)buffer) + 1;
		height= swap_word(*(unsigned int *)(buffer + 2)) + 1;

		DatenOffset = 6;

		strncpy(smurf_struct->smurf_pic->format_name, "Turbo Pascal Image .TPI", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = 4;

		smurf_struct->services->reset_busybox(128, "Turbo Pascal 4 Bit");

		if((ziel = SMalloc(((long)width * (long)height) >> 1)) == 0)
			return(M_MEMORY);
		else
		{
			src_pos = DatenOffset;
			w = (width + 7) / 8;
			plh = (height * w); /* H”he einer Plane in Byte*/
	
			y = 0;
			do /* height */
			{
				plo = (y * w); /* Offset vom Planeanfang in Bytes */
				for(p = 0; p < 4; p++) /* 4 Planes */
				{
					pla = (plh * p);	/* Abstand dieser Plane vom Bildanfang */
					dst_pos = (pla + plo); /* Zieladresse der dekodierten Scanline */
					x = 0;	
					do
					{
						ziel[dst_pos++] = buffer[src_pos++];
						x++;
					} while(x < w); /* width */
				} /* p */
				y++;
			} while (y < height); /* height */
		
			smurf_struct->smurf_pic->pic_data = ziel;

			smurf_struct->smurf_pic->format_type = 1;

			pal = smurf_struct->smurf_pic->palette;

			ppal = stdpal;
			for(i = 0; i < 16; i++)
			{
				pal[pal_pos++] = ppal->r;
				pal[pal_pos++] = ppal->g;
				pal[pal_pos++] = ppal->b;
				ppal++;
			}

			smurf_struct->smurf_pic->col_format = RGB;
	
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);
	return(M_PICDONE);
}