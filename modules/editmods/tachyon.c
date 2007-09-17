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
/*							Tachyon							*/
/* Version 0.1  --  10.9.2000 - 20.9.2000					*/
/*	  1, 2, 4, 7, 8, 24 Bit									*/
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

#define DEBUG 0

void do_tachyon(char *data, unsigned int width, unsigned int height);
extern void tachyon_pixel(char *data, char *cliptab, unsigned int width);

int (*busybox)(int pos);

MOD_INFO module_info = {"Tachyon",
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
						1, 2, 4, 7, 8,
						24, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*						Tachyon						*/
/*	Invertiert die Helligkeit ohne die Farbigkeit	*/
/*	anzutasten.										*/
/*		1-8 und 24 Bit 								*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 BitsPerPixel;
	
	unsigned int width, height;
	
	unsigned long length;
	

/* wie schnell sind wir? */
/*	init_timer(); */

/* Wenn das Modul zum ersten Mal gestartet wurde */
	if(smurf_struct->module_mode == MSTART)
	{
		smurf_struct->module_mode = M_STARTED;
		return;
	}
	else
	if(smurf_struct->module_mode == MEXEC)
	{
		busybox = smurf_struct->services->busybox;

		BitsPerPixel = smurf_struct->smurf_pic->depth;

		if(BitsPerPixel != 16)
		{
			if(BitsPerPixel == 24 || BitsPerPixel == 1)
			{
				data = smurf_struct->smurf_pic->pic_data;

				width = smurf_struct->smurf_pic->pic_width;
				height = smurf_struct->smurf_pic->pic_height;
			}
			else
				data = smurf_struct->smurf_pic->palette;

			if(BitsPerPixel == 1)
			{
				length = (unsigned long)((width + 7) / 8) * (unsigned long)height;

				while(length--)
					*data++ = ~*data;
			}
			else
			{
				if(BitsPerPixel != 24)
				{
					width = 256;
					height = 1;
				}

				do_tachyon(data, width, height);
			}
		}
	
/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */
			
			smurf_struct->module_mode = M_DONEEXIT;
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


/* Holt sich den Helligkeitswert des Pixels, */
/* invertiert den und konvertiert zurÅck. */
/* l = r * 0,299 + g * 0,587 + b * 0,114 */
void do_tachyon(char *data, unsigned int width, unsigned int height)
{
	char *cliptab;
	char r, g, b;

	int r2, g2, b2;
	unsigned int x, y, bh, bl;

	long l, w;


#if DEBUG
	Cur_home();
#endif

/* wie schnell sind wir? */
/*	init_timer(); */

	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	w = width * 3;

	/* Cliptable aufbauen */
	cliptab = (char *)Malloc(127 + 256 + 127);
	memset(cliptab, 0x0, 127);
	y = 0;
	do
	{
		*(cliptab + 127 + y) = y;
	} while(++y < 256);
	memset(cliptab + 383, 0xff, 127);

	y = 0;
	while(y++ < height)
	{
		if(!(y%bh))
		{
			busybox(bl);
			bl += 12;
		}

		x = width;
		while(x--)
		{
			r = *data;
			g = *(data + 1);
			b = *(data + 2);


#if DEBUG
			printf("r: %d, g: %d, b: %d\n", (int)r, (int)g, (int)b);
#endif

			l = (long)r * 1225L + (long)g * 2404L + (long)b * 467L;


			/* die eigentliche Funktion und Aufgabe des Moduls, */
			/* invertieren der Helligkeit, die 2048 sind 0,5 << 12 */
			/* zum Ausgleich der Nachkommastellen */
			l = 1048576L - (l << 1) + 2048L;


			r2 = (int)((((long)r << 12L) + l) >> 12L);
			r2 = cliptab[127 + r2];
/*
			if(r2 > 255)
				r2 = 255;
			else
				if(r2 < 0)
					r2 = 0;
*/
			g2 = (int)((((long)g << 12L) + l) >> 12L);
			g2 = cliptab[127 + g2];
/*
			if(g2 > 255)
				g2 = 255;
			else
				if(g2 < 0)
					g2 = 0;
*/
			b2 = (int)((((long)b << 12L) + l) >> 12L);
			b2 = cliptab[127 + b2];
/*
			if(b2 < 0)
				b2 = 0;
			else
				if(b2 > 255)
					b2 = 255;
*/
#if DEBUG
			printf(": %ld\n", l);
			printf("r2: %d, g2: %d, b2: %d\n", r2, g2, b2);
				printf("r: %d, g: %d, b: %d\n\n", (int)r, (int)g, (int)b);
			getch();
#endif

			*data++ = (char)r2;
			*data++ = (char)g2;
			*data++ = (char)b2;
		}

/*		tachyon_pixel(data, cliptab, width);
		data += w; */
	}

	Mfree(cliptab);

/* wie schnell waren wir? */
/*	printf("\n%lu", get_timer());
	getch(); */

	return;
} /* do_tachyon */