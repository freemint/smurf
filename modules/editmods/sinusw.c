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
/*						Sinuswellen-Modul					*/
/* Version 0.1  --  11.07.98 - 12.07.98						*/
/*	  8, 24 Bit												*/
/* Version 0.2  --  22.08.98								*/
/*	  Die Bildgr”že kann jetzt beibehalten werden.			*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include <math.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include <..\..\demolib.h>

#define EXTEND	0
#define HOLD	1
#define HORIZONTAL	1
#define VERTIKAL	2

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*busybox)(int pos);

char *sinus_vertikal(char *buffer, long *sinustab, unsigned int width, unsigned int height, unsigned int sinheight, char BitsPerPixel);
char *sinus_vertikal2(char *buffer, long *sinustab, unsigned int width, unsigned int height, char BitsPerPixel);
char *sinus_vertikal3(char *buffer, long *sinustab, unsigned int width, unsigned int height, char BitsPerPixel);
char *sinus_horizontal(char *buffer, long *sinustab, unsigned int width, unsigned int height, unsigned int sinheight, char BitsPerPixel);
char *sinus_horizontal3(char *buffer, long *sinustab, unsigned int width, unsigned int height, char BitsPerPixel);

MOD_INFO module_info = {"Sinuswellen",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"St„rke",
						"Weite",
						"",
						"",
						"horizontal",
						"vertikal",
						"Gr”že gleich",
						"",
						"",
						"",
						"",
						"",
						0,255,
						0,255,
						0,100,
						0,100,
						0,10,
						0,10,
						0,10,
						0,10,
						16, 40, 20, 20,
						3, 4, 0, 0,
						0, 0, 0, 0,
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
/*					Sinuswellen						*/
/*		8, 24 Bit 									*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel,
		 BitsPerPixel, direction, mode;
	
	int	module_id;
	unsigned int i, width, height, sinheight, wide;

	long *sinustab,
		 realwidth, offset;

	double sindiv = 180.0 / M_PI, widemul;


	module_id = smurf_struct->module_number;

/* Wenn das Modul zum ersten Mal gestartet wurde */
	if(smurf_struct->module_mode == MSTART)
	{
		smurf_struct->services->f_module_prefs(&module_info, module_id);
		smurf_struct->module_mode = M_WAITING;
		return;
	}
	else
		if(smurf_struct->module_mode == MEXEC)
		{
/* wie schnell sind wir? */
/*	init_timer(); */
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			busybox = smurf_struct->services->busybox;

			if(smurf_struct->check1 > smurf_struct->check2)
				direction = HORIZONTAL;
			else
				direction = VERTIKAL;

			mode = smurf_struct->check3;

			sinheight = (int)smurf_struct->slide1;
			wide = (int)smurf_struct->slide2;
			widemul = 180 / wide;

			BitsPerPixel = smurf_struct->smurf_pic->depth;
	
			buffer = smurf_struct->smurf_pic->pic_data;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			sinustab = (long *)Malloc(width * 4);

			realwidth = (width * BitsPerPixel) >> 3;

			if(direction == HORIZONTAL)
				offset = 1;
			else
				offset = realwidth;

			i = 0;
			do
			{
				sinustab[i] = (long)(sin(i * widemul / sindiv) * (double)sinheight) * offset;
			} while(++i < width);

			if(direction == HORIZONTAL)
				if(mode == HOLD)
					ziel = sinus_horizontal3(buffer, sinustab, width, height, BitsPerPixel);
				else
					ziel = sinus_horizontal(buffer, sinustab, width, height, sinheight, BitsPerPixel);
			else
				if(mode == HOLD)
					ziel = sinus_vertikal3(buffer, sinustab, width, height, BitsPerPixel);
				else
					ziel = sinus_vertikal(buffer, sinustab, width, height, sinheight, BitsPerPixel);

			Mfree(sinustab);

			if(ziel == NULL)
			{
				smurf_struct->module_mode = M_MEMORY;
				return;
			}
			else
			{
				if(mode == EXTEND)
					if(direction == HORIZONTAL)
						smurf_struct->smurf_pic->pic_width = width + 2 * sinheight;
					else
						smurf_struct->smurf_pic->pic_height = height + 2 * sinheight;
				smurf_struct->smurf_pic->pic_data = ziel;
				SMfree(buffer);
			}

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */

			smurf_struct->module_mode = M_PICDONE;
			return;
		}

		/* Mterm empfangen - Speicher freigeben und beenden */
		else 
			if(smurf_struct->module_mode == MTERM)
			{
				smurf_struct->module_mode = M_EXIT;
				return;
			}
}


