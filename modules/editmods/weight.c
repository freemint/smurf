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
/*						Weight of Color						*/
/* Version 0.1  --  06.11.96								*/
/*	  1-8, 16, 24 Bit										*/
/* Version 0.2  --  23.02.97								*/
/*	  Fuck gebixt, bei 16 Bit wurde aus data und nicht		*/
/*	  data16 ausgelesen -> schwarzes Bild					*/
/* Version 0.3  --  30.07.97								*/
/*	  Ein Wuxel ward mir noch weggeschlÅpft. Die Tabellen	*/
/*	  mÅssen latÅrnich immer bestÅckt werden.				*/
/* Version 0.4  --  18.08.98								*/
/*	  Da auch énderungen an monochromen Bildern auffallen	*/
/*	  mÅssen, dort aber eine geÑnderte Palette nichts be-	*/
/*	  wirkt, wird jetzt 2 Bit als kleinste Farbtiefe fest-	*/
/*	  gelegt.												*/
/* =========================================================*/

#ifdef GERMAN
#define TEXT1 "Weight of Color"
#define TEXT2 "Rot"
#define TEXT3 "GrÅn"
#define TEXT4 "Blau"
#else
#ifdef ENGLISH
#define TEXT1 "Weight of Color"
#define TEXT2 "Red"
#define TEXT3 "Green"
#define TEXT4 "Blue"
#else
#ifdef FRENCH
#define TEXT1 "Weight of Color"
#define TEXT2 "Rouge"
#define TEXT3 "Vert"
#define TEXT4 "Bleu"
#else
#error "Keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include "..\..\src\lib\demolib.h"

MOD_INFO module_info = {TEXT1,
						0x0040,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						TEXT2,
						TEXT3,
						TEXT4,
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						-255,255,
						-255,255,
						-255,255,
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


MOD_ABILITY  module_ability = {
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
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Weight of Color					*/
/*		1-8, 16 und 24 Bit							*/
/* Und noch ein Modul mit so einem schînen Namen.	*/
/* Es kann hiermit die Gewichtung des Rot-, GrÅn-	*/
/* und Blauanteils verÑndert werden.				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 r, g, b, BitsPerPixel;

	int module_id, transr[256], transg[256], transb[256],
		i, k, rweight, gweight, bweight;
	unsigned int *data16, width, height;

	unsigned long length;


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

			rweight = smurf_struct->slide1;
			gweight = smurf_struct->slide2;
			bweight = smurf_struct->slide3;

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			i = 0;
			do
			{
				k = i + rweight;
				if(k < 0)
					transr[i] = 0;
				else
					if(k > 255)
						transr[i] = 255;
					else
						transr[i] = k;
			} while(++i < 256);

			i = 0;
			do
			{
				k = i + gweight;
				if(k < 0)
					transg[i] = 0;
				else
					if(k > 255)
						transg[i] = 255;
					else
						transg[i] = k;
			} while(++i < 256);

			i = 0;
			do
			{
				k = i + bweight;
				if(k < 0)
					transb[i] = 0;
				else
					if(k > 255)
						transb[i] = 255;
					else
						transb[i] = k;
			} while(++i < 256);


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

				do
				{
					*data++ = transr[*data];
					*data++ = transg[*data];
					*data++ = transb[*data];
				} while(--length);
			}
			else
			{
				data16 = (unsigned int *)smurf_struct->smurf_pic->pic_data;

				length = (unsigned long)width * (unsigned long)height;

				do
				{
					r = transr[(*data16 & 0xf800) >> 8];
					g = transg[(*data16 & 0x7e0) >> 3];
					b = transb[(*data16 & 0x1f) << 3];

					*data16++ = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
				} while(--length);
			}

/* wie schnell waren wir? */
/*	printf("\n%lu\n", get_timer());
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