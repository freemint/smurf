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
/*						TGA-Format Importer					*/
/* Version 0.1  --  xx.xx.96								*/
/*	  Coke of Therapy, keine weiteren Infos vorhanden		*/
/* Version 0.2  --  xx.xx.96								*/
/*	  1, 8, 16, 24, 32 Bit									*/
/* Version 0.3  --  xx.xx.96								*/
/*	  von Dale bernommen und berarbeitet					*/
/* Version 0.4  --  21.03.97								*/
/*	  nochmal umgestrickt und auf meine Programmiernorm		*/
/*	  umgestellt. Unterroutinen in Funktionen gepackt.		*/
/*	  Fr Blockroutinen vorbereitet.						*/
/*	  Andere Palettenoffsets als 0 funktionieren			*/
/*	  wahrscheinlich nicht, ich br„uchte Testbilder.		*/
/* Version 0.5  --  13.04.97								*/
/*	  Fehler bei RLE von 16 Bit Bildern behoben.			*/
/* Version 0.6  --  03.05.97								*/
/*	  16 Bit Decoder wird bei komprimierten Bildern nun		*/
/*	  auch aufgerufen.										*/
/* Version 0.7  --  15.06.97								*/
/*	  Irgendwo sind mir die Zeilen abhanden gekommen, in	*/
/*	  denen der Formattyp gesetzt wird -> bei 1 Bit kam		*/
/*	  totaler Mist raus.									*/
/* Version 0.8  --  21.10.97								*/
/*	  šberwachung korrupter Runs die Speicher berschreiben	*/
/*	  wrden eingebaut.										*/ 
/* Version 0.9  --  12.12.97								*/
/*	  monochrome TGA anderer Programme werden nun nicht		*/
/*	  mehr invertiert angezeigt.							*/
/* Version 1.0  --  12.07.98								*/
/*	  Einige Sicherheitsabfragen auf ungltige Headerwerte	*/
/*	  eingebaut.											*/
/* Version 1.1  --  12.09.98								*/
/*	  Untersttzung fr umstrittenes aber von manchen Pro-	*/
/*	  grammen (z.B. MGIF) geschriebenes Format				*/
/*	  (Image Type == 3, BitsPerPixel == 8 -> Graustufenbild	*/
/*	  ohne Palette) eingebaut.								*/
/*	  Dabei einen Vertipperer aus der 1.0					*/
/*	  ((!CMapStart & !CMapLength) && ImageType == 1 ||)		*/
/*	  korrigiert											*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <demolib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);
int (*f_alert)(char *alertstring, char *b1, char *b2, char *b3, char defbt);

char *fileext(char *filename);
void read_1Bit(char *buffer, char *ziel, unsigned int w, unsigned int height);
void read_8Bit(char *buffer, char *ziel, unsigned int w, unsigned int height);
void read_16Bit(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height);
void read_24Bit(char *buffer, char *ziel, unsigned int width, unsigned int height, char BitsPerPixel);
void decode_1Bit(char *buffer, char *ziel, unsigned int w, unsigned int height);
void decode_8Bit(char *buffer, char *ziel, unsigned int w, unsigned int height);
void decode_16Bit(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height);
void decode_24Bit(char *buffer, char *ziel, unsigned int width, unsigned int height, char BitsPerPixel);
void switch_orient(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel);

/* Dies bastelt direkt ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"TGA Bitmap Importer",
						0x0110,
						"Christian Eyrich",
						"TGA","","","","",
						"","","","","",
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
/*				Targa Dekomprimierer (TGA)			*/
/*		1, 8, 16, 24 und 32 Bit, unkomprimiert, RLE	*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *pal, *ppal, *fname,
		 IDLength, ImageType, ColorMap, CMapDepth, Descriptor,
		 comp, BitsPerPixel, topdown;
	char dummy[3], impmessag[21];

	unsigned int t, CMapStart, CMapLength, width, height, w, memwidth;


/* wie schnell sind wir? */
/*	init_timer(); */
	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;
	f_alert = smurf_struct->services->f_alert;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	fname = smurf_struct->smurf_pic->filename;
	if(stricmp(fileext(fname), "TGA") != 0)
		return(M_INVALID);
	else
	{
		IDLength = *buffer;
		if(IDLength > 0)
			strcpy(smurf_struct->smurf_pic->infotext, buffer + 0x12);

		/* 0 = keine Bilddaten im File */
		/* 1 = Palettenbild, unkomprimiert */
		/* 2 = TC-Bild, unkomprimiert */
		/* 3 = Monochrom/Graustufen-Bild, unkomprimiet */
		/* 9 = Palettenbild, komprimiert */
		/* 10 = TC-Bild, komprimiert */
		/* 11 = Monochrom/Graustufen-Bild, komprimiet */
		ImageType = *(buffer + 0x02);
		if(ImageType >= 1 && ImageType <= 3)
			comp = 0;
		else
			if(ImageType >= 9 && ImageType <= 11)
			{
				comp = 1;
				ImageType -= 8;						/* Type gleich unkomprimiert setzen */
			}
			else
				return(M_PICERR);

		ColorMap = *(buffer + 0x01);
		/* kein swap_word() da es hier auf ungerade Adressen angesetzt*/
		/* werden mžte und deshalb auf'm ST zum Absturz fhrt */
/*		CMapStart = swap_word(*(unsigned int *)(buffer + 0x03)); */
		CMapStart = (unsigned int)*(buffer + 0x03) | ((unsigned int)*(buffer + 0x04) << 8);
/*		CMapLength = swap_word(*(unsigned int *)(buffer + 0x05)); */
		CMapLength = (unsigned int)*(buffer + 0x05) | ((unsigned int)*(buffer + 0x06) << 8);
		CMapDepth = *(buffer + 0x07);

		BitsPerPixel = *(buffer + 0x10);
		if(BitsPerPixel != 1 && BitsPerPixel != 8 &&
		   BitsPerPixel != 16 && BitsPerPixel != 24 && BitsPerPixel != 32)
		{
			f_alert("Ungltige Farbtiefe. Bild kann nicht geladen werden.", "OK", NULL, NULL, 1);
			return(M_PICERR);
		}

		if(ImageType == 1 && BitsPerPixel != 8 ||
		   ImageType == 2 && BitsPerPixel < 16 ||
		   ImageType == 3 && (BitsPerPixel != 1 && BitsPerPixel != 8) ||
		   (!CMapStart && !CMapLength) && ImageType == 1 ||
		   (CMapStart || CMapLength) && ImageType != 1)
		{
			if(form_alert(1, "[2][Ungltige Werte im Bildheader.|Bild auf eigene Gefahr laden?][Abbruch|OK]") == 1)
				return(M_PICERR);
		}

		width = swap_word(*(unsigned int *)(buffer + 0x0c));
		height = swap_word(*(unsigned int *)(buffer + 0x0e));

		Descriptor = *(buffer + 0x11);
		if((Descriptor & 0x20) == 0)
			topdown = 1;
		else
			topdown = 0;

		strncpy(smurf_struct->smurf_pic->format_name, "TGA Bitmap .TGA", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;

		if(BitsPerPixel == 32)
			smurf_struct->smurf_pic->depth = 24;
		else
			smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "TGA ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		buffer += 0x12;
		buffer += IDLength;

		ppal = buffer + CMapStart * 3;
		buffer += (long)CMapLength * 3L;

		if(BitsPerPixel == 1)
		{
			w = ((width + 7) / 8);
			memwidth = w * 8;
		}
		else
		{
			w = width;
			memwidth = width;
		}

		if((ziel = SMalloc(((long)memwidth * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			if(!comp)
			{
				if(BitsPerPixel == 1)
					read_1Bit(buffer, ziel, w, height);
				else
					if(BitsPerPixel == 8)
						read_8Bit(buffer, ziel, w, height);
					else
						if(BitsPerPixel == 16)
							read_16Bit((unsigned int *)buffer, (unsigned int *)ziel, width, height);
						else
							if(BitsPerPixel > 16)
							{
								read_24Bit(buffer, ziel, width, height, BitsPerPixel);
								BitsPerPixel = 24;
							}
			}
			else
			{
				if(BitsPerPixel == 1)
					decode_1Bit(buffer, ziel, w, height);
				else
					if(BitsPerPixel == 8)
						decode_8Bit(buffer, ziel, w, height);
					else
						if(BitsPerPixel == 16)
							decode_16Bit((unsigned int *)buffer, (unsigned int *)ziel, width, height);
						else
							if(BitsPerPixel > 16)
							{
								decode_24Bit(buffer, ziel, width, height, BitsPerPixel);
								BitsPerPixel = 24;
							}
			}

			buffer = obuffer;

			if(topdown)											/* Bild falschrum */
				switch_orient(ziel, width, height, BitsPerPixel);

			if(BitsPerPixel == 1)
				smurf_struct->smurf_pic->format_type = FORM_STANDARD;
			else
				smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

			smurf_struct->smurf_pic->pic_data = ziel;

		/* Palette */
			pal = smurf_struct->smurf_pic->palette;

			if(ImageType == 3)					/* monochrome or grayscale image */
			{
				if(BitsPerPixel == 1)
				{
					pal[0] = 0;
					pal[1] = 0;
					pal[2] = 0;
					pal[3] = 255;
					pal[4] = 255;
					pal[5] = 255;
				}
				else
				{
					for(t = 0; t < 256; t++)
					{
						*pal++ = (char)t;
						*pal++ = (char)t;
						*pal++ = (char)t;
					}
				}
			}
			else
				if(ImageType == 1)				/* colormapped image */
				{
					for(t = CMapStart; t < CMapLength; t++)
					{
						*pal++ = *(ppal++ + 2);
						*pal++ = *ppal++;
						*pal++ = *(ppal++ - 2);
					}
				}

			if(ImageType == 3 && BitsPerPixel == 8)
				smurf_struct->smurf_pic->col_format = GREY;
			else
				smurf_struct->smurf_pic->col_format = RGB;
		} /* Malloc */
	} /* Erkennung */

/* wie schnell waren wir? */
/*	printf("%lu", get_timer());
	getch(); */

	SMfree(buffer);

	return(M_PICDONE);
}


/* leider wird eine eigene Funktion ben”tigt weil monochrome Bilder */
/* invertiert vorliegen */
void read_1Bit(char *buffer, char *ziel, unsigned int w, unsigned int height)
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
	} while(++y < height);

	return;
} /* read_1Bit */


void read_8Bit(char *buffer, char *ziel, unsigned int w, unsigned int height)
{
	unsigned int x, y;


	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *buffer++;
		} while(++x < w);
	} while(++y < height);

	return;
} /* read_8Bit */


