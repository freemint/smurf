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
/*						Helligkeits-Modul					*/
/* Version 0.1  --  14.03.97								*/
/*	  16 und 24 Bit											*/
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

MOD_INFO module_info = {"NTSC",
						0x0010,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"% abdunkeln",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						0,100,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						30,0,0,0,
						0,0,0,0,
						0,0,0,0,
						1
						};


MOD_ABILITY module_ability = {
						16, 24, 0, 0, 0,
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
/*						NTSC						*/
/* Courtesy Frank Neumanns NTSC-Filter fr PPM:		*/
/* This creates an effect similar to what I've once	*/
/* seen in the video clip 'You could be mine' by	*/
/* Guns'n'Roses. In the scene I'm talking about you	*/
/* can see John Connor on his motorbike, looking up	*/
/* from the water trench (?) he's standing in.		*/
/* While the camera pulls back, the image gets		*/
/* 'normal' by brightening up the alternate rows of	*/
/* it. I thought this would be an interesting effect*/
/* to try in MPEG. I did not yet check this out,	*/
/* however. Try for yourself.						*/
/*		16 und 24 Bit (prinzipbedingt)				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/

void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 BitsPerPixel, darken;

	int module_id, transtab[256];
	unsigned int *data16, i, x, y, width, height, val;


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
/* wie schnell sind wir? */
/*	init_timer(); */

			darken = (char)smurf_struct->slide1;
			darken = (char)(((long)darken * 255) / 100);

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			i = 0;
			do
			{
				transtab[i] = i - darken;
				if(transtab[i] < 0)
					transtab[i] = 0;
				else
					if(transtab[i] > 255)
						transtab[i] = 255;
			} while(++i < 256);

			if(BitsPerPixel == 24)
			{
				width = smurf_struct->smurf_pic->pic_width;
				height = smurf_struct->smurf_pic->pic_height;
				data = smurf_struct->smurf_pic->pic_data;

				y = 0;
				do
				{
					if(y%2)
					{
						x = 0;
						do
						{
							*data++ = transtab[*data];
							*data++ = transtab[*data];
							*data++ = transtab[*data];
						} while(++x < width);
					}
					else
						data += width * 3;
				} while(++y < height);
			} /* BitsPerPixel != 16? */
			else
				if(BitsPerPixel == 16)
				{
					width = smurf_struct->smurf_pic->pic_width;
					height = smurf_struct->smurf_pic->pic_height;
					data16 = (unsigned int *)smurf_struct->smurf_pic->pic_data;
			
					y = 0;
					do
					{
						if(y%2)
						{
							x = 0;
							do
							{
								val = *data16;
								*data16 = ((transtab[(val & 0xf800) >> 8] & 0x00f8) << 8);
								*data16 |= ((transtab[(val & 0x7e0) >> 3] & 0x00fc) << 3);
								*data16++ |= (transtab[(val & 0x1f) << 3] >> 3);
							} while(++x < width);
						}
						else
							data16 += width;
					} while(++y < height);
				}

/* wie schnell waren wir? */
/*	printf("\n%lu", get_timer());
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