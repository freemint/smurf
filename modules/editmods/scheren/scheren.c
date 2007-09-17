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

/*
	float xangle   -90 - 90
	float yangle   -90 - 90
	float xshearfac   0 - 1
	float yshearfac   0 - 1

	float new0   width * yshearfac   -> bei Breite von 32768 = 440
	bzw.
	float new0   height * xshearfac   -> bei Breite von 32768 = 440

	unsigned long fracnew0   Nachkomma new0 * 4096  -> 0 - 4096
	unsigned long omfracnew0   4096 - fracnew0      -> 0 - 4096

	Produkt 1   max-255 * fracnew0		-> 1044480
	Produkt 2   max-255 * omfracnew0	-> 1044480
*/

/* =========================================================*/
/*						Scheren-Modul						*/
/* Version 0.1  --  11.03.96								*/
/*	  24 Bit, horizontal mit und ohne Antialiasing			*/
/* Version 0.2  --  11.03.96								*/
/*	  8 und 16 Bit, horizontal ohne Antialiasing			*/
/* Version 0.3  --  13.03.96								*/
/*	  8, 16 und 24 Bit, vertikal ohne Antialiasing			*/
/* Version 0.4  --  14.03.96								*/
/*	  24 Bit, vertikal mit Antialiasing						*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include <math.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <..\..\..\demolib.h>

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*busybox)(int pos);

void xshear_normal(char *buffer, char *ziel, unsigned int width, unsigned int height, unsigned int newwidth, float xangle, float xshearfac, char bytes);
void xshear_antialias(char *buffer, char *ziel, unsigned int width, unsigned int height, unsigned int newwidth, float xangle, float xshearfac);
void yshear_normal(char *buffer, char *ziel, unsigned int width, unsigned int height, float yangle, float yshearfac, char bytes);
void yshear_antialias(char *buffer, char *ziel, unsigned int width, unsigned int height, float yangle, float yshearfac);

void scheren_xschleife(long *par, int width);
void scheren_yschleife(long *par, int height);

#define M_PI    3.14159265358979323846
#define abso(x)	(x < 0 ? -x : x)

SERVICE_FUNCTIONS *Services;

MOD_INFO module_info = {"Scheren",
						0x0040,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"",
						"",
						"",
						"",
						"interpolierend",
						"",
						"",
						"",
						"X-Winkel",
						"Y-Winkel",
						"",
						"",
						0,64,
						0,64,
						0,64,
						0,64,
						-90,90,
						-90,90,
						0,10,
						0,10,
						0, 0, 0, 0,
						1, 0, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						8, 16, 24, 0, 0,
						0, 0, 0,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Scheren-Modul					*/
/*	Schert sich einen Dreck um das Bild und das		*/
/*	Bild um einen anzugebenden Winkel von +90 bis	*/
/*	-90 Grad.										*/
/*	Bei Werten nahe 90 Grad ist das Bild allerdings	*/
/*	sehr h„žlich.									*/
/*		24 Bit										*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel,
		 BitsPerPixel, antialias;

	int module_id;
	unsigned int width, height, newwidth, newheight;

	float xangle, yangle, xshearfac, yshearfac;


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
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			busybox = smurf_struct->services->busybox;

/* wie schnell sind wir? */
/*	init_timer(); */

			antialias = smurf_struct->check1;
			xangle = (float)smurf_struct->edit1;
			yangle = (float)smurf_struct->edit2;

			if(!xangle && !yangle)
			{
				smurf_struct->module_mode = M_WAITING;
				return;
			}

			BitsPerPixel = smurf_struct->smurf_pic->depth;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			buffer = smurf_struct->smurf_pic->pic_data;

			if(antialias && BitsPerPixel != 24)
			{
				smurf_struct->module_mode = M_WAITING;
				return;
			}

			if(xangle)
			{
				smurf_struct->services->reset_busybox(0, "X zum Teufel scheren");

				/* Winkel zwischen -90 und +90 Grad */
				xangle = xangle * M_PI / 180.0;     /* nach Radiant konvertieren */
				xshearfac = abso(tan(xangle));

				newwidth = (unsigned int)(height * xshearfac + width + 0.999999);    /* Anzahl neuer Spalten berechnen */

				if((ziel = SMalloc(((unsigned long)newwidth * (unsigned long)height * BitsPerPixel) >> 3)) == 0)
				{
					smurf_struct->module_mode = M_MEMORY;
					return;
				}
				memset(ziel, 0x0, ((unsigned long)newwidth * (unsigned long)height * BitsPerPixel) >> 3);

				if(antialias)
					xshear_antialias(buffer, ziel, width, height, newwidth, xangle, xshearfac);
				else
					xshear_normal(buffer, ziel, width, height, newwidth, xangle, xshearfac, BitsPerPixel >> 3);

				SMfree(buffer);

				smurf_struct->smurf_pic->pic_width = newwidth;

				buffer = ziel;			/* damit y-shear gleiche Bedingungen vorfindet, */
				width = newwidth;		/* egal ob x-shear ausgefhrt wurde, oder nicht */
			}


			if(yangle)
			{
				smurf_struct->services->reset_busybox(0, "Y zum Teufel scheren");

				/* Winkel zwischen -90 und +90 Grad */
				yangle = yangle * M_PI / 180.0;     /* nach Radiant konvertieren */
				yshearfac = abso(tan(yangle));

				newheight = (unsigned int)(width * yshearfac + height + 0.999999);    /* Anzahl neuer Zeilen berechnen */

				if((ziel = SMalloc(((unsigned long)width * (unsigned long)newheight * BitsPerPixel) >> 3)) == 0)
				{
					smurf_struct->module_mode = M_MEMORY;
					return;
				}
				memset(ziel, 0x0, ((unsigned long)width * (unsigned long)newheight * BitsPerPixel) >> 3);

				if(antialias)
					yshear_antialias(buffer, ziel, width, height, yangle, yshearfac);
				else
					yshear_normal(buffer, ziel, width, height, yangle, yshearfac, BitsPerPixel >> 3);

				SMfree(buffer);

				smurf_struct->smurf_pic->pic_height = newheight;
			}

			smurf_struct->smurf_pic->pic_data = ziel;


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