void read_16Bit(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height)
{
	unsigned int x, y, pixel16;


	y = 0;
	do
	{
		x = 0;
		do
		{
		/* Vorsicht: Nach Motorola und dann arrrrrgg|gggbbbbb nach rrrrrggg|ggabbbbb ... */
			pixel16 = swap_word(*buffer++);
			*ziel++ = ((pixel16 & 0x7fe0) << 1) | (pixel16 & 0x001f);
		} while(++x < width);
	} while(++y < height);

	return;
} /* read_16Bit */


void read_24Bit(char *buffer, char *ziel, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char skip = 0;

	unsigned int x, y;


	if(BitsPerPixel == 32)
		skip = 1;

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *(buffer++ + 2);
			*ziel++ = *buffer++;
			*ziel++ = *(buffer++ - 2);
			if(skip)							/* Alphachannel berspringen */
				buffer++;
		} while(++x < width);
	} while(++y < height);

	return;
} /* read_24Bit */


/* leider wird eine eigene Funktion ben”tigt weil monochrome Bilder */
/* invertiert vorliegen */
void decode_1Bit(char *buffer, char *ziel, unsigned int w, unsigned int height)
{
	char v1, v2, n;

	unsigned int x, y;

	long maxlen;


	maxlen = (long)w * (long)height;

	y = 0;
	do
	{
		x = 0;
		do
		{
			v1 = *buffer++;

			if(v1&0x80)
			{
				v2 = ~*buffer++;

				n = v1 - 0x7f;

				x += n;

				while(n--)
				{
					*ziel++ = v2;
					if(!--maxlen)
						goto end;
				}
			}
			else
			{
				n = v1 + 1;

				x += n;

				while(n--)
				{
					*ziel++ = ~*buffer++;
					if(!--maxlen)
						goto end;
				}
			}
		} while(x < w);
	} while(++y < height);

end:

	return;
} /* decode_1Bit */


