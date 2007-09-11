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
#include <..\..\..\demolib.h>

#define TRUE	1
#define FALSE	0

#define MAXBITS	12
#define HSIZE	5003
#define HSHIFT	4

#define TIMER	0

char *encode_lzw_17bit(char *buffer, char *ziel, unsigned int width, unsigned int height, char BitsPerPixel);
char *encode_lzw_8bit(char *buffer, char *ziel, unsigned int width, unsigned int height);
void getpix_std_line(void *st_pic, void *buf16, int planes, long planelen, int howmany);

extern int *lacetab;


char *encode_lzw_17bit(char *obuffer, char *ziel, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *buffer, *pixbuf, *opixbuf,
		 data_size;

	int clear, end_of_information, entries, available;
	int i, blocklenbytes, clearcodebytes;
	int *ctab,
		c, disp, ent, linelen;
	unsigned int y;

	long *vtab,
		 fcode;

	char *pData;
	int pBitsLeft, pCodeSize, pCount;
	unsigned long pDatum, w, planelength;


#if TIMER
/* wie schnell sind wir? */
	init_timer();
#endif

	ctab = (int *)Malloc(HSIZE * 2L);
	vtab = (long *)Malloc(HSIZE * 4L);

	/* Initalisieren der LZW-Variablen */
	if(BitsPerPixel == 1)
		data_size = 2;						/* muž lt. Doku so sein */
	else
		data_size = BitsPerPixel;
	*ziel++ = data_size;					/* aktuelle Codegr”že schreiben */
	clear = 1 << data_size;					/* Clearcode bestimmen */
	end_of_information = clear + 1;			/* end-of-information Code */
	available = clear + 2;					/* erster freier Tabelleneintrag */
	pCodeSize = data_size + 1;				/* Bitanzahl der n„chsten Codegr”že */
	entries = 1 << pCodeSize;				/* erster Code der n„chsten Codegr”že */

	pBitsLeft = 0;
	pDatum = 0;
	pCount = 0;
	pData = ziel;
	blocklenbytes = 0;
	clearcodebytes = 0;

	*pData++ = pCount = 255;
	blocklenbytes++;
	/* ersten Clearcode schreiben */
	clearcodebytes++;

	pDatum |= (unsigned long)clear << pBitsLeft;
	pBitsLeft += pCodeSize;

	w = (long)(width + 7) / 8L;
	planelength = w * (long)height;
	pixbuf = (char *)Malloc(width + 7);
	opixbuf = pixbuf;
	memset(pixbuf, 0x0, width);
	getpix_std_line(obuffer, pixbuf, BitsPerPixel, planelength, width);

	linelen = width;

	ent = *pixbuf++;
	linelen--;

	for(i = 0; i < HSIZE; i++)
		vtab[i] = -1;

	y = 0;
	while(y++ < height)
	{
		buffer = obuffer + (unsigned long)lacetab[y] * (unsigned long)w;

		do
		{
			c = *pixbuf++;

			fcode = (long)(((long)c << MAXBITS)|ent);
			i = (((int)c << HSHIFT)^ent);


			if(vtab[i] != -1)					/* empty slot */
			{
				if(vtab[i] == fcode)			/* right code found */
				{
					ent = ctab[i];
					continue;
				}

				/* secondary hash (after G. Knott) */
				if(i == 0)
					disp = 1;
				else
					disp = HSIZE - i;

				/* probe */
				for(;;)
				{
					if((i -= disp) < 0)
						i += HSIZE;

					if(vtab[i] == fcode)		/* right code found */
					{
						ent = ctab[i];
						goto next;
					}
					else
						if(vtab[i] == -1)		/* empty slot */
							break;
				}
			}

			/* nomatch */
			pDatum |= (unsigned long)ent << pBitsLeft;
			pBitsLeft += pCodeSize;

			while(pBitsLeft >= 8)
			{
				if(pCount == 0)
				{
					*pData++ = pCount = 255;
					blocklenbytes++;
				}

				*pData++ = (char)pDatum;	/* &0xff kann ich mir wg. cast sparen */
				pDatum >>= 8;
				pBitsLeft -= 8;
				pCount--;
			}

			if(available < 4096)
			{
				if(available >= entries)
				{
					pCodeSize++;
					entries <<= 1;
				}

				ctab[i] = available++;		/* code -> hashtable */
				vtab[i] = fcode;
			}
			else
			{
				/* Reset Encoder */
				available = clear + 2;

				/* Clearcode ausstožen */
				clearcodebytes++;

				pDatum |= (unsigned long)clear << pBitsLeft;
				pBitsLeft += pCodeSize;

				while(pBitsLeft >= 8)
				{
					if(pCount == 0)
					{
						*pData++ = pCount = 255;
						blocklenbytes++;
					}

					*pData++ = (char)pDatum;	/* &0xff kann ich mir wg. cast sparen */
					pDatum >>= 8;
					pBitsLeft -= 8;
					pCount--;
				}

				pCodeSize = data_size + 1;
				entries = 1 << pCodeSize;

				for(i = 0; i < HSIZE; i++)
					vtab[i] = -1;
			}

			ent = c;

next:
			continue;
		} while(--linelen);

		linelen = width;
		pixbuf = opixbuf;
		memset(pixbuf, 0x0, width);
		getpix_std_line(buffer, pixbuf, BitsPerPixel, planelength, width);
	} /* while y */

	/* letztes Byte ausgeben */
	pDatum |= (unsigned long)ent << pBitsLeft;
	pBitsLeft += pCodeSize;
/*
	if(pCount == 0)
	{
		*pData++ = pCount = 255;
		blocklenbytes++;
	}

	*pData++ = (char)(pDatum&0xff);
	pDatum >>= 8;
	pBitsLeft -= 8;
	pCount--;
*/

	/* EOI ausgeben */
	pDatum |= (unsigned long)end_of_information << pBitsLeft;
	pBitsLeft += pCodeSize;

	while(pBitsLeft > 0)							/* bis wirklich alles geschrieben ist */
	{
		if(pCount == 0)
		{
			*pData++ = pCount = 255;
			blocklenbytes++;
		}

		*pData++ = (char)pDatum;	/* &0xff kann ich mir wg. cast sparen */
		pDatum >>= 8;
		pBitsLeft -= 8;
		pCount--;
	}

	*(pData - (255 - pCount) - 1) = 255 - pCount;	/* zuletzt geschriebene Blockl„nge korrigieren */

	*pData++ = 0;									/* n„chste Blockl„nge == 0 */

#if TIMER
/* wie schnell waren wir? */
	printf("%lu\n", get_timer());
	getch();
#endif

	pixbuf = opixbuf;
	Mfree(pixbuf);

	return(pData);
} /* encode_lzw_17bit */


