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
/*					X Window Dump Format					*/
/* Version 0.1  --  12.01.96 - 26.01.97						*/
/*	  1, 4, 8, 24 Bit										*/
/* Version 0.2  --  30.05.98								*/
/*	  neue setpix_std_line eingebaut						*/
/* =========================================================*/

#ifdef GERMAN
#define ERROR "[3][ Auch wenn das Bild korrekt | dargestellt werden sollte, | bitte schicken Sie es uns. |          Danke!        ][ Ok ]"
#else
#ifdef ENGLISH
#define ERROR "[3][ Auch wenn das Bild korrekt | dargestellt werden sollte, | bitte schicken Sie es uns. |          Danke!        ][ Ok ]"
#else
#ifdef FRENCH
#define ERROR "[3][ Auch wenn das Bild korrekt | dargestellt werden sollte, | bitte schicken Sie es uns. |          Danke!        ][ Ok ]"
#else
#error "Keine Sprache!"
#endif
#endif
#endif
 
#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define LSB 0
#define MSB 1

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int setpix_std_line(char *buf, char *std, int depth, long planelen, int howmany);
char *fileext(char *filename);
 
/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"X Window Dump-Format",
						0x0010,
						"Christian Eyrich",
						"XWD", "", "", "", "",
						"", "", "", "", "",
						"Slider 1",
						"Slider 2",
						"Slider 3",
						"Slider 4",
						"Checkbox 1",
						"Checkbox 2",
						"Checkbox 3",
						"Checkbox 4",
						"Edit 1",
						"Edit 2",
						"Edit 3",
						"Edit 4",
						0,128,
						0,128,
						0,128,
						0,128,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0
						};
 