void decode_8Bit(char *buffer, char *ziel, unsigned int w, unsigned int height)
{
	char v1, v2, n;

	unsigned int x, y;

	long maxlen;


	maxlen = (long)w * (long)height;

	y = 0;
	do
	{
		x = 0;
		do
		{
			v1 = *buffer++;

			if(v1&0x80)
			{
				v2 = *buffer++;

				n = v1 - 0x7f;

				x += n;

				while(n--)
				{
					*ziel++ = v2;
					if(!--maxlen)
						goto end;
				}
			}
			else
			{
				n = v1 + 1;

				x += n;

				while(n--)
				{
					*ziel++ = *buffer++;
					if(!--maxlen)
						goto end;
				}
			}
		} while(x < w);
	} while(++y < height);

end:

	return;
} /* decode_8Bit */


void decode_16Bit(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height)
{
	char n;

	unsigned int x, y, v1, v2, pixel16;

	long maxlen;


	maxlen = (long)width * (long)height;

	y = 0;
	do
	{
		x = 0;
		do
		{
			v1 = *((char *)buffer)++;

			if(v1&0x80)
			{
				/* Vorsicht: Nach Motorola und dann arrrrrgg|gggbbbbb nach rrrrrggg|ggabbbbb ... */
				pixel16 = swap_word(*buffer++);
				v2 = ((pixel16 & 0x7fe0) << 1) | (pixel16 & 0x001f);

				n = v1 - 0x7f;

				x += n;

				while(n--)
				{
					*ziel++ = v2;
					if(!--maxlen)
						goto end;
				}
			}
			else
			{
				n = v1 + 1;

				x += n;

				while(n--)
				{
				/* Vorsicht: Nach Motorola und dann arrrrrgg|gggbbbbb nach rrrrrggg|ggabbbbb ... */
					pixel16 = swap_word(*buffer++);
					*ziel++ = ((pixel16 & 0x7fe0) << 1) | (pixel16 & 0x001f);
					if(!--maxlen)
						goto end;
				}
			}
		} while(x < width);
	} while(++y < height);

end:

	return;
} /* decode_16Bit */


