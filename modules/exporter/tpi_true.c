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
/*				Truepaint Image-Format Encoder				*/
/* Version 0.1  --  11.08.96								*/
/*	  16 Bit												*/
/* Version 0.2  --  12.08.96								*/
/*	  1, 2, 4, 8 Bit										*/
/* Version 0.3  --  30.01.97								*/
/*	  Paletteneintr„ge werden nach VDI umgebaut.			*/
/* Version 0.4  --  13.06.97								*/
/*	  Die Paletteneintr„ge wurden umgebaut - aber halt		*/
/*	  falsch. Ist nun korrigiert.							*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include <..\..\demolib.h>

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Truepaint Image",
						0x0040,
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
						0,0,0,0,
						0,0,0,0,
						0,0,0,0,
						0
						};


MOD_ABILITY  module_ability = {
						1, 2, 4, 8, 16,
						0, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0
						};


/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*			Truepaint Image-Format (TPI)			*/
/*		1, 2, 4, 8 und 16 Bit, unkomprimiert		*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel, *oziel, *pal, *plane_table,
		 p, v, Planes, BitsPerPixel;
	char table2bit[] = {0, 2, 3, 1};
	char table4bit[] = {0, 15, 1, 2, 4, 6, 3, 5, 7, 8, 9, 10, 12, 14, 11, 13};

	unsigned int *buffer16, *ziel16, *ppal, ppal2[48],
				 DatenOffset, cols, width, height, memwidth, i, j, w, x, y;

	unsigned long f_len;

	typedef struct
	{
		char magic[4];
		unsigned int unknown;
		unsigned int cols;
		unsigned int width;
		unsigned int height;
		unsigned int BitsPerPixel;
		unsigned int reserved;
		unsigned long len;
	} HEAD;

	HEAD *tpi_header;


	switch(smurf_struct->module_mode)
	{
		case MSTART:	
			smurf_struct->module_mode = M_WAITING;

			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:	
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			buffer = smurf_struct->smurf_pic->pic_data;

			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			if(BitsPerPixel != 16)
			{
				w = (width + 7) / 8;
				memwidth = ((width + 15) / 16) * 16;
				v = (memwidth >> 3) - w;
			}
			else
			{
				w = (width + 15) / 16;
				w = w * 16;
				memwidth = w;
				v = memwidth - width;
			}

			if(BitsPerPixel <= 8)
				cols = 1 << BitsPerPixel;
			else
				cols = 0;

			f_len = (memwidth * (unsigned long)smurf_struct->smurf_pic->pic_height * (unsigned long)BitsPerPixel) >> 3;
			f_len += 0x80 + cols * 6;

			if((ziel = (char *)SMalloc(f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				memset(ziel, 0x0, f_len);
				oziel = ziel;

				tpi_header = (HEAD *)ziel;
			 	strncpy(tpi_header->magic, "PNT", 4);
				tpi_header->unknown = 0x0100;

				tpi_header->cols = cols;

				tpi_header->width = width;
				tpi_header->height = height;

				tpi_header->BitsPerPixel = BitsPerPixel;

				tpi_header->reserved = 0x0;

				tpi_header->len = f_len - 0x80 - cols * 6;

				strncpy(smurf_struct->smurf_pic->format_name, "Truepaint Image .TPI", 21);

				DatenOffset = 0x80;

				if(BitsPerPixel != 16)
					DatenOffset += cols * 6;

				if(BitsPerPixel != 16)
				{
					ziel += DatenOffset;
					buffer16 = (unsigned int *)buffer;

					Planes = BitsPerPixel;

					p = 0;
					do /* Planes */
					{
						ziel16 = (unsigned int *)ziel + p;
						y = 0;
						do /* height */
						{
							x = 0;
							do /* width */
							{
								*ziel16 = *buffer16++;
								ziel16 += Planes;
								x += 2;
							} while(x < w); /* x */
							((char *)buffer16) -= v;
						} while(++y < height); /* y */
					} while(++p < Planes); /* p */
				} /* Palette oder TC? */
				else
				{			
					ziel += DatenOffset;

					ziel16 = (unsigned int *)ziel;
					buffer16 = (unsigned int *)buffer;

					y = 0;
					do /* height */
					{
						x = 0;
						do /* width */
						{
							*ziel16++ = *buffer16++;
						} while(++x < width); /* x */
						ziel16 += v;
						y++;
					} while(y < height); /* y */
				} /* Palette oder TC? */

				ziel = oziel;

				if(cols > 0)
				{
					ppal = (unsigned int *)(ziel + 0x80);

					pal = smurf_struct->smurf_pic->palette;
					for(i = 0; i < cols; i++)
					{
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
					}
					/* umbiegen weil VDI-Indizes im Bild stehen */
					if(cols == 4 || cols == 16)
					{
						if(cols == 4)
							plane_table = table2bit;
						else
							plane_table = table4bit;
						ppal = (unsigned int *)(ziel + 0x80);
						memcpy(ppal2, ppal, cols * 6);
						for(i = 0; i < cols; i++)
						{
							j = plane_table[i];
							ppal[i + i + i] = ppal2[j + j + j];
							ppal[i + i + i + 1] = ppal2[j + j + j + 1];
							ppal[i + i + i + 2] = ppal2[j + j + j + 2];
						}
					}
				}

				exp_pic->pic_data = ziel;
				exp_pic->f_len = f_len;
			} /* Malloc */

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