char *encode_lzw_8bit(char *obuffer, char *ziel, unsigned int width, unsigned int height)
{
	char *buffer,
		 data_size;

	int clear, end_of_information, entries, available;
	int i, blocklenbytes, clearcodebytes;
	int *ctab,
		c, disp, ent;
	unsigned int x, y;

	long *vtab,
		 fcode;

	char *pData;
	int pBitsLeft, pCodeSize, pCount;
	unsigned long pDatum;


#if TIMER
/* wie schnell sind wir? */
	init_timer();
#endif

	ctab = (int *)Malloc(HSIZE * 2L);
	vtab = (long *)Malloc(HSIZE * 4L);

	/* Initalisieren der LZW-Variablen */
	data_size = 8;
	*ziel++ = data_size;					/* aktuelle Codegr”že schreiben */
	clear = 1 << data_size;					/* Clearcode bestimmen */
	end_of_information = clear + 1;			/* end-of-information Code */
	available = clear + 2;					/* erster freier Tabelleneintrag */
	pCodeSize = data_size + 1;				/* Bitanzahl der n„chsten Codegr”že */
	entries = 1 << pCodeSize;				/* erster Code der n„chsten Codegr”že */

	pBitsLeft = 0;
	pDatum = 0;
	pCount = 0;
	pData = ziel;
	blocklenbytes = 0;
	clearcodebytes = 0;

	*pData++ = pCount = 255;
	blocklenbytes++;
	/* ersten Clearcode schreiben */
	clearcodebytes++;

	pDatum |= (unsigned long)clear << pBitsLeft;
	pBitsLeft += pCodeSize;

	buffer = obuffer;
	ent = *buffer++;

	for(i = 0; i < HSIZE; i++)
		vtab[i] = -1;


	y = 0; 
	do
	{
	x = width;
	if(y == 0)
		/* die erste Zeile hat ein Byte weniger weil oben schon weggenommen */
		x--;
	else
		/* aužer in der ersten Zeile neu aufsetzen */
		buffer = obuffer + (unsigned long)lacetab[y] * (unsigned long)width;

	while(x)
	{
		c = *buffer++;
		x--;

		fcode = (long)(((long)c << MAXBITS)|ent);
		i = (((int)c << HSHIFT)^ent);


		if(vtab[i] != -1)					/* empty slot */
		{
			if(vtab[i] == fcode)			/* right code found */
			{
				ent = ctab[i];
				continue;
			}

			/* secondary hash (after G. Knott) */
			if(i == 0)
				disp = 1;
			else
				disp = HSIZE - i;

			/* probe */
			for(;;)
			{
				if((i -= disp) < 0)
					i += HSIZE;

				if(vtab[i] == fcode)		/* right code found */
				{
					ent = ctab[i];
					goto next;
				}
				else
					if(vtab[i] == -1)		/* empty slot */
						break;
			}
		}

		/* nomatch */
		pDatum |= (unsigned long)ent << pBitsLeft;
		pBitsLeft += pCodeSize;

		while(pBitsLeft >= 8)
		{
			if(pCount == 0)
			{
				*pData++ = pCount = 255;
				blocklenbytes++;
			}

			*pData++ = (char)pDatum;	/* &0xff kann ich mir wg. cast sparen */
			pDatum >>= 8;
			pBitsLeft -= 8;
			pCount--;
		}

		if(available < 4096)
		{
			if(available >= entries)
			{
				pCodeSize++;
				entries <<= 1;
			}

			ctab[i] = available++;		/* code -> hashtable */
			vtab[i] = fcode;
		}
		else
		{
			/* Reset Encoder */
			available = clear + 2;

			/* Clearcode ausstožen */
			clearcodebytes++;

			pDatum |= (unsigned long)clear << pBitsLeft;
			pBitsLeft += pCodeSize;

			while(pBitsLeft >= 8)
			{
				if(pCount == 0)
				{
					*pData++ = pCount = 255;
					blocklenbytes++;
				}

				*pData++ = (char)pDatum;	/* &0xff kann ich mir wg. cast sparen */
				pDatum >>= 8;
				pBitsLeft -= 8;
				pCount--;
			}

			pCodeSize = data_size + 1;
			entries = 1 << pCodeSize;

			for(i = 0; i < HSIZE; i++)
				vtab[i] = -1;
		}

		ent = c;

next:
		continue;
	} /* while x */
	} while(++y < height);

	/* letztes Byte ausgeben */
	pDatum |= (unsigned long)ent << pBitsLeft;
	pBitsLeft += pCodeSize;
/*
	if(pCount == 0)
	{
		*pData++ = pCount = 255;
		blocklenbytes++;
	}

	*pData++ = (char)(pDatum&0xff);
	pDatum >>= 8;
	pBitsLeft -= 8;
	pCount--;
*/

	/* EOI ausgeben */
	pDatum |= (unsigned long)end_of_information << pBitsLeft;
	pBitsLeft += pCodeSize;

	while(pBitsLeft > 0)							/* bis wirklich alles geschrieben ist */
	{
		if(pCount == 0)
		{
			*pData++ = pCount = 255;
			blocklenbytes++;
		}

		*pData++ = (char)pDatum;	/* &0xff kann ich mir wg. cast sparen */
		pDatum >>= 8;
		pBitsLeft -= 8;
		pCount--;
	}

	*(pData - (255 - pCount) - 1) = 255 - pCount;	/* zuletzt geschriebene Blockl„nge korrigieren */

	*pData++ = 0;									/* n„chste Blockl„nge == 0 */

#if TIMER
/* wie schnell waren wir? */
	printf("%lu\n", get_timer());
	getch();
#endif

	return(pData);
} /* encode_lzw_8bit */