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
/*				PCPaint/Pictor Page Format					*/
/* Version 0.1  --  22.02.97 - 27.02.97						*/
/*	  1, 4, 8 Bit - bei manchen 4 Bit gibt es noch Probleme	*/
/*	  (ULX001BT.PIC), die sind anscheinend in einem anderen	*/
/*	  Format.												*/
/* Version 0.2  --  30.05.98								*/
/*	  neue setpix_std_line eingebaut						*/
/* Version 0.3  --  xx.xx.98								*/
/*	  16, 24 Bit											*/
/* Version 0.4  --  xx.xx.98								*/
/*	  BSAVE Format wird unterstÅtzt							*/
/* Version 0.5  --  xx.xx.98								*/
/*	  Clipping Format wird unterstÅtzt						*/
/* Version 0.6  --  xx.xx.98								*/
/*	  Overlay Format wird unterstÅtzt						*/
/* =========================================================*/

#ifdef GERMAN
#define ERROR1 "[1][Zu wenig Speicher zum Korrigieren | der Orientierung und Formatwandlung][ Ok ]"
#define ERROR2 "[1][Zu wenig Speicher zum Korrigieren | der Orientierung][ Ok ]"
#else
#ifdef ENGLISH
#define ERROR1 "[1][Not enough memory to correct the  | orientation and format conversion! ][ OK ]"
#define ERROR2 "[1][Not enough memry to correct | the orientation!][ OK ]"
#else
#ifdef FRENCH
#define ERROR1 "[1][Not enough memory to correct the  | orientation and format conversion! ][ OK ]"
#define ERROR2 "[1][Not enough memry to correct | the orientation!][ OK ]"
#else
#error "Keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int setpix_std_line(char *buf, char *std, int depth, long planelen, int howmany);

int switch_orient(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel, char Planes);
char *tfm_pp_to_std(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel);

/* Dies bastelt direct ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Pictor PC Paint",
						0x0020,
						"Christian Eyrich",
						"PIC", "", "", "", "",
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
						0,0,0,0,
						0,0,0,0,
						0,0,0,0,
						0
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Pictor PC Paint					*/
/*		1, 2, 4, 8, 16, 24 Bit, ungepackt, RLE		*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel, *pal, *ppal, *temp,
		 BitsPerPixel, Planes, palflag, VideoMode,
		 RunMarker, v1, v2;
	char dummy[3], impmessag[21];

	unsigned int width, height,
				 Offset, PalType, PalSize, Blocks, x, z, osize, BlockSize, n;

	unsigned long w;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	if(*(unsigned int *)buffer != 0x3412)
		return(M_INVALID);
	else
	{
	
		width = swap_word(*(unsigned int *)(buffer + 0x02));
		height = swap_word(*(unsigned int *)(buffer + 0x04));

		/* Erweiterte Planeinfo fÅr HiColor-Format
		   0x01 = 1 Plane,   1 BPP, 2 Farben
		   0x02 = 1 Plane,   2 BPP, 4 Farben
		   0x04 = 1 Plane,   4 BPP, 16 Farben
		   0x08 = 1 Plane,   8 BPP, 256 Farben
		   0x10 = 1 Plane,  16 BPP, 32768/65536 Farben
		   0x11 = 2 Planes,  1 BPP, 4 Farben
		   0x18 = 1 Plane,  24 BPP, 16777216 Farben
		   0x28 = 3 Planes,  8 BPP, 16777216 Farben
		   0x31 = 4 Planes,  1 BPP, 16 Farben */

		BitsPerPixel = *(buffer + 0x0a);
		switch(BitsPerPixel)
		{
			case 0x10: BitsPerPixel = 16;
					   Planes = 1;
					   break;
			case 0x11: BitsPerPixel = 2;
					   Planes = 2;
					   break;
			case 0x18: BitsPerPixel = 24;
					   Planes = 1;
					   break;
			case 0x28: BitsPerPixel = 24;
					   Planes = 3;
					   break;
			case 0x31: BitsPerPixel = 4;
					   Planes = 4;
					   break;
			case 0x71: BitsPerPixel = 8;
					   Planes = 8;
					   break;
			default:   Planes = 1;
					   break;
		}

		palflag = *(buffer + 0x0b);

		VideoMode = *(buffer + 0x0c);

		if(palflag != 0xff)								/* Version < 2.0 */
			Offset = 0x0c;
		else											/* Version >= 2.0 */
		{
			Offset = 0x11;

			/* Art der Palette */
			/* 0 = keine, 1 = CGA-Palette, 2 = PCjr.-Palette,
			   3 = EGA-Palette, 4 = VGA-Palette */
			PalType = swap_word(*(unsigned int *)(buffer + 0x0d));
			/* LÑnge der Palette */
			PalSize = swap_word(*(unsigned int *)(buffer + 0x0f));

			Offset += PalSize;
		}

		/* gepackte Blîcke, 0 = ungepackte Daten */
		Blocks = swap_word(*(unsigned int *)(buffer + Offset));
	
		strncpy(smurf_struct->smurf_pic->format_name, "Pictor PC Paint", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "Pictor Paint ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);
	
		if(BitsPerPixel == 1 || Planes == 2 || Planes == 4 || Planes == 8)
			w = (unsigned long)(width + 7) / 8 * 8;
		else
			w = (unsigned long)width;
		

		if((ziel = SMalloc((w * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			oziel = ziel;
			memset(ziel, 0x0, (w * (long)height * BitsPerPixel) >> 3);

			buffer += 0x02 + Offset;								/* auf ersten Paketkopf setzen */

			z = 0;
			do
			{
				BlockSize = swap_word(*((unsigned int *)buffer)++);	/* Blockgrîûe inkl. Header */
				osize = swap_word(*((unsigned int *)buffer)++);		/* Grîûe der Originaldaten */
				RunMarker = *buffer++;								/* Marker fÅr Bytefolgen */

				x = 0;
				while(x < osize)
				{

					v1 = *buffer++;
					if(v1 == RunMarker)
					{
						n = *buffer++;
						if(n)
						{
							v2 = *buffer++;

							x += n;
							while(n--)
								*ziel++ = v2;
						}
						else
						{
							n = swap_word(*((unsigned int *)buffer)++);
							v2 = *buffer++;

							x += n;
							while(n--)
								*ziel++ = v2;
						}
					}
					else
					{
						x++;

						*ziel++ = v1;
					}
				} /* noch Daten im Block */
			} while(++z < Blocks); /* Blîcke */

			buffer = obuffer;
			ziel = oziel;

			if((BitsPerPixel == 2 || BitsPerPixel == 4) && Planes == 1)
				if((temp = tfm_pp_to_std(ziel, width, height, BitsPerPixel)) == 0)
				{
					form_alert(1, ERROR1 );
					SMfree(ziel);
					return(M_MEMORY);
				}
				else
					ziel = temp;
			else
				if(switch_orient(ziel, width, height, BitsPerPixel, Planes) != 0)
					form_alert(1, ERROR2 );
	
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
/*					if(PalType == 1)					/* CGA-style Palette */
					if(PalType == 2)					/* PC jr-style Palette */
					if(PalType == 3)					/* EGA-style Palette */ */
					if(PalType == 4)					/* VGA-style Palette */
					{
						ppal = buffer + 0x11;

						for(x = 0; x < PalSize; x++)
							*pal++ = *ppal++ << 2;	/* Palette hat lt. Holtorf pro Element 6 Bit */
					}
				}
			}

			if((BitsPerPixel == 8 && Planes == 1) ||
				 BitsPerPixel == 16 || BitsPerPixel == 24)
				smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
			else
				smurf_struct->smurf_pic->format_type = FORM_STANDARD;

			smurf_struct->smurf_pic->col_format = RGB;
	
			smurf_struct->smurf_pic->pic_data = ziel;

		} /* Malloc */
	} /* Erkennung */

/* wie schnell waren wir? */
/*  printf("%lu", get_timer);
	getch(); */

	SMfree(buffer);

	return(M_PICDONE);
}


