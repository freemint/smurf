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
/*                      Graustufen-Modul                    */
/* Version 0.1  --  12.03.96                                */
/*    TC- und Palettenbilder                                */
/* Version 0.2  --  12.03.96                                */
/*	  Flieûkomma rausgeschmiûen -> Faktor 60 schneller		*/
/* Version 0.3  --  23.03.96                                */
/*	  HC-Bilder												*/
/* Version 0.4  --  24.03.96                                */
/*	  HC-Modus hatte nicht so (Ñh, gar nicht) funktioniert	*/
/*	  VerknÅpfung des Graustufenbildes in wÑhlbarer StÑrke	*/
/*	  mit Ursprungsbild. Sieht gut aus, jedoch werde ich	*/
/*	  jetzt wohl ein Preview brauchen. :)					*/
/* Version 0.5  --  03.05.96                                */
/*	  Variable Anzahl (2 bis 32 und 256) Graustufen; dabei  */
/*	  fiel nochmal eine Beschleunigung ab					*/
/* Version 0.6  --  21.05.96                                */
/*	  ein kleiner Fehler bei der Schleifenendebedingung		*/
/*	  war noch drinnen. Sah wie ein öberlauf aus.			*/
/* Version 0.7  --  01.11.96                                */
/*	  Es wurde eine Graustufe zu wenig erzeugt				*/
/* Version 0.8  --  16.02.97                                */
/*	  RGB-Faktoren von 0,299; 0,587; 0,114 auf				*/
/*	  0,212671; 0,715160; 0,072169 umgestellt				*/
/*	  (s. Colorspace-FAQ)									*/
/*	  MOD_INFO wurde falsch bestÅckt						*/
/* Version 0.9  --  21.09.97                                */
/*	  IntensitÑtsstufen von 2-64 auf benutzerfreundliche	*/
/*	  1-100 umgestellt.										*/
/* Version 1.0  --  31.01.98                                */
/*	  Bei 8 Bit Bildern wird jetzt eine lineare Palette		*/
/*	  genommen.												*/
/* Version 1.1  --  05.05.98                                */
/*	  Bei Erzeugung einer linearen Graustufenpalette muû	*/
/*	  auch das Bild verÑndert werden. Das ging auch - bei	*/
/*	  8 Bit Pixelpacked, fÅr Standardformat gab es aber		*/
/*	  keine Routine ...										*/
/* Version 1.2  --  18.08.98                                */
/*	  Stufenbegrenzung des Editfeldes waren falsch (0-10).	*/
/*	  Bei 256 Stufen im Editfeld wird jetzt auch auf Smooth	*/
/*	  geschalten.											*/
/* =========================================================*/

#ifdef GERMAN
#define TEXT1 "Graustufen"
#define TEXT2 "IntensitÑt"
#define TEXT3 "256 Stufen"
#define TEXT4 "Stufen"
#else
#ifdef ENGLISH
#define TEXT1 "Graustufen"
#define TEXT2 "IntensitÑt"
#define TEXT3 "256 Stufen"
#define TEXT4 "Stufen"
#else
#ifdef FRENCH
#define TEXT1 "Graustufen"
#define TEXT2 "IntensitÑt"
#define TEXT3 "256 Stufen"
#define TEXT4 "Stufen"
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
#include "..\..\SRC\LIB\demolib.h"


void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);
int setpix_std_line(char *buf, char *std, int depth, long planelen, int howmany);

