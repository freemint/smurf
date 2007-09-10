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
/*						GEM-Image Encoder					*/
/*				Spezialversion fÅr Clipboard				*/
/* Version 0.1  --  07.02.97								*/
/*	  1-8 und 24 Bit										*/
/*	  Nur BitString zur schnellen Abarbeitung.				*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#define IMG		0
#define XIMG	1

void *(*SMalloc)(long amount);

int (*f_module_window)(WINDOW *mod_window);

unsigned long write_RGB(char *buffer, char *ziel, unsigned long w, char v, unsigned int height);
unsigned long write_Plane(char *buffer, char *ziel, unsigned long w, unsigned int height, char BitsPerPixel);


/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Clipboard",
						0x0010,
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


MOD_ABILITY module_ability = {
						1, 2, 3, 4, 5,
						7, 8, 24,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_PIXELPAK,
						0
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*			GEM-(X)IMG Komprimierer (IMG)			*/
/*			Spezialversion fÅr Clipboard			*/
/*		1-8 , 24 Bit, eigene Komprimierungen		*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/

EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel, *pal,
		 v, BitsPerPixel, Planes;

	static int module_id;
	unsigned int *ppal, 
				 k, width, height, strings, Header,
				 img_headsize, ximg_headsize, Palette, cols;
	
	unsigned long w, memwidth;

	unsigned long f_len;

	typedef struct
	{
		unsigned int version;
		unsigned int headsize;
		unsigned int BitsPerPixel;
		unsigned int patlen;
		unsigned int pixwidth;
		unsigned int pixheight;
		unsigned int width;
		unsigned int height;
	} IMG_HEAD;

	typedef enum 
	{
		RGB_,
		CMY_,
		HLS_,
		PANTONE_
	} XIMG_COLMODEL;

	typedef struct
	{
		unsigned long imgtype;
		XIMG_COLMODEL colmodel;
	} XIMG_HEAD;

	IMG_HEAD *img_header;
	XIMG_HEAD *ximg_header;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			module_id = smurf_struct->module_number;

			smurf_struct->module_mode = M_WAITING;
			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:
			SMalloc = smurf_struct->services->SMalloc;

			img_headsize = (unsigned int)sizeof(IMG_HEAD);
			ximg_headsize = (unsigned int)sizeof(XIMG_HEAD);

			buffer = smurf_struct->smurf_pic->pic_data;

			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

		/* allgemeine Variablen fÅllen */
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			BitsPerPixel = smurf_struct->smurf_pic->depth;
			if(BitsPerPixel == 24)
				Planes = 1;
			else
				Planes = BitsPerPixel;

			Header = img_headsize;
			if(BitsPerPixel != 1)
			{
				if(BitsPerPixel == 24)
					Palette = 0;
				else
					Palette = (1 << BitsPerPixel) * 6;
				Header += (unsigned int)(ximg_headsize + Palette);
			}
			else
				Palette = 0;

			if(BitsPerPixel == 24)
			{
				w = (unsigned long)width * 3L;
				memwidth = (unsigned long)width;
				v = (char)(((width + 15) / 16) * 16 - width) * 3;
			}
			else
			{
				w = (width + 7) / 8; /* Auf volle Byte gerundete ZeilenlÑnge in Byte */
				memwidth = w * 8;
				v = 0;
			}

			strings = (unsigned int)(w / 255 + 1);

			/* Daten + Bit Strings im worst case + 1 Padbyte gepackt als Solid Run */
			f_len = ((memwidth * (long)height * BitsPerPixel) >> 3) +
					((unsigned long)strings * 2L + 1) * (long)height * (long)Planes;

			if((ziel = (char *)SMalloc(Header + f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				img_header = (IMG_HEAD *)ziel;

			/* Header fÅllen */
				img_header->version = 1;
				img_header->headsize = Header >> 1;
				img_header->BitsPerPixel = BitsPerPixel;
				img_header->patlen = 3;
				img_header->pixwidth = 0x174;
				img_header->pixheight = 0x174;
				img_header->width = width;
				img_header->height = height;

				if(BitsPerPixel != 1)
				{
					ximg_header = (XIMG_HEAD *)(ziel + img_headsize);
			 		ximg_header->imgtype = 'XIMG';
				 	ximg_header->colmodel = RGB_;
				}

				strncpy(smurf_struct->smurf_pic->format_name, "GEM-(X)IMG .IMG", 21);

			/* Farbpalette fÅllen */
				if(BitsPerPixel != 1 && BitsPerPixel != 24)
				{
					ppal = (unsigned int *)(ziel + img_headsize + ximg_headsize);

					pal = smurf_struct->smurf_pic->palette;
					cols = 1 << BitsPerPixel;
					for(k = 0; k < cols; k++)
					{
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
					}
				}

				f_len = Header;

				if(BitsPerPixel == 24)
					f_len += write_RGB(buffer, ziel + Header, w, v, height);
				else
					f_len += write_Plane(buffer, ziel + Header, w, height, BitsPerPixel);

				Mshrink(0, ziel, f_len);
				exp_pic->pic_data = ziel;
				exp_pic->f_len = f_len;
			} /* Malloc */

			smurf_struct->module_mode = M_DONEEXIT;
			return(exp_pic);

/* Mterm empfangen - Speicher freigeben und beenden */
		case MTERM:
			/* exp_pic wird hier mit Absicht nicht freigegeben */
			smurf_struct->module_mode = M_EXIT;
			break;

		default:
			smurf_struct->module_mode = M_WAITING;
			break;
	} /* switch */

	return(NULL);
}


unsigned long write_RGB(char *buffer, char *ziel, unsigned long w, char v, unsigned int height)
{
	unsigned int k, x, y;

	unsigned long run, complen;


	complen = 0;

	y = 0;
	do
	{
		x = 0;
		do
		{
			/* BitString-Head schreiben */
			*ziel++ = 0x80;
			if((run = w - x) > 0xff)
				run = 0xff;

			*ziel++ = run;
			complen += run + 2;

			k = 0;
			do
			{
				*ziel++ = *buffer++;
			} while(++k < run);

			x += k;
		} while(x < w);

	/* noch v Bytes als SOLID0 dazu */
		if(v)
		{
			*ziel++ = v;
			complen++;
		}
	} while(++y < height);

	return(complen);
} /* write_RGB */


unsigned long write_Plane(char *buffer, char *ziel, unsigned long w, unsigned int height, char BitsPerPixel)
{
	char *obuffer,
		 Planes, p;

	unsigned int k, x, y;

	unsigned long pla, plh, plo, run, complen;


	Planes = BitsPerPixel;

	plh = (unsigned long)height * w;		/* Hîhe einer Plane in Byte */

	complen = 0;

	obuffer = buffer;

	y = 0;
	do
	{
		plo = y * w; /* Offset vom Planeanfang in Bytes */

		p = 0;
		do
		{
			pla = plh * p;	/* Abstand dieser Plane vom Bildanfang */
			buffer = obuffer + pla + plo; /* Quelladresse der zu kodierenden Scanline */

			x = 0;
			do
			{
				/* BitString-Head schreiben */
				*ziel++ = 0x80;
				if((run = w - x) > 0xff)
					run = 0xff;

				*ziel++ = run;
				complen += run + 2;

				k = 0;
				do
				{
					*ziel++ = *buffer++;
				} while(++k < run);

				x += k;
			} while(x < w);
		} while(++p < Planes);
	} while(++y < height);

	return(complen);
} /* write_Plane */