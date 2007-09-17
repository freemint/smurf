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
/*						Expander-Modul						*/
/* Version 0.1  --  21.04.97								*/
/*	  2-8, 24 Bit											*/
/* Version 0.2  --  29.09.97								*/
/*	  Bescheuerten Fehler gefixt, der bei Palettenbildern	*/
/*	  zu einem ganz schwarzen Bild fÅhrte.					*/
/* Version 0.3  --  27.03.98								*/
/*	  Zeilenweise Assemblerleseroutine eingebaut.			*/
/* Version 0.4  --  xx.xx.98								*/
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

/* Get Standardformat-Pixel Assembler-Rout */
void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);

int (*busybox)(int pos);

MOD_INFO module_info = {"Expander",
						0x0030,
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
/*		Der Expander (auch als normalize bekannt)	*/
/*		2-8, 16, 24 Bit 							*/
/* Die Expander skaliert den Helligkeitsumfang auf	*/
/* den vollen Umfang von 255.						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data, *odata, *palette, *pal, *normalize_map, *pixbuf, *line,
		 BitsPerPixel, pixval, greyval, high, low;

	unsigned int x, y, width, height, bh, bl;

	unsigned long *histogram,
				  length, i, intensity, threshold_intensity;
	unsigned long w, planelength;


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

			histogram = (unsigned long *)calloc(256L * 4L, 1);
			normalize_map = (char *)calloc(256L, 1);

			if(histogram == 0 || normalize_map == 0)
			{
				free(normalize_map);
				free(histogram);
				smurf_struct->module_mode = M_MEMORY;
				return;
			}			
	
			if(BitsPerPixel != 16)
			{
				if(BitsPerPixel == 24)
				{
					data = smurf_struct->smurf_pic->pic_data;
					odata = data;
				}
				else
				{
					data = smurf_struct->smurf_pic->pic_data;
					odata = data;
					palette = smurf_struct->smurf_pic->palette;
				}

				if((bh = height / 5) == 0)	 	/* busy-height */
					bh = height;
				bl = 0;							/* busy-length */

				/* Histogramm (Graustufen) erstellen */
				if(BitsPerPixel == 24)
				{
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

							histogram[greyval]++;
						} while(++x < width);
					} while(++y < height);
				}
				else
				{
					if(smurf_struct->smurf_pic->format_type == FORM_STANDARD)
					{
						pixbuf = (char *)Malloc(width + 7);

						w = (unsigned long)((width + 7) / 8);
						planelength = w * (unsigned long)height;   /* LÑnge einer Plane in Bytes */

						y = 0;
						do
						{
							if(!(y%bh))
							{
								busybox(bl);
								bl += 25;
							}

							memset(pixbuf, 0x0, width);
							getpix_std_line(data, pixbuf, BitsPerPixel, planelength, width);
							data += w;
							line = pixbuf;

							x = 0;
							do
							{
								pixval = *line++;
								pal = palette + pixval + pixval + pixval;

								greyval = (char)((((long)*pal++ * 871L)
												+ ((long)*pal++ * 2929L)
												+ ((long)*pal * 295L)) >> 12);

								histogram[greyval]++;
							} while(++x < width);
						} while(++y < height);

						Mfree(pixbuf);
					}
					else
					{
						y = 0;
						do
						{
							if(!(y%bh))
							{
								busybox(bl);
								bl += 25;
							}

							x = 0;
							do
							{
								pixval = *data++;
								pal = palette + pixval + pixval + pixval;

								greyval = (char)((((long)*pal * 871L)
												+ ((long)*(pal + 1) * 2929L)
												+ ((long)*(pal + 2) * 295L)) >> 12);

								histogram[greyval]++;
							} while(++x < width);
						} while(++y < height);
					}
				}

				data = odata;

				/* Histogrammgrenzen durch Suche nach 1 Prozent Schritt finden */
				threshold_intensity = (unsigned int)(((long)width * (long)height) / 100);

				intensity = 0;
				for(low = 0; low < 255; low++)
				{
					intensity += histogram[low];

					if(intensity > threshold_intensity)
						break;
				}

				intensity = 0;
				for(high = 255; high != 0; high--)
				{
					intensity += histogram[high];

					if(intensity > threshold_intensity)
						break;
				}

				if(low == high)
				{
					/* Fast kein Kontrast; 0-Grenze fÅr Grenzfindung benutzen. */
					threshold_intensity = 0;
					intensity = 0;
					for(low = 0; low < 255; low++)
					{
						intensity += histogram[low];

						if(intensity > threshold_intensity)
							break;
					}

					intensity = 0;
					for(high = 255; high != 0; high--)
					{
						intensity += histogram[high];

						if(intensity > threshold_intensity)
							break;
					}

					if(low == high)
						return;  /* zero span bound */
				}

				/* Das Histogramm aufziehen um ein normalisiertes Mapping zu erstellen */
				for(i = 0; i < 256; i++)
					if(i < (int)low)
						normalize_map[i] = 0;
					else
						if(i > (int)high)
							normalize_map[i] = 255 - 1;
						else
							normalize_map[i] = (255 - 1) * (i - (int)low) / (int)(high - low);

				/* Normalize */
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
							*data++ = normalize_map[*data];
							*data++ = normalize_map[*data];
							*data++ = normalize_map[*data];
						} while(++x < width);
					} while(++y < height);
				}
				else
				{
					pal = palette;
					length = 256L;
					while(length--)
					{
						*pal++ = normalize_map[*pal];
						*pal++ = normalize_map[*pal];
						*pal++ = normalize_map[*pal];
					}
				}
			}

			free(histogram);
			free(normalize_map);			

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