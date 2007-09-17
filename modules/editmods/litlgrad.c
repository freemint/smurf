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
/* Version 0.1  --  30.12.95								*/
/*	  TC- und HC-Bilder										*/
/* Version 0.2  --  13.03.96								*/
/*	  Palettenbilder und TC-Bilder von BGR nach RGB			*/
/* Version 0.3  --  31.03.96								*/
/*	  Das war doch alles Scheiže. Erstens war der Ansatz	*/
/*	  falsch und zweitens langsam.							*/
/*															*/
/*						Kontrast-Modul						*/
/* Version 0.1  --  31.03.96								*/
/*	  Nur Kontrast und dann auch noch betrogener Spezialfall*/
/* Version 0.2  --  02.04.96								*/
/*	  Auf die allgemeine Geradengleichung umgestellt.		*/
/*															*/
/*				Helligkeits-Kontrast-Modul					*/
/* Version 0.1  --  02.04.96								*/
/*	  Weil es sich so ergeben hat, habe ich die beiden		*/
/*	  Module zusammengelegt.								*/
/* Version 0.2  --  03.05.96								*/
/*	  Die drei Farbkan„le k”nnen nun auch getrennt bearbei-	*/
/*	  tet werden.											*/
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

MOD_INFO module_info = {"Helligkeit-Kontrast",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"Helligkeit",
						"Kontrast",
						"",
						"",
						"rot",
						"grn",
						"blau",
						"",
						"",
						"",
						"",
						"",
						-255, 255,
						-128, 127,
						0, 64,
						0, 64,
						0, 10,
						0, 10,
						0, 10,
						0, 10,
						0, 0, 0, 0,
						1, 1, 1, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						1, 2, 4, 7, 8,
						16, 24, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						0
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Helligkeit-Kontrast				*/
/*		1-8, 16 und 24 Bit							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/

void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 BitsPerPixel, rc, gc, bc;

	int module_id;
	int slidvalb, slidvalk, i, abs, m1, m2, transtab[256];

	unsigned int *data16, width, height, t_pos, val;

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

/* Einstellformular wurde mit START verlassen - Funktion ausfhren */
	else 
		if(smurf_struct->module_mode == MEXEC)
		{
/* wie schnell sind wir? */
/*	init_timer(); */

			slidvalb = (int)smurf_struct->slide1;
			slidvalk = (int)smurf_struct->slide2;
			rc = (int)smurf_struct->check1;
			gc = (int)smurf_struct->check2;
			bc = (int)smurf_struct->check3;
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			if(rc != 1 && gc != 1 && bc != 1)
			{
				smurf_struct->module_mode = M_WAITING;
				return;
			}

			if(slidvalb != 0 || slidvalk != 0)
			{
				i = 0;
				t_pos = 0;
				if(slidvalk >= 0)
				{
					m2 = 256 - (2 * slidvalk);
					abs = slidvalk * 256 / m2;
					do
					{
						transtab[t_pos] = (unsigned int)i * 256 / m2 - abs + slidvalb;
						if(transtab[t_pos] < 0)
							transtab[t_pos] = 0;
						else
							if(transtab[t_pos] > 255)
								transtab[t_pos] = 255;
						t_pos++;
					} while(++i < 256);
				}
				else
					if(slidvalk < 0)
					{
						m1 = 128 + slidvalk;
						do
						{
							transtab[t_pos] = (unsigned int)i * m1 / 128 - slidvalk + slidvalb;
							if(transtab[t_pos] < 0)
								transtab[t_pos] = 0;
							else
								if(transtab[t_pos] > 255)
									transtab[t_pos] = 255;
							t_pos++;
						} while(++i < 256);
					}
			}
			else
			{
				smurf_struct->module_mode = M_WAITING;
				return;
			}

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

				if(rc == 1 && gc == 1 && bc == 1)
					do
					{
						*data++ = transtab[*data];
						*data++ = transtab[*data];
						*data++ = transtab[*data];
					} while(--length);
				else
					do
					{
						if(rc == 1)
							*data++ = transtab[*data];
						else
							data++;
						if(gc == 1)
							*data++ = transtab[*data];
						else
							data++;
						if(bc == 1)
							*data++ = transtab[*data];
						else
							data++;
					} while(--length);
				
			} /* BitsPerPixel != 16? */
			else
				if(BitsPerPixel == 16)
				{
					width = smurf_struct->smurf_pic->pic_width;
					height = smurf_struct->smurf_pic->pic_height;
					data16 = (unsigned int *)smurf_struct->smurf_pic->pic_data;
			
					length = (unsigned long)width * (unsigned long)height;

					if(rc == 1 && gc == 1 && bc == 1)
						do
						{
							val = *data16;
							*data16 = ((transtab[(val & 0xf800) >> 8] & 0x00f8) << 8);
							*data16 |= ((transtab[(val & 0x7e0) >> 3] & 0x00fc) << 3);
							*data16++ |= (transtab[(val & 0x1f) << 3] >> 3);
						} while(--length);
					else
						do
						{
							if(rc == 1)
								*data16 &= ((transtab[(*data16 & 0xf800) >> 8] & 0x00f8) << 8);
							if(gc == 1)
								*data16 &= ((transtab[(*data16 & 0x7e0) >> 3] & 0x00fc) << 3);
							if(bc == 1)
								*data16 &= (transtab[(*data16 & 0x1f) << 3] >> 3);
							data16++;
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