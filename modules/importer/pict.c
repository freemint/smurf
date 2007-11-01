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
/*					Macintosh PICT Decoder					*/
/* Version 0.1  --  03.11.96								*/
/*	  Erster Versuch. PICT ist ein Metafile, da momentan	*/
/*	  sowieso nur Rastergrafiken vom Smurf unterst�tzt		*/
/*	  werden, wird auch nur der Teil gelesen. Das sollten	*/
/*	  aber sowieso die h�ufigsten sein.						*/
/*	  Komprimierung ist RLE und JPEG (Quickdraw);			*/
/*	  JPEG lasse ich auch mal auf unbestimmte Zeit weg -	*/
/*	  die Gr�nde d�rften bekannt sein.						*/
/*	  1 Bit													*/
/*	  Version 1 und 2										*/
/* Version 0.2 -- 04.11.96									*/
/*	  8	Bit													*/
/* Version 0.3 -- 06.11.96									*/
/*	  16 Bit												*/
/* Version 0.4 -- 10.11.96									*/
/*	  Gr��ere Umbauten (Dekoder in Funktionen);				*/
/*	  24 und 32 Bit, unkomprimierte							*/
/* Version 0.5 -- 12.11.96									*/
/*	  4 Bit													*/
/* Version 0.6 -- 19.11.96 - 26.11.96						*/
/*    Importer total umgeschrieben. Der Infoteil des		*/
/*    Files wird nun nicht mehr starr gelesen, sondern		*/
/*    interpretiert um alle Files lesen zu k�nnen.			*/
/* Version 0.7 -- 16.03.97									*/
/*	  1 und 8 Bit Bilder mit ungerader Zeilenl�nge werden	*/
/*	  nun auch korrekt dekodiert (die sind n�mlich unver-	*/
/*	  st�ndlicherweise auf gerade Anzahl Pixel aligned		*/
/*	  abgespeichert. Zumindest bei RLE, wie es unkomprimiert*/
/*	  ist, wei� ich nicht, da der Grafikkonverter die nicht	*/
/*	  kann).												*/
/* Version 0.8 -- 30.05.98									*/
/*	  neue setpix_std_line eingebaut						*/
/* Version 0.9 -- xx.xx.xx									*/
/*	  Regions werden verstanden und ausgewertet.			*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
/* #include <demolib.h>*/

#define NOMEM		0
#define WORD_LEN	(-1)

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int setpix_std_line(char *buf, char *std, int depth, long planelen, int howmany);

void Dispatch(struct pixMap *p);
char *readunpacked(unsigned int width, unsigned int height, char BitsPerPixel, char packType);
char *decode(unsigned int width, unsigned int height, char BitsPerPixel, char ctrlByteLen);
char *decode4(unsigned int width, unsigned int height, char ctrlByteLen, char packType);
char *decode16(unsigned int width, unsigned int height, char ctrlByteLen);
char *decode24(unsigned int width, unsigned int height, char cmpCount, char ctrlByteLen);

void nop(void);
unsigned int get_op(unsigned int version);
char read_byte(void);
void Clip(void);
void LongComment(void);
void BitsRect(void);
void do_bitmap(int is_region);
void read_color_table(void);

