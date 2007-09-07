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
/*	  sowieso nur Rastergrafiken vom Smurf unterstÅtzt		*/
/*	  werden, wird auch nur der Teil gelesen. Das sollten	*/
/*	  aber sowieso die hÑufigsten sein.						*/
/*	  Komprimierung ist RLE und JPEG (Quickdraw);			*/
/*	  JPEG lasse ich auch mal auf unbestimmte Zeit weg -	*/
/*	  die GrÅnde dÅrften bekannt sein.						*/
/*	  1 Bit													*/
/*	  Version 1 und 2										*/
/* Version 0.2 -- 04.11.96									*/
/*	  8	Bit													*/
/* Version 0.3 -- 06.11.96									*/
/*	  16 Bit												*/
/* Version 0.4 -- 10.11.96									*/
/*	  Grîûere Umbauten (Dekoder in Funktionen);				*/
/*	  24 und 32 Bit, unkomprimierte							*/
/* Version 0.5 -- 12.11.96									*/
/*	  4 Bit													*/
/* Version 0.6 -- 19.11.96 - 26.11.96						*/
/*    Importer total umgeschrieben. Der Infoteil des		*/
/*    Files wird nun nicht mehr starr gelesen, sondern		*/
/*    interpretiert um alle Files lesen zu kînnen.			*/
/* Version 0.7 -- 16.03.97									*/
/*	  1 und 8 Bit Bilder mit ungerader ZeilenlÑnge werden	*/
/*	  nun auch korrekt dekodiert (die sind nÑmlich unver-	*/
/*	  stÑndlicherweise auf gerade Anzahl Pixel aligned		*/
/*	  abgespeichert. Zumindest bei RLE, wie es unkomprimiert*/
/*	  ist, weiû ich nicht, da der Grafikkonverter die nicht	*/
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
#define WORD_LEN	(-1)#define RGB_LEN		(6)#define NA			(0)

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
char read_byte(void);unsigned int read_word(void);unsigned long read_long(void);void skip(unsigned int n);void BkPixPat(void);void PnPixPat(void);void FillPixPat();void read_pattern(void);void read_pixmap(struct pixMap *p, unsigned int *rowBytes);
void Clip(void);void skip_text(void);void LongText(void);void DHText(void);void DVText(void);void DHDVText(void);
void LongComment(void);void skip_poly_or_region(void);void read_rect(struct Rect *r);
void BitsRect(void);void BitsRegion(void);void Opcode_9A(void);
void do_bitmap(int is_region);void do_pixmap(int is_region);
void read_color_table(void);void unpackbits(struct Rect *bounds, unsigned int rowBytes, int pixelSize);

/* Infostruktur fÅr Hauptmodul */
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

struct Rect {	unsigned int top;	unsigned int left;	unsigned int bottom;	unsigned int right;};

struct pixMap {	struct Rect Bounds;	unsigned int version;	unsigned int packType;	unsigned long packSize;	unsigned long hRes;	unsigned long vRes;	unsigned int pixelType;	unsigned int pixelSize;	unsigned int cmpCount;	unsigned int cmpSize;	unsigned long planeBytes;	unsigned long pmTable;	unsigned long pmReserved;	char ctrlByte;
};struct ct_entry {	unsigned int red;	unsigned int green;	unsigned int blue;};

struct opdef
{	int len;	void (*impl)();};

struct Rect picFrame;
GARGAMEL *giveitme;

/* for reserved opcodes of known length */#define res(length) {(length), NULL}

/* for reserved opcodes of length determined by a function */#define resf(skipfunction) {NA, (skipfunction)}

