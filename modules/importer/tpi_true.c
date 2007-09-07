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
/*				Truepaint Image-Format Decoder				*/
/* Version 0.1  --  24.02.96								*/
/*	  1 Bit													*/
/* Version 0.2  --  27.02.96								*/
/*	  2, 4 und 8 Bit, obwohl ein Falcon-Malprogramm	werden	*/
/*	  Bilder im TPI-Format als interleaved Bitmaps ge-		*/
/*	  speichert ... Noch falsche Farben bei 4 Bit-Bildern.	*/
/* Version 0.3  --  03.03.96								*/
/*	  16 Bit werden nun auch dekodiert.						*/
/* Version 0.4  --  04.03.96								*/
/*	  16 Bit wurden aus RÅcksicht auf den Smurf als 24 Bit  */
/*	  ausgegeben. Jetzt sind es echte 16 Bit-Bilder.		*/
/* Version 0.5  --  07.03.96								*/
/*	  Kleinen aber erheblichen Fehler in der 16 Bit			*/
/*	  Dekodierung gefixt.									*/
/* Version 0.6  --  11.08.96								*/
/*	  Kleine énderung in der 16 Bit Dekodierung				*/
/* Version 0.7  --  13.06.97								*/
/*	  Die Palette von 2 und 4 Bit Bildern wird nun			*/
/*	  korrigiert und es erscheinen endlich richtige Farben.	*/
/* Version 0.8  --  10.08.98								*/
/*	  öberarbeitung des Palettenhandlings					*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Truepaint Image-Format",
						0x0080,
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
/*			Truepaint Image-Format (TPI)			*/
/*		1, 2, 4, 8 und 16 Bit, unkomprimiert		*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel, *pal, *plane_table,
		 p, v, Planes, BitsPerPixel;
	char table2bit[] = {0, 2, 3, 1};
	char table3bit[] = {0, 2, 3, 6, 4, 7, 5, 1};
	char table4bit[] = {0, 2, 3, 6, 4, 7, 5, 8, 9, 10, 11, 14, 12, 15, 13, 1};


	char dummy[3], impmessag[21];

	unsigned int *buffer16, *ziel16, *ppal,
				 i, j, x, y, w, memwidth, width, height,
				 cols, DatenOffset;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;

	if(*(unsigned long *)buffer != 'PNT\0')
		return(M_INVALID);
	else
	{
		cols = *(unsigned int *)(buffer + 0x06);

		width = *(unsigned int *)(buffer + 0x08); 
		height = *(unsigned int *)(buffer + 0x0a);

		BitsPerPixel = *(unsigned int *)(buffer + 0x0c);
		Planes = BitsPerPixel;

		DatenOffset = 0x80 + *(unsigned int *)(buffer + 0x06) * 6;

		strncpy(smurf_struct->smurf_pic->format_name, "Truepaint Image .TPI", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "Truepaint ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if(BitsPerPixel != 16)
		{
			w = (width + 7) / 8;
			memwidth = w * 8;
			v = (width + 15) / 16;
			v = v * 2 - w;
		}
		else
		{
			w = (width + 15) / 16;
			w = w * 16;
			memwidth = width;
			v = w - width;
		}

		if((ziel = SMalloc((((long)memwidth * (long)height * BitsPerPixel) >> 3) + 1)) == 0)
			return(M_MEMORY);
		else
		{
			obuffer = buffer;
			oziel = ziel;

			buffer += DatenOffset;

			if(BitsPerPixel != 16)
			{
				ziel16 = (unsigned int *)ziel;

				p = 0;
				do
				{
					buffer16 = (unsigned int *)buffer + p;

					y = 0;
					do
					{
						x = 0;
						do
						{
							*ziel16++ = *buffer16;
							buffer16 += Planes;
							x += 2;
						} while(x < w);

						(char *)ziel16 -= v;
					} while(++y < height);
				} while(++p < Planes);

				smurf_struct->smurf_pic->format_type = FORM_STANDARD;
			}
			else
			{			
				ziel16 = (unsigned int *)ziel;
				buffer16 = (unsigned int *)buffer;

				y = 0;
				do
				{
					x = 0;
					do
					{
						*ziel16++ = *buffer16++;
					} while(++x < width);

					buffer16 += v;
				} while(++y < height);

				smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
			}

			buffer = obuffer;
			ziel = oziel;
	
			smurf_struct->smurf_pic->pic_data = ziel;
	
			if(cols > 0)
			{
				ppal = (unsigned int *)(buffer + 0x80);
				pal = smurf_struct->smurf_pic->palette;

				if(cols == 4)
					plane_table = table2bit;
				else
					if(cols == 8)
						plane_table = table3bit;
					else
						plane_table = table4bit;

				for(i = 0; i < cols; i++)
				{
					if(cols < 256)
						j = plane_table[i];
					else
						j = i;

					pal[i + i + i] = (char)(((long)ppal[j + j + j] * 255L) / 1000L);
					pal[i + i + i + 1] = (char)(((long)ppal[j + j + j + 1] * 255L) / 1000L);
					pal[i + i + i + 2] = (char)(((long)ppal[j + j + j + 2] * 255L) / 1000L);
				}
			}
	
			smurf_struct->smurf_pic->col_format = RGB;
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);
	return(M_PICDONE);
}