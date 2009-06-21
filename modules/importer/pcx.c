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
/*						PCX/SCR-Decoder						*/
/* Version 0.1  --  09.10.97 - 13.10.97						*/
/*	  1-8 (auûer 2 Bit), 24 Bit								*/
/*		Der totale Scheiû weil manche Programme meinen, quer*/
/*		Åber Planes und Zeilen packen zu mÅssen. Dadurch	*/
/*		sind die vielen Abfragen mitten im RLE nîtig.		*/
/*		Das ist hÑûlich, langsam und Scheiûe zu warten.		*/
/* Version 0.2  --  xx.xx.97								*/
/* Version 0.3  --  xx.xx.97								*/
/*	  DCX-UnterstÅtzung										*/
/* =========================================================*/

#ifdef GERMAN
#define ERROR1 "[1][Eine Palette fehlt leider im Bild. BehelfsmÑûig | wird eine Graustufenpalette erzeugt.][ OK ]"
#else
#ifdef ENGLISH
#define ERROR1 "[1][Sorry, a palette is missing in the image.|To help a greyscale palette will be created.][ OK ]"
#else
#ifdef FRENCH
#define ERROR1 "[1][Sorry, a palette is missing in the image.|To help a greyscale palette will be created.][ OK ]"
#else
#error "Keine Sprache!"
#endif
#endif
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<tos.h>
#include<ext.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define DEBUG	0
#define	PCX		1
#define SCR		2

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

void read_1Bit(char *ziel, char *buffer, int w, int height, char v);
char *read_8and24Bit(char *ziel, char *buffer, int w, int height, char Planes, char v);
char *read_3and4Bit(char *ziel, char *buffer, int w, int height, char Planes, char v);
void decode_1Bit(char *ziel, char *buffer, int w, int height, char v);
char *decode_3and4Bit(char *ziel, char *buffer, int w, int height, char Planes, char v);
char *decode_8and24Bit(char *ziel, char *buffer, int w, int height, char Planes, char v);

