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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ext.h>
#include <tos.h>
#include "..\..\..\src\lib\demolib.h"

#define MaxStackSize	4096

#define TRUE	1
#define FALSE	0

#define DEBUG	0
#define TIMER	0

#define ASSEMBLER	1
#define	MC68020	2
#define	MC68030	4
#define	MC68040	8


/* Set 16 Pixel (Standard Format) Assembler-Rout */
int setpix_standard(char *buf16, char *dest, int depth, long planelen, int howmany);
int decode_lzw_normal(char *buffer, char *ziel, int width, int height, char BitsPerPixel);
int decode_lzw_fast(char *buffer, char *ziel);

extern	void decode_fast(int *par1, long *par2);
extern	void decode_fast020(int *par1, long *par2);

extern int PROCESSOR;
extern long filelen;

/* decodiert alle GIF mit 1-7 Bit */
int decode_lzw_normal(char *buffer, char *ziel, int width, int height, char BitsPerPixel)
{
	int *prefix;
	char *suffix, *out, *opos, *pixbuf;
	char data_size;

	int code, oldcode, clear, end_of_information, entries, available;
	int x, l, c, k, finished, out_count, width2, v, d;

	char *pData;
	unsigned int pBitsLeft, pCodeMask, pCodeSize, pCount;
	unsigned long pDatum;

	long planelength, len_to_go;

#if TIMER
/* wie schnell sind wir? */
	init_timer();
#endif

	planelength = (((long)width + 7) / 8) * height; /* Auf volle Byte gerundete ZeilenlÑnge in Byte */
	width2 = width / 16 * 16;
	v = width%16;

	if(v == 0)
	{
		width2 -= 16;
		v = 16;
	}

	/* Speicher fÅr LZW-Arrays anfordern */
	prefix = (int *)malloc(MaxStackSize * sizeof(int));
	suffix = (char *)malloc(MaxStackSize);
	out = (char *)malloc(MaxStackSize);
	pixbuf = (char *)malloc((width + 15) / 16 * 16);
	if((prefix == (int *)NULL) ||
	   (suffix == (char *)NULL) ||
	   (out == (char *)NULL) ||
	   (pixbuf == (char *)NULL))
		return(-1);

	/* Initalisieren der LZW-Variablen */
	data_size = *buffer++;					/* aktuelle Codegrîûe auslesen */
	clear = 1 << data_size;					/* Clearcode bestimmen */
	end_of_information = clear + 1;			/* end-of-information Code */
	available = clear + 2;					/* erster freier Tabelleneintrag */
	pCodeSize = data_size + 1;				/* Bitanzahl der nÑchsten Codegrîûe */
	entries = 1 << pCodeSize;					/* erster Code der nÑchsten Codegrîûe */
	pCodeMask = (1 << pCodeSize) - 1;		/* Codemaske setzen */

	/* initialisieren der ersten 1 << Codegrîûe EintrÑge */
	for(code = 0; code < clear; code++)
	{
		prefix[code] = -1;
		suffix[code] = code;
	}

#if DEBUG
	printf("\ndata_size: %d, Position: %p", (int)data_size, buffer);
	getch();
#endif

	pBitsLeft = 0;
	pDatum = 0;
	pCount = 0;
	pData = buffer;

	len_to_go = filelen;
	out_count = 0;
	x = 0;
	finished = FALSE;
	while(!finished)
	{
		/* Code holen */
		if(pBitsLeft < pCodeSize)
		{
			if(pCount == 0)					/* neuen Block beginnen */
			{
				pCount = *pData++;			/* BlocklÑnge auslesen */
				if(pCount == 0 || (len_to_go -= pCount) < 0)
					break;
			}
	
	    	pDatum |= (unsigned long)*pData++ << pBitsLeft;
			pCount--;
			pBitsLeft += 8;

			if(pCount == 0)					/* neuen Block beginnen */
			{
				pCount = *pData++;			/* BlocklÑnge auslesen */
				if(pCount == 0 || (len_to_go -= pCount) < 0)
					break;
			}
	
	    	pDatum |= (unsigned long)*pData++ << pBitsLeft;
			pCount--;
			pBitsLeft += 8;
		}

		/* long muû auf int und gebracht und dann verundet werden, */
		/* oder die Maske muû auf long gebracht werden - aber die long- */
		/* VerknÅpfung wÑre wahrscheinlich langsamer. */
		code = (int)pDatum&pCodeMask;
		pDatum >>= pCodeSize;
		pBitsLeft -= pCodeSize;

		if(code == clear)						/* Clearcode? */
		{
			/* Reset Decoder */
			available = clear + 2;
			pCodeSize = data_size + 1;
			entries = 1 << pCodeSize;
			pCodeMask = (1 << pCodeSize) - 1;

			/* Code holen */
			if(pBitsLeft < pCodeSize)
			{
				if(pCount == 0)					/* neuen Block beginnen */
					pCount = *pData++;			/* BlocklÑnge auslesen */
	
	    		pDatum |= (unsigned long)*pData++ << pBitsLeft;
				pCount--;
				pBitsLeft += 8;

				if(pCount == 0)					/* neuen Block beginnen */
					pCount = *pData++;			/* BlocklÑnge auslesen */
	
	    		pDatum |= (unsigned long)*pData++ << pBitsLeft;
				pCount--;
				pBitsLeft += 8;
			}

			/* long muû auf int und gebracht und dann verundet werden, */
			/* oder die Maske muû auf long gebracht werden - aber die long- */
			/* VerknÅpfung wÑre wahrscheinlich langsamer. */
			code = (int)pDatum&pCodeMask;
			pDatum >>= pCodeSize;
			pBitsLeft -= pCodeSize;

			pixbuf[x++] = code;					/* Ist garantiert ein bekannter Code */

			if(x == width)
			{
				out_count = 0;
				while(out_count < width2)
				{
					setpix_standard(pixbuf + out_count, ziel, BitsPerPixel, planelength, 16);
					ziel += 2;
					out_count += 16;
				}

				d = setpix_standard(pixbuf + out_count, ziel, BitsPerPixel, planelength, v);
				ziel += d;

				x = 0;
			}

			oldcode = code;
		}
		else
		{
			if(code == end_of_information)
				finished = TRUE;
			else
			{
				/* Dekodierbarer Code */
				if(code < available)			/* Code schon bekannt? */
				{
					/* Schreibe Muster in Ausgabe */
					opos = out;
					l = 0;
					c = code;
					do
					{
						*opos++ = suffix[c];
						l++;
					} while((c = prefix[c]) != -1);	/* von hinten her aufrollen */

					k = *(opos - 1);			/* 1. Zeichen output stream merken */
				}
				else
				{
					opos = out + 1;				/* vorne Platz lassen */
					l = 1;
					c = oldcode;
					do
					{
						*opos++ = suffix[c];
						l++;
					} while((c = prefix[c]) != -1);	/* von hinten her aufrollen */

					k = *(opos - 1);			/* 1. Zeichen output stream merken */
					*out = k;					/* und vorne anhÑngen */
				} /* else */
        
				/* Ausgabe der Zeichenkette richtig herum */
				do
				{
					pixbuf[x++] = *(--opos);

					if(x == width)
					{
						out_count = 0;
						while(out_count < width2)
						{
							setpix_standard(pixbuf + out_count, ziel, BitsPerPixel, planelength, 16);
							ziel += 2;
							out_count += 16;
						}

						d = setpix_standard(pixbuf + out_count, ziel, BitsPerPixel, planelength, v);
						ziel += d;

						x = 0;
					}
				} while(--l);

				/* pflegen der Tabelle */
				prefix[available] = oldcode;
				suffix[available] = k;
				available++;
				oldcode = code;
			}

			/* Codegrîûe erhîhen wenn nîtig und mîglich */
			if(available >= entries && pCodeSize < 12)
			{
				pCodeSize++;
				pCodeMask += available;
				entries <<= 1;
			}
		}
	} /* while */

#if TIMER
/* wie schnell waren wir? */
	printf("\nZeit: %lu", get_timer());
	getch();
#endif

	free(pixbuf);
	free(out);
	free(suffix);
	free(prefix);

	return(0);
} /* decode_lzw_normal */


