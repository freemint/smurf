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


/* TODO: */
/* Standardpalette auch fÅr 8 Farben umsetzen */

/* =========================================================*/
/*						GEM-Image Decoder					*/
/* Version 0.1  --  24.12.95								*/
/*	  1 Bit IMG	im Standardformat							*/
/* Version 0.2  --  xx.xx.xx								*/
/*	  4 + 8 + 24 Bit (X)IMG									*/
/*	  imgtype = I: Image, imgtype = X: XImage				*/
/* Version 0.3  --  27.01.96								*/
/*	  Die spÑte (aber immerhin) Erkenntnis, daû im XImage	*/
/*	  Palettenwerte im Promillebereich benutzen hatte bahn-	*/
/*	  brechende Wirkung! Olaf machte eine Menge Wind (ins	*/
/*	  Bild, manche konnten darin auch um 90 Grad gedrehte	*/
/*	  Eiszapfen sehen), Dale sagte gar nichts und ich		*/
/*	  schrieb eine blîde und viel zu lange Anmerkung in die	*/
/*	  Source.												*/
/* Version 0.4  --  11.02.96								*/
/*	  GEM-Views Truecolor Images sind auf 16 Pixel alligned,*/
/*	  dank des Tips von Dieter Fiebelkorn werden nun alle	*/
/*	  GEM-View-24 Bit Images richtig dargestellt.			*/
/* Version 0.5  --  11.02.96								*/
/*	  Kleiner Bug seit der 0.3, die includeten Palettenwerte*/
/*	  waren schon korrekt in Werten bis 256, wurden jedoch	*/
/*	  trotzdem umgerechnet.									*/
/* Version 0.6  --  18.02.96								*/
/*	  Probleme mit IMGs (Indizes auf Farbpalette) sind bei-	*/
/*	  gelegt. 256 wurde in ein char geschoben ...			*/
/* Version 0.7  --  25.02.96								*/
/*	  Probleme mit yel_ston.IMG (7 Bit Image) behoben.		*/
/* Version 0.8  --  28.05.96								*/
/*	  Ab jetzt wird direkt addressiert und nicht mehr per	*/
/*	  Index. 25% Geschwindigkeitsgewinn sind der Lohn.		*/
/* Version 0.9  --  12.01.97								*/
/*	  Bugfix, bei nicht auf 16 Pixel ausgerichteten TC-IMGs	*/
/*	  kopierte der vrc zu viel.								*/
/* Version 1.0  --  17.01.97								*/
/*	  VRC-Erkennung aus x-Schleife genommen. Das ist etwas	*/
/*	  schneller und auch korrekt.							*/
/* Version 1.1 --  22.01.97									*/
/*	  CMY-IMGs werden korrekt an Smurf weitergegeben.		*/
/* Version 1.2 --  12.02.97									*/
/*	  Verbesserte Erkennung, bzw. Ablehnung falscher Files	*/
/* Version 1.3  --  03.12.97								*/
/*	  öberwachung gegen zuviele vrc-Wiederholungen im Bild.	*/
/* Version 1.4  --  06.01.98								*/
/*	  in Versin 1.3 wurde y fÑlschlicherweise in der p-		*/
/*	  Schleife hochgezÑhlt - das war zwar bei 1 Bit-Bildern	*/
/*	  ok, aber Farbbildern wurden damit zermatscht.			*/
/*	  Hilfsvariable y2 eingefÅhrt, damit geht's nun.		*/
/* Version 1.5  --  xx.xx.97								*/
/*	  PixArts 24 Bit-Images werden nun  korrekt gelesen.	*/
/* Version 1.6  --  01.01.2009								*/
/*	  Fehlermeldung bei TIMG GEM-Images Bilder              */
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define GVW	1
#define PA	2
#define GC	3

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

char *fileext(char *filename);

/* Get 16 Pixel (Interleaved Standard Format) Assembler-Rout */
void get_IBPLine(char *dest, char *src, long lineplanelen);
/*void get_standard_pix(void *st_pic, void *buf16, int planes, long planelen); */