struct opdef optable[] =
{/* 0x00 */	{0, NULL},						/* NOP *//* 0x01 */	{NA, Clip},						/* Clip *//* 0x02 */	{8, NULL},						/* BkPat *//* 0x03 */	{2, NULL},						/* TxFont *//* 0x04 */	{1, NULL},						/* TxFace *//* 0x05 */	{2, NULL},						/* TxMode *//* 0x06 */	{4, NULL},						/* SpExtra *//* 0x07 */	{4, NULL},						/* PnSize *//* 0x08 */	{2, NULL},						/* PnMode *//* 0x09 */	{8, NULL},						/* PnPat *//* 0x0a */	{8, NULL},						/* FillPat *//* 0x0b */	{4, NULL},						/* OvSize *//* 0x0c */	{4, NULL},						/* Origin *//* 0x0d */	{2, NULL},						/* TxSize *//* 0x0e */	{4, NULL},						/* FgColor *//* 0x0f */	{4, NULL},						/* BkColor *//* 0x10 */	{8, NULL},						/* TxRatio *//* 0x11 */	{1, NULL},						/* Version *//* 0x12 */	{NA, BkPixPat},					/* BkPixPat *//* 0x13 */	{NA, PnPixPat},					/* PnPixPat *//* 0x14 */	{NA, FillPixPat},				/* FillPixPat *//* 0x15 */	{2, NULL},						/* PnLocHFrac *//* 0x16 */	{2, NULL},						/* ChExtra *//* 0x17 */	res(0),/* 0x18 */	res(0),/* 0x19 */	res(0),/* 0x1a */	{RGB_LEN, NULL},				/* RGBFgCol *//* 0x1b */	{RGB_LEN, NULL},				/* RGBBkCol *//* 0x1c */	{0, NULL},						/* HiliteMode *//* 0x1d */	{RGB_LEN, NULL},				/* HiliteColor *//* 0x1e */	{0, NULL},						/* DefHilite *//* 0x1f */	{RGB_LEN, NULL},				/* OpColor *//* 0x20 */	{8, NULL},						/* Line *//* 0x21 */	{4, NULL},						/* LineFrom *//* 0x22 */	{6, NULL},						/* ShortLine *//* 0x23 */	{2, NULL},						/* ShortLineFrom *//* 0x24 */	res(WORD_LEN),/* 0x25 */	res(WORD_LEN),/* 0x26 */	res(WORD_LEN),/* 0x27 */	res(WORD_LEN),/* 0x28 */	{NA, LongText},					/* LongText *//* 0x29 */	{NA, DHText},					/* DHText *//* 0x2a */	{NA, DVText},					/* DVText *//* 0x2b */	{NA, DHDVText},					/* DHDVText *//* 0x2c */	res(WORD_LEN),/* 0x2d */	res(WORD_LEN),/* 0x2e */	res(WORD_LEN),/* 0x2f */	res(WORD_LEN),/* 0x30 */	{8, NULL},						/* frameRect *//* 0x31 */	{8, NULL},						/* paintRect *//* 0x32 */	{8, NULL},						/* eraseRect *//* 0x33 */	{8, NULL},						/* invertRect *//* 0x34 */	{8, NULL},						/* fillRect *//* 0x35 */	res(8),/* 0x36 */	res(8),/* 0x37 */	res(8),/* 0x38 */	{0, NULL},						/* frameSameRect *//* 0x39 */	{0, NULL},						/* paintSameRect *//* 0x3a */	{0, NULL},						/* eraseSameRect *//* 0x3b */	{0, NULL},						/* invertSameRect *//* 0x3c */	{0, NULL},						/* fillSameRect *//* 0x3d */	res(0),/* 0x3e */	res(0),/* 0x3f */	res(0),/* 0x40 */	{8, NULL},						/* frameRRect *//* 0x41 */	{8, NULL},						/* paintRRect *//* 0x42 */	{8, NULL},						/* eraseRRect *//* 0x43 */	{8, NULL},						/* invertRRect *//* 0x44 */	{8, NULL},						/* fillRRrect *//* 0x45 */	res(8),/* 0x46 */	res(8),/* 0x47 */	res(8),/* 0x48 */	{0, NULL},						/* frameSameRRect *//* 0x49 */	{0, NULL},						/* paintSameRRect *//* 0x4a */	{0, NULL},						/* eraseSameRRect *//* 0x4b */	{0, NULL},						/* invertSameRRect *//* 0x4c */	{0, NULL},						/* fillSameRRect *//* 0x4d */	res(0),/* 0x4e */	res(0),/* 0x4f */	res(0),/* 0x50 */	{8, NULL},						/* frameOval *//* 0x51 */	{8, NULL},						/* paintOval *//* 0x52 */	{8, NULL},						/* eraseOval *//* 0x53 */	{8, NULL},						/* invertOval *//* 0x54 */	{8, NULL},						/* fillOval *//* 0x55 */	res(8),/* 0x56 */	res(8),/* 0x57 */	res(8),/* 0x58 */	{0, NULL},						/* frameSameOval *//* 0x59 */	{0, NULL},						/* paintSameOval *//* 0x5a */	{0, NULL},						/* eraseSameOval *//* 0x5b */	{0, NULL},						/* invertSameOval *//* 0x5c */	{0, NULL},						/* fillSameOval *//* 0x5d */	res(0),/* 0x5e */	res(0),/* 0x5f */	res(0),/* 0x60 */	{12, NULL},						/* frameArc *//* 0x61 */	{12, NULL},						/* paintArc *//* 0x62 */	{12, NULL},						/* eraseArc *//* 0x63 */	{12, NULL},						/* invertArc *//* 0x64 */	{12, NULL},						/* fillArc *//* 0x65 */	res(12),/* 0x66 */	res(12),/* 0x67 */	res(12),
/* 0x68 */	{4, NULL},						/* frameSameArc *//* 0x69 */	{4, NULL},						/* paintSameArc *//* 0x6a */	{4, NULL},						/* eraseSameArc *//* 0x6b */	{4, NULL},						/* invertSameArc *//* 0x6c */	{4, NULL},						/* fillSameArc *//* 0x6d */	res(4),/* 0x6e */	res(4),/* 0x6f */	res(4),/* 0x70 */	{NA, skip_poly_or_region},		/* framePoly *//* 0x71 */	{NA, skip_poly_or_region},		/* paintPoly *//* 0x72 */	{NA, skip_poly_or_region},		/* erasePoly *//* 0x73 */	{NA, skip_poly_or_region},		/* invertPoly *//* 0x74 */	{NA, skip_poly_or_region},		/* fillPoly *//* 0x75 */	resf(skip_poly_or_region),/* 0x76 */	resf(skip_poly_or_region),/* 0x77 */	resf(skip_poly_or_region),/* 0x78 */	{0, NULL},						/* frameSamePoly *//* 0x79 */	{0, NULL},						/* paintSamePoly *//* 0x7a */	{0, NULL},						/* eraseSamePoly *//* 0x7b */	{0, NULL},						/* invertSamePoly *//* 0x7c */	{0, NULL},						/* fillSamePoly *//* 0x7d */	res(0),/* 0x7e */	res(0),/* 0x7f */	res(0),/* 0x80 */	{NA, skip_poly_or_region},		/* frameRgn *//* 0x81 */	{NA, skip_poly_or_region},		/* paintRgn *//* 0x82 */	{NA, skip_poly_or_region},		/* eraseRgn *//* 0x83 */	{NA, skip_poly_or_region},		/* invertRgn *//* 0x84 */	{NA, skip_poly_or_region},		/* fillRgn *//* 0x85 */	resf(skip_poly_or_region),/* 0x86 */	resf(skip_poly_or_region),/* 0x87 */	resf(skip_poly_or_region),/* 0x88 */	{0, NULL},						/* frameSameRgn *//* 0x89 */	{0, NULL},						/* paintSameRgn *//* 0x8a */	{0, NULL},						/* eraseSameRgn *//* 0x8b */	{0, NULL},						/* invertSameRgn *//* 0x8c */	{0, NULL},						/* fillSameRgn *//* 0x8d */	res(0),
/* 0x8e */	res(0),/* 0x8f */	res(0),/* 0x90 */	{NA, BitsRect},					/* BitsRect *//* 0x91 */	{NA, BitsRegion},				/* BitsRgn *//* 0x92 */	res(WORD_LEN),/* 0x93 */	res(WORD_LEN),/* 0x94 */	res(WORD_LEN),/* 0x95 */	res(WORD_LEN),/* 0x96 */	res(WORD_LEN),/* 0x97 */	res(WORD_LEN),/* 0x98 */	{NA, BitsRect},					/* PackBitsRect *//* 0x99 */	{NA, BitsRegion},				/* PackBitsRgn *//* 0x9a */	{NA, Opcode_9A},				/* Opcode_9A *//* 0x9b */	res(WORD_LEN),/* 0x9c */	res(WORD_LEN),/* 0x9d */	res(WORD_LEN),/* 0x9e */	res(WORD_LEN),/* 0x9f */	res(WORD_LEN),/* 0xa0 */	{2, NULL},						/* ShortComment *//* 0xa1 */	{NA, LongComment}				/* LongComment */};

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

	/* nichtsnutzigen Header Åberspringen */
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
/*	   Diese PrÅfung ist leider nicht hundertprozentig zuverlÑssig, da es Apple geschafft hat,
	   fÅr die Filegrîûe nur ein int zu reservieren, die gespeicherte Grîûe nach 2 Byte
	   abgeschnitten wird und somit halt Åberhaupt nicht stimmt. Bravo :-( */
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
			{				if(optable[opcode].impl != NULL)					(*optable[opcode].impl)();				else
					if(optable[opcode].len >= 0)						skip(optable[opcode].len);					else
						switch(optable[opcode].len)
						{							case WORD_LEN:								len = read_word();								skip(len);								break;							default:								break;						}			}			else
				if(opcode == 0xc00)
					skip(24);				else
					if(opcode >= 0xa2 && opcode <= 0xaf)
						skip(read_word());					else
						if(opcode >= 0xb0 && opcode <= 0xcf)
							/* just a reserved opcode, no data */							nop();
						else
							if(opcode >= 0xd0 && opcode <= 0xfe)
								skip((unsigned int)read_long());							else
								if(opcode >= 0x100 && opcode <= 0x7fff)
									skip((opcode >> 7) & 0xff);								else
									if(opcode >= 0x8000 && opcode <= 0x80ff)
										/* just a reserved opcode */										nop();
									else
										if(opcode >= 0x8100 && (unsigned long)opcode <= 0xffff)
											skip((unsigned int)read_long());										else
										{
											res = form_alert(2, "[2][Unbekannter Opcode! Soll versucht werden, | ihn einfach zu Åberlesen?][ Ja ][ Nein ]");
											if(res == 2)
												return(M_PICERR);
										}		} /* opcode*/

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
		w = (unsigned long)((width + 7) / 8);			/* Auf volle Byte gerundete ZeilenlÑnge in Byte */
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
		w = (unsigned long)((width + 7) / 8); /* Auf volle Byte gerundete ZeilenlÑnge in Byte */
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


