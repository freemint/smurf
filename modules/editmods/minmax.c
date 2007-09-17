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
/*					Minimum/Maximum-Modul					*/
/* Version 0.1  --  19.04.98								*/
/*	  24 Bit, Engine aus dem Paintmodul genommen und zur	*/
/*	  öberraschung festgestellt, daû der Bereich doch		*/
/*	  quadratisch und nicht rund sein muû.					*/
/* Version 0.2  --  04.05.98								*/
/*	  Es wurde immer ein Max ausgefÅhrt, auch wenn Min		*/
/*	  eingestellt war.										*/
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

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

void min_24Bit(char *ziel, char *buffer, unsigned int width, unsigned int height, char radius, long realwidth);
void max_24Bit(char *ziel, char *buffer, unsigned int width, unsigned int height, char radius, long realwidth);

int (*busybox)(int pos);

MOD_INFO module_info = {"Minimum/Maximum",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"",
						"",
						"",
						"",
						"Min",
						"Max",
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
						3,8,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						2, 3, 0, 0,
						3, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						24, 0, 0, 0, 0,
						0, 0, 0,
						FORM_PIXELPAK,
						FORM_BOTH,
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
/*				Minimum/Maximum-Modul				*/
/* Jedes Pixel wird durch die in der einstellbaren	*/
/* Nachbarschaft dunkelste/hellste Farbe setzt.		*/
/*		24 Bit 										*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel,
		 BitsPerPixel, BytesPerPixel, radius, min;

	int module_id;
	unsigned int width, height;

	long realwidth;

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

			if(smurf_struct->check1 == 1)
				min = 1;
			else
				min = 0;
			radius = smurf_struct->edit1 - 2;

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			BytesPerPixel = BitsPerPixel >> 3;

			if(BitsPerPixel != 16)
			{
				buffer = smurf_struct->smurf_pic->pic_data;
				obuffer = buffer;

				realwidth = width * BytesPerPixel;

				if((ziel = (char *)SMalloc((long)realwidth * (long)height)) == 0)
				{
					smurf_struct->module_mode = M_MEMORY;
					return;
				}
				else
				{
					oziel = ziel;
					memcpy(ziel, buffer, (long)realwidth * (long)height);

					buffer += realwidth * radius;
					ziel += realwidth * radius;

					if(min == 1)
						min_24Bit(ziel, buffer, width, height, radius, realwidth);
					else
						max_24Bit(ziel, buffer, width, height, radius, realwidth);
				}

				buffer = obuffer;
				ziel = oziel;

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


void max_24Bit(char *ziel, char *buffer, unsigned int width, unsigned int height, char radius, long realwidth)
{
	char *temp,
		 help, i, j, k, paintwidth, xoffset, max, r, g, b;

	unsigned int x, y, bh, bl;


	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	xoffset = radius * 3;

	if(radius&1)				/* Radius ungerade? */
		help = 1;
	else
		help = 0;

	paintwidth = radius + help + radius;

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
			/* den hellsten Pixel ermitteln */
			max = 0;
			r = 0;
			g = 0;
			b = 0;

			for(i = 0; i < paintwidth; i++)
			{
				temp = buffer - (radius - i) * realwidth - (radius * 3);

				for(j = 0; j < paintwidth; j++)
				{
					/* obere Zeilen */
					k = (char)((((long)*temp * 872L)
							  + ((long)*(temp + 1) * 2930L)
							  + ((long)*(temp + 2) * 296L)) >> 12);
					if(k > max)
					{
						max = k;
						r = *temp;
						g = *(temp + 1);
						b = *(temp + 2);
					}

					temp += 3;
				}
			}

			buffer += 3;
			*ziel++ = r;
			*ziel++ = g;
			*ziel++ = b;
		} while(++x < width - radius);

		buffer += xoffset;
		ziel += xoffset;
	} while(++y < height - radius);

	return;
} /* max_24Bit */


void min_24Bit(char *ziel, char *buffer, unsigned int width, unsigned int height, char radius, long realwidth)
{
	char *temp,
		 help, i, j, k, paintwidth, xoffset, max, r, g, b;

	unsigned int x, y, bh, bl;


	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	xoffset = radius * 3;

	if(radius&1)				/* Radius ungerade? */
		help = 1;
	else
		help = 0;

	paintwidth = radius + help + radius;

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
			/* den dunkelsten Pixel ermitteln */
			max = 255;
			r = 255;
			g = 255;
			b = 255;

			for(i = 0; i < paintwidth; i++)
			{
				temp = buffer - (radius - i) * realwidth - (radius * 3);

				for(j = 0; j < paintwidth; j++)
				{
					/* obere Zeilen */
					k = (char)((((long)*temp * 872L)
							  + ((long)*(temp + 1) * 2930L)
							  + ((long)*(temp + 2) * 296L)) >> 12);
					if(k < max)
					{
						max = k;
						r = *temp;
						g = *(temp + 1);
						b = *(temp + 2);
					}

					temp += 3;
				}
			}

			buffer += 3;
			*ziel++ = r;
			*ziel++ = g;
			*ziel++ = b;
		} while(++x < width - radius);

		buffer += xoffset;
		ziel += xoffset;
	} while(++y < height - radius);

	return;
} /* min_24Bit */