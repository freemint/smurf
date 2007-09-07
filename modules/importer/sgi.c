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
/*				SGI - Silicon Graphic-Format				*/
/*				  developed by P. Haeberli					*/
/* Version 0.1  --  11.01.96								*/
/*	  xxx													*/
/* Version 0.2  --  13.01.96								*/
/*	  Komprimierung war immer eingeschaltet und es gab MÅll */
/*	  Wahrscheinlich noch Probleme mit komprimierten 24 Bit */
/*	  Ich brauche Testbilder!								*/
/* Version 0.3  --  26.02.97								*/
/*	  Von Arrays auf Zeiger umgestellt						*/
/*	  8 Bit Palette fÅr "dithered" Bilder wird erstellt, ob	*/
/*	  das korrekt ist, steht nirgends.						*/
/*	  Probleme bereiten 8 Bit Indexbilder, die Palette ist	*/
/*	  in einem extra File gespeichert, nur wie rankommen?	*/
/* Version 0.4  --  14.03.97								*/
/*	  16 Bit mÅûten nun funktionieren - ich habe aber keine	*/
/*	  Testbilder dafÅr.										*/
/*==========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

void decode_1bpc(char *ziel, char *buffer, unsigned int width, unsigned int height, unsigned long *table, char Planes, char comp);
void decode_2bpc(char *ziel, unsigned int *buffer, unsigned int width, unsigned int height, unsigned long *table, char Planes, char comp);
void make_pal(char *buffer, char *pal, char BitsPerPixel, unsigned long length, char ColorMap);
int switch_orient(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel);
void nulltospace(char *string, char length);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"SGI-Format",
						0x0040,
						"Christian Eyrich",
						"SGI", "RLE", "BW", "RGB", "RGBA",
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
/*      SGI - Silicon Graphic-Format (SGI)          */
/*		8, 24 Bit (in 3 Ebenen),					*/
/*		ohne Komprimierung/RLE						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel,
		 comp, Planes, BitsPerPixel, ColorMap, Bpc, Dim;
	char dummy[3], impmessag[21];
	
	unsigned int width, height;

	unsigned long *table,
				  tablen;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;

	if(*(unsigned int *)buffer != 0x01da)
		return(M_INVALID);
	else
	{
		comp = *(buffer + 0x02);

		Bpc = *(buffer + 0x03);

		/* unwichtig */
		Dim = (char)*(unsigned int *)(buffer + 0x04);

		width = *(unsigned int *)(buffer + 0x06); 
		height = *(unsigned int *)(buffer + 0x08);

		/* 1 = BW, dithered, colormapped; 3 = RGB; 4 = RGB + alpha channel */
		Planes = (char)*(unsigned int *)(buffer + 0x0a);
		if(Planes > 3)						/* sonst wÑren das ja ZustÑnde */
			Planes = 3;

		/* 0x00 - Normal pixel values: 1 Channel Greyscale, 3 Channels RGB */
		/* 0x01 - Dithered Images: (8 BPP 3+3+2) */
		/* 0x02 - single Channel Images */
		/* 0x03 - color Map */
		ColorMap = *(unsigned long *)(buffer + 0x6c);
		if(ColorMap > 3)
			return(M_UNKNOWN_TYPE);

		BitsPerPixel = Planes * 8;


		nulltospace(buffer + 0x18, 80);
		strcpy(smurf_struct->smurf_pic->infotext, buffer + 0x18);
		strncpy(smurf_struct->smurf_pic->format_name, "SGI-Format .SGI, .RLE", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "SGI ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if((ziel = (char *)SMalloc(((long)width * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			if(comp)
			{
				tablen = (long)height * (long)Planes * 4 * 2;

				if((table = (unsigned long *)Malloc(tablen)) == 0)
				{
					SMfree(ziel);
					return(M_MEMORY);
				}
				else
					memcpy((char *)table, buffer + 0x200, tablen);
			}
			else
				table = 0;

			if(Bpc == 1)
				decode_1bpc(ziel, buffer, width, height, table, Planes, comp);
			else
				decode_2bpc(ziel, (unsigned int *)buffer, width, height, table, Planes, comp);

			Mfree(table);

			if(!comp)
				if(switch_orient(ziel, width, height, BitsPerPixel) != 0)
					form_alert(1, "[1][Zu wenig Speicher zum Korrigieren | der Orientierung][ Ok ]");
			
			if(BitsPerPixel <= 8)
				make_pal(ziel, smurf_struct->smurf_pic->palette, BitsPerPixel, (long)width * (long)height, ColorMap);

			smurf_struct->smurf_pic->pic_data = ziel;

			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

			if(BitsPerPixel == 8)
				smurf_struct->smurf_pic->col_format = GREY;
			else
				smurf_struct->smurf_pic->col_format = RGB;
		
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);
	return(M_PICDONE);
}


