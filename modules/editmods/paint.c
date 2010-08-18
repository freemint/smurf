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
/*						Oil Paint-Modul						*/
/* Version 0.1  --  19.10.97								*/
/*	  24 Bit												*/
/* Version 0.2  --  20.10.97								*/
/*	  8 Bit													*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include "..\..\src\lib\demolib.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

void paint_8Bit(char *ziel, char *buffer, unsigned int width, unsigned int height, char *pal, unsigned int *histogram, char radius, unsigned int realwidth);
void paint_24Bit(char *ziel, char *buffer, unsigned int width, unsigned int height, unsigned int *histogram, char radius, unsigned int realwidth);

int (*busybox)(int pos);

MOD_INFO module_info = {"Painting",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"Radius",
						"",
						"",
						"",
						0,64,
						0,64,
						0,64,
						0,64,
						2,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						3, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						8, 24, 0, 0, 0,
						0, 0, 0,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Oil Paint-Modul					*/
/* Erzeugt ein Bild welches wie ein ™lgem„lde aus-	*/
/* sehen soll. Jedes Pixel wird durch die in der	*/
/* einstellbaren Nachbarschaft h„ufigste Farbe er-	*/
/* setzt.											*/
/*		8, 16, 24 Bit 								*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel,
		 BitsPerPixel, BytesPerPixel, radius;

	int module_id;
	unsigned int *histogram,
				 width, height, realwidth;


	module_id = smurf_struct->module_number;

/* Wenn das Modul zum ersten Mal gestartet wurde */
	switch(smurf_struct->module_mode)
	{
		case MSTART:
			smurf_struct->services->f_module_prefs(&module_info, module_id);
			smurf_struct->module_mode = M_WAITING;
			return;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			busybox = smurf_struct->services->busybox;

			radius = smurf_struct->edit1;

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			BytesPerPixel = BitsPerPixel >> 3;

			smurf_struct->services->reset_busybox(0, "Painting ...");

			if((histogram = (unsigned int *)calloc(256L * 2L, 1)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return;
			}			
	
			if(BitsPerPixel != 16)
			{
				buffer = smurf_struct->smurf_pic->pic_data;
				obuffer = buffer;

				realwidth = width * BytesPerPixel;

				if((ziel = (char *)SMalloc((long)realwidth * (long)height)) == 0)
				{
					free(histogram);
					smurf_struct->module_mode = M_MEMORY;
					return;
				}
				else
				{
					oziel = ziel;
					memcpy(ziel, buffer, (long)realwidth * (long)height);

					buffer += realwidth * radius;
					ziel += realwidth * radius;

					if(BitsPerPixel == 8)
						paint_8Bit(ziel, buffer, width, height, smurf_struct->smurf_pic->palette, histogram, radius, realwidth);
					else
						paint_24Bit(ziel, buffer, width, height, histogram, radius, realwidth);
				}

				buffer = obuffer;
				ziel = oziel;

				free(histogram);
				smurf_struct->smurf_pic->pic_data = ziel;
				SMfree(buffer);
			}

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */
			
			smurf_struct->module_mode = M_PICDONE;
			return;

		/* Mterm empfangen - Speicher freigeben und beenden */
		case MTERM:
			smurf_struct->module_mode = M_EXIT;
			return;
	}
}