/* Dispatcher - entscheidet, welche Routine fÅr die Dekodierung */
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
/* Ungerade Daten werden mit einem Nullbyte aufgefÅllt. */
unsigned int get_op(unsigned int version){	if((align & 1) && version == 2)
		read_byte();	if (version == 1)
		return(read_byte());	else		return(read_word());}


char read_byte(){	align++;
	return(*buffer++ & 0xff);}

unsigned int read_word(){
	align += 2;

	return(*((unsigned int *)buffer)++);}

unsigned long read_long(){	align += 4;

	return(*((unsigned long *)buffer)++);}
void skip(unsigned int n){	buffer += n;	align += n;
	return;
}
/* Diese 3 Åberspringen einfach ihre Daten. */void BkPixPat(){	read_pattern();
	return;
}void PnPixPat(){	read_pattern();
	return;
}void FillPixPat(){	read_pattern();
	return;
}
/* öberliest einfach das nÑchste Version 2 Pattern. */
void read_pattern(){	unsigned int PatType;	unsigned int rowBytes;
	PatType = read_word();	switch(PatType)
	{		case 2: skip(8); /* old pattern data */				skip(5); /* RGB for pattern */				break;		case 1:	skip(8); /* old pattern data */				read_pixmap(&p, &rowBytes);				read_color_table();				unpackbits(&p.Bounds, rowBytes, p.pixelSize);				break;		default: /* unknown pattern type in read_pattern */
				 break;	}
	return;
}
void read_pixmap(struct pixMap *p, unsigned int *rowBytes){	if(rowBytes != NULL)		*rowBytes = read_word();	read_rect(&p->Bounds);
	p->version = read_word();	p->packType = read_word();	p->packSize = read_long();	p->hRes = read_long();	p->vRes = read_long();
	p->pixelType = read_word();	p->pixelSize = read_word();	p->cmpCount = read_word();
	p->cmpSize = read_word();	p->planeBytes = read_long();	p->pmTable = read_long();	p->pmReserved = read_long();
	return;
}

