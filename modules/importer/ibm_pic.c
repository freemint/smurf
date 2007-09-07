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
/*					IBM Picture Maker-Format				*/
/* Version 0.1  --  26.01.96								*/
/*	  8 Bit													*/
/* Version 0.2  --  18.10.97								*/
/*	  Ab jetzt wird direkt addressiert und nicht mehr per	*/
/*	  Index.												*/
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

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"IBM Picture Maker-Format",
						0x0020,
						"Christian Eyrich",
						"PIC", "", "", "", "",
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
/*			IBM Picture Maker-Format (PIC)			*/
/*		8 Bit, wahlweise RLE						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel, *pal, *ppal,
		 v1, v2, comp, BitsPerPixel, DatenOffset;

	unsigned int x, y, j = 0, n, cols, width, height;

	unsigned long i;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	if(*buffer != 0x00 || (*(buffer + 1) != 0x85 && *(buffer + 1) != 0x86) || *(buffer + 2) != 0xc1)
		return(M_INVALID);
	else
	{
		comp = *(buffer + 0x09);
		BitsPerPixel = 8;
		
		width = *(buffer + 0x05) + (*(buffer + 0x06) << 8); 
		height = *(buffer + 0x07) + (*(buffer + 0x08) << 8);

		cols = *(buffer + 0xa) + (*(buffer + 0xb) << 8);

		DatenOffset = 0x80 + cols * 3;

		strncpy(smurf_struct->smurf_pic->format_name, "IBM Picture Maker-File .PIC", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		smurf_struct->services->reset_busybox(128, "IBM Picturemaker 8 Bit");

		if((ziel = SMalloc(((long)width * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			oziel = ziel;

			buffer += DatenOffset;	

			y = 0;
			do
			{
				x = 0;
				do
				{
					if(!comp)
					{
						*ziel++ = *buffer++;
						x++;
					}
					else
					{
						v1 = *buffer++;

						if(v1 > 0x7f)
						{
							if(v1 == 0xff)
								n = *buffer++ + (*buffer++ << 8);
							else
								n = v1 - 0x80;

							v2 = *buffer++;

							x += n;

							while(n--)
								*ziel++ = v2;
						}
						else
						{
							x += v1;             

							while(v1--)
								*ziel++ = *buffer++;
						}
					}
				} while(x < width);
			} while(++y < height);

			buffer = obuffer;
			ziel = oziel;

			smurf_struct->smurf_pic->pic_data = ziel;

			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

			ppal = buffer + 0x80;
			while(j++ < 3)
			{
				pal = smurf_struct->smurf_pic->palette + j;
				for(i = 0; i < cols; i++)
				{
					*pal = *ppal++;
					pal += 3;
				}
			}

			smurf_struct->smurf_pic->col_format = RGB;
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);

	return(M_PICDONE);
}