/* Infostruktur f�r Hauptmodul */
MOD_INFO module_info = {"Mac PICT Importer",
						0x0080,
						"Christian Eyrich",
						"PIC", "PCT", "PICT", "", "",
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

char *buffer, *obuffer, *ziel, *pal,
	 got_pic = FALSE;

unsigned long align = 0;

struct Rect {

struct pixMap {
};

struct opdef
{

struct Rect picFrame;
GARGAMEL *giveitme;

/* for reserved opcodes of known length */

/* for reserved opcodes of length determined by a function */

struct opdef optable[] =
{
/* 0x68 */	{4, NULL},						/* frameSameArc */
/* 0x8e */	res(0),

struct pixMap p;
/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*		Macintosh PICT Dekomprimierer (PICT)		*/
/*		1, 4, 8, 16, 24 Bit, RLE					*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	int plus, res;
	unsigned int opcode, version, len, PictSize;

	giveitme = smurf_struct;

	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	/* nichtsnutzigen Header �berspringen */
	if(*buffer == 0x0 && *(buffer + 1) == 0x0)
	{
		buffer += 0x200;
		plus = 0x200;
	}
	else
	{
		buffer += 0x280;
		plus = 0x280;
	}

	PictSize = read_word();
	if(smurf_struct->smurf_pic->file_len < 520 ||
/*	   Diese Pr�fung ist leider nicht hundertprozentig zuverl�ssig, da es Apple geschafft hat,
	   f�r die Filegr��e nur ein int zu reservieren, die gespeicherte Gr��e nach 2 Byte
	   abgeschnitten wird und somit halt �berhaupt nicht stimmt. Bravo :-( */
	   (PictSize + plus != (unsigned int)smurf_struct->smurf_pic->file_len &&
	    PictSize != (unsigned int)smurf_struct->smurf_pic->file_len))
		return(M_INVALID);
	else
	{
		read_rect(&picFrame);

		if(*buffer == 0x11 && *(buffer + 0x01) == 0x01)
		{
			version = 1;
			buffer += 0x02;
		}
		else
			if(*(unsigned int *)buffer == 0x0011 && *(buffer + 0x02) == 0x02)
			{
				version = 2;
				if(*(buffer + 0x03) != 0xff)		/* Nur Unterversion 0xff ist bekannt */
					return(M_UNKNOWN_TYPE);

				buffer += 0x04;
			}
			else
				return(M_INVALID);

		while((opcode = get_op(version)) != 0xff)
		{
			if(opcode < 0xa2)
			{
					if(optable[opcode].len >= 0)
						switch(optable[opcode].len)
						{
				if(opcode == 0xc00)
					skip(24);
					if(opcode >= 0xa2 && opcode <= 0xaf)
						skip(read_word());
						if(opcode >= 0xb0 && opcode <= 0xcf)
							/* just a reserved opcode, no data */
						else
							if(opcode >= 0xd0 && opcode <= 0xfe)
								skip((unsigned int)read_long());
								if(opcode >= 0x100 && opcode <= 0x7fff)
									skip((opcode >> 7) & 0xff);
									if(opcode >= 0x8000 && opcode <= 0x80ff)
										/* just a reserved opcode */
									else
										if(opcode >= 0x8100 && (unsigned long)opcode <= 0xffff)
											skip((unsigned int)read_long());
										{
											res = form_alert(2, "[2][Unbekannter Opcode! Soll versucht werden, | ihn einfach zu �berlesen?][ Ja ][ Nein ]");
											if(res == 2)
												return(M_PICERR);
										}

		if(opcode == 0x00ff && got_pic)
			if(ziel == NOMEM)
				return(M_MEMORY);
			else
				smurf_struct->smurf_pic->pic_data = ziel;
		else
			return(M_UNKNOWN_TYPE);
	} /* Erkennung */

	buffer = obuffer;

	SMfree(buffer);

	return(M_PICDONE);
}



/* Liest unkomprimierte (und pseudokomprimierte) PICTs */
char *readunpacked(unsigned int width, unsigned int height, char BitsPerPixel, char packType)
{
	char *ziel, *oziel,
		 direkt;

	unsigned int x, y;

	unsigned long w, memwidth;


	if(BitsPerPixel == 1)
	{
		w = (unsigned long)((width + 7) / 8);			/* Auf volle Byte gerundete Zeilenl�nge in Byte */
		memwidth = w * 8;
	}
	else
		if(BitsPerPixel == 24 && packType == 1)
		{
			w = (unsigned long)width;
			memwidth = (unsigned long)width;
		}
		else
		{
			w = ((unsigned long)width * BitsPerPixel) >> 3;
			memwidth = (unsigned long)width;
		}

	
	if(BitsPerPixel == 32 && packType == 1)
		direkt = 0;
	else
		direkt = 1;

	if((ziel = SMalloc((memwidth * (unsigned long)height * BitsPerPixel) >> 3)) == 0)
		return(NOMEM);
	else
	{
		oziel = ziel;

		y = 0;
		do
		{
			if(direkt)
			{
				memcpy(ziel, buffer, w);
				buffer += w;
			}
			else
			{
				x = 0;
				do
				{
					*ziel++ = *buffer++;
					*ziel++ = *buffer++;
					*ziel++ = *buffer++;
					buffer++;
				} while(++x < w);
			}
			align += w;							/* die buffer += w, bzw. buffer++ */
		} while(++y < height);
	} /* Malloc */

	ziel = oziel;

	got_pic = TRUE;

	return(ziel);
} /* readunpacked */



/* Dekodiert PICT mit byteweisem Packing */
char *decode(unsigned int width, unsigned int height, char BitsPerPixel, char CtrlByteLen)
{
	char *ziel, *oziel,
		 v, v1, v2;

	unsigned int x, y, spaketlength, dpaketlength;

	unsigned long w, memwidth;


	if(BitsPerPixel == 1)
	{
		w = (unsigned long)((width + 7) / 8); /* Auf volle Byte gerundete Zeilenl�nge in Byte */
		memwidth = w * 8;
	}
	else
	{
		w = (unsigned long)width;
		memwidth = (unsigned long)width;
	}

	if(BitsPerPixel == 1 || BitsPerPixel == 8)
		v = width & 1;
	else
		v = 0;

	if((ziel = SMalloc((memwidth * (unsigned long)height * BitsPerPixel) >> 3)) == 0)
		return(NOMEM);
	else
	{
		oziel = ziel;

		y = 0;
		do
		{
			buffer += CtrlByteLen;
			align += CtrlByteLen;						/* den buffer += CtrlByteLen */

			x = 0;
			do
			{
				spaketlength = dpaketlength = 0;

				v1 = *buffer++;
/*				spaketlength++; */

				if(v1 > 0x7f)							/* Encoded Run */
				{ 	
/*					printf("align: %lu\n", align);
					getch(); */
					v1 = 0x101 - v1;

					x += v1;

					v2 = *buffer++;
/*					spaketlength++; */

					align += 2;							/* die beiden buffer++ */

					while(v1--)
					{
						*ziel++ = v2;
						dpaketlength++;
					}
/*					printf("run - dpaketlength: %u\n", dpaketlength); */
				} /* x != 0 */
				else									/* Literal Run */
				{
/*					printf("align: %lu\n", align);
					getch(); */
					v1++;

					x += v1;

					align += v1 + 1;					/* den buffer++ und die v1 folgenden */

					while(v1--)
					{
						*ziel++ = *buffer++;
						dpaketlength++;
/*						spaketlength++; */
					}
/*					printf("literal - dpaketlength: %u\n", dpaketlength); */
				}
/*				getch(); */
			} while(x < w);
			ziel -= v;
/*			printf("x: %u, w: %lu, align: %lu\n\n", x, w, align);
			getch(); */
		} while(++y < height);
	} /* Malloc */

	ziel = oziel;

	got_pic = TRUE;

	return(ziel);
} /* decode */



/* Dekodiert 4 Bit PICT mit und ohne Kompression und konvertiert */
/* es gleichzeitig ins Standardformat */
char *decode4(unsigned int width, unsigned int height, char CtrlByteLen, char packType)
{
	char *ziel, *oziel, *pixbuf, *opixbuf,
		 v1, v2;

	unsigned int x, y;

	unsigned long w, memwidth, planelength;


	w = (unsigned long)((width + 1) / 2);
	memwidth = (unsigned long)((width + 7) / 8) * 8L;

	planelength = (unsigned long)((width + 7) / 8) * (unsigned long)height;

	if((ziel = SMalloc(memwidth * (unsigned long)height + 7)) == 0)
		return(NOMEM);
	else
	{
		oziel = ziel;

		opixbuf = pixbuf = (char *)Malloc(width);

		y = 0;
		do
		{
			if(packType == 1)
			{
				x = 0;
				do
				{
					v2 = *buffer++;
					*pixbuf++ = v2 >> 4;
					*pixbuf++ = v2 & 0x0f;
				} while(++x < w);

				align += w;									/* den buffer += w */
			}
			else
			{
				buffer += CtrlByteLen;
				align += CtrlByteLen;						/* den buffer += CtrlByteLen */

				x = 0;
				do
				{
					v1 = *buffer++;

					if(v1 > 0x7f)							/* Encoded Run */
					{ 	
						v1 = 0x101 - v1;

						v2 = *buffer++;

						x += v1;

						align += 2;							/* die beiden buffer++ */

						while(v1--)
						{
							*pixbuf++ = v2 >> 4;
							*pixbuf++ = v2 & 0x0f;
						}
					} /* x != 0 */
					else									/* Literal Run */
					{
						v1++;

						x += v1;

						align += v1 + 1;					/* den buffer++ und die v1 folgenden */

						while(v1--)
						{
							v2 = *buffer++;
							*pixbuf++ = v2 >> 4;
							*pixbuf++ = v2 & 0x0f;
						}
					}
				} while(x < w);
			}
/*			printf("x: %u, w: %lu\n", x, w);
			getch(); */

			pixbuf = opixbuf;
			ziel += setpix_std_line(pixbuf, ziel, 4, planelength, width);
		} while(++y < height);

		Mfree(pixbuf);
	} /* Malloc */

	ziel = oziel;

	got_pic = TRUE;

	return(ziel);
} /* decode4 */



/* Dekodiert 16 Bit PICT mit Kompressionsarten 0 und 3 (wordweise gepackt) */
char *decode16(unsigned int width, unsigned int height, char CtrlByteLen)
{
	unsigned int *buffer16, *ziel16, *oziel16,
				 x, y, v1, v2;

	unsigned long w, more;


	buffer16 = (unsigned int *)buffer;

	w = (unsigned long)width;

	more = align;

	if((ziel16 = (unsigned int *)SMalloc((unsigned long)width * (unsigned long)height * 2)) == 0)
		return(NOMEM);
	else
	{
		oziel16 = ziel16;

		y = 0;
		do
		{			
			(char *)buffer16 += CtrlByteLen;
			align += CtrlByteLen;						/* den buffer += CtrlByteLen */

			x = 0;
			do
			{
				v1 = *((char *)buffer16)++;

				if(v1 > 0x7f)							/* Encoded Run */
				{ 	
					v1 = 0x101 - v1;

					v2 = *buffer16++;
					v2 = ((v2&0x7fe0) << 1) | (v2&0x001f);

					x += v1;

					align += 3;							/* die beiden buffer++ */

					while(v1--) 
						*ziel16++ = v2;
				} /* x != 0 */
				else									/* Literal Run */
				{
					v1++;

						x += v1;

						align += (v1 << 1) + 1;			/* den buffer++ und die v1 folgenden */

						while(v1--)
						{
							v2 = *buffer16++;
							*ziel16++ = ((v2&0x7fe0) << 1) | (v2&0x001f);
						}
				}
			} while(x < w);
/*			printf("x: %u, w: %lu\n", x, w);
			getch(); */
		} while(++y < height);
	} /* Malloc */

	buffer += align - more;

	ziel16 = oziel16;

	got_pic = TRUE;

	return((char *)ziel16);
} /* decode16 */



/* Dekodiert 24 Bit PICT mit Kompressionsarten 0 und 4 */
char *decode24(unsigned int width, unsigned int height, char cmpCount, char CtrlByteLen)
{
	char *ziel, *oziel, *ziel2, *oziel2,
		 v1, v2, pixlen;

	unsigned int x, y;

	unsigned long w;


	w = (unsigned long)width * cmpCount;

	pixlen = cmpCount;

	if((ziel = SMalloc((unsigned long)width * (unsigned long)height * 3)) == 0)
		return(NOMEM);
	else
	{
		if((ziel2 = Malloc((unsigned long)width * pixlen)) == 0)	/* Zeilenbuffer */
		{
			SMfree(ziel);
			return(NOMEM);
		}

		oziel = ziel;
		oziel2 = ziel2;

		y = 0;
		do
		{
			buffer += CtrlByteLen;
			align += CtrlByteLen;						/* den buffer += CtrlByteLen */

			x = 0;
			do
			{
				v1 = *buffer++;

				if(v1 > 0x7f)							/* Encoded Run */
				{ 	
					v1 = 0x101 - v1;

					v2 = *buffer++;

					x += v1;

					align += 2;							/* die beiden buffer++ */

					while(v1--) 
						*ziel2++ = v2;
				} /* x != 0 */
				else									/* Literal Run */
				{
					v1++;

					x += v1;

					align += v1 + 1;					/* den buffer++ und die v1 folgenden */

					while(v1--)
						*ziel2++ = *buffer++;
				}
			} while(x < w);
/*			printf("x: %u, w: %lu\n", x, w);
			getch(); */
			/* RGB-Folgen aus dem Zeilenbuffer wieder umsortieren */
			ziel2 = oziel2;
			if(cmpCount == 4)
				ziel2 += width;
			x = 0;
			do
			{
				*ziel++ = *ziel2;
				*ziel++ = *(ziel2 + width);
				*ziel++ = *(ziel2 + (width << 1));
				ziel2++;
			} while(++x < width);
			ziel2 = oziel2;
		} while(++y < height);
	} /* Malloc */

	ziel = oziel;

	Mfree(ziel2);

	got_pic = TRUE;

	return(ziel);
} /* decode24 */


/* Dispatcher - entscheidet, welche Routine f�r die Dekodierung */
/* des eigentlichen Bildes aufgerufen wird */
void Dispatch(struct pixMap *p)
{
	char BitsPerPixel;
	char dummy[3], impmessag[21];

	unsigned int width, height, cmpCount, packType, ctrlByte, ctrlByteLen;


	ctrlByte = p->ctrlByte;							/* 0x80 = Byte, andere = Word */
	/*
	   packType:
	   0 - default packing (pixelSize 16 defaults to packType 3 and
		   pixelSize 32 defaults to packType 4)
	   1 - no packing
	   2 - remove pad byte (32-bit pixels only)
	   3 - run-length encoding by pixel size chunks (16-bit pixels only)
	   4 - run-length encoding, all of one component at the time, one scan
		   line at a time (24-bit pixels only)
	*/
	packType = p->packType;
	cmpCount = p->cmpCount;							/* 3 = RGB, 4 = aRGB */
	BitsPerPixel = p->cmpSize * cmpCount;
	if(BitsPerPixel == 15)
		BitsPerPixel = 16;

	if(ctrlByte == 0x80)
		ctrlByteLen = 1;
	else
		ctrlByteLen = 2;

	width = picFrame.right - picFrame.left;
	height = picFrame.bottom - picFrame.top;

	strncpy(giveitme->smurf_pic->format_name, "Macintosh PICT .PICT", 21);
	giveitme->smurf_pic->pic_width = width;
	giveitme->smurf_pic->pic_height = height;
	giveitme->smurf_pic->depth = BitsPerPixel;

	strcpy(impmessag, "Macintosh PICT ");
	strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
	strcat(impmessag, " Bit");
	giveitme->services->reset_busybox(128, impmessag);

	if(BitsPerPixel == 16 && (packType == 0 || packType == 3))
		ziel = decode16(width, height, ctrlByteLen);
	else
		if(BitsPerPixel == 24 && (packType == 0 || packType == 4))
			ziel = decode24(width, height, cmpCount, ctrlByteLen);
		else
			if(BitsPerPixel == 4)
				ziel = decode4(width, height, ctrlByteLen, packType);
			else
				if(packType == 1 || packType == 2)
					ziel = readunpacked(width, height, BitsPerPixel, packType);
				else
					ziel = decode(width, height, BitsPerPixel, ctrlByteLen);

	if(BitsPerPixel == 1 || BitsPerPixel == 4)
		giveitme->smurf_pic->format_type = FORM_STANDARD;
	else
		giveitme->smurf_pic->format_type = FORM_PIXELPAK;

	giveitme->smurf_pic->col_format = RGB;

	return;
} /* Dispatch */


/* Alle Daten in Version 2-Bildern sind wordaligned. */
/* Ungerade Daten werden mit einem Nullbyte aufgef�llt. */
unsigned int get_op(unsigned int version)
		read_byte();
		return(read_byte());


char read_byte()
	return(*buffer++ & 0xff);

unsigned int read_word()
	align += 2;

	return(*((unsigned int *)buffer)++);

unsigned long read_long()

	return(*((unsigned long *)buffer)++);

	return;
}

	return;
}
	return;
}
	return;
}

void read_pattern()

	{
				 break;
	return;
}
void read_pixmap(struct pixMap *p, unsigned int *rowBytes)
	p->version = read_word();
	p->pixelType = read_word();
	p->cmpSize = read_word();
	return;
}

void Clip()
	return;
}

	return;
}
	return;
}
	return;
}
	return;
}
	return;
}
	return;
}
void skip_poly_or_region()
	return;
}

	r->left = read_word();
	return;
}
void BitsRect()

	rowBytes = read_word();
	p.ctrlByte = (char)(rowBytes >> 8);

	return;
}

	rowBytes = read_word();
	p.ctrlByte = (char)(rowBytes >> 8);

	return;
}

void Opcode_9A()

	skip(4);						/* mysterious 0x000000ff */
	p.packType = read_word();
	Dispatch(&p);

	return;
}


void do_bitmap(int is_region)

	Dispatch(&p);

	return;
}
void do_pixmap(int is_region)
	read_pixmap(&p, NULL);
	read_rect(&srcRect);
	read_word();					/* mode */
	if(is_region)
	Dispatch(&p);

	return;
}

void read_color_table()
	unsigned long ctSeed;
	ctSeed = read_long();
	pal = giveitme->smurf_pic->palette;

	for(i = 0; i <= ctSize; i++)
	{
		*pal++ = *(buffer + 2);
		*pal++ = *(buffer + 4);
		*pal++ = *(buffer + 6);
		buffer += 8;
	}

	return;
}

void unpackbits(struct Rect *bounds, unsigned int rowBytes, int pixelSize)


		pixwidth *= 2;
		if(pixelSize == 32)
		skip(rowBytes * pixheight);
	{
		if(rowBytes > 250 || pixelSize > 8)
			while(pixheight--)
		else
			while(pixheight--)
	}
	return;
}

void nop()
{
	return;
}