/*
 * decodiert alle GIF mit 8 Bit 
 */
int decode_lzw_fast(char *buffer, char *ziel)
{
	char *firstcodes, *ofirstcodes;
	int *srclen;
	char *out, *dst;
	char data_size;

	int code, oldcode, clear, end_of_information, entries, available;
/*	int i finished, codelen, oldcodelen; */

	long *src;

	char *pData;
	unsigned int pBitsLeft, pCodeMask, pCodeSize, pCount;
	unsigned long pDatum;

	int par1[20];
	long par2[20];

#if TIMER
	init_timer();
#endif

	/*
	 * Speicher fÅr LZW-Arrays anfordern
	 */
	srclen = (int *)malloc(MaxStackSize * sizeof(int));
	src = (long *)malloc(MaxStackSize * sizeof(long));
	ofirstcodes = firstcodes = (char *)malloc(256);			/* maximum needed */
	if((srclen == (int *)NULL) ||
	   (firstcodes == (char *)NULL) ||
	   (src == (long *)NULL))
		return(-1);

	/*
	 * initalisieren der LZW-Variablen
	 */
	data_size = *buffer++;					/* aktuelle Codegrîûe auslesen */
	clear = 1 << data_size;					/* Clearcode bestimmen */
	end_of_information = clear + 1;			/* end-of-information Code */
	available = clear + 2;					/* erster freier Tabelleneintrag */
	pCodeSize = data_size + 1;				/* Bitanzahl der nÑchsten Codegrîûe */
	entries = 1 << pCodeSize;				/* erster Code der nÑchsten Codegrîûe */
	pCodeMask = (1 << pCodeSize) - 1;		/* Codemaske setzen */

	/*
	 * initialisieren der ersten 1 << Codegrîûe EintrÑge
	 */
	for(code = 0; code < clear; code++)
	{
	#ifdef ASSEMBLER
		srclen[code] = 0;		/* mÅûte eigentlich 1 sein, aber in der ASS-Rout wird dbra verwendet, um das Muster zu schreiben */
	#else
		srclen[code] = 1;
	#endif

		*firstcodes = code;
		src[code] = (long)&(*firstcodes);
		firstcodes++;
	}

	pBitsLeft = 0;
	pDatum = 0;
	pCount = 0;
	pData = buffer;

#if ASSEMBLER
	par1[0]=data_size;
	par1[1]=clear;
	par1[2]=end_of_information;
	par1[3]=available;
	par1[4]=pCodeSize;
	par1[5]=entries;
	par1[6]=pCodeMask;
	par2[0]=(long)src;
	par2[1]=(long)srclen;
	par2[2]=(long)ziel;
	par2[3]=(long)pData;
	if(PROCESSOR&MC68020 || PROCESSOR&MC68030 || PROCESSOR&MC68040)
		decode_fast020(par1, par2);
	else
		decode_fast(par1, par2);

#if DEBUG
	printf("Decoded!");
	getch();
#endif

#else
	finished = FALSE;
	while(!finished)
	{
		/* Code holen */
		if(pBitsLeft < pCodeSize)
		{
			if(pCount == 0)					/* neuen Block beginnen */
			{
				pCount = *pData++;			/* BlocklÑnge auslesen */
				if(pCount == 0)
					break;
			}
	
	    	pDatum |= (unsigned long)*pData++ << pBitsLeft;
			pCount--;
			pBitsLeft += 8;

			if(pCount == 0)					/* neuen Block beginnen */
			{
				pCount = *pData++;			/* BlocklÑnge auslesen */
				if(pCount == 0)
					break;
			}
	
	    	pDatum |= (unsigned long)*pData++ << pBitsLeft;
			pCount--;
			pBitsLeft += 8;
		}

		code = (int)pDatum&pCodeMask;
		pDatum >>= pCodeSize;
		pBitsLeft -= pCodeSize;

		if(code == clear)						/* Clearcode? */
		{
			/* Reset Decoder */
			available = clear + 2;
			pCodeSize = data_size + 1;
			entries = 1 << pCodeSize;
			pCodeMask = (1 << pCodeSize) - 1;

			/* Code holen */
			if(pBitsLeft < pCodeSize)
			{
				if(pCount == 0)					/* neuen Block beginnen */
					pCount = *pData++;			/* BlocklÑnge auslesen */
	
		    	pDatum |= (unsigned long)*pData++ << pBitsLeft;
				pCount--;
				pBitsLeft += 8;

				if(pCount == 0)					/* neuen Block beginnen */
					pCount = *pData++;			/* BlocklÑnge auslesen */
	
		    	pDatum |= (unsigned long)*pData++ << pBitsLeft;
				pCount--;
				pBitsLeft += 8;
			}

			code = (int)pDatum&pCodeMask;
			pDatum >>= pCodeSize;
			pBitsLeft -= pCodeSize;

			*ziel++ = code;						/* Ist garantiert ein bekannter Code */

			oldcode = code;
			oldcodelen = 1;
		}
		else
		{
			if(code == end_of_information)
				finished = TRUE;
			else
			{
				/* Dekodierbarer Code */
				if(code < available)			/* Code schon bekannt? */
				{
					dst = ziel - oldcodelen;

					/* Schreibe Muster in Ausgabe */
					i = codelen = srclen[code];
					out = (char *)src[code];
					do
					{
						*ziel++ = *out++;
					} while(--i);
				}
				else							/* neuer Code */
				{
					dst = ziel;

					/* Schreibe Muster in Ausgabe */
					i = codelen = srclen[oldcode];
					out = (char *)src[oldcode];
					do
					{
						*ziel++ = *out++;
					} while(--i);
					*ziel++ = *(char *)src[oldcode];
					codelen++;
				} /* else */

				/* pflegen der Tabelle */
				srclen[available] = oldcodelen + 1;
				src[available] = (long)&(*dst);
				available++;
				oldcode = code;
				oldcodelen = codelen;
			}

			/* Codegrîûe erhîhen wenn nîtig und mîglich */
			if(available >= entries && pCodeSize < 12)
			{
				pCodeSize++;
				pCodeMask += available;
				entries <<= 1;
			}
		}
	} /* while */
#endif

#if TIMER
	printf("\nZeit: %lu", get_timer());
#endif

	free(ofirstcodes);
	free(src);
	free(srclen);

	return(0);
} /* decode_lzw_fast */