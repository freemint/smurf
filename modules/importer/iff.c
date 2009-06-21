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
/*                  Amiga IFF-Format Decoder				*/
/* Version 0.1  --  16.01.96 - 16.02.96						*/
/*	  unkomprimierte und komprimierte 8 Bit					*/
/* Version 0.2  --  16.02.96								*/
/*	  1, 4, 8, 24 Bit										*/
/* Version 0.3  --  24.08.96								*/
/*	  24 Planes werden zu einer Plane, 3 Byte				*/
/* Version 0.4  --  07.11.96								*/
/*	  HAM-Modus (6 Bit) wird jetzt unterstÅtzt				*/
/* Version 0.5  --  22.10.97								*/
/*	  Runderneuerung, u.a eigene Funktion fÅr PBM			*/
/* Version 0.6  --  07.04.98								*/
/*	  5 Bit IFF gehen jetzt auch (gingen eigentlich schon	*/
/*	  immer, nur war unnîtigerweise eine PrÅfung drinnen,	*/
/*	  die 5 Bit IFF verhindert hat). Weiterhin werden 6 Bit	*/
/*	  IFF jetzt nicht mehr immer zu HAM erklÑrt, sondern	*/
/*	  nur explizit als HAM ausgezeichnete IFF werden als	*/
/*	  HAM gewertet.											*/
/* Version 0.7  --  11.05.98								*/
/*	  evtl. vorhandene Maskenplanes werden jetzt korrekt 	*/
/*	  Åbergangen											*/
/* Version 0.8  --  xx.xx.98								*/
/*	  UnterstÅtzung von HALFBRITE							*/
/* =========================================================*/

#ifdef GERMAN
#define ERROR1 "[1][Kein ILBM- oder PBM-Bild][ OK ]"
#else
#ifdef ENGLISH
#define ERROR1 "[1][Kein ILBM- oder PBM-Bild][ OK ]"
#else
#ifdef FRENCH
#define ERROR1 "[1][Kein ILBM- oder PBM-Bild][ OK ]"
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

#define HAM			0x800
#define HALFBRITE	0x80

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Get 16 Pixel (Interleaved Standard Format) Assembler-Rout */
void get_IBPLine(char *dest, char *src, long lineplanelen);
void get_IBPLine8(char *dest, char *src, long lineplanelen);

char *srchchunk(char *buffer, unsigned long var);
char *decodeIFF1(char *buffer, unsigned int width, unsigned int height, char comp);
char *decodeIFF(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel, char comp, char mask);
char *decodePBM(char *buffer, unsigned int w, unsigned int height, char BitsPerPixel,char comp);
char *decodeTCIFF(char *buffer, unsigned int width, unsigned int height, char comp);
char *decodeHAM(char *buffer, unsigned int width, unsigned int height, char *palette, char Planes, char comp);

int (*busybox)(int pos);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Amiga-IFF Importer",
						0x0070,
						"Christian Eyrich",
						"IFF", "LBM", "HAM", "ILM", "ILBM",
						"PBM", "", "", "", "",
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