/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*			X Window Dump-Format (XWD)				*/
/*		1, 4, 8 und 24 Bit, unkomprimiert			*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel, *pixbuf, *opixbuf, *pal, *ppal, *fname,
		 BitsPerPixel, version, PixmapFormat, ByteOrder,
		 BitOrder, PadBytes, colent, translate[256], PalentrySize,
		 Palpad, val, map;
	char dummy[3], impmessag[21];

	unsigned int i, x, y, width, height, cols;

	unsigned long w, memwidth, DatenOffset, pal_of, planelength;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
 
	/* Header Check */
	fname = smurf_struct->smurf_pic->filename;
	if(stricmp(fileext(fname), "XWD") != 0 ||
/* Kannste l”ten, die Headerl„nge ist nicht garantiert */
/*	   (*(unsigned long *)buffer != 40 && *(unsigned long *)buffer != 100) || */
	   (*(buffer + 0x07) != 6 && *(buffer + 0x07) != 7))
		return(M_INVALID);
	else
	{
		version = *(buffer + 0x04);

	/* X10 */
		if(version == 6)
		{
			BitsPerPixel = (char)*(unsigned long *)(buffer + 0x0c);
			/* 0 = monochrom, 2 = pseudo color */
			PixmapFormat = (char)*(unsigned long *)(buffer + 0x10);
			width = (unsigned int)*(unsigned long *)(buffer + 0x14);
			height = (unsigned int)*(unsigned long *)(buffer + 0x18);
			ByteOrder = 0;
			BitOrder = 0;
			PadBytes = 0;
			colent = 8;
			PalentrySize = 2;
			Palpad = 0;
		}
	/* X11 */
		else
		{
			/* 0 = monochrom, 1 = single planes, 2 = pseudo color */
			PixmapFormat = (char)*(unsigned long *)(buffer + 0x08);
			BitsPerPixel = (char)*(unsigned long *)(buffer + 0x0c);
			width = (unsigned int)*(unsigned long *)(buffer + 0x10);
			height = (unsigned int)*(unsigned long *)(buffer + 0x14);
		/* 0 = LSB, 1 = MSB */
			ByteOrder = (char)*(unsigned long *)(buffer + 0x1c);
		/* 0 = LSB, 1 = MSB */
			BitOrder = (char)*(unsigned long *)(buffer + 0x24);
			PadBytes = ((char)*(unsigned long *)(buffer + 0x28)) >> 3;
			colent = 12;
			PalentrySize = 4;
			Palpad = 2;
		}

		if(ByteOrder == LSB || BitOrder == LSB)
			form_alert(1, ERROR );

		if(ByteOrder == LSB)
			map = 2;

		i = 0;
		do
		{
		/* Bitreihenfolge umdrehen ... */
			if(BitOrder == LSB)
			{
				translate[i] = (char)((i & 0x01) << 7);
				translate[i] |= (char)((i & 0x02) << 5);
				translate[i] |= (char)((i & 0x04) << 3);
				translate[i] |= (char)((i & 0x08) << 1);
				translate[i] |= (char)((i & 0x10) >> 1);
				translate[i] |= (char)((i & 0x20) >> 3);
				translate[i] |= (char)((i & 0x40) >> 5);
				translate[i] |= (char)((i & 0x80) >> 7);
			}
		/* ... oder auch nicht */
			else
				translate[i] = (char)i;
		} while(++i < 256);
	 
		pal_of = *(unsigned long *)buffer; 
		if(version == 6)
		{
			if(BitsPerPixel <= 8)
				cols = 1 << BitsPerPixel;
		}
		else
			cols = (unsigned int)*(unsigned long *)(buffer + 0x4c);
		DatenOffset = pal_of + (colent * cols);

		strncpy(smurf_struct->smurf_pic->format_name, "X Window Dump .XWD", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "X Window Dump ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);
 
		if(BitsPerPixel == 24)
		{
			w = (unsigned long)width;
			memwidth = (unsigned long)width;
		}
		else
			if(BitsPerPixel == 1)
			{
				w = (unsigned long)((width + 7) / 8);
				memwidth = w * 8;
			}
			else
				if(BitsPerPixel == 4)
				{
					w = (unsigned long)(width + 1) / 2;
					memwidth = (unsigned long)((width + 7) / 8) * 8;
				}
				else
				{
					w = (unsigned long)width;
					memwidth = (unsigned long)width;
				}

		if((ziel = SMalloc((memwidth * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			oziel = ziel;
			obuffer = buffer;
			buffer += DatenOffset;

			planelength = (unsigned long)((width + 7) / 8) * (unsigned long)height;

			if(BitsPerPixel <= 8)
			{
				if(BitsPerPixel == 1 || BitsPerPixel == 8)
				{
					y = 0;
					do
					{
						x = 0;
						do
						{
							*ziel++ = translate[*buffer++];
						} while(++x < w);
					} while(++y < height);
				}
				else
					if(BitsPerPixel == 4)
					{
						opixbuf = pixbuf = (char *)Malloc(width);

						y = 0;
						do
						{
							x = 0;
							do
							{
								val = translate[*buffer++];
								*pixbuf++ = val >> 4;
								*pixbuf++ = val & 0x0f;
							} while(++x < w);

							pixbuf = opixbuf;
							ziel += setpix_std_line(pixbuf, ziel, 4, planelength, width);
						} while(++y < height);

						Mfree(pixbuf);
					}
			}
			else
			{
				y = 0;
				do
				{
					x = 0;
					do
					{
						buffer++;
						*(ziel++ + map) = translate[*buffer++];
						*ziel++ = translate[*buffer++];
						*(ziel++ - map) = translate[*buffer++];
					} while(++x < w);
				} while(++y < height);
			}

			ziel = oziel;
			buffer = obuffer;
 
			smurf_struct->smurf_pic->pic_data = ziel;

			if(BitsPerPixel < 8)
				smurf_struct->smurf_pic->format_type = FORM_STANDARD;
			else
				smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
 
			if(BitsPerPixel <= 8)
			{
				pal = smurf_struct->smurf_pic->palette;
				if(BitsPerPixel == 1)
				{
					pal[0] = 255;
					pal[1] = 255;
					pal[2] = 255;
					pal[3] = 0;
					pal[4] = 0;
					pal[5] = 0;
				}
				else
				{
					ppal = buffer + pal_of;
					for(i = 0; i < cols; i++)
					{
						ppal += PalentrySize;
						*pal++ = *ppal;
						ppal += 2;
						*pal++ = *ppal;
						ppal += 2;
						*pal++ = *ppal;
						ppal += 2;
						ppal += Palpad;
					}
				}
			}
		
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);

	return(M_PICDONE);
}
 
 
char *fileext(char *filename)
{
	char *extstart;


	if((extstart = strrchr(filename, '.')) != NULL)
		extstart++;
	else
		extstart = strrchr(filename, '\0');
	
	return(extstart);
} /* fileext */