/* schert horizontal ohne Antialiasing */
void xshear_normal(char *buffer, char *ziel, unsigned int width, unsigned int height, unsigned int newwidth, float xangle, float xshearfac, char bytes)
{
	unsigned int y, intnew0, bh, bl;

	unsigned long realwidth, realnewwidth;


	realwidth = (long)width * bytes;
	realnewwidth = (long)newwidth * bytes;

	if((bh = height / 8) == 0)	 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	y = 0;
	do
	{
		if(!(y%bh))
		{
			busybox(bl);
			bl += 16;
		}

		if(xangle > 0.0)
			intnew0 = (unsigned int)(y * xshearfac);
		else
			intnew0 = (unsigned int)((height - y) * xshearfac);

		memcpy(ziel + (intnew0 * bytes), buffer, realwidth);
		buffer += realwidth;
		ziel += realnewwidth;
	} while(++y < height);

	return;
} /* xshear_normal */


/* schert horizontal mit Antialiasing */
void xshear_antialias(char *buffer, char *ziel, unsigned int width, unsigned int height, unsigned int newwidth, float xangle, float xshearfac)
{
	unsigned int y, bh, bl, intnew0, fracnew0, omfracnew0;

	long par[4];
	unsigned long realwidth, realnewwidth;

	float new0;


	realwidth = (long)width * 3L;
	realnewwidth = (long)newwidth * 3L;

	if((bh = height / 8) == 0)	 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */	

	y = 0;
	do
	{
		if(!(y%bh))
		{
			busybox(bl);
			bl += 16;
		}

		if(xangle > 0.0)
			new0 = y * xshearfac;
		else
			new0 = (height - y) * xshearfac;

		intnew0 = (unsigned int)new0;

/* muž float bleiben, da sonst fracnew0 viel zu ungenau wird :-( */
		fracnew0 = (new0 - intnew0) * 4096;
		omfracnew0 = 4096 - fracnew0;

		par[0] = (long)fracnew0;
		par[1] = (long)omfracnew0;
		par[2] = (long)(ziel + (intnew0 * 3));
		par[3] = (long)buffer;
		scheren_xschleife(par, width);

		buffer += realwidth;
		ziel += realnewwidth;
	} while(++y < height);

	return;
} /* xshear_antialias */


/* schert vertikal ohne Antialiasing */
void yshear_normal(char *buffer, char *ziel, unsigned int width, unsigned int height, float yangle, float yshearfac, char bytes)
{
	char *obuffer, *oziel;

	unsigned int x, y, bh, bl, intnew0;

	unsigned long realwidth, off;


	obuffer = buffer;
	oziel = ziel;

	realwidth = (long)width * bytes;

	if((bh = width / 10) == 0)	 	/* busy-height */
		bh = width;
	bl = 0;							/* busy-length */

	x = 0;
	do
	{
		buffer = obuffer + x * bytes;
		ziel = oziel + x * bytes;

		if(!(x%bh))
		{
			busybox(bl);
			bl += 12;
		}

		if(yangle > 0)
			intnew0 = (unsigned int)((width - x) * yshearfac);
		else
			intnew0 = (unsigned int)(x * yshearfac);

		off = intnew0 * realwidth;
		y = 0;
		do
		{
			*(ziel + off) = *buffer;
			if(bytes > 1)
			{
				*(ziel + off + 1) = *(buffer + 1);
				if(bytes > 2)
					*(ziel + off + 2) = *(buffer + 2);
			}

			buffer += realwidth;
			off += realwidth;
		} while(++y < height);
	} while(++x < width);

	return;
} /* yshear_normal */


/* schert vertikal mit Antialiasing */
void yshear_antialias(char *buffer, char *ziel, unsigned int width, unsigned int height, float yangle, float yshearfac)
{
	char *obuffer, *oziel;

	unsigned int x, bh, bl, intnew0, fracnew0, omfracnew0;

	long par[5];
	unsigned long realwidth, off;

	float new0;


	obuffer = buffer;
	oziel = ziel;

	realwidth = (long)width * 3;

	if((bh = width / 10) == 0)	 	/* busy-height */
		bh = width;
	bl = 0;							/* busy-length */

	x = 0;
	do
	{
		buffer = obuffer + x * 3;
		ziel = oziel + x * 3;

		if(!(x%bh))
		{
			busybox(bl);
			bl += 12;
		}

		if(yangle > 0)
			new0 = (width - x) * yshearfac;
		else
			new0 = x * yshearfac;

		intnew0 = (unsigned int)new0;

/* muž float bleiben, da sonst fracnew0 viel zu ungenau :-( */
		fracnew0 = (new0 - intnew0) * 4096L;
		omfracnew0 = 4096 - fracnew0;

		off = intnew0 * realwidth;

		par[0] = (long)fracnew0;
		par[1] = (long)omfracnew0;
		par[2] = (long)realwidth;
		par[3] = (long)(ziel + off);
		par[4] = (long)buffer;
		scheren_yschleife(par, height);
	} while(++x < width);

	return;
} /* yshear_antialias */