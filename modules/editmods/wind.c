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
/*							Wind-Modul						*/
/* Version 0.1  --  22.09.97								*/
/*	  24 Bit, eine St„rke und nur von links					*/
/* Version 0.2  --  29.09.97								*/
/*	  Nun auch von rechts und in beiden Richtungen in zwei	*/
/*	  St„rken.												*/
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

#define WIND	1
#define STORM	2
#define LINKS	1
#define RECHTS	2

int wind_left(char *data, unsigned int width, unsigned int height);
int wind_right(char *data, unsigned int width, unsigned int height, char BitsPerPixel);
int storm_left(char *data, unsigned int width, unsigned int height);
int storm_right(char *data, unsigned int width, unsigned int height, char BitsPerPixel);

MOD_INFO module_info = {"Wind",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"",
						"",
						"",
						"",
						"Wind von links",
						"Wind von rechts",
						"Sturm von links",
						"Sturm von rechts",
						"",
						"",
						"",
						"",
						0,64,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						2, 3, 3, 3,
						0, 0, 0, 0,
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
/*						Wind						*/
/*	 Macht m„chtig Wind um nichts, s. Photoshop		*/
/*		24 Bit 										*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 BitsPerPixel;
	
	int module_id;
	unsigned int width, height;

	
	module_id = smurf_struct->module_number;

	/* Wenn das Modul zum ersten Mal gestartet wurde, */
	/* ein Einstellformular anfordern....             */
	if(smurf_struct->module_mode == MSTART)
	{
		smurf_struct->services->f_module_prefs(&module_info, module_id);
		smurf_struct->module_mode = M_WAITING;
		return;
	}
	/* Einstellformular wurde mit START verlassen - Funktion ausfhren */
	else
		if(smurf_struct->module_mode == MEXEC)
		{
			data = smurf_struct->smurf_pic->pic_data;

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;


/* wie schnell sind wir? */
/*	init_timer(); */

			/* Zufallsgenerator zuf„llig initialisieren */
			srand((unsigned int)time(NULL));

			if(smurf_struct->check1)
				wind_left(data, width, height);
			else
				if(smurf_struct->check2)
					wind_right(data, width, height, BitsPerPixel);
				else
					if(smurf_struct->check3)
						storm_left(data, width, height);
					else
						storm_right(data, width, height, BitsPerPixel);

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


/* bl„st von links sanft durchs Bild */
int wind_left(char *data, unsigned int width, unsigned int height)
{
	char r, g, b, init_pix, this_pix;

	int intens;
	unsigned int x, y, daempfung;



	y = height;
	while(y--)
	{
		init_pix = (char)((((long)*data * 872L)
						 + ((long)*(data + 1) * 2930L)
						 + ((long)*(data + 2) * 296L)) >> 12);

		r = *data++;
		g = *data++;
		b = *data++;

		/* Wert fr ersten Start initialisieren */
		intens = 95;
		daempfung = (random(45) >> 1) + 3;

		this_pix = (char)((((long)*data * 872L)
						 + ((long)*(data + 1) * 2930L)
						 + ((long)*(data + 2) * 296L)) >> 12);

		x = width - 1;
		while(x--)
		{
			while(init_pix > this_pix && intens > 0 && x)
			{
				*data++ = (char)((r * intens + *data * (128 - intens)) >> 7);
				*data++ = (char)((g * intens + *data * (128 - intens)) >> 7);
				*data++ = (char)((b * intens + *data * (128 - intens)) >> 7);
				x--;

				intens -= daempfung;

				this_pix = (char)((((long)*data * 872L)
								 + ((long)*(data + 1) * 2930L)
								 + ((long)*(data + 2) * 296L)) >> 12);
			}

			data -= 3;
			x++;

			do
			{
				data += 3;
				x--;

				init_pix = this_pix;

				this_pix = (char)((((long)*(data + 3) * 872L)
								 + ((long)*(data + 4) * 2930L)
								 + ((long)*(data + 5) * 296L)) >> 12);
			} while(init_pix <= this_pix && x);

			r = *data++;
			g = *data++;
			b = *data++;

			/* Wert fr neuen Start initialisieren */
			intens = 95;
			daempfung = (random(45) >> 1) + 3;
		}
	}

	return(0);
} /* wind_left */


/* bl„st von rechts sanft durchs Bild */
int wind_right(char *data, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char r, g, b, init_pix, this_pix;

	int intens;
	unsigned int x, y, daempfung;


	/* auf Blaukomponente des letzten Bildpixels setzen */
	data += (((long)width * (long)height * BitsPerPixel) >> 3) - 1;

	y = height;
	while(y--)
	{
		init_pix = (char)((((long)*(data - 2) * 872L)
						 + ((long)*(data - 1) * 2930L)
						 + ((long)*data * 296L)) >> 12);

		b = *data--;
		g = *data--;
		r = *data--;

		/* Wert fr ersten Start initialisieren */
		intens = 95;
		daempfung = (random(45) >> 1) + 3;

		this_pix = (char)((((long)*(data - 2) * 871L)
						 + ((long)*(data - 1) * 2929L)
						 + ((long)*data * 295L)) >> 12);

		x = width - 1;
		while(x--)
		{
			intens -= daempfung;

			while(init_pix > this_pix && intens > 0 && x)
			{
				*data-- = (char)((b * intens + *data * (128 - intens)) >> 7);
				*data-- = (char)((g * intens + *data * (128 - intens)) >> 7);
				*data-- = (char)((r * intens + *data * (128 - intens)) >> 7);
				x--;

				intens -= daempfung;

				this_pix = (char)((((long)*(data - 2) * 872L)
								 + ((long)*(data - 1) * 2930L)
								 + ((long)*data * 296L)) >> 12);
			}

			data += 3;
			x++;

			do
			{
				data -= 3;
				x--;

				init_pix = this_pix;

				this_pix = (char)((((long)*(data - 5) * 872L)
								 + ((long)*(data - 4) * 2930L)
								 + ((long)*(data - 3) * 296L)) >> 12);
			} while(init_pix <= this_pix && x);

			b = *data--;
			g = *data--;
			r = *data--;

			/* Wert fr neuen Start initialisieren */
			intens = 95;
			daempfung = (random(45) >> 1) + 3;
		}
	}

	return(0);
} /* wind_right */


/* bl„st von links kr„ftig durchs Bild */
int storm_left(char *data, unsigned int width, unsigned int height)
{
	char r, g, b, init_pix, this_pix;

	int intens, intens2;
	unsigned int x, y, daempfung;



	y = height;
	while(y--)
	{
		init_pix = (char)((((long)*data * 872L)
						 + ((long)*(data + 1) * 2930L)
						 + ((long)*(data + 2) * 296L)) >> 12);

		r = *data++;
		g = *data++;
		b = *data++;

		/* Wert fr ersten Start initialisieren */
		intens2 = intens = 105;
		daempfung = (random(45) >> 1) + 3;

		this_pix = (char)((((long)*data * 872L)
						 + ((long)*(data + 1) * 2930L)
						 + ((long)*(data + 2) * 296L)) >> 12);

		x = width - 1;
		while(x--)
		{
			while(init_pix > this_pix && intens > 0 && x)
			{
				*data++ = (char)((r * intens2 + *data * (128 - intens2)) >> 7);
				*data++ = (char)((g * intens2 + *data * (128 - intens2)) >> 7);
				*data++ = (char)((b * intens2 + *data * (128 - intens2)) >> 7);
				x--;

				intens -= daempfung;

				this_pix = (char)((((long)*data * 872L)
								 + ((long)*(data + 1) * 2930L)
								 + ((long)*(data + 2) * 296L)) >> 12);
			}

			data -= 3;
			x++;

			do
			{
				data += 3;
				x--;

				init_pix = this_pix;

				this_pix = (char)((((long)*(data + 3) * 872L)
								 + ((long)*(data + 4) * 2930L)
								 + ((long)*(data + 5) * 296L)) >> 12);
			} while(init_pix <= this_pix && x);

			r = *data++;
			g = *data++;
			b = *data++;

			/* Wert fr neuen Start initialisieren */
			intens2 = intens = 105;
			daempfung = (random(45) >> 1) + 3;
		}
	}

	return(0);
} /* storm_left */


/* bl„st von rechts kr„ftig durchs Bild */
int storm_right(char *data, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char r, g, b, init_pix, this_pix;

	int intens, intens2;
	unsigned int x, y, daempfung;


	/* auf Blaukomponente des letzten Bildpixels setzen */
	data += (((long)width * (long)height * BitsPerPixel) >> 3) - 1;

	y = height;
	while(y--)
	{
		init_pix = (char)((((long)*(data - 2) * 872L)
						 + ((long)*(data - 1) * 2930L)
						 + ((long)*data * 296L)) >> 12);

		b = *data--;
		g = *data--;
		r = *data--;

		/* Wert fr ersten Start initialisieren */
		intens2 = intens = 105;
		daempfung = (random(45) >> 1) + 3;

		this_pix = (char)((((long)*(data - 2) * 872L)
						 + ((long)*(data - 1) * 2930L)
						 + ((long)*data * 296L)) >> 12);

		x = width - 1;
		while(x--)
		{
			intens -= daempfung;

			while(init_pix > this_pix && intens > 0 && x)
			{
				*data-- = (char)((b * intens2 + *data * (128 - intens2)) >> 7);
				*data-- = (char)((g * intens2 + *data * (128 - intens2)) >> 7);
				*data-- = (char)((r * intens2 + *data * (128 - intens2)) >> 7);
				x--;

				intens -= daempfung;

				this_pix = (char)((((long)*(data - 2) * 872L)
								 + ((long)*(data - 1) * 2930L)
								 + ((long)*data * 296L)) >> 12);
			}

			data += 3;
			x++;

			do
			{
				data -= 3;
				x--;

				init_pix = this_pix;

				this_pix = (char)((((long)*(data - 5) * 872L)
								 + ((long)*(data - 4) * 2930L)
								 + ((long)*(data - 3) * 296L)) >> 12);
			} while(init_pix <= this_pix && x);

			b = *data--;
			g = *data--;
			r = *data--;

			/* Wert fr neuen Start initialisieren */
			intens2 = intens = 105;
			daempfung = (random(45) >> 1) + 3;
		}
	}

	return(0);
} /* storm_right */