/* Generiert die Sinuswelle in Y-Richtung */
char *sinus_horizontal(char *buffer, long *sinustab, unsigned int width, unsigned int height, unsigned int sinheight, char BitsPerPixel)
{
	char *ziel, *oziel;

	unsigned int x, y;

	long newrealwidth;


	newrealwidth = ((long)(width + 2 * sinheight) * BitsPerPixel) >> 3;

	if((ziel = (char *)SMalloc(newrealwidth * (long)height)) == 0)
		return(NULL);
	else
	{
		oziel = ziel;

		if(BitsPerPixel == 8)
		{
			y = 0;
			do
			{
				ziel = oziel + (newrealwidth * y) + sinheight + *sinustab++;

				x = 0;
				do
				{
					*ziel++ = *buffer++;
				} while(++x < width);
			} while(++y < height);
		}
		else
			if(BitsPerPixel == 24)
			{
				y = 0;
				do
				{
					ziel = oziel + (newrealwidth * y) + (sinheight + *sinustab++) * 3L;

					x = 0;
					do
					{
						*ziel++ = *buffer++;
						*ziel++ = *buffer++;
						*ziel++ = *buffer++;
					} while(++x < width);
				} while(++y < height);
			}
	}

	ziel = oziel;

	return(ziel);
} /* sinus_horizontal */


/* Generiert die Sinuswelle in Y-Richtung */
char *sinus_horizontal3(char *buffer, long *sinustab, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *ziel, *oziel, *addr, *begin, *end;

	unsigned int x, y;

	long realwidth;


	realwidth = ((long)width * BitsPerPixel) >> 3;

	if((ziel = (char *)SMalloc(realwidth * (long)height)) == 0)
		return(NULL);
	else
	{
		oziel = ziel;

		if(BitsPerPixel == 8)
		{
			y = 0;
			do
			{
				ziel = oziel + realwidth * y;

				begin = ziel;
				end = ziel + realwidth - 1;

				ziel += *sinustab++;

				x = 0;
				do
				{
					if(ziel < begin)
						addr = ziel++ + realwidth;
					else
						if(ziel > end)
							addr = ziel++ - realwidth;
						else
							addr = ziel++;

					*addr = *buffer++;
				} while(++x < width);
			} while(++y < height);
		}
		else
			if(BitsPerPixel == 24)
			{
				y = 0;
				do
				{
					ziel = oziel + realwidth * y;

					begin = ziel;
					end = ziel + realwidth - 3;

					ziel += *sinustab++ * 3L;

					x = 0;
					do
					{
						if(ziel < begin)
							addr = ziel + realwidth;
						else
							if(ziel > end)
								addr = ziel - realwidth;
							else
								addr = ziel;

						*addr++ = *buffer++;
						*addr++ = *buffer++;
						*addr = *buffer++;
						ziel += 3;
					} while(++x < width);
				} while(++y < height);
			}
	}

	ziel = oziel;

	return(ziel);
} /* sinus_horizontal3 */