void decode_24Bit(char *buffer, char *ziel, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char v1, v2[3], n, skip;

	unsigned int x, y;

	long maxlen;


	if(BitsPerPixel == 32)
		skip = 1;
	else
		skip = 0;

	maxlen = (long)width * (long)height;

	y = 0;
	do
	{
		x = 0;
		do
		{
			v1 = *buffer++;

			if(v1&0x80)
			{
				v2[0] = *buffer++;
				v2[1] = *buffer++;
				v2[2] = *buffer++;
				if(skip)
					buffer++;

				n = v1 - 0x7f;

				x += n;

				while(n--)
				{
					*ziel++ = v2[2];
					*ziel++ = v2[1];
					*ziel++ = v2[0];
					if(!--maxlen)
						goto end;
				}
			}
			else
			{
				n = v1 + 1;

				x += n;

				while(n--)
				{
					*ziel++ = *(buffer++ + 2);
					*ziel++ = *buffer++;
					*ziel++ = *(buffer++ - 2);
					if(skip)
						buffer++;
					if(!--maxlen)
						goto end;
				}
			}
		} while(x < width);
	} while(++y < height);

end:

	return;
} /* decode_24Bit */


void switch_orient(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *buffer2, *temp;

	unsigned int y;

	unsigned long realwidth;


	if(BitsPerPixel == 1)
		realwidth = (long)((width + 7) / 8);
	else
		realwidth = ((long)width * (long)BitsPerPixel) >> 3;

	if((temp = Malloc(realwidth)) == 0)
	{
		f_alert("Nicht genug Speicher um die Orientierung zu „ndern", "OK", NULL, NULL, 1);
		return;
	}
	else
	{
		buffer2 = buffer + (height - 1) * realwidth;

		y = height >> 1;
		do
		{
			memcpy(temp, buffer, realwidth);
			memcpy(buffer, buffer2, realwidth);
			memcpy(buffer2, temp, realwidth);

			buffer += realwidth;
			buffer2 -= realwidth;
		} while(--y);
	}

	Mfree(temp);

	return;
} /* switch_orientation */


char *fileext(char *filename)
{
	char *extstart;


	if((extstart = strrchr(filename, '.')) != NULL)
		extstart++;
	else
		extstart = strrchr(filename, '\0');
	
	return(extstart);
} /* fileext */