/* éndert die Orientierung der Bilder */
int switch_orient(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel, char Planes)
{
	char *buffero, *bufferu, *temp,
		 p;

	unsigned int t;

	unsigned long w, len, planelength;

	if(BitsPerPixel == 1 || Planes == 2 || Planes == 4 || Planes == 8)
		w = (unsigned long)(width + 7) / 8 * 8;
	else
		w = (unsigned long)width;	

	len = (w * BitsPerPixel) >> 3;
	planelength = len * height;

	if((temp = Malloc(len)) == 0)
		return(-1);

	p = 0;
	do
	{
		buffero = buffer + planelength * p;
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
	} while(++p < Planes);

	Mfree(temp);

	return(0);
} /* switch_orient */


/* Wandelt 2 und 4 Bit Pixelpacked nach Standardformat */
/* und Ñndert gleich die Orientierung mit */
char *tfm_pp_to_std(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *bufferu, *ziel, *oziel, *pixbuf,
		 val;

	unsigned int x, y, oline;

	unsigned long planelength;


	/* Hier kommen sowieso nur 2 und 4 Bit rein */
	planelength = (unsigned long)((width + 7) / 8) * (unsigned long)height;

	if(BitsPerPixel == 2)
		oline = (width + 3) / 4;
	else
		oline = (width + 1) / 2;

	if((ziel = SMalloc(planelength * BitsPerPixel)) == 0)
		return(0);
	else
	{
		oziel = ziel;

		pixbuf = (char *)Malloc(width);

		bufferu = buffer + (long)oline * (long)(height - 1);

		y = 0;
		do
		{
			x = 0;
			do
			{
				val = *bufferu++;

				if(BitsPerPixel == 2)
				{
					pixbuf[x++] = val >> 6;
					pixbuf[x++] = val >> 4;
					pixbuf[x++] = val >> 2;
					pixbuf[x++] = val & 0x03;
				}
				else
				{
					pixbuf[x++] = val >> 4;
					pixbuf[x++] = val & 0x0f;
				}
			} while(x < width);

			ziel += setpix_std_line(pixbuf, ziel, BitsPerPixel, planelength, width);
			bufferu -= oline + oline;
		} while(++y < height);

		Mfree(pixbuf);

		ziel = oziel;
	} /* Malloc */

	SMfree(buffer);

	return(ziel);
} /* tfm_pp_to_std */