/* Dekodiert Bilder mit einem und drei Bytes pro Pixel. */
void decode_1bpc(char *ziel, char *buffer, unsigned int width, unsigned int height, unsigned long *table, char Planes, char comp)
{
	char *oziel, *obuffer,
		 p, n, v1, v2;

	unsigned int x, y;

	unsigned long tablen3;


	obuffer = buffer;
	oziel = ziel;

	buffer += 0x200;	

	p = 0;
	do
	{
		tablen3 = height * p;

		ziel = oziel + p;

		y = height;
		while(y--)				/* y absteigend, damit die Tabelle von unten her gelesen wird - */
		{						/* so wird ein flip bei komprimierten Bildern ÅberflÅssig */
			if(comp)
				buffer = obuffer + table[tablen3 + y];

			x = 0;
			do /* width */
			{
				if(!comp)
				{
					x++;
					*ziel = *buffer++;
					ziel += Planes;
				}
				else
				{
					v1 = *buffer++;
					if(!(n = v1 & 0x7f))
						return;

					if(v1 & 0x80)
					{
						x += n;

						while(n--)
						{
							*ziel = *buffer++;
							ziel += Planes;
						}
					}
					else
					{
						v2 = *buffer++;

						x += n;
						while(n--)
						{
							*ziel = v2;
							ziel += Planes;
						}
					}
				} /* comp? */
			} while(x < width); /* x */
		} /* y */
	} while(++p < Planes);

	return;
} /* decode_1bpc */


/* Dekodiert Bilder mit 3 mal zwei Bytes pro Kanal. */
/* Das sind Bilder, bei denen jeder RGB-Teil mit 16 Bit Genauigkeit vorliegt. */
/* Die unteren 8 Bit shifte ich hier einfach weg. */
void decode_2bpc(char *ziel, unsigned int *buffer, unsigned int width, unsigned int height, unsigned long *table, char Planes, char comp)
{
	char *obuffer, *oziel,
		 p, n, v1, v2;

	unsigned int x, y;

	unsigned long tablen3;


	obuffer = (char *)buffer;
	oziel = ziel;

	(char *)buffer += 0x200;

	p = 0;
	do
	{
		tablen3 = height * p;

		ziel = oziel + p;

		y = height;
		while(y--)
		{
			if(comp)
				buffer = (unsigned int *)(obuffer + table[tablen3 + y]);

			x = 0;
			do /* width */
			{
				if(comp == 0)
				{
					x++;
					*ziel++ = *buffer++ >> 8;
				}
				else
				{
					v1 = (char)*buffer++;
					if(!(n = v1 & 0x7f))
						return;
					if(v1 & 0x80)
					{
						x += n;
						while(n--)
							*ziel++ = *buffer++ >> 8;
					}
					else
					{
						v2 = *buffer++ >> 8;

						x += n;
						while(n--)
							*ziel++ = v2;
					}
				} /* comp? */
			} while(x < width); /* x */
		} /* y */
	} while(++p < Planes);

	return;
} /* decode_2bpc */


/* Legt eine Palette fÅr "dithered" Bilder an */
void make_pal(char *buffer, char *palette, char BitsPerPixel, unsigned long length, char ColorMap)
{
	char *pal,
		 pixval;

	unsigned int i;


	if(BitsPerPixel == 1)
	{
		pal = palette;

		pal[0] = 255;
		pal[1] = 255;
		pal[2] = 255;
		pal[3] = 0;
		pal[4] = 0;
		pal[5] = 0;
	}
	else
		if(BitsPerPixel == 8)
		{
			if(ColorMap == 0x0)			/* Graustufenpalette fÅr 1 Channel 8 Bit erstellen */
			{
				pal = palette;

				for(i = 0; i < 256; i++)
				{
					*pal++ = (char)i;
					*pal++ = (char)i;
					*pal++ = (char)i;
				}
			}
			else
				if(ColorMap == 0x01)		/* Palette aus 3r3g2b Bits erstellen */
					while(length--)
					{
						pixval = *buffer++;
						pal = palette + pixval + pixval + pixval;

						*pal++ = (pixval & 0x07) << 5;
						*pal++ = (pixval & 0x38) << 2;
						*pal++ = pixval & 0xc0;
					}
				else			
					if(ColorMap == 0x03)	/* Palette aus extra File Åbertragen */
					/* da ich das nicht kann, wird momentan eine Graustufenpalette erzeugt */
					{
						pal = palette;

						for(i = 0; i < 256; i++)
						{
							*pal++ = (char)i;
							*pal++ = (char)i;
							*pal++ = (char)i;
						}
					}
		}
} /* make_pal */


/* éndert die Orientierung der Bilder */
int switch_orient(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *buffero, *bufferu, *temp;

	unsigned int t;

	unsigned long w, len, planelength;


	w = (unsigned long)width;	

	len = (w * BitsPerPixel) >> 3;
	planelength = len * height;

	if((temp = Malloc(len)) == 0)
		return(-1);

	buffero = buffer;
	bufferu = buffero + planelength - len;

	t = height >> 1;
	while(t--)
	{
		memcpy(temp, buffero, len);
		memcpy(buffero, bufferu, len);
		memcpy(bufferu, temp, len);

		buffero += len;
		bufferu -= len;
	}

	Mfree(temp);

	return(0);
} /* switch_orient */


void nulltospace(char *string, char length)
{
	while(--length)
	{
		if(*string == '\0')
			*string = ' ';
		string++;
	}
} /* nulltospace */