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
/*					Verrauschen-Modul						*/
/* Version 0.1  --  14.02.97								*/
/*	  24 Bit, grau und farbig								*/
/* Version 0.2  --  13.10.97								*/
/*	  Es werden nicht mehr nur positive Rauschwerte erzeugt.*/
/*	  Beschleunigung um Faktor 2,5							*/
/* Version 0.3  --  17.10.97								*/
/*	  FarbkanÑle unabhÑngig voneinander verrauschen und		*/
/*	  nochmalige Beschleunigung um Faktor 2 - 2,5 durch		*/
/*	  Cliptables											*/
/* Version 0.4  --  09.05.99								*/
/*	  Mehr kommentiert.										*/
/* Version 0.5  --  xx.xx.99								*/
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

void noise_grey(char *data, int width, int height, int amountgrey);
void noise_color(char *data, int width, int height, int amountr, int amountg, int amountb);

int (*busybox)(int pos);

MOD_INFO module_info = {"Noise",
						0x0040,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"StÑrke Grau",
						"StÑrke R",
						"StÑrke G",
						"StÑrke B",
						"grau",
						"farbig",
						"",
						"",
						"",
						"",
						"",
						"",
						0,100,
						0,100,
						0,100,
						0,100,
						0,10,
						0,10,
						0,10,
						0,10,
						20, 20, 20, 20,
						4, 3, 3, 3,
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
/*					Verrauschen						*/
/*		24 Bit										*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 BitsPerPixel, mode;
	
	int module_id, amountgrey, amountr, amountg, amountb;
	unsigned int width, height;


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

			busybox = smurf_struct->services->busybox;

			if(smurf_struct->check1 > smurf_struct->check2)
				mode = 0;
			else
				mode = 1;
			/* mit den Shiftings 5 Bit hoch und 6 Bit herunter */
			/* ergibt sich somit eine Skalierung von 100 auf 250 */
			amountgrey = (int)((int)smurf_struct->slide1 * 5);
			amountr = (int)((int)smurf_struct->slide2 * 5);
			amountg = (int)((int)smurf_struct->slide3 * 5);
			amountb = (int)((int)smurf_struct->slide4 * 5);

			/* Zufallsgenerator zufÑllig initialisieren */
			srand((unsigned int)time(NULL));

			BitsPerPixel = smurf_struct->smurf_pic->depth;
	
			data = smurf_struct->smurf_pic->pic_data;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			if(mode == 0)
				noise_grey(data, width, height, amountgrey);
			else
				noise_color(data, width, height, amountr, amountg, amountb);

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


void noise_grey(char *data, int width, int height, int amountgrey)
{
	char cliptable_grey[512],
		 newpix;
	
	int plusval;
	unsigned int x, y, bh, bl, i, j, k;


	/* amountgrey auf Wert zwischen 0 und 16000 hochschieben */
	amountgrey <<= 5;

	/* k auf Wert zwischen 0 und 250 runterschieben */
	k = amountgrey >> 7;

	/* erstellen der Cliptable */
	for(i = 0; i < k; i++)
		cliptable_grey[i] = 0;
	for(k += 255, j = 0; i < k; i++, j++)
		cliptable_grey[i] = j;
	for(k += amountgrey >> 7; i < k; i++)
		cliptable_grey[i] = 255;


	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	y = 0;
	do
	{
		if(!(y%bh))
		{
			busybox(bl);
			bl += 12;
		}

		x = 0;
		do
		{
			/* plusval aus dem Bereich von 0 bis 250 erzeugen. */
			/* Damit werden zwar die neuen Pixel nur um */
			/* +/- 125 bewegt, aber das langt IMHO. */
			plusval = (int)(random(amountgrey) >> 6);

			newpix = cliptable_grey[(int)*data + plusval];
			*data++ = newpix;

			newpix = cliptable_grey[(int)*data + plusval];
			*data++ = newpix;

			newpix = cliptable_grey[(int)*data + plusval];
			*data++ = newpix;
		} while(++x < width);
	} while(++y < height);

	return;
} /* noise_grey */


void noise_color(char *data, int width, int height, int amountr, int amountg, int amountb)
{
	char cliptable_r[512], cliptable_g[512], cliptable_b[512],
		 newpix;
	
	unsigned int x, y, bh, bl, i, j, k;


	amountr <<= 5;
	k = amountr >> 7;

	/* Erstellen der Cliptable fÅr Rot */
	for(i = 0; i < k; i++)
		cliptable_r[i] = 0;
	for(k += 255, j = 0; i < 255; i++, j++)
		cliptable_r[i] = j;
	for(k += amountr >> 7; i < k; i++)
		cliptable_r[i] = 255;


	amountg <<= 5;
	k = amountg >> 7;

	/* Erstellen der Cliptable fÅr GrÅn */
	for(i = 0; i < k; i++)
		cliptable_g[i] = 0;
	for(k += 255, j = 0; i < 255; i++, j++)
		cliptable_g[i] = j;
	for(k += amountg >> 7; i < k; i++)
		cliptable_g[i] = 255;


	amountb <<= 5;
	k = amountb >> 7;

	/* Erstellen der Cliptable fÅr Blau */
	for(i = 0; i < k; i++)
		cliptable_b[i] = 0;
	for(k += 255, j = 0; i < 255; i++, j++)
		cliptable_b[i] = j;
	for(k += amountb >> 7; i < k; i++)
		cliptable_b[i] = 255;


	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	y = 0;
	do
	{
		if(!(y%bh))
		{
			busybox(bl);
			bl += 12;
		}

		x = 0;
		do
		{
			newpix = cliptable_r[*data + (int)(random(amountr) >> 6)];
			*data++ = (char)newpix;

			newpix = cliptable_g[*data + (int)(random(amountg) >> 6)];
			*data++ = (char)newpix;

			newpix = cliptable_b[*data + (int)(random(amountb) >> 6)];
			*data++ = (char)newpix;
		} while(++x < width);
	} while(++y < height);

	return;
} /* noise_color */