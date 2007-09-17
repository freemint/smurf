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

/* Problem: Wie sorge ich dafÅr, daû bei Interpolation */
/* nur 24 Bit Bilder kommen? */

/* =========================================================*/
/*						Aufpixel-Modul						*/
/* Version 0.1  --  09.03.96 - 15.03.96						*/
/*	  24 Bit												*/
/* Version 0.2  --  16.03.96								*/
/*	  8 (pixelpacked), 16 Bit								*/
/* Version 0.3  --  26.03.96								*/
/*	  Bei 16 und 24 bitigen Bildern war noch ein ganz		*/
/*	  bescheuerter Fehler in der Hauptschleife				*/
/* Version 0.4  --  25.02.97								*/
/*	  Optional kann interpoliert werden (nur 24 Bit).		*/
/*	  Den Versuch, Zeit durch Ausbau der Divisionen zu		*/
/*	  kann ich vergessen, es wird nicht schneller ...		*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <..\..\..\demolib.h>

void normalpixel(char *data, unsigned int width, unsigned int height, char xpix, char ypix, char BitsPerPixel);
void interpolpixel(char *data, unsigned int width, unsigned int height, char xpix, char ypix, SERVICE_FUNCTIONS *service);
void do_block(unsigned char *data, unsigned long *pixtab, int blocksize);

MOD_INFO module_info = {"Aufpixeln",
						0x0040,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"",
						"",
						"",
						"",
						"interpolierend (24 Bit)",
						"",
						"",
						"",
						"Breite",
						"Hîhe",
						"",
						"",
						0,64,
						0,64,
						0,64,
						0,64,
						2,16,
						2,16,
						0,10,
						0,10,
						0, 0, 0, 0,
						1, 0, 0, 0,
						3, 3, 0, 0,
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
/*		Pixeliser (in Dankbarkeit an den			*/
/*              groûen Maler Studio Photo)			*/
/*		8 (pixelpacked), 16 und 24 Bit				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data, dummy[3], pixstr[15],
		 BitsPerPixel, xpix, ypix, interpol;

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

/* Einstellformular wurde mit START verlassen - Funktion ausfÅhren */
	else
		if(smurf_struct->module_mode == MEXEC)
		{
/* wie schnell sind wir? */
/*	init_timer(); */

			interpol = smurf_struct->check1;
			xpix = (char)smurf_struct->edit1;
			ypix = (char)smurf_struct->edit2;

			BitsPerPixel = smurf_struct->smurf_pic->depth;
		
			data = smurf_struct->smurf_pic->pic_data;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			strcpy(pixstr, "Aufpixeln ");
			strcat(pixstr, itoa(xpix, dummy, 10));
			strcat(pixstr, "x");
			strcat(pixstr, itoa(ypix, dummy, 10));
			smurf_struct->services->reset_busybox(0, pixstr);

			if(interpol)				/* interpolierend */
				if(BitsPerPixel != 24)
				{
					smurf_struct->module_mode = M_WAITING;
					return;
				}
				else
					interpolpixel(data, width, height, xpix, ypix, smurf_struct->services);
			else						/* Normalmodus */
				normalpixel(data, width, height, xpix - 1, ypix - 1, BitsPerPixel);


/* wie schnell waren wir? */
/*	printf("%lu", get_timer());
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



/* Macht Blîcke von xpix * ypix Grîûe. */
/* Durch die fehlende Interpolation sieht es bei Blîcken */
/* > 4*4 schon ziemlich hÑûlich aus, dafÅr ist es aber schnell. */
void normalpixel(char *data, unsigned int width, unsigned int height, char xpix, char ypix, char BitsPerPixel)
{
	char byte, pix, pix2r;

	unsigned int x, y, pix2;

	unsigned long realwidth;

	pix2 = width / (xpix + 1);
	pix2r = width%(xpix + 1);
	/* -1 damit bei einem Pixel nicht auf Block gemacht wird */
	if(pix2r > 0)
		pix2r--;

	byte = BitsPerPixel >> 3;

	realwidth = (unsigned long)width * (unsigned long)byte;

	y = 0; /* height */
	do      
	{
		x = 0; /* width */
		while(x++ < pix2) /* x */		/* horizontal aufpixeln ... */
		{
			pix = xpix;					/* ... aber nur ganze Blîcke */
			while(pix--) /* pix */
			{
				*(data++ + byte) = *data;
				if(byte > 1)			/* Kaskade, damit 8, 16 und 24 Bit */
				{						/* zusammen abgearbeitet werden kînnen */
					*(data++ + byte) = *data;
					if(byte > 2)
						*(data++ + byte) = *data;
				}
			}

			/* und Åbers Blockende hieven */
			data += byte;
		}

		if(pix2r)
		{
			pix = pix2r;
			while(pix--)					/* ... und am Zeilenende nur teilweisen Block */
			{
				*(data++ + byte) = *data;
				if(byte > 1)
				{
					*(data++ + byte) = *data;
					if(byte > 2)
						*(data++ + byte) = *data;
				}
			}

			data += byte;					/* Åber Blockende hieven */
		}

		pix = ypix;
		y++;
		while(pix-- && y < height)		/* vertikal "aufpixeln" */
		{
			memcpy(data, data - realwidth, realwidth);
			data += realwidth;
			y++;
		}
	} while(y < height); /* y */

	return;
} /* normalpixel */



/* Macht Blîcke von xpix * ypix Grîûe und interpoliert dabei. */
/* Ein Problem ist noch die untere Bildkante, das ist nicht schîn gelîst. */
void interpolpixel(char *data, unsigned int width, unsigned int height, char xpix, char ypix, SERVICE_FUNCTIONS *services)
{
	char xcount, ycount, xpix2r, ypix2r, bwidth, bwidth2;

	unsigned int x, y, blocksize, blocksize2, xpix2, ypix2, bh, bl;

	unsigned long *pixtab, *pixtab2,
			 	  lines, temp;


	xpix2 = width / xpix;
	xpix2r = width%xpix;
	ypix2 = height / ypix;
	ypix2r = height%ypix;

	bwidth = xpix * 3;						/* BlocklÑnge ganzer Blîcke */
	bwidth2 = xpix2r * 3;					/* Blockbreite restlicher Blîcke */

/* *** */

/* FÅr Blîcke mit voller Hîhe */

	lines = (unsigned long)(ypix - 1) * (unsigned long)width * 3L;

	blocksize = (unsigned int)xpix * (unsigned int)ypix;
	pixtab = (unsigned long *)calloc(1, blocksize * 4);

	/* Tabelle mit Pixeloffsets des ganzen Blocks anlegen */
	x = 0;
	ycount = 0;
	while(ycount < ypix)
	{
		temp = ycount * width * 3;

		xcount = 0;
		while(xcount < xpix)
		{
			pixtab[x++] = temp + xcount * 3;
			xcount++;
		}

		ycount++;
	}


	blocksize2 = (unsigned int)xpix2r * (unsigned int)ypix;
	pixtab2 = (unsigned long *)calloc(1, blocksize2 * 4);

	/* Tabelle mit Pixeloffsets des restlichen Blocks anlegen */
	x = 0;
	ycount = 0;
	while(ycount < ypix)
	{
		temp = ycount * width * 3;

		xcount = 0;
		while(xcount < xpix2r)
		{
			pixtab2[x++] = temp + xcount * 3;
			xcount++;
		}

		ycount++;
	}

	if((bh = ypix2 / 10) == 0)		/* busy-height */
		bh = ypix2;
	bl = 0;							/* busy-length */

	y = 0;
	while(y++ < ypix2)
	{
		if(!(y%bh))
		{
			services->busybox(bl);
			bl += 12;
		}

		x = 0; /* width */
		while(x++ < xpix2) /* x */		/* horizontal aufpixeln ... */
		{								/* ... aber nur ganze Blîcke */
			do_block(data, pixtab, blocksize);

			/* Einen Block weiter */
			data += bwidth;
		}

		if(xpix2r)						/* ... und am Zeilenende nur teilweisen Block */
		{
			do_block(data, pixtab2, blocksize2);

			/* den Restblock weiter */
			data += bwidth2;
		} /* if xpix2r */

		data += lines;				/* Blockhîhe Zeilen - 1 Åberspringen */
	} /* y */

/* *** */

/* FÅr Blîcke mit verringerter Hîhe */

	if(ypix2r)
	{
		memset(pixtab, 0x0, blocksize * 4);		/* Block in alter Grîûe initialisieren */
		blocksize = (unsigned int)xpix * (unsigned int)ypix2r;

		/* Tabelle mit Pixeloffsets des ganzen Blocks anlegen */
		x = 0;
		ycount = 0;
		while(ycount < ypix2r)
		{
			temp = ycount * width * 3;

			xcount = 0;
			while(xcount < xpix)
			{
				pixtab[x++] = temp + xcount * 3;
				xcount++;
			}

			ycount++;
		}


		memset(pixtab2, 0x0, blocksize2 * 4);	/* Block in alter Grîûe initialisieren */
		blocksize2 = (unsigned int)xpix2r * (unsigned int)ypix2r;

		/* Tabelle mit Pixeloffsets des restlichen Blocks anlegen */
		x = 0;
		ycount = 0;
		while(ycount < ypix2r)
		{
			temp = ycount * width * 3;

			xcount = 0;
			while(xcount < xpix2r)
			{
				pixtab2[x++] = temp + xcount * 3;
				xcount++;
			}

			ycount++;
		}


		x = 0; /* width */
		while(x++ < xpix2) /* x */		/* horizontal aufpixeln ... */
		{								/* ... aber nur ganze Blîcke */
			do_block(data, pixtab, blocksize);

			/* Einen Block weiter */
			data += bwidth;
		} /* Blockschleife */

		if(xpix2r)				/* ... und am Zeilenende nur teilweisen Block */
		{
			do_block(data, pixtab2, blocksize2);

			/* den Restblock weiter */
			data += bwidth2;
		} /* if xpix2r */
	} /* if ypix2r */

	free(pixtab);
	free(pixtab2);

	return;
} /* interpolpixel */