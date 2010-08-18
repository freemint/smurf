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
/*							Clip'n'Pic						*/
/* Version 0.1  --  27.02.98								*/
/*	  8 (pixelpacked), 16, 24 Bit							*/
/* Version 0.2  --  01.03.98								*/
/*	  1-8 Bit Standardformat								*/
/* Version 0.3  --  27.03.98								*/
/*	  Zeilenweise Assemblerleseroutine eingebaut.			*/
/* Version 0.4  --  16.05.98								*/
/*	  es wird nun die Rearrange Line benutzt, die auch auf	*/
/*	  STs lÑuft. Weiterhin auch die Assemblerroutine zum	*/
/*	  Auslesen eines einzelnen Pixels aus dem Standardformat*/
/* Version 0.5  --  23.05.98								*/
/*	  NatÅrlich bei dem ganzen Umbau wieder einen Fehler	*/
/*	  reingebaut, der bei Randgrîûen link Åber 7 Pixeln zum	*/
/*	  Zerschieûen des Bildes gefÅhrt hatte.					*/
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
#include "..\..\src\lib\demolib.h"

void rearrange_line(char *ptr, long bytes, unsigned int pixels);
void rearrange_line2(char *ptr, char *buf, long bytes, unsigned int pixels);
/* Get Standardformat-Pixel Assembler-Rout */
void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);
void getpix_std_1(char *st_pic, int *pixval, int planes, long planelen, int which);

