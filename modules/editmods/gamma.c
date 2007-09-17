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
/*						Gammakorrektur-Modul				*/
/* Version 0.1  --  04.06.97								*/
/*	  24 Bit und Palettenbilder								*/
/* Version 0.2  --  22.09.97								*/
/*	  jetzt auch in 16 Bit									*/
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

MOD_INFO module_info = {"Gammakorrektur",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"Gamma R",
						"Gamma G",
						"Gamma B",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						1,40,
						1,40,
						1,40,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						10, 10, 10, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						4, 7, 8, 16, 24,
						0, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Gammakorrektur					*/
/*		4-8, 16 und 24 Bit 							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 r, g, b, BitsPerPixel;
	
	int module_id;
	unsigned int *data16, 
				 width, height, i;
	unsigned int redgammamap[256], grngammamap[256], blugammamap[256];
	
	unsigned long length;

	double gammared, gammagrn, gammablu;	


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

			gammared = (int)smurf_struct->slide1 / 10.0;
			gammagrn = (int)smurf_struct->slide2 / 10.0;
			gammablu = (int)smurf_struct->slide3 / 10.0;

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			for( i = 0; i < 256; i++ )
			{
		    	redgammamap[i] = (unsigned int)(0.5 + 255 * pow(i / 255.0, 1.0 / gammared));
		    	grngammamap[i] = (unsigned int)(0.5 + 255 * pow(i / 255.0, 1.0 / gammagrn));
		    	blugammamap[i] = (unsigned int)(0.5 + 255 * pow(i / 255.0, 1.0 / gammablu));
		    }
		
			if(BitsPerPixel != 16)
			{
				if(BitsPerPixel == 24)
				{
					data = smurf_struct->smurf_pic->pic_data;

					length = (unsigned long)width * (unsigned long)height;
				}
				else
				{
					data = smurf_struct->smurf_pic->palette;

					length = 256L;
				}

				while(length--)
				{
					*data++ = redgammamap[*data];
					*data++ = grngammamap[*data];
					*data++ = blugammamap[*data];
				}
			}
			else
				if(BitsPerPixel == 16)
				{
					data16 = smurf_struct->smurf_pic->pic_data;

					length = (unsigned long)width * (unsigned long)height;
	
					do
					{
						r = redgammamap[(*data16 & 0xf800) >> 8];
						g = grngammamap[(*data16 & 0x7e0) >> 3];
						b = blugammamap[(*data16 & 0x1f) << 3];

						*data16++ = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
					} while(--length);

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