void Clip(){	skip(read_word() - 2);
	return;
}
/* Zeugs fÅr Textausgabe */void skip_text(){	skip(read_byte());
	return;
}void LongText(){	skip(4);	skip_text();
	return;
}void DHText(){	skip(1);	skip_text();
	return;
}void DVText(){	skip(1);	skip_text();
	return;
}void DHDVText(){	skip(2);	skip_text();
	return;
}void LongComment(){	skip(2);	skip(read_word());
	return;
}
void skip_poly_or_region(){	skip(read_word() - 2);
	return;
}
void read_rect(struct Rect *r){	r->top = read_word();
	r->left = read_word();	r->bottom = read_word();	r->right = read_word();
	return;
}
void BitsRect(){	unsigned int rowBytes;

	rowBytes = read_word();
	p.ctrlByte = (char)(rowBytes >> 8);
	if(rowBytes & 0x8000)		do_pixmap(0);	else		do_bitmap(0);
	return;
}void BitsRegion(){	unsigned int rowBytes;

	rowBytes = read_word();
	p.ctrlByte = (char)(rowBytes >> 8);
	if(rowBytes & 0x8000)		do_pixmap(1);	else		do_bitmap(1);
	return;
}

void Opcode_9A(){	struct Rect srcRect;	struct Rect dstRect;

	skip(4);						/* mysterious 0x000000ff */	p.version = read_word();	read_rect(&p.Bounds);	read_word();					/* overread unknown field */
	p.packType = read_word();	p.packSize = read_long();	p.hRes = read_long();	p.vRes = read_long();	p.pixelType = read_word();	p.pixelSize = read_word();	p.cmpCount = read_word();	p.cmpSize = read_word();	p.planeBytes = read_long();	p.pmTable = read_long();	p.pmReserved = read_long();	read_rect(&srcRect);	read_rect(&dstRect);	read_word();					/* mode */
	Dispatch(&p);

	return;
}