MOD_INFO module_info = {"Clip'n'Pic",
						0x0050,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
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
						"",
						0,64,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						0,0,0,0,
						0,0,0,0,
						0,0,0,0,
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
/*						Clip'n'Pic					*/
/*	Schneidet selbstÑndig RÑnder um ein Bild weg.	*/
/*		1-8, 16 und 24 Bit							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/

void edit_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *start, *pixbuf, *line,
		 BitsPerPixel, p, did = FALSE, red, green, blue, idx,
		 minus, out, outstart, pixels;

	int picval = 0;
	unsigned int *buffer16, *obuffer16, *ziel16, *start16,
				 x, y, width, height, newwidth, newheight,
				 leftborder = 0, rightborder = 0, upperborder = 0, lowerborder = 0,
				 leftbordermod, color;

	long offset, realwidth, newrealwidth, planelength, w;


/* wie schnell sind wir? */
/*	init_timer(); */

/* Wenn das Modul zum ersten Mal gestartet wurde */
	if(smurf_struct->module_mode == MSTART)
	{
		smurf_struct->module_mode = M_STARTED;
		return;
	}
	else 
		if(smurf_struct->module_mode == MEXEC)
		{
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			newwidth = width = smurf_struct->smurf_pic->pic_width;
			newheight = height = smurf_struct->smurf_pic->pic_height;

			if(BitsPerPixel == 24)
			{
				ziel = start = obuffer = buffer = smurf_struct->smurf_pic->pic_data;

				realwidth = (long)width * 3L;

				red = *buffer;
				green = *(buffer + 1);
				blue = *(buffer + 2);

				y = 0;
				do
				{
					x = 0;
					do
					{
						if(*buffer != red || *(buffer + 1) != green || *(buffer + 2) != blue)
							goto oende24;
						else
							buffer += 3;
					} while(++x < width);

					start = buffer;				/* Startpunkt verschieben */
					newheight--;				/* neue Bildhîhe reduzieren */
				} while(++y < height);

oende24:
				buffer = obuffer;
				red = *buffer;
				green = *(buffer + 1);
				blue = *(buffer + 2);

				x = 0;
				do
				{
					buffer = obuffer + (long)x * 3L;

					y = 0;
					do
					{
						if(*buffer != red || *(buffer + 2) != green || *(buffer + 1) != blue)
							goto lende24;
						else
							buffer += realwidth;
					} while(++y < height);

					newwidth--;					/* neue Bildbreite reduzieren */
					leftborder++;				/* linken Rand aufaddieren */
				} while(++x < width);

lende24:
				start += leftborder * 3;		/* Startpunkt verschieben */

				buffer = obuffer + (long)(width - 1) * 3L;
				red = *buffer;
				green = *(buffer + 1);
				blue = *(buffer + 2);

				x = width - 1;
				do
				{
					buffer = obuffer + (long)x * 3L;

					y = 0;
					do
					{
						if(*buffer != red || *(buffer + 2) != green || *(buffer + 1) != blue)
							goto rende24;
						else
							buffer += realwidth;
					} while(++y < height);

					newwidth--;					/* neue Bildbreite reduzieren */
					rightborder++;				/* rechten Rand aufaddieren */
				} while(x--);

rende24:
				/* Summe aus linkem und rechten Rand */
				offset = (long)(leftborder + rightborder) * 3L;

				buffer = obuffer + (long)(height - 1) * (long)realwidth;
				red = *buffer;
				green = *(buffer + 1);
				blue = *(buffer + 2);

				y = height - 1;
				do
				{
					buffer = obuffer + (long)y * (long)realwidth;

					x = 0;
					do
					{
						if(*buffer != red || *(buffer + 1) != green || *(buffer + 2) != blue)
							goto uende24;
						else
							buffer += 3;
					} while(++x < width);

					newheight--;				/* neue Bildhîhe reduzieren */
				} while(y--);

uende24:
				if(newwidth != width || newheight != height)
				{
					newrealwidth = (long)newwidth * 3L;

					buffer = start;

					y = 0;
					do
					{
						memcpy(ziel, buffer, newrealwidth);
						ziel += newrealwidth;
						buffer += newrealwidth + offset;
					} while(++y < newheight);

					buffer = obuffer;

					Mshrink(0, buffer, newrealwidth * (long)newheight);

					did = TRUE;
				}
			} /* BitsPerPixel == 24? */
			else
			if(BitsPerPixel == 8 && smurf_struct->smurf_pic->format_type == FORM_PIXELPAK)
			{
				ziel = start = obuffer = buffer = smurf_struct->smurf_pic->pic_data;

				idx = *buffer;

				y = 0;
				do
				{
					x = 0;
					do
					{
						if(*buffer != idx)
							goto oende8;
						else
							buffer++;
					} while(++x < width);

					start = buffer;				/* Startpunkt verschieben */
					newheight--;				/* neue Bildhîhe reduzieren */
				} while(++y < height);

oende8:
				buffer = obuffer;
				idx = *buffer;

				x = 0;
				do
				{
					buffer = obuffer + x;

					y = 0;
					do
					{
						if(*buffer != idx)
							goto lende8;
						else
							buffer += width;
					} while(++y < height);

					newwidth--;					/* neue Breite reduzieren */
					leftborder++;				/* linken Rand aufaddieren */
				} while(++x < width);

lende8:
				start += leftborder;			/* Startpunkt verschieben */

				buffer = obuffer + (width - 1);
				idx = *buffer;

				x = width - 1;					/* Start in vorletzter Spalte */
				do
				{
					buffer = obuffer + x;

					y = 0;
					do
					{
						if(*buffer != idx)
							goto rende8;
						else
							buffer += width;
					} while(++y < height);

					newwidth--;					/* neue Breite reduzieren */
					rightborder++;				/* rechten Rand aufaddieren */
				} while(x--);

rende8:
				/* Summe aus linkem und rechten Rand */
				offset = leftborder + rightborder;

				buffer = obuffer + (long)(height - 1) * (long)width;
				idx = *buffer;

				y = height - 1;					/* Start in vorletzter Zeile */
				do
				{
					buffer = obuffer + (long)y * (long)width;

					x = 0;
					do
					{
						if(*buffer != idx)
							goto uende8;
						else
							buffer++;
					} while(++x < width);

					newheight--;				/* neue Bildhîhe reduzieren */
				} while(y--);

uende8:
				if(newwidth != width || newheight != height)
				{
					buffer = start;

					y = 0;
					do
					{
						memcpy(ziel, buffer, newwidth);
						ziel += newwidth;
						buffer += newwidth + offset;
					} while(++y < newheight);

					buffer = obuffer;

					Mshrink(0, buffer, (long)newwidth * (long)newheight);
					did = TRUE;
				}
			} /* BitsPerPixel == 8 && FORM_PIXELPAK? */
			else
			if(smurf_struct->smurf_pic->format_type == FORM_STANDARD)
			{
				ziel = start = obuffer = buffer = smurf_struct->smurf_pic->pic_data;

				pixbuf = (char *)Malloc(width + 7);

				w = (unsigned long)((width + 7) / 8);
				realwidth = w;
				planelength = realwidth * (long)height;

				/* ganz linkes Pixel auslesen */
				getpix_std_1(buffer, &picval, BitsPerPixel, planelength, 0);
				idx = (char)picval;

				y = 0;
				do
				{
					memset(pixbuf, 0x0, width);
					getpix_std_line(buffer, pixbuf, BitsPerPixel, planelength, width);
					buffer += w;
					line = pixbuf;

					x = 0;
					while(x++ < width)
					{
						if(*line != idx)
							goto oendes;
						else
							line++;
					}

					newheight--;				/* neue Bildhîhe reduzieren */
					upperborder++;				/* oberen Rand aufaddieren */
				} while(++y < height);

oendes:
				buffer = obuffer;

				/* ganz linkes Pixel auslesen */
				getpix_std_1(buffer, &picval, BitsPerPixel, planelength, 0);
				idx = (char)picval;

				x = 0;
				do
				{
					out = 0;
					do
					{
						buffer = obuffer + (x >> 3);

						y = 0;
						do
						{
							getpix_std_1(buffer, &picval, BitsPerPixel, planelength, (int)out);

							if(picval != idx)
								goto lendes;
							else
								buffer += realwidth;
						} while(++y < height);

						newwidth--;				/* neue Breite reduzieren */
						leftborder++;			/* linken Rand aufaddieren */
						x++;
					} while(++out < 8 && x < width);
				} while(x < width);

lendes:
				minus = (width%8 == 0) ? 8 : width%8;
				buffer = obuffer + ((width - 1) >> 3);
				outstart = minus - 1;
				getpix_std_1(buffer, &picval, BitsPerPixel, planelength, outstart);
				idx = (char)picval;

				x = width - 1;
				do
				{
					out = outstart;
					do
					{
						outstart = 7;
						buffer = obuffer + (x >> 3);

						y = 0;
						do
						{
							getpix_std_1(buffer, &picval, BitsPerPixel, planelength, out);

							if(picval != idx)
								goto rendes;
							else
								buffer += realwidth;
						} while(++y < height);

						newwidth--;				/* neue Breite reduzieren */
						rightborder++;			/* rechten Rand aufaddieren */
						x--;
					} while(out-- && x);
				} while(x);

rendes:
				/* Summe aus linkem und rechten Rand */
				offset = (leftborder + rightborder + 7) / 8;

				buffer = obuffer + (long)(height - 1) * (long)realwidth;
				/* ganz linkes Pixel auslesen */
				getpix_std_1(buffer, &picval, BitsPerPixel, planelength, 1);
				idx = (char)picval;

				y = height - 1;
				do
				{
					buffer = obuffer + (long)y * (long)realwidth;
					memset(pixbuf, 0x0, width);
					getpix_std_line(buffer, pixbuf, BitsPerPixel, planelength, width);
					line = pixbuf;

					x = 0;
					while(x++ < width)
					{
						if(*line != idx)
							goto uendes;
						else
							line++;
					}

					newheight--;				/* neue Bildhîhe reduzieren */
					lowerborder++;				/* unteren Rand aufaddieren */
				} while(y--);

uendes:
				if(newwidth != width || newheight != height)
				{
					newrealwidth = (newwidth + 7) / 8;
					leftbordermod = leftborder%8;
					pixels = 8 - leftbordermod;

					buffer = obuffer;

/*					printf("leftborder: %u, rightborder: %u\n", leftborder, rightborder); */

					p = 0;
					do
					{
						buffer += realwidth * (long)upperborder;

						y = 0;
						do
						{
							rearrange_line2(buffer + (leftborder >> 3), ziel, newrealwidth, pixels);
							ziel += newrealwidth;
							buffer += (width + 7) >> 3;
						} while(++y < newheight);

						buffer += realwidth * (long)lowerborder;
					} while(++p < BitsPerPixel);

					buffer = obuffer;

					Mshrink(0, buffer, (long)newrealwidth * (long)newheight * (long)p);
					did = TRUE;
				}

				Mfree(pixbuf);
			} /* FORM_STANDARD? */
			else
			if(BitsPerPixel == 16)
			{
				ziel16 = start16 = obuffer16 = buffer16 = (unsigned int *)smurf_struct->smurf_pic->pic_data;

				color = *buffer16;

				y = 0;
				do
				{
					x = 0;
					do
					{
						if(*buffer16 != color)
							goto oende16;
						else
							buffer16++;
					} while(++x < width);

					start16 = buffer16;			/* Startpunkt verschieben */
					newheight--;				/* neue Bildhîhe reduzieren */
				} while(++y < height);

oende16:
				buffer16 = obuffer16;
				color = *buffer16;

				x = 0;
				do
				{
					buffer16 = obuffer16 + x;

					y = 0;
					do
					{
						if(*buffer16 != color)
							goto lende16;
						else
							buffer16 += width;
					} while(++y < height);

					newwidth--;					/* neue Bildbreite reduzieren */
					leftborder++;				/* linken Rand aufaddieren */
				} while(++x < width);

lende16:
				start16 += leftborder;

				buffer16 = obuffer16 + (width - 1);
				color = *buffer16;

				x = width - 1;					/* Start in vorletzter Spalte */
				do
				{
					buffer16 = obuffer16 + x;

					y = 0;
					do
					{
						if(*buffer16 != color)
							goto rende16;
						else
							buffer16 += width;
					} while(++y < height);

					newwidth--;					/* neue Bildbreite reduzieren */
					rightborder++;				/* rechten Rand aufaddieren */
				} while(x--);

rende16:
				/* Summe aus linkem und rechten Rand */
				offset = leftborder + rightborder;

				buffer16 = obuffer16 + (long)(height - 1) * (long)width;
				color = *buffer16;

				y = height - 1;					/* Start in vorletzter Zeile */
				do
				{
					buffer16 = obuffer16 + (long)y * (long)width;

					x = 1;
					do
					{
						if(*buffer16 != color)
							goto uende16;
						else
							buffer16++;
					} while(x++ < width);

					newheight--;				/* neue Bildhîhe reduzieren */
				} while(y--);

uende16:
				if(newwidth != width || newheight != height)
				{
					newrealwidth = (long)newwidth * 2L;

					buffer16 = start16;

					y = 0;
					do
					{
						memcpy(ziel16, buffer16, newrealwidth);
						ziel16 += newwidth;
						buffer16 += newwidth + offset;
					} while(++y < newheight);

					buffer16 = obuffer16;

					Mshrink(0, buffer16, newrealwidth * (long)newheight);
					did = TRUE;
				}
			} /* BitsPerPixel == 16? */

/* wie schnell waren wir? */
/*	printf("\n%lu", get_timer());
	getch(); */

/*		printf("newwidth: %u, newheight: %u, leftborder: %u, rightborder: %u, upperborder: %u, lowerborder: %u\n", newwidth, newheight, leftborder, rightborder, upperborder, lowerborder); */

		if(did == TRUE)
		{
			smurf_struct->smurf_pic->pic_width = newwidth;
			smurf_struct->smurf_pic->pic_height = newheight;

			smurf_struct->module_mode = M_DONEEXIT;
		}
		else
			smurf_struct->module_mode = M_WAITING;

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