char *FileEnd;
char *obuf;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*	IFF Dekomprimierer (ILBM) - CHUNK-Food-Format	*/
/*		1, 4, 5, 6, 8, 24 Bit, RLE					*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *pal, *ppal,
		 BitsPerPixel, comp, imgtype, HAMmode = FALSE, Planes, mask;
	char dummy[3], impmessag[21];

	unsigned int cols, k, width, height, DatenOffset;

	unsigned long FileLen;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	busybox = smurf_struct->services->busybox;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuf = obuffer = buffer;

	if(*(unsigned long *)buffer != 'FORM')				/* FORM */
		return(M_INVALID);
	else
	{
	/* Bereichskennung */
		if(*(unsigned long *)(buffer + 0x08) == 'ILBM')
			imgtype = 'I';
		else
			if(*(unsigned long *)(buffer + 0x08) == 'PBM ')
				imgtype = 'P';
			else
			{
				form_alert(0, ERROR1 );
				return(M_UNKNOWN_TYPE);
			}

		/* LÑnge des Files - manchmal inklusive der ersten 8 Bytes, manchmal ohne */
		FileLen = *(unsigned long *)(buffer + 0x04);
		FileEnd = buffer + FileLen;
		
		/* search for BMHD-Chunk */
		if((buffer = srchchunk(obuffer, 'BMHD')) == NULL)
			return(M_PICERR);

		width = *(unsigned int *)(buffer + 0x08); 
		height = *(unsigned int *)(buffer + 0x0a);

		Planes = BitsPerPixel = *(buffer + 0x10);

		mask = *(buffer + 0x11);

		comp = *(buffer + 0x12);

		/* search for CAMG-Chunk */
		/* muû aber nicht vorhanden sein! */
		if((buffer = srchchunk(obuffer, 'CAMG')) != NULL)
		{		
			if(*(unsigned long *)(buffer + 0x08) & HAM)
				HAMmode = TRUE;
		}

/*		if(BitsPerPixel == 6)
			HAMmode = TRUE; */

		/* search for CMAP-Chunk */
		if(BitsPerPixel < 24)
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
				if((buffer = srchchunk(obuffer, 'CMAP')) == NULL)
					return(M_PICERR);

				cols = (unsigned int)*(unsigned long *)(buffer + 0x04) / 3;
	
				ppal = buffer + 0x08;

				for(k = 0; k < cols; k++)
				{
					*pal++ = *ppal++;
					*pal++ = *ppal++;
					*pal++ = *ppal++;
				}
			}
		}

		/* search for BODY-Chunk */
		if((buffer = srchchunk(obuffer, 'BODY')) == NULL)
			return(M_PICERR);

		DatenOffset = 0x08;

		if(HAMmode == TRUE)
			BitsPerPixel = 24;

		if(imgtype == 'I')
			strncpy(smurf_struct->smurf_pic->format_name, "IFF-ILBM-Image .LBM", 21);
		else
			strncpy(smurf_struct->smurf_pic->format_name, "IFF-PBM-Image .LBM", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		if(imgtype == 'I')
			if(HAMmode == TRUE)
				strcpy(impmessag, "HAM-ILBM ");
			else
				strcpy(impmessag, "IFF-ILBM ");
		else
			strcpy(impmessag, "IFF-PBM ");
		strcat(impmessag, itoa(Planes, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(0, impmessag);

		if(imgtype == 'I')
			if(BitsPerPixel < 24)
				if(BitsPerPixel == 1)
					ziel = decodeIFF1(buffer + DatenOffset, width, height, comp);
				else
					ziel = decodeIFF(buffer + DatenOffset, width, height, BitsPerPixel, comp, mask & 0x01);
			else
				if(HAMmode == TRUE)
					ziel = decodeHAM(buffer + DatenOffset, width, height, smurf_struct->smurf_pic->palette, Planes, comp);
				else
					ziel = decodeTCIFF(buffer + DatenOffset, width, height, comp);
		else
			ziel = decodePBM(buffer + DatenOffset, width, height, BitsPerPixel, comp);

		if(ziel == NULL)
			return(M_MEMORY);
		else
			smurf_struct->smurf_pic->pic_data = ziel;

		buffer = obuffer;

		if(BitsPerPixel == 24 || imgtype == 'P')
			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
		else
			smurf_struct->smurf_pic->format_type = FORM_STANDARD;

		smurf_struct->smurf_pic->col_format = RGB;
	} /* Erkennung */

	SMfree(buffer);

	return(M_PICDONE);
}


/* Dekodiert ganz normale IFF mit 1 Plane aus dem Interleaved- */
/* Bitplane ins Standardformat, gepackt und ungepackt */
/* Muû wegen der Invertierung leider in eigene Funktion */
char *decodeIFF1(char *buffer, unsigned int width, unsigned int height, char comp)
{		
	char *ziel, *oziel,
		v, v1, v2, n;

	unsigned int x, y, bh, bl;

	unsigned long w, w16, memwidth;


	w = (unsigned long)((width + 7) / 8);			/* Auf volle Byte gerundete ZeilenlÑnge in Byte */
	memwidth = w * 8L;
	w16 = (unsigned long)((width + 15) / 16) * 2;	/* Auf volle Byte gerundete ZeilenlÑnge in Byte */
	v = w16 - w;									/* da manche Encoder auf 16 Pixel runden ... */

	if((ziel = SMalloc(((memwidth * (unsigned long)height) >> 3) + 1)) == 0)
		return(NULL);
	else
	{
		oziel = ziel;

		if((bh = height / 10) == 0) 	/* busy-height */
			bh = height;
		bl = 0;							/* busy-length */

		y = 0;
		do
		{
			if(!(y%bh))
			{
				busybox(bl);
				bl += 12;
			}

			if(!comp)
			{
				x = 0;
				do
				{
					*ziel++ = ~*buffer++;
				} while(++x < w16);
				ziel -= v;
			}
			else
			{
				x = 0;
				do
				{
					v1 = *buffer++;
					if(v1 > 0x7f)
					{
						n = 0x101 - v1;

						v2 = ~*buffer++;

						x += n;

						while(n--)
							*ziel++ = v2;
					}
					else
					{
						n = v1 + 1;

						x += n;

						while(n--)
							*ziel++ = ~*buffer++;
					}
				} while(x < w16);
				ziel -= v;
			} /* comp? */
		} while(++y < height);
	} /* Malloc */

	ziel = oziel;

	return(ziel);
} /* decodeIFF1 */


/* Dekodiert ganz normale IFF mit 2-8 Planes aus dem Interleaved- */
/* Bitplane ins Standardformat, gepackt und ungepackt */
char *decodeIFF(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel, char comp, char mask)
{		
	char *ziel, *oziel,
		 v1, v2, n, p, Planes;

	unsigned int x, y, bh, bl;

	unsigned long w, w16, memwidth, pla, plh, plo;


	w = (unsigned long)((width + 7) / 8);			/* Auf volle Byte gerundete ZeilenlÑnge in Byte */
	memwidth = w * 8L;
	w16 = (unsigned long)((width + 15) / 16) * 2;	/* Auf volle Byte gerundete ZeilenlÑnge in Byte */

	Planes = BitsPerPixel;

	if((ziel = SMalloc(((memwidth * (unsigned long)height * BitsPerPixel) >> 3) + 1)) == 0)
		return(NULL);
	else
	{
		oziel = ziel;

		plh = (unsigned long)height * w;	/* Hîhe einer Plane in Byte */

		if((bh = height / 10) == 0) 		/* busy-height */
			bh = height;
		bl = 0;								/* busy-length */

		y = 0;
		do
		{
			if(!(y%bh))
			{
				busybox(bl);
				bl += 12;
			}

			plo = (unsigned long)y * w;			/* Offset vom Planeanfang in Bytes */
			pla = 0;

			p = 0;
			do
			{
				ziel = oziel + pla + plo;		/* Zieladresse der dekodierten Scanline */

				if(!comp)
				{
					x = 0;
					do
					{
						*ziel++ = *buffer++;
					} while(++x < w16);
				}
				else
				{
					x = 0;
					do
					{
						v1 = *buffer++;
						if(v1 > 0x7f)
						{
							n = 0x101 - v1;

							v2 = *buffer++;

							x += n;

							while(n--)
								*ziel++ = v2;
						}
						else
						{
							n = v1 + 1;

							x += n;

							while(n--)
								*ziel++ = *buffer++;
						}
					} while(x < w16);
				} /* comp? */
				pla += plh;						/* Abstand dieser Plane vom Bildanfang */
			} while(++p < Planes);

			if(mask)
			{
				if(!comp)
					buffer += w16;
				else
				{
					x = 0;
					do
					{
						v1 = *buffer++;
						if(v1 > 0x7f)
						{
							n = 0x101 - v1;

							buffer++;

							x += n;
						}
						else
						{
							n = v1 + 1;

							x += n;

							buffer += n;
						}
					} while(x < w16);
				} /* comp? */
			}
		} while(++y < height);
	} /* Malloc */

	ziel = oziel;

	return(ziel);
} /* decodeIFF */


/* Dekodiert TC-IFF (24 Planes), aus dem Interleaved */
/* Bitplaneformat nach pixelpacked, gepackt und ungepackt */
char *decodeTCIFF(char *buffer, unsigned int width, unsigned int height, char comp)
{
	char *ziel, *oziel, *pixbuf, *opixbuf,
		 v1, v2, n, p;

	unsigned int x, y, bh, bl;

	unsigned long w, linelen;


	w = (width + 15) / 16; /* Auf 16 Pixel gerundete ZeilenlÑnge in Byte */
	w = w * 2;

	linelen = (unsigned long)width * 3L;

	pixbuf = (char *)SMalloc(w * 24L);	/* Puffer fÅr Standarddaten */
	opixbuf = pixbuf;

	/* So viel Speicher mehr weil die Assemblerroutine zuviel schreibt */
	if((ziel = SMalloc(((unsigned long)w * 8L) * (unsigned long)height * 3L + 1L)) == 0)
		return(NULL);
	else
	{
		oziel = ziel;

		memset(ziel, 0x0, (unsigned long)width * (unsigned long)height * 3L);

		if((bh = height / 10) == 0) 		/* busy-height */
			bh = height;
		bl = 0;								/* busy-length */

		y = 0;
		do
		{
			if(!(y%bh))
			{
				busybox(bl);
				bl += 12;
			}

			if(!comp)
			{
				get_IBPLine(ziel, buffer, w);
				buffer += 24L * w;
				ziel += linelen;
			}
			else
			{
				p = 0;
				do
				{
					x = 0;
					do
					{
						v1 = *buffer++;

						if(v1 > 0x7f)
						{
							n = 0x101 - v1;

							v2 = *buffer++;

							x += n;

							while(n--)
								*pixbuf++ = v2;
						}
						else
						{
							n = v1 + 1;

							x += n;

							while(n--)
								*pixbuf++ = *buffer++;
						}
					} while(x < w);
				} while(++p < 24);

				pixbuf = opixbuf;
				get_IBPLine(ziel, pixbuf, w);
				ziel += linelen;
			} /* comp? */
		} while(++y < height);
	} /* Malloc */

	ziel = oziel;

	SMfree(pixbuf);

	return(ziel);
} /* decodeTCIFF */



/* Dekodiert HAM-IFF mit 6-8 Planes aus dem Interleaved */
/* Bitplaneformat nach pixelpacked, gepackt und ungepackt */
/* Danach wird dieses pixelpacked-Bild verHAMt. */
char *decodeHAM(char *buffer, unsigned int width, unsigned int height, char *palette, char Planes, char comp)
{
	char *ziel, *oziel, *pixbuf, *opixbuf, *pal,
		 v1, v2, n, p, val, hamval,
		 pix, lastvalr, lastvalg, lastvalb, hambits, colbits, colbits_complement, colmask, pixmask;

	unsigned int x, y, bh, bl;

	unsigned long w;


	hambits = 2;
	pixmask = (1 << Planes) - 1;
	colbits = Planes - hambits;
	colbits_complement = 8 - colbits;
	colmask = (1 << colbits) - 1;

	w = (width + 15) / 16; /* Auf volle Byte gerundete ZeilenlÑnge in Byte */
	w = w * 2;

	pixbuf = (char *)SMalloc(w * 24L);		/* Puffer fÅr Standarddaten */
	opixbuf = pixbuf;

	/* Gleich ein TC-Bild */
	/* So viel Speicher mehr weil die Assemblerroutine zuviel schreibt */
	if((ziel = SMalloc(((unsigned long)w * 8) * (unsigned long)height * 3L + 1L)) == 0)
		return(NULL); 
	else
	{
		oziel = ziel;

		memset(ziel, 0x0, (unsigned long)width * (unsigned long)height * 3L);

		if((bh = height / 10) == 0) 		/* busy-height */
			bh = height;
		bl = 0;								/* busy-length */

		y = 0;
		do
		{
			if(!(y%bh))
			{
				busybox(bl);
				bl += 12;
			}

			memset(pixbuf, 0x0, w * 24L);

			if(!comp)
			{
				memcpy(pixbuf, buffer, w * Planes);
				get_IBPLine8(ziel, buffer, w);
				buffer += Planes * w;
			}
			else
			{
				p = 0;
				do
				{
					x = 0;
					do
					{
						v1 = *buffer++;

						if(v1 > 0x7f)
						{
							n = 0x101 - v1;

							v2 = *buffer++;

							x += n;

							while(n--)
								*pixbuf++ = v2;
						}
						else
						{
							n = v1 + 1;

							x += n;

							while(n--)
								*pixbuf++ = *buffer++;
						}
					} while(x < w);
				} while(++p < Planes);

				pixbuf = opixbuf;
				get_IBPLine8(ziel, pixbuf, w);
			} /* comp? */

			/* VerHAMung */
			lastvalr = lastvalg = lastvalb = 0;

			x = 0;
			do
			{
				pix = *ziel & pixmask;
				val = pix & colmask;
				hamval = pix >> colbits;

				if(hamval == 0x0)
				{
					pal = palette + val + val + val;

					lastvalr = *ziel++ = *pal;
					lastvalg = *ziel++ = *(pal + 1);
					lastvalb = *ziel++ = *(pal + 2);
				}
				else
					if(hamval == 0x1)
					{
						*ziel++ = lastvalr;
						*ziel++ = lastvalg;
						lastvalb = *ziel++ = val << colbits_complement;
					}
					else
						if(hamval == 0x2)
						{
							lastvalr = *ziel++ = val << colbits_complement;
							*ziel++ = lastvalg;
							*ziel++ = lastvalb;
						}
						else
							if(hamval == 0x3)
							{
								*ziel++ = lastvalr;
								lastvalg = *ziel++ = val << colbits_complement;
								*ziel++ = lastvalb;
							}
			} while(++x < width);
		} while(++y < height);

		ziel = oziel;
	}

	return(ziel);
} /* decodeHAM */


/* Dekodiert pixelpacked PBM, gepackt und ungepackt */
char *decodePBM(char *buffer, unsigned int w, unsigned int height, char BitsPerPixel, char comp)
{		
	char *ziel, *oziel, *ooziel,
		 v1, v2, n, p, Planes;

	unsigned int x, y, bh, bl;

	unsigned long offset;


	Planes = BitsPerPixel >> 3;

	offset = w * Planes;	

	if((ziel = SMalloc((w * (unsigned long)height * BitsPerPixel) >> 3)) == 0)
		return(NULL);
	else
	{
		ooziel = oziel = ziel;

		if((bh = height / 10) == 0) 		/* busy-height */
			bh = height;
		bl = 0;								/* busy-length */

		y = 0;
		do
		{
			if(!(y%bh))
			{
				busybox(bl);
				bl += 12;
			}

			p = 0;
			do
			{
				ziel = oziel + p;				/* Zieladresse der dekodierten Scanline */

				if(!comp)
				{
					x = 0;
					do
					{
						*ziel = *buffer++;
						ziel += Planes;
					} while(++x < w);
				}
				else
				{
					x = 0;
					do
					{
						v1 = *buffer++;
						if(v1 > 0x7f)
						{
							n = 0x101 - v1;

							v2 = *buffer++;

							x += n;

							while(n--)
							{
								*ziel = v2;
								ziel += Planes;
							}
						}
						else
						{
							n = v1 + 1;

							x += n;

							while(n--)
							{
								*ziel = *buffer++;
								ziel += Planes;
							}
						}
					} while(x < w);
				} /* comp? */
			} while(++p < Planes);
			oziel += offset;
		} while(++y < height);
	} /* Malloc */

	ziel = ooziel;

	return(ziel);
} /* decodePBM */


/* sucht den nÑchsten Junk mit der angegebenen Kennung */
char *srchchunk(char *buffer, unsigned long var)
{
	while(*(unsigned long *)buffer != var && buffer != FileEnd)
		buffer++;

	if(*(unsigned long *)buffer == var)
		return(buffer);
	else
		return(NULL);
}