/* Dies bastelt direkt ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"PCX/SCR-Importer",
						0x0010,
						"Christian Eyrich",
						"PCX", "SCR", "", "", "",
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
/*					PCX/SCR-Decoder					*/
/*	  1-8, 24 Bit, unkomprimiert und RLE			*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel, *pal, *ppal, *back,
		 v, v0, v1, ImageType, version, comp, BitsPerPixel, Planes;
	char dummy[3], impmessag[21];

	unsigned int width, height, i,
				 HRes, VRes, BytesPerLine, cols, w;

	long f_len, memwidth, pos;


	f_len = smurf_struct->smurf_pic->file_len;

	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;

	v0 = *buffer;
	v1 = *(buffer + 0x01); 
	if(v0 != 0x0a && v0 != 0xcd || (v1 != 0 && v1 != 2 && v1 != 3 && v1 != 5) || (*(buffer + 2) >= 2))
		return(M_INVALID);
	{
		if(v0 == 0x0a)
			ImageType = PCX;
		else
			if(v0 == 0xcd)
				ImageType = SCR;

		version = *(buffer + 0x01);
		comp = *(buffer + 0x02);
		BitsPerPixel = *(buffer + 0x03);

		/* XEnd - XStart */
		width = swap_word(*(unsigned int *)(buffer + 0x08)) - swap_word(*(unsigned int *)(buffer + 0x04)) + 1;
		/* YEnd - YStart */
		height = swap_word(*(unsigned int *)(buffer + 0x0a)) - swap_word(*(unsigned int *)(buffer + 0x06)) + 1;
		HRes = swap_word(*(unsigned int *)(buffer + 0x0c));
		VRes = swap_word(*(unsigned int *)(buffer + 0x0e));
		ppal = buffer + 0x10;
		Planes = *(buffer + 0x41);
		BitsPerPixel *= Planes;
		BytesPerLine = swap_word(*(unsigned int *)(buffer + 0x42));

		strncpy(smurf_struct->smurf_pic->format_name, "PCX Paintbrush .PCX", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "PCX ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if(BitsPerPixel < 8)
		{
			w = (width + 7) / 8;
			memwidth = w * 8;
		}
		else
		{
			w = width;
			memwidth = width;
		}

		v = BytesPerLine - w;
/*
		printf("BytesPerLine: %d\n", (int)BytesPerLine);
		printf("comp: %d, BitsPerPixel: %d, v : %d\n", (int)comp, (int)BitsPerPixel, (int)v);
		getch();
*/
		if((ziel = SMalloc((((long)memwidth * (long)height * BitsPerPixel) >> 3) + 1)) == 0)
			return(M_MEMORY);
		else
		{
			if(!comp)
				switch((int)BitsPerPixel)
				{
					case 1: read_1Bit(ziel, buffer + 128, w, height, v);
							break;
/*					case 2: back = read_2Bit(ziel, buffer, v);
							break; */
					case 3:
					case 4: back = read_3and4Bit(ziel, buffer + 128, w, height, BitsPerPixel, v);
							break;
					case 8:
					case 24: back = read_8and24Bit(ziel, buffer + 128, w, height, BitsPerPixel >> 3, v);
							 break;
					default: SMfree(ziel);
							 return(M_PICERR);
				}
			else
				switch((int)BitsPerPixel)
				{
					case 1: decode_1Bit(ziel, buffer + 128, w, height, v);
							break;
/*					case 2: back = decode_2Bit(ziel, buffer, v);
							break; */
					case 3:
					case 4: back = decode_3and4Bit(ziel, buffer + 128, w, height, BitsPerPixel, v);
							break;
					case 8:
					case 24: back = decode_8and24Bit(ziel, buffer + 128, w, height, BitsPerPixel >> 3, v);
							 break;
					default: SMfree(ziel);
							 return(M_PICERR);
				}

			if(BitsPerPixel < 8)
				smurf_struct->smurf_pic->format_type = FORM_STANDARD;
			else
				smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

			smurf_struct->smurf_pic->pic_data = ziel;

			smurf_struct->smurf_pic->col_format = RGB;

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
					pos = back - buffer;
/*
					printf("pos: %ld, f_len - pos: %ld\n", pos, f_len - pos);
					getch();
*/
					cols = 1 << BitsPerPixel;

					/* Langt die EGA-Palette sowieso nicht oder liegt */
					/* das letzte Bildbyte mindestends 768 Bytes vor */
					/* Fileende (also wahrscheinlich eine Palette vorhanden) */
					/* und ist die Palettenmarkierung vorhanden? */
					if(BitsPerPixel > 4)
						if(f_len - pos >= 3 * cols &&			/* Palette vorhanden */
						   *(buffer + f_len - 769) == 0x0c)
						{
							ppal = buffer + f_len - 768;

							for(i = 0; i < cols; i++)
							{
								*pal++ = *ppal++;
								*pal++ = *ppal++;
								*pal++ = *ppal++;
							}
						}
						else
						{
							for(i = 0; i < cols; i++)
							{
								*pal++ = (char)i;
								*pal++ = (char)i;
								*pal++ = (char)i;
							}

							form_alert(0, ERROR1 );
						}
					else
						if(BitsPerPixel <= 4)
							/* EGA-Palette im Fileheader auslesen */
							for(i = 0; i < cols; i++)
							{
								*pal++ = *ppal++;
								*pal++ = *ppal++;
								*pal++ = *ppal++;
							}
				}
			}
		} /* Malloc */
	} /* Erkennung */

/* wie schnell waren wir? */
/*	printf("%lu", get_timer());
	getch(); */

	SMfree(buffer);
	
	return(M_PICDONE);
}


/* Muû wegen der Invertierung leider in eigene Funktion */
void read_1Bit(char *ziel, char *buffer, int w, int height, char v)
{
	unsigned int x, y;


	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = ~*buffer++;
		} while(++x < w);
		buffer += v;
	} while(++y < height);

	return;
} /* read_1Bit */


char *read_3and4Bit(char *ziel, char *buffer, int w, int height, char Planes, char v)
{
	char *oziel,
		 p;

	unsigned int x, y;

	long offset, planelength;


	offset = w;
	planelength = (long)w * (long)height;

	oziel = ziel;

	y = 0;
	do
	{
		p = 0;
		do
		{
			ziel = oziel + p * planelength;

			x = 0;
			do
			{
				*ziel++ = *buffer++;
			} while(++x < w);
			buffer += v;
		} while(++p < Planes);
		oziel += offset;
	} while(++y < height);

	return(buffer);
} /* read_3and4Bit */