/* Generiert die Sinuswelle in X-Richtung */
char *sinus_vertikal(char *buffer, long *sinustab, unsigned int width, unsigned int height, unsigned int sinheight, char BitsPerPixel)
{
	char *ziel, *oziel;

	unsigned int x, y;

	long *osinustab = sinustab,
		 realwidth;


	realwidth = ((long)width * BitsPerPixel) >> 3;

	if((ziel = (char *)SMalloc(realwidth * ((long)height + 2 * sinheight))) == 0)
		return(NULL);
	else
	{
		oziel = ziel;

		ziel += realwidth * sinheight;

		if(BitsPerPixel == 8)
		{
			y = 0;
			do
			{
				sinustab = osinustab;

				x = 0;
				do
				{
					*(ziel++ + *sinustab++) = *buffer++;
				} while(++x < width);
			} while(++y < height);
		}
		else
			if(BitsPerPixel == 24)
			{
				y = 0;
				do
				{
					sinustab = osinustab;

					x = 0;
					do
					{
						*(ziel++ + *sinustab) = *buffer++;
						*(ziel++ + *sinustab) = *buffer++;
						*(ziel++ + *sinustab++) = *buffer++;
					} while(++x < width);
				} while(++y < height);
			}
	}

	ziel = oziel;

	return(ziel);
} /* sinus_vertikal */


/* Generiert die Sinuswelle in X-Richtung unter Beibehaltung der Bildgr”že */
char *sinus_vertikal2(char *buffer, long *sinustab, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *ziel, *oziel, *addr, *begin, *end;

	unsigned int x, y;

	long *osinustab = sinustab,
		 picheight;


	picheight = (((long)width * BitsPerPixel) >> 3) * (long)height;

	if((ziel = (char *)SMalloc(picheight)) == 0)
		return(NULL);
	else
	{
		oziel = ziel;

		begin = ziel;

		if(BitsPerPixel == 8)
		{
			end = ziel + picheight - 1;

			y = 0;
			do
			{
				sinustab = osinustab;

				x = 0;
				do
				{
					if((addr = ziel + *sinustab++) >= begin &&
					   addr <= end)
						*addr = *buffer;

					ziel++;
					buffer++;
				} while(++x < width);
			} while(++y < height);
		}
		else
			if(BitsPerPixel == 24)
			{
				end = ziel + picheight - 3;

				y = 0;
				do
				{
					sinustab = osinustab;

					x = 0;
					do
					{
						if((addr = ziel + *sinustab++) >= begin &&
						   addr <= end)
						{
							*addr = *buffer;
							*(addr + 1) = *(buffer + 1);
							*(addr + 2) = *(buffer + 2);
						}

						ziel += 3;
						buffer += 3;
					} while(++x < width);
				} while(++y < height);
			}
	}

	ziel = oziel;

	return(ziel);
} /* sinus_vertikal2 */


/* Generiert die Sinuswelle in X-Richtung unter Beibehaltung der Bildgr”že */
char *sinus_vertikal3(char *buffer, long *sinustab, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *ziel, *oziel, *addr, *begin, *end;

	unsigned int x, y;

	long *osinustab = sinustab,
		 picheight;


	picheight = (((long)width * BitsPerPixel) >> 3) * (long)height;

	if((ziel = (char *)SMalloc(picheight)) == 0)
		return(NULL);
	else
	{
		oziel = ziel;

		begin = ziel;

		if(BitsPerPixel == 8)
		{
			end = ziel + picheight - 1;

			y = 0;
			do
			{
				sinustab = osinustab;

				x = 0;
				do
				{
					if((addr = ziel + *sinustab++) < begin)
						addr += picheight;
					else
						if(addr > end)
							addr -= picheight;

					*addr = *buffer++;

					ziel++;
				} while(++x < width);
			} while(++y < height);
		}
		else
			if(BitsPerPixel == 24)
			{
				end = ziel + picheight - 3;

				y = 0;
				do
				{
					sinustab = osinustab;

					x = 0;
					do
					{
						if((addr = ziel + *sinustab++) < begin)
							addr += picheight;
						else
							if(addr > end)
								addr -= picheight;

						*addr = *buffer++;
						*(addr + 1) = *buffer++;
						*(addr + 2) = *buffer++;

						ziel += 3;
					} while(++x < width);
				} while(++y < height);
			}
	}

	ziel = oziel;

	return(ziel);
} /* sinus_vertikal3 */