int decode_GVW_TC(char *ziel, char *buffer, unsigned int height, char PattLength, char Planes, long w, long srcw);
int decode_PA_TC(char *ziel, char *buffer, unsigned int height, char PattLength, long w, long srcw);
int decode_GC_TC(char *ziel, char *buffer, unsigned int width, unsigned int height, char PattLength, char BitsPerPixel);
int convert_palette(char *pal, char *buffer, char BitsPerPixel, char imgtype);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"GEM-(X)IMG Importer",
						0x0160,
						"Christian Eyrich",
						"IMG", "", "", "", "",
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
/*			GEM (X)IMG Dekomprimierer (IMG)			*/
/*		1, 2, 4, 7, 8, 24 Bit,						*/
/*		eigene Komprimierungen						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel, *fname,
		 BitsPerPixel, Planes, PattLength, imgtype, tcmode, colmodel;
	char dummy[3], impmessag[17];

	unsigned int width, height, DatenOffset;

	unsigned long w, memwidth, srcw;

	enum
	{
		_RGB,
		_CMY,
		_HLS
	};


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;

	/* falls XIMG, steht in Byte 0x10 - 0x13  XIMG */
	if(*(unsigned long *)(buffer + 0x10) == 'XIMG')
		imgtype = 'X';
	else
	{
		if(*(unsigned long *)(buffer + 0x10) == 'TIMG')
			imgtype = 'T';
		else
			imgtype = 'I';
	}

	/* erstes Word ist leider nicht immer 1, da gibt es */
	/* zumindest ein Proramm, das 2 reinschreibt (s. DONNA.IMG) */
	fname = smurf_struct->smurf_pic->filename;
	if(*(unsigned int *)buffer > 0x02 ||
	   (*(unsigned int *)(buffer + 0x02) != 8 && imgtype != 'X' && imgtype != 'T') ||
	   stricmp(fileext(fname), "IMG") != 0)
		return(M_INVALID);
	else
	{
		if(imgtype == 'T')
		{
			form_alert(0, "[1][TrueColor GEM-Image werden noch nicht|unterstÅtzt.][ OK ]");
			return(M_PICERR);
		}
		
		if(*((unsigned int *)(buffer + 0x02)) == 9)
		{
			form_alert(0, "[1][Ventura-Images werden noch nicht|unterstÅtzt. Bild bitte an uns einschicken!][ OK ]");
			return(M_PICERR);
		}

		DatenOffset = *((unsigned int *)(buffer + 0x02)) << 1;

		BitsPerPixel = *(unsigned int *)(buffer + 0x04);

		PattLength = *(unsigned int *)(buffer + 0x06);
		if(BitsPerPixel == 24)
			if(PattLength == 3)
				tcmode = GVW;
			else
				tcmode = PA;

		width = *((unsigned int *)(buffer + 0x0c)); 
		height = *((unsigned int *)(buffer + 0x0e));

		if(imgtype == 'X')
			colmodel = *(buffer + 0x14);
		else
			colmodel = _RGB;

		strncpy(smurf_struct->smurf_pic->format_name, "GEM-Image .IMG", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "GEM-Image ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if(BitsPerPixel == 24 && tcmode == GVW)
		{
			w = (width + 15) / 16; /* Auf volle Byte gerundete ZeilenlÑnge in Byte */
			memwidth = w * 16;
			w = (w * 16 * 3);
			srcw = (unsigned long)width * 3L;
			Planes = 1;
		}
		else
		{
			w = (width + 7) / 8; /* Auf volle Byte gerundete ZeilenlÑnge in Byte */
			memwidth = w * 8;
			srcw = w;
			Planes = BitsPerPixel;
		}

		if((ziel = SMalloc(((long)memwidth * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			if(BitsPerPixel == 24 && tcmode == PA)
				decode_PA_TC(ziel, buffer + DatenOffset, height, PattLength, w, srcw);
			else
/*
				if(BitsPerPixel == 24 && tcmode == GC)
					decode_GC_TC(ziel, buffer + DatenOffset, height, PattLength, w, srcw);
				else
*/
					decode_GVW_TC(ziel, buffer + DatenOffset, height, PattLength, Planes, w, srcw);

			smurf_struct->smurf_pic->pic_data = ziel;

			if(BitsPerPixel == 24)
				smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
			else
			{
				convert_palette(smurf_struct->smurf_pic->palette, buffer, BitsPerPixel, imgtype);
				smurf_struct->smurf_pic->format_type = FORM_STANDARD;
			}

			if(colmodel == _RGB)
				smurf_struct->smurf_pic->col_format = RGB;
			else
				smurf_struct->smurf_pic->col_format = CMY;
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);

	return(M_PICDONE);
}


/* decodiert alle 1-8 Bit IMGs und 24 Bit IMGs im GEM-View-Format */
int decode_GVW_TC(char *ziel, char *buffer, unsigned int height, char PattLength, char Planes, long w, long srcw)
{
	char *oziel,
		 j, p, vrc, v1, v2, v3;

	unsigned int i, x, y, y2;

	unsigned long pla, plh, plo;


	plh = (unsigned long)height * srcw;		/* Hîhe einer Plane in Byte */

	oziel = ziel;
/*
	printf("Planes: %d, PattLength: %d, srcw: %ld\n", (int)Planes, (int)PattLength, srcw);
	getch();
*/
	y = 0;
	do
	{
		vrc = 1;
		plo = y * srcw; 				/* Offset vom Planeanfang in Bytes */
		for(p = 0; p < Planes; p++)
		{
			pla = plh * p;				/* Abstand dieser Plane vom Bildanfang */
			ziel = oziel + pla + plo;	/* Zieladresse der dekodierten Scanline */

			v1 = *buffer++;
			v2 = *buffer++;
			v3 = *buffer++;
			if(v1 == 0x0 && v2 == 0x0 && v3 == 0xff)	/* Vertical Replication */
				vrc = *buffer++;
			else
				buffer -= 3;

			x = 0;
			do
			{
				v1 = *buffer++;

				switch(v1)
				{
					case 0x0:							/* Pattern Run */
						v2 = *buffer++;
						x += (v2 * PattLength);
						for(i = 0; i < v2; i++)
							for(j = 0; j < PattLength; j++)
								*ziel++ = *(buffer + j);
						buffer += PattLength;
						break;
					case 0x80:                              /* Bit String */
						v2 = *buffer++;
						x += v2;
						for (i = 0; i < v2; i++) 
							*ziel++ = *buffer++;
						break;
					default:                                /* Solid Run */
						v2 = v1 & 0x7f;
						x += v2;
						if(v1 > 0x80)
							while(v2--)
								*ziel++ = 0xff;
						else
							while(v2--)
								*ziel++ = 0x00;
						break;
				} /* case */
			} while(x < w);
/*
			if(x > w)
			{
				printf("y: %u, p: %u\n", y, (unsigned int)p);
				printf("x: %u, w: %lu\n", x, w);
				getch();
			}
*/
			y2 = y;
			/* vrc, aber nicht auf sich selbst */
			for(i = 1; i < vrc && ++y2 < height; i++)
				memcpy(oziel + pla + (y2 * srcw), oziel + pla + plo, srcw);
		}
		y += vrc;
	} while(y < height);

	return(0);
} /* decode_GVW_TC */


/* decodiert 24 Bit IMGs im Pixart-Format (Standardformat mit 24 Planes) */
int decode_PA_TC(char *ziel, char *buffer, unsigned int height, char PattLength, long w, long srcw)
{
	char *oziel, *ooziel, *pixbuf,
		 j, p, vrc, v1, v2, v3;

	unsigned int i, x, y;

	unsigned long pla, plh, plo;


	pixbuf = (char *)SMalloc(w * 24L + 32);	/* Puffer fÅr Standarddaten */

	ooziel = oziel = ziel;
/*
	printf("PattLength: %d, srcw: %ld\n", (int)PattLength, srcw);
	getch();
*/
	y = 0;
	do
	{
		vrc = 1;
		pla = y * 24L * srcw;		/* Abstand dieser Plane vom Bildanfang */
		for(p = 0; p < 24; p++) 	/* Planes */
		{
			oziel = ziel;

			v1 = *buffer++;
			v2 = *buffer++;
			v3 = *buffer++;
			if(v1 == 0x0 && v2 == 0x0 && v3 == 0xff)	/* Vertical Replication */
				vrc = *buffer++;
			else
				buffer -= 3;

			x = 0;
			do
			{
				v1 = *buffer++;

				switch(v1)
				{
					case 0x0:							/* Pattern Run */
						v2 = *buffer++;
						x += (v2 * PattLength);
						for(i = 0; i < v2; i++)
							for(j = 0; j < PattLength; j++)
								*ziel++ = *(buffer + j);
						buffer += PattLength;
						break;
					case 0x80:                              /* Bit String */
						v2 = *buffer++;
						x += v2;
						for (i = 0; i < v2; i++) 
							*ziel++ = *buffer++;
						break;
					default:                                /* Solid Run */
						v2 = v1 & 0x7f;
						x += v2;
						if(v1 > 0x80)
							while(v2--)
								*ziel++ = 0xff;
						else
							while(v2--)
								*ziel++ = 0x00;
						break;
				} /* case */
			} while(x < w);
/*			if(x != w)
			{
				printf("y: %u, p: %u\n", y, (unsigned int)p);
				printf("x: %u, w: %lu\n", x, w);
				getch();
			} */
/*
			for(i = 1; i < vrc; i++) /* vrc, aber nicht auf sich selbst */
				memcpy(oziel + i * srcw, oziel, srcw);
*/
		}
		y += vrc;
	} while(y < height);

	ziel = ooziel;

	pla = 24L * w;
	y = 0;
	do
	{
		memcpy(pixbuf, ziel, pla);
		get_IBPLine(ziel, pixbuf, w);
		ziel += pla;
	} while(++y < height);

	SMfree(pixbuf);

	return(0);
} /* decode_PA_TC */

/*
/* decodiert 24 Bit IMGs im Grafikkonverter-Format (3 Planes) */
int decode_GC_TC(char *ziel, char *buffer, unsigned int width, unsigned int height, char PattLength, char Planes)
{
	signed char shiftval;
	char *oziel, *ziel2,
		 j, p, vrc, v1, v2, v3, val;

	unsigned int i, k, x, y, x2, linelen;

	unsigned long w, memwidth, srcw, pla, plh, plo;


	plh = (unsigned long)height * srcw;		/* Hîhe einer Plane in Byte */

	linelen = (unsigned long)width * 3L;

	oziel = ziel;

	for(p = 0; p < Planes; p++) /* Planes */
	{
		ziel = oziel + p; /* Zieladresse der dekodierten Scanline */
		ziel2 = ziel;

		y = 0;
		do /* height */
		{
			vrc = 1;
			plo = y * w; /* Offset vom Planeanfang in Bytes */

			x = 0;
			x2 = 0;
			do /* width */
			{
				v1 = *buffer++;
				v2 = *buffer++;
				v3 = *buffer++;
				if(v1 == 0x0 && v2 == 0x0 && v3 == 0xff)    /* Vertical Replication */
					vrc = *buffer++;
				else
					buffer -= 3;

				v1 = *buffer++;

				switch(v1)
				{
					case 0x0:                               /* Pattern Run */
						v2 = *buffer++;
						x += (v2 * PattLength);
						for(i = 0; i < v2; i++)
							for (j = 0; j < PattLength; j++)
							{
								val = *(buffer + j);
								shiftval = 7;
								do
								{
									*ziel2 |= ((val >> shiftval) & 1) << p;
									ziel2 += 3;
									x2++;
								} while(--shiftval > -1 && x2 < width); /* shiftval */
							}
						buffer += PattLength;
						break;
					case 0x80:                              /* Bit String */
						v2 = *buffer++;
						x += v2;
						for (i = 0; i < v2; i++)
						{
							val = *buffer++;
							shiftval = 7;
							do
							{
								*ziel2 |= ((val >> shiftval) & 1) << p;
								ziel2 += 3;
								x2++;
							} while(--shiftval > -1 && x2 < width);
						}
						break;
					default:                                /* Solid Run */
						v2 = v1 & 0x7f;
						x += v2;
						if(v1 > 0x80)
							while(v2--)
							{
								val = 0xff;
								shiftval = 7;
								do
								{
									*ziel2 |= ((val >> shiftval) & 1) << p;
									ziel2 += 3;
									x2++;
								} while(--shiftval > -1 && x2 < width);
							}
						else
							while(v2--)
							{
								val = 0x0;
								shiftval = 7;
								do
								{
									*ziel2 |= ((val >> shiftval) & 1) << p;
									ziel2 += 3;
									x2++;
								} while(--shiftval > -1 && x2 < width);
							}
						break;
				} /* case */
			} while(x < width); /* x */
/*			if(x != width)
			{
				printf("x: %u, width: %u\n", x, w);
				getch();
			} */
			ziel += linelen;

			for(i = 1; i < vrc; i++) /* vrc, aber nicht auf sich selbst */
				memcpy(oziel + ((y + i) * w), oziel + plo, w);
			y += vrc;
		} while(y < height); /* y */
	} /* p */

	return(0);
} /* decode_GC_TC */
*/

int convert_palette(char *pal, char *buffer, char BitsPerPixel, char imgtype)
{
	char *plane_table;

	char table2bit[] = {0, 2, 3, 1};
	char table4bit[] = {0, 2, 3, 6, 4, 7, 5, 8, 9, 10, 11, 14, 12, 15, 13, 1};

	unsigned int i, k, cols;

	struct pal
	{
		unsigned int r;
		unsigned int g;
		unsigned int b;
	} *ppal;

	struct pal *stdpal;
	struct pal stdpal2bit[] = {0xff, 0xff, 0xff,
							   0x00, 0x00, 0x00,
							   0xff, 0x00, 0x00,
							   0xd3, 0xd3, 0xd3};
	struct pal stdpal4bit[] = {0xff, 0xff, 0xff,
							   0x00, 0x00, 0x00,
							   0xff, 0x00, 0x00,
							   0x00, 0xff, 0x00,
							   0x00, 0x00, 0xff,
							   0x00, 0xff, 0xff,
							   0xff, 0xff, 0x00,
							   0xff, 0x00, 0xff,
							   0xc0, 0xc0, 0xc0,
							   0x80, 0x80, 0x80,
							   0xb6, 0x00, 0x00,
							   0x00, 0xb6, 0x00,
							   0x00, 0x00, 0xb6,
							   0x00, 0xb6, 0xb6,
							   0xb6, 0xb6, 0x00,
							   0xb6, 0x00, 0xb6};


	if(BitsPerPixel > 1)
	{
		cols = 1 << BitsPerPixel;

		if(imgtype == 'X')
		{
			ppal = (struct pal *)(buffer + 0x16);
			for(k = 0; k < cols; k++)
			{
				*pal++ = (char)((((unsigned long)ppal->r - ((ppal->r == 0x3e8) ? 1 : 0)) << 8) / 1000L);
				*pal++ = (char)((((unsigned long)ppal->g - ((ppal->g == 0x3e8) ? 1 : 0)) << 8) / 1000L);
				*pal++ = (char)((((unsigned long)ppal->b - ((ppal->b == 0x3e8) ? 1 : 0)) << 8) / 1000L);
				ppal++;
			}
		}
		else
		{
			/* umbiegen weil VDI-Indizes im Bild stehen */
			if(cols == 4 || cols == 16)
			{
				if(cols == 4)
				{
					stdpal = stdpal2bit;
					plane_table = table2bit;
				}
				else
				{
					stdpal = stdpal4bit;
					plane_table = table4bit;
				}

				for(i = 0; i < cols; i++)
				{
					k = plane_table[i];
					*pal++ = stdpal[k].r;
					*pal++ = stdpal[k].g;
					*pal++ = stdpal[k].b;
				}
			}
		}
	}
	else
	{
		pal[0] = 255;
		pal[1] = 255;
		pal[2] = 255;
		pal[3] = 0;
		pal[4] = 0;
		pal[5] = 0;
	}

	return(0);
} /* convert_palette */


char *fileext(char *filename)
{
	char *extstart;


	if((extstart = strrchr(filename, '.')) != NULL)
		extstart++;
	else
		extstart = strrchr(filename, '\0');
	
	return(extstart);
} /* fileext */