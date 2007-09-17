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
/*						Equalizer-Modul						*/
/* Version 0.1  --  21.04.97								*/
/*	  1-8, 24 Bit											*/
/* Version 0.2  --  xx.xx.97								*/
/*	  16 Bit												*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include <..\..\demolib.h>

int (*busybox)(int pos);

MOD_INFO module_info = {"Equalizer",
						0x0010,
						"Christian Eyrich",
						"", "", "", "", "",
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
						0,64,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						2, 4, 7, 8, 24,
						0, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Der Equalizer					*/
/*		1-8, 24 Bit 								*/
/* Setzt die Helligkeit im ganzen Bild auf den		*/
/* gleichen Wert.									*/
/* Daž die Ergebnisse bei WERKZE24.TGA und anderen	*/
/* so bescheiden sind, ist normal und tritt bei		*/
/* anderen Programmen auch auf.						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data, *odata, *equalize_map,
		 BitsPerPixel, greyval;

	unsigned int x, y, width, height, bh, bl;

	unsigned long *histogram, *map,
				  length, i, j, scale_factor;


/* Wenn das Modul zum ersten Mal gestartet wurde */
	switch(smurf_struct->module_mode)
	{
		case MSTART:
			smurf_struct->module_mode = M_STARTED;
			return;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */
			busybox = smurf_struct->services->busybox;

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			histogram = (unsigned long *)malloc(256L * 4L);			map = (unsigned long *)malloc(256L * 4L);			equalize_map = (char *)malloc(256L);
			if(histogram == 0 || map == 0 || equalize_map == 0)			{				free(histogram);
				free(map);
				free(equalize_map);
				smurf_struct->module_mode = M_MEMORY;
				return;			}			
	
			if(BitsPerPixel != 16)
			{
				if(BitsPerPixel == 24)
				{
					data = smurf_struct->smurf_pic->pic_data;
					odata = data;
				}
				else
				{
					data = smurf_struct->smurf_pic->palette;
					odata = data;
	
					length = 256L;
				}

				memset(histogram, 0x0, 256L * 4L);

				/* Histogramm (Graustufen) erstellen */
				if(BitsPerPixel == 24)
				{
					if((bh = height / 5) == 0)	 	/* busy-height */
						bh = height;
					bl = 0;							/* busy-length */

					y = 0;
					do
					{
						if(!(y%bh))
						{
							busybox(bl);
							bl += 15;
						}

						x = 0;
						do
						{
							greyval = (char)((((long)*data++ * 871L)
											+ ((long)*data++ * 2929L)
											+ ((long)*data++ * 295L)) >> 12);

							histogram[greyval]++;						} while(++x < width);					} while(++y < height);				}
				else
				{
					i = 0;
					while(i++ < length)
					{
						greyval = (char)((((long)*data++ * 871L)
										+ ((long)*data++ * 2929L)
										+ ((long)*data++ * 295L)) >> 12);

						histogram[greyval]++;					}
				}

				data = odata;

				/* Integrate the histogram to get the equalization map. */				j = 0;				i = 0;
				while(i < 256)
				{					j += histogram[i];					map[i] = j;
					i++;
				}
				free((void *)histogram);
				if(map[255] == 0)				{					free((void *)equalize_map);					free((void *)map);
					smurf_struct->module_mode = M_DONEEXIT;
					return;				}


				/* Equalize */
				scale_factor = (255L << 16) / map[255];				for(i = 0; i < 256; i++)
					equalize_map[i] = (char)(((map[i] * scale_factor) + (1L << 15)) >> 16);

				if(BitsPerPixel == 24)
				{
					if((bh = height / 5) == 0)	 	/* busy-height */
						bh = height;

					y = 0;
					do
					{
						if(!(y%bh))
						{
							busybox(bl);
							bl += 10;
						}

						x = 0;
						do
						{
							*data++ = equalize_map[*data];							*data++ = equalize_map[*data];							*data++ = equalize_map[*data];						} while(++x < width);
					} while(++y < height);
				}
				else
				{
					while(length--)
					{
						*data++ = equalize_map[*data];						*data++ = equalize_map[*data];						*data++ = equalize_map[*data];					}
				}
			}

			free(map);
			free(equalize_map);

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */
			
			smurf_struct->module_mode = M_DONEEXIT;
			return;

		/* Mterm empfangen - Speicher freigeben und beenden */
		case MTERM:
			smurf_struct->module_mode = M_EXIT;
			return;
	}
}