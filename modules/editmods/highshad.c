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
/*					Highlight/Shadow-Modul					*/
/* Version 0.1  --  21.10.97								*/
/*	  1-8, 16 und 24 Bit									*/
/* Version 0.2  --  13.11.97								*/
/*	  Sliderwertebereich berichtigt und Shadow-Berechnung	*/
/*	  vollkommen umgestellt. šberl„ufe sollten nun der Ver-	*/
/*	  gangenheit angeh”ren.									*/
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

MOD_INFO module_info = {"Highlight/Shadow",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"Highlight",
						"Shadow",
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
						0, 100,
						0, 100,
						0, 64,
						0, 64,
						0, 10,
						0, 10,
						0, 10,
						0, 10,
						100, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						2, 4, 7, 8, 16,
						24, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						0
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				Highlight/Shadow-Modul				*/
/*		1-8, 16 und 24 Bit							*/
/*	Die Ver„nderung des Highlight setzt die Hellig-	*/
/*	keit der Pixel ber dem Schwellwert auf 255 und	*/
/*	erh”ht die aller anderen leicht.				*/
/*	Beim Shadow das Gleiche andersherum.			*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/

void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data, highlighttab[256], shadowtab[256],
		 BitsPerPixel, slidvalh, slidvals, r, g, b, bereich;

	int module_id, i, val;
	unsigned int *data16, width, height;

	long sval;
	unsigned long length, hval;


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
			slidvalh = (int)smurf_struct->slide1 * 255 / 100;
			slidvals = (int)smurf_struct->slide2 * 255 / 100;
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			bereich = 255 - slidvals;

			hval = (long)(255 - slidvalh) * 100 / slidvalh + 1;
			i = 0;
			do 
			{
				val = i + i * hval / 100;
				if(val > 255)
					val = 255;
				highlighttab[i] = val;

				sval = (bereich - (i - slidvals)) * 100 / (bereich + 10);
				val = i - i * sval / 100;
				if(val < 0)
					val = 0;
				shadowtab[i] = val;
			} while(++i < 255);

			if(BitsPerPixel != 16)
			{
				if(BitsPerPixel == 24)
				{
					width = smurf_struct->smurf_pic->pic_width;
					height = smurf_struct->smurf_pic->pic_height;
					data = smurf_struct->smurf_pic->pic_data;

					length = (unsigned long)width * (unsigned long)height;
				}
				else
				{
					data = smurf_struct->smurf_pic->palette;
					length = 256L;
				}

				if(slidvalh < 255 && slidvals > 0)
					do
					{
						/* Rot */
						if(*data >= slidvalh)
							*data++ = 255;
						else
						{
							*data = highlighttab[*data];
							if(*data <= slidvals)
								*data++ = 0;
							else
								*data++ = shadowtab[*data];
						}

						/* Grn */
						if(*data >= slidvalh)
							*data++ = 255;
						else
						{
							*data = highlighttab[*data];
							if(*data <= slidvals)
								*data++ = 0;
							else
								*data++ = shadowtab[*data];
						}

						/* Blau */
						if(*data >= slidvalh)
							*data++ = 255;
						else
						{
							*data = highlighttab[*data];
							if(*data <= slidvals)
								*data++ = 0;
							else
								*data++ = shadowtab[*data];
						}
					} while(--length);
				else
					do
					{
						if(slidvalh < 255)
						{
							/* Rot */
							if(*data >= slidvalh)
								*data++ = 255;
							else
								*data++ = highlighttab[*data];

							/* Grn */
							if(*data >= slidvalh)
								*data++ = 255;
							else
								*data++ = highlighttab[*data];

							/* Blau */
							if(*data >= slidvalh)
								*data++ = 255;
							else
								*data++ = highlighttab[*data];
						}
	
						if(slidvals > 0)
						{
							/* Rot */
							if(*data <= slidvals)
								*data++ = 0;
							else
								*data++ = shadowtab[*data];

							/* Grn */
							if(*data <= slidvals)
								*data++ = 0;
							else
								*data++ = shadowtab[*data];

							/* Blau */
							if(*data <= slidvals)
								*data++ = 0;
							else
								*data++ = shadowtab[*data];
						}
				} while(--length);
			} /* BitsPerPixel != 16? */
			else
				if(BitsPerPixel == 16)
				{
					width = smurf_struct->smurf_pic->pic_width;
					height = smurf_struct->smurf_pic->pic_height;
					data16 = (unsigned int *)smurf_struct->smurf_pic->pic_data;
			
					length = (unsigned long)width * (unsigned long)height;

					do
					{
						if(slidvalh < 255)
						{
							r = (char)((*data16 & 0xf800) >> 8);
							if(r >= slidvalh)
								r = 255;
							else
								r = highlighttab[*data];

							g = (char)((*data16 & 0x7e0) >> 3);
							if(g >= slidvalh)
								g = 255;
							else
								g = highlighttab[*data];

							b = (char)((*data16 & 0x1f) << 3);
							if(b >= slidvalh)
								b = 255;
							else
								b = highlighttab[*data];
						}

						if(slidvals > 0)
						{
							r = (char)((*data16 & 0xf800) >> 8);
							if(r <= slidvals)
								r = 0;
							else
								r = shadowtab[*data];

							g = (char)((*data16 & 0x7e0) >> 3);
							if(g <= slidvals)
								g = 0;
							else
								g = shadowtab[*data];

							b = (char)((*data16 & 0x1f) << 3);
							if(b <= slidvals)
								b = 0;
							else
								b = shadowtab[*data];
						}

						*data16++ = ((r & 0x00f8) << 8) | ((g & 0x00fc) << 3) | (b >> 3);
					} while(--length);
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