void paint_8Bit(char *ziel, char *buffer, unsigned int width, unsigned int height, char *pal, unsigned int *histogram, char radius, unsigned int realwidth)
{
	char *ppal, *temp, *tempupper, *templower, *frequent,
		 i, j, k, paintwidth, xoffset, pixval;

	unsigned int x, y, count, bh, bl;


	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	xoffset = radius;

	y = radius;
	do
	{
		if(!(y%bh))
		{
			busybox(bl);
			bl += 12;
		}

		buffer += xoffset;
		ziel += xoffset;
		x = radius;
		do
		{
			/* die h„ufigste Farbe ermitteln */
			count = 0;
			memset(histogram, 0x0, 512L);

			for(i = 0; i < radius; i++)
			{
				tempupper = buffer - (radius - i) * realwidth - i;
				templower = buffer + (radius - i) * realwidth - i;

				paintwidth = i + 1 + i;

				for(j = 0; j < paintwidth; j++)
				{
				/* obere Zeilen */
					pixval = *tempupper;
					ppal = pal + pixval + pixval + pixval;
					k = (char)((((long)*ppal * 872L)
							  + ((long)*(ppal + 1) * 2930L)
							  + ((long)*(ppal + 2) * 296L)) >> 12);
					histogram[k]++;
					if(histogram[k] > count)
					{
						count = histogram[k];
						frequent = tempupper;
					}

					tempupper++;

				/* untere Zeilen */
					pixval = *templower;
					ppal = pal + pixval + pixval + pixval;
					k = (char)((((long)*templower * 872L)
							  + ((long)*(templower + 1) * 2930L)
							  + ((long)*(templower + 2) * 296L)) >> 12);
					histogram[k]++;
					if(histogram[k] > count)
						if(k < count)
						{
							count = histogram[k];
							frequent = templower;
						}

					templower++;
				}
			}

		/* mittlere Zeile */
			temp = buffer - radius;

			paintwidth = radius + 1 + radius;

			for(j = 0; j < paintwidth; j++)
			{
				pixval = *temp;
				ppal = pal + pixval + pixval + pixval;
				k = (char)((((long)*ppal * 872L)
						  + ((long)*(ppal + 1) * 2930L)
						  + ((long)*(ppal + 2) * 296L)) >> 12);
				histogram[k]++;
				if(histogram[k] > count)
				if(k < count)
				{
					count = histogram[k];
					frequent = temp;
				}

				temp++;
			}

			buffer++;
			*ziel++ = *frequent++;
		} while(++x < width - radius);
		buffer += xoffset;
		ziel += xoffset;
	} while(++y < height - radius);

	return;
} /* paint_8Bit */


void paint_24Bit(char *ziel, char *buffer, unsigned int width, unsigned int height, unsigned int *histogram, char radius, unsigned int realwidth)
{
	char *temp, *tempupper, *templower, *frequent,
		 i, j, k, paintwidth, xoffset;

	unsigned int x, y, count, bh, bl;


	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	xoffset = radius * 3;

	y = radius;
	do
	{
		if(!(y%bh))
		{
			busybox(bl);
			bl += 12;
		}

		buffer += xoffset;
		ziel += xoffset;
		x = radius;
		do
		{
			/* die h„ufigste Farbe ermitteln */
			count = 0;
			memset(histogram, 0x0, 512L);

			for(i = 0; i < radius; i++)
			{
				tempupper = buffer - (radius - i) * realwidth - (i * 3);
				templower = buffer + (radius - i) * realwidth - (i * 3);

				paintwidth = i + 1 + i;

				for(j = 0; j < paintwidth; j++)
				{
				/* obere Zeilen */
					k = (char)((((long)*tempupper * 871L)
							  + ((long)*(tempupper + 1) * 2929L)
							  + ((long)*(tempupper + 2) * 295L)) >> 12);
					histogram[k]++;
					if(histogram[k] > count)
					{
						count = histogram[k];
						frequent = tempupper;
					}

					tempupper += 3;

				/* untere Zeilen */
					k = (char)((((long)*templower * 871L)
							  + ((long)*(templower + 1) * 2929L)
							  + ((long)*(templower + 2) * 295L)) >> 12);
					histogram[k]++;
					if(histogram[k] > count)
					if(k < count)
					{
						count = histogram[k];
						frequent = templower;
					}

					templower += 3;
				}
			}

		/* mittlere Zeile */
			temp = buffer - (radius * 3);

			paintwidth = radius + 1 + radius;

			for(j = 0; j < paintwidth; j++)
			{
				k = (char)((((long)*temp * 871L)
						  + ((long)*(temp + 1) * 2929L)
						  + ((long)*(temp + 2) * 295L)) >> 12);
				histogram[k]++;
				if(histogram[k] > count)
				if(k < count)
				{
					count = histogram[k];
					frequent = temp;
				}

				temp += 3;
			}

			buffer += 3;
			*ziel++ = *frequent++;
			*ziel++ = *frequent++;
			*ziel++ = *frequent++;
		} while(++x < width - radius);
		buffer += xoffset;
		ziel += xoffset;
	} while(++y < height - radius);

	return;
} /* paint_24Bit */