char *read_8and24Bit(char *ziel, char *buffer, int w, int height, char Planes, char v)
{
	char *oziel,
		 p;

	unsigned int x, y;

	long offset;


	offset = w * Planes;

	oziel = ziel;

	y = 0;
	do
	{
		p = 0;
		do
		{
			ziel = oziel + p;

			x = 0;
			do
			{
				*ziel = *buffer++;
				ziel += Planes;
			} while(++x < w);
			buffer += v;
		} while(++p < Planes);
		oziel += offset;
	} while(++y < height);

	return(buffer);
} /* read_8and24Bit */


/* Muû wegen der Invertierung leider in eigene Funktion */
void decode_1Bit(char *ziel, char *buffer, int w, int height, char v)
{
	char n, v1, v2;

	unsigned int x;

	long length, x2;


	length = (long)w * (long)height;

	w += v;
	x = 0;
	x2 = 0;
	do
	{
		v1 = *buffer++;
		if((v1&0xc0) == 0xc0)				/* Run */
		{	
			n = v1&0x3f;

			x2 += n;
			v2 = ~*buffer++;

			while(n--)
			{
				*ziel++ = v2;
				if(++x >= w)
				{
					x = 0;
					ziel -= v;
					x2 -= v;
				}
			}
		}
		else								/* direkt */
		{
			x2++;

			*ziel++ = ~v1;
			if(++x >= w)
			{
				x = 0;
				ziel -= v;
				x2 -= v;
			}
		}
	} while(x2 < length);

	return;
} /* decode_1Bit */


char *decode_3and4Bit(char *ziel, char *buffer, int w, int height, char Planes, char v)
{
	char *oziel,
		 p, n, v1, v2;

	unsigned int x;

	long offset, planelength, length, x2 = 0;


	offset = w;
	planelength = (long)w * (long)height;
	length = (long)w * (long)height * Planes;

	oziel = ziel;

	w += v;

	x = 0;
	x2 = 0;
	p = 0;
	do
	{
		v1 = *buffer++;
		if((v1&0xc0) == 0xc0)				/* Run */
		{	
			n = v1&0x3f;

			x2 += n;
			v2 = *buffer++;

			while(n--)
			{
				*ziel++ = v2;
				if(++x >= w)
				{
					x = 0;
					if(++p >= Planes)
					{
						p = 0;
						oziel += offset;
					}
					ziel = oziel + p * planelength;
					x2 -= v;
				}
			}
		}
		else								/* direkt */
		{
			x2++;

			*ziel++ = v1;
			if(++x >= w)
			{
				x = 0;
				if(++p >= Planes)
				{
					p = 0;
					oziel += offset;
				}
				ziel = oziel + p * planelength;
				x2 -= v;
			}
		}
	} while(x2 < length);

	return(buffer);
} /* decode_3and4Bit */


char *decode_8and24Bit(char *ziel, char *buffer, int w, int height, char Planes, char v)
{
	char *oziel,
		 p, n, v1, v2;

	unsigned int x;

	long length, offset, x2;


	length = (long)w * (long)height * Planes;

	offset = w * Planes;

	oziel = ziel;

	w += v;
	x = 0;
	x2 = 0;
	p = 0;
	do
	{
		v1 = *buffer++;
		if((v1&0xc0) == 0xc0)				/* Run */
		{	
			n = v1&0x3f;

			x2 += n;
			v2 = *buffer++;

			while(n--)
			{
				*ziel = v2;
				ziel += Planes;
				if(++x >= w)
				{
					x = 0;
					if(++p >= Planes)
					{
						p = 0;
						oziel += offset;
					}
					ziel = oziel + p;
					ziel -= v * Planes;
					x2 -= v;
				}
			}
		}
		else								/* direkt */
		{
			x2++;

			*ziel = v1;
			ziel += Planes;
			if(++x >= w)
			{
				x = 0;
				if(++p >= Planes)
				{
					p = 0;
					oziel += offset;
				}
				ziel = oziel + p;
				ziel -= v * Planes;
				x2 -= v;
			}
		}
	} while(x2 < length);

	return(buffer);
} /* decode_8and24Bit */