MOD_INFO module_info = {TEXT1,
						0x0120,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						TEXT2,
						"",
						"",
						"",
						TEXT3,
						"",
						"",
						"",
						TEXT4,
						"",
						"",
						"",
						1,100,
						0,64,
						0,64,
						0,64,
						2,256,
						0,10,
						0,10,
						0,10,
						100, 0, 0, 0,
						1, 0, 0, 0,
						2, 0, 0, 0,
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
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*                  Graustufenwandler               */
/*		1-8, 16 und 24 Bit							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data, *pal, *palette, *line, *pixbuf,
		 BitsPerPixel, val, r, g, b,
		 grenze, grenzen[256], stepwidth,
		 smooth, i, j, l, lower, upper, areawidth;
	
	int module_id;
	unsigned int *data16,
				 x, y, width, height, intens, k, stufen;
	
	unsigned long length, planelength;
	

	module_id = smurf_struct->module_number;

/* Wenn das Modul zum ersten Mal gestartet wurde */
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
			if(smurf_struct->smurf_pic->col_format == GREY)
			{
				smurf_struct->module_mode = M_WAITING;
				return;			
			}

			intens = ((int)smurf_struct->slide1 * 64) / 100;
			stufen = (unsigned int)smurf_struct->edit1;
			if(smurf_struct->check1 || stufen == 256)
				smooth = 1;

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			if(smooth == 1)
			{
				k = 0;
				do
				{
					grenzen[k] = (char)k;
				} while(++k < 256);
			}
			else
			{
				stepwidth = 256 / (stufen - 1);
				grenze = 256 / stufen;
				stufen--;

				k = 0;
				l = 0;
				i = 0;
				do
				{
					j = 0;
					do
					{
						grenzen[k++] = l;
					} while(++j < grenze);

					l += stepwidth;
				} while(++i < stufen);
				do
				{
					grenzen[k] = 255;
				} while(++k < 256);
			}

			if(BitsPerPixel != 16)
			{
				if(BitsPerPixel == 24 || (BitsPerPixel == 8 && intens == 64))
				{
					data = smurf_struct->smurf_pic->pic_data;

					length = (unsigned long)width * (unsigned long)height;
				}
				else
				{
					data = smurf_struct->smurf_pic->palette;

					length = 256L;
				}

				if(intens == 64)
					if(BitsPerPixel == 8)
					{
						palette = smurf_struct->smurf_pic->palette;

						if(smurf_struct->smurf_pic->format_type == FORM_PIXELPAK)
							do
							{
								val = *data;
								pal = palette + val + val + val;
								*data++ = grenzen[(((long)*pal++ * 872L)
												 + ((long)*pal++ * 2930L)
												 + ((long)*pal * 296L)) >> 12];
							} while(--length);
						else
						{
							planelength = (long)(width + 7) / 8 * (long)height;

							pixbuf = (char *)Malloc(width + 7);

							y = 0;
							do
							{
								memset(pixbuf, 0x0, width);
								getpix_std_line(data, pixbuf, BitsPerPixel, planelength, width);
								line = pixbuf;

								x = 0;
								do
								{
									val = *line;
									pal = palette + val + val + val;
									*line++ = grenzen[(((long)*pal++ * 872L)
													 + ((long)*pal++ * 2930L)
													 + ((long)*pal * 296L)) >> 12];
								} while(++x < width);

								data += setpix_std_line(pixbuf, data, BitsPerPixel, planelength, width);
							} while(++y < height);

							Mfree(pixbuf);
						}

						/* lineare Palette erzeugen */
						pal = smurf_struct->smurf_pic->palette;
						for(k = 0; k < 256; k++)
						{
							*pal++ = (char)k;
							*pal++ = (char)k;
							*pal++ = (char)k;
						}
					}
					else
					{
						do
						{
							val = grenzen[(((long)*data * 872L)
										 + ((long)*(data + 1) * 2930L)
										 + ((long)*(data + 2) * 296L)) >> 12];

							*data++ = val;
							*data++ = val;
							*data++ = val;
						} while(--length);
					}
				else
					do
					{
						val = grenzen[(((long)*data * 872L)
									 + ((long)*(data + 1) * 2930L)
									 + ((long)*(data + 2) * 296L)) >> 12];

						*data++ = (char)((val * intens + *data * (64 - intens)) >> 6);
						*data++ = (char)((val * intens + *data * (64 - intens)) >> 6);
						*data++ = (char)((val * intens + *data * (64 - intens)) >> 6);
					} while(--length);
			}
			else
				if(BitsPerPixel == 16)
				{
					data16 = (unsigned int *)smurf_struct->smurf_pic->pic_data;
			
					length = (unsigned long)width * (unsigned long)height;
	
					if(intens == 64)
						do
						{
							val = grenzen[(((long)((*data16 & 0xf800) >> 8) * 872L)
										 + ((long)((*data16 & 0x7e0) >> 3) * 2930L)
										 + ((long)((*data16 & 0x1f) << 3) * 296L)) >> 12];

							*data16++ = ((val & 0x00f8) << 8) | ((val & 0x00fc) << 3) | (val >> 3);
						} while(--length);
					else
						do
						{
							val = grenzen[(((long)(r = (*data16 & 0xf800) >> 8) * 872L)
										 + ((long)(g = (*data16 & 0x7e0) >> 3) * 2930L)
										 + ((long)(b = (*data16 & 0x1f) << 3) * 296L)) >> 12];

							*data16 = (((val * intens + r * (64 - intens)) >> 6) & 0x00f8) << 8;
							*data16 |= ((((val * intens + g * (64 - intens)) >> 6) & 0x00fc) << 3);
							*data16++ |= ((((val * intens + b * (64 - intens)) >> 6)) >> 3);
						} while(--length);
				}


			if(intens == 64)
				smurf_struct->smurf_pic->col_format = GREY;

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

/*
	upper = 230;
	lower = 50;
	areawidth = upper - lower + 1;
				stepwidth = areawidth / stufen;
				grenze = areawidth / stufen;
				stufen--;

				k = 0;
				l = lower;
				i = 0;
				do
				{
					grenzen[k++] = l;
				} while(k < lower);

				l += stepwidth;
				do
				{
					j = 0;
					do
					{
						grenzen[k++] = l;
						printf("k: %d, l: %d\n", (int)k - 1, (int)l);
					} while(++j < grenze);

					l += stepwidth;
				} while(++i < stufen);
	getch();
				do
				{
					grenzen[k++] = upper;
				} while(k < 256);
*/