void do_bitmap(int is_region){	struct Rect Bounds;	struct Rect srcRect;	struct Rect dstRect;
	read_rect(&Bounds);	read_rect(&srcRect);	read_rect(&dstRect);	read_word();					/* mode */	if(is_region)		skip_poly_or_region();
	Dispatch(&p);

	return;
}
void do_pixmap(int is_region){	struct Rect srcRect;	struct Rect dstRect;
	read_pixmap(&p, NULL);	read_color_table();
	read_rect(&srcRect);	read_rect(&dstRect);
	read_word();					/* mode */
	if(is_region)		skip_poly_or_region();
	Dispatch(&p);

	return;
}

void read_color_table(){	unsigned int i, ctFlags, ctSize;
	unsigned long ctSeed;
	ctSeed = read_long();	ctFlags = read_word();	ctSize = read_word();
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

void unpackbits(struct Rect *bounds, unsigned int rowBytes, int pixelSize){	unsigned int pixwidth, pixheight;
	if(pixelSize <= 8)		rowBytes &= 0x7fff;	pixwidth = bounds->right - bounds->left;	pixheight = bounds->bottom - bounds->top;
	if(pixelSize == 16)
		pixwidth *= 2;	else
		if(pixelSize == 32)			pixwidth *= 3;		if(rowBytes == 0)		rowBytes = pixwidth;	if(rowBytes < 8)
		skip(rowBytes * pixheight);	else
	{
		if(rowBytes > 250 || pixelSize > 8)
			while(pixheight--)				skip(read_word());
		else
			while(pixheight--)				skip(read_byte());
	}
	return;
}

void nop()
{
	return;
}