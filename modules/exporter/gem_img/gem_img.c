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
/* Version 0.1  --  04.06.96 - 09.06.96						*/
/*	  Nur XIMG.	Und nur Bilder im korrekten Format.			*/
/* Version 0.2  --  10.01.97 - 20.01.97						*/
/*	  Kompressionstheorem Åberarbeitet und Bugs gefixt		*/
/*	  wegen denen in anderen Speicher geschreiben wurde.	*/
/* Version 0.3  --  28.04.97								*/
/*	  Eigenes Konfigurationsformular eingebaut.				*/
/* Version 0.4  --  24.09.98								*/
/*	  1 Bit Bilder jetzt auch als IMG.						*/
/* Version 0.5  --  17.11.98								*/
/*	  Nun auch Bilder bis <= 8 Bit als IMG.					*/
/* Version 0.6  --  xx.xx.98								*/
/*	  CMY-IMGs kînnen nun erstellt werden.					*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <..\..\..\demolib.h>

#define ENGLISCH 0

#if ENGLISCH
	#include "gem_img\en\gem_img.rsh"
	#include "gem_img\en\gem_img.rh"
#else
	#include "gem_img\de\gem_img.rsh"
	#include "gem_img\de\gem_img.rh"
#endif


#define SOLID0 1
#define SOLID1 2
#define PATRUN 3
#define BITSTR 4

#define IMG		0
#define XIMG	1

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);

unsigned long write_RGB(char *buffer, char *ziel, unsigned long w, char v, unsigned int height, unsigned int strings);
unsigned long write_Plane(char *buffer, char *ziel, unsigned long w, unsigned int height, char BitsPerPixel, unsigned int strings);


/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"GEM-(X)Image",
						0x0050,
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
						1, 2, 4, 6, 7,
						8, 24, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_BOTH,
						2				/* More */
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*			GEM-(X)IMG Komprimierer (IMG)			*/
/*		1-8 , 24 Bit, eigene Komprimierungen		*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel, *pal,
		 v, BitsPerPixel, Planes, PattLength, t;
	char wt[] = "GEM-(X)IMG Exporter";

	static int module_id;
	unsigned int *ppal, 
				 k, width, height, strings, Header,
				 img_headsize, ximg_headsize, Palette, Button;
	
	unsigned long w, memwidth;

	unsigned long f_len;

	static WINDOW window;
	static OBJECT *win_form;

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

	typedef struct
	{
		char format;	/* IMG oder XIMG */
		char mode;		/* GVW oder Pixart */
	} CONFIG;

	IMG_HEAD *img_header;
	XIMG_HEAD *ximg_header;
	static CONFIG config;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			/* falls Åbergeben, Konfig Åbernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy((char *)&config, (char *)*(long *)&smurf_struct->event_par[0], sizeof(CONFIG));
			else
			{
				config.format = XIMG;
				config.mode = GVW;
			}

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[XIMG_EXPORT];			/* Resourcebaum holen */

			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			smurf_struct->module_mode = M_WAITING;
			break;

		case MMORE:
			/* Ressource aktualisieren */
			if(config.mode == GVW)
			{
				win_form[GVW].ob_state |= SELECTED;
				win_form[PIXA].ob_state &= ~SELECTED;
			}
			else
			{
				win_form[GVW].ob_state &= ~SELECTED;
				win_form[PIXA].ob_state |= SELECTED;
			}

			if(config.format == XIMG)
				win_form[FORMAT].ob_state |= SELECTED;
			else
				win_form[FORMAT].ob_state &= ~SELECTED;

			f_module_window = smurf_struct->services->f_module_window;	/* Windowfunktion */
	
			window.whandlem = 0;				/* evtl. Handle lîschen */
			window.module = module_id;			/* ID in die Fensterstruktur eintragen  */
			window.wnum = 1;					/* Fenster nummer 1...  */
			window.wx = -1;						/* Fenster X-...    	*/
			window.wy = -1;						/* ...und Y-Pos     	*/
			window.ww = win_form->ob_width;		/* Fensterbreite    	*/
			window.wh = win_form->ob_height;	/* Fensterhîhe      	*/
			strcpy(window.wtitle, wt);			/* Titel reinkopieren   */
			window.resource_form = win_form;	/* Resource         	*/
			window.picture = NULL;				/* kein Bild.       	*/ 
			window.editob = 0;					/* erstes Editobjekt	*/
			window.nextedit = 0;				/* nÑchstes Editobjekt	*/
			window.editx = 0;

			smurf_struct->wind_struct = &window;  /* und die Fensterstruktur in die Gargamel */

			if(f_module_window(&window) == -1)			/* Gib mir 'n Fenster! */
				smurf_struct->module_mode = M_EXIT;		/* keins mehr da? */
			else 
				smurf_struct->module_mode = M_WAITING;	/* doch? Ich warte... */

			break;

	/* Closer geklickt, Default wieder her */
		case MMORECANC:
			/* falls Åbergeben, Konfig Åbernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy((char *)&config, (char *)*(long *)&smurf_struct->event_par[0], sizeof(CONFIG));
			else
			{
				config.format = XIMG;
				config.mode = GVW;
			}

			smurf_struct->module_mode = M_WAITING;

			break;

	/* Buttonevent */
		case MBEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&config;
				smurf_struct->event_par[2] = (int)sizeof(CONFIG);

				smurf_struct->module_mode = M_MOREOK;
			}
			else
			if(Button == SAVE)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&config;
				smurf_struct->event_par[2] = (unsigned int)sizeof(CONFIG);

				smurf_struct->module_mode = M_CONFSAVE;
			}
			else
			{
				if(Button == FORMAT)
					config.format ^= 1;				/* Format togglen */
				else				
					if(Button == GVW || Button == PIXA)
						config.mode = (char)Button;

				smurf_struct->module_mode = M_WAITING;
			}

			break;

	/* Keyboardevent */
		case MKEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
				smurf_struct->module_mode = M_MOREOK;

			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */

			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			img_headsize = (unsigned int)sizeof(IMG_HEAD);
			ximg_headsize = (unsigned int)sizeof(XIMG_HEAD);

			buffer = smurf_struct->smurf_pic->pic_data;

			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			/* allgemeine Variablen fÅllen */
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			BitsPerPixel = smurf_struct->smurf_pic->depth;
			if(BitsPerPixel == 24 && config.mode == GVW)
			{
				Planes = 1;
				PattLength = 3;
			}
			else
			{
				Planes = BitsPerPixel;
				PattLength = 1;
			}

			Header = img_headsize;
			if(config.format == XIMG || BitsPerPixel == 24)
			{
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
				img_header->patlen = PattLength;
				img_header->pixwidth = 0x174;
				img_header->pixheight = 0x174;
				img_header->width = width;
				img_header->height = height;

				/* erweiterten Header fÅllen */
				if(config.format == XIMG || BitsPerPixel == 24)
				{
					ximg_header = (XIMG_HEAD *)(ziel + img_headsize);
			 		ximg_header->imgtype = 'XIMG';
				 	ximg_header->colmodel = RGB_;
				 }

				strncpy(smurf_struct->smurf_pic->format_name, "GEM-(X)IMG .IMG", 21);

				/* Farbpalette fÅllen */
				if((config.format == XIMG || BitsPerPixel > 1) && BitsPerPixel != 24)
				{
					ppal = (unsigned int *)(ziel + img_headsize + ximg_headsize);

					pal = smurf_struct->smurf_pic->palette;
					for(k = 0; k < (1 << BitsPerPixel); k++)
					{
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
						*ppal++ = (unsigned int)(((unsigned long)*pal++ * 1000L) / 255);
					}
				}

				f_len = Header;

				if(BitsPerPixel == 24 && config.mode == GVW)
					f_len += write_RGB(buffer, ziel + Header, w, v, height, strings);
				else
					f_len += write_Plane(buffer, ziel + Header, w, height, BitsPerPixel, strings);

				Mshrink(0, ziel, f_len);
				exp_pic->pic_data = ziel;
				exp_pic->f_len = f_len;
			} /* Malloc */

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */

			smurf_struct->module_mode = M_DONEEXIT;
			return(exp_pic);

/* Mterm empfangen - Speicher freigeben und beenden */
		case MTERM:
			SMfree(exp_pic->pic_data);
			SMfree((char *)exp_pic);
			smurf_struct->module_mode = M_EXIT;
			break;

		default:
			smurf_struct->module_mode = M_WAITING;
			break;
	} /* switch */

	return(NULL);
}


unsigned long write_RGB(char *buffer, char *ziel, unsigned long w, char v, unsigned int height, unsigned int strings)
{
	char *obuffer, *vbuffer,
		 vrc, v3[3], count, run;

	unsigned int x, y, k;

	unsigned long plo, complen, complenp, uncomplen, left;


	complen = 0;
	complenp = 0;
	uncomplen = (unsigned long)strings * 2L + w;

	obuffer = buffer;

	y = 0;
	do /* height */
	{
		plo = y * w; /* Offset vom Planeanfang in Bytes */

	/* vrc mîglich? */
		vrc = 1;
		while(y + vrc < height && vrc < 255)
		{
			vbuffer = obuffer + plo;

			if(memcmp(vbuffer, vbuffer + w, w) != 0)
				goto whileoff;
			plo += w;
			vrc++;
		}

whileoff:
		if(vrc > 1)		/* Bringt's der if? */
		{
			y += vrc - 1;
			plo = y * w; /* Offset vom Planeanfang in Bytes */

			*ziel++ = 0x0;
			*ziel++ = 0x0;
			*ziel++ = 0xff;
			*ziel++ = vrc;
			complen += 4;
		}

		buffer = obuffer + plo; /* Quelladresse der zu kodierenden Scanline */

		x = 0;
		do /* width */
		{
			count = 1;

			v3[0] = *buffer++;
			v3[1] = *buffer++;
			v3[2] = *buffer++;
			x += 3;

		 	while(x + 3 <= w && count < 255)
			{
				if(*buffer != v3[0] || *(buffer + 1) != v3[1] || *(buffer + 2) != v3[2])
					break;
				else
				{
					buffer += 3;
					x += 3;
					count++;
				}
			}

			if(count > 1)			/* PATRUN */
			{
				*ziel++ = 0x0;
				*ziel++ = count;
				*ziel++ = v3[0];
				*ziel++ = v3[1];
				*ziel++ = v3[2];
				complenp += 5;
			}
			else					/* BITSTR */
			{
				*ziel++ = 0x80;
				ziel++;
				*ziel++ = v3[0];
				*ziel++ = v3[1];
				*ziel++ = v3[2];
				count = 3;
				while(x < w && count < 253)
				{
					if(x + 6 > w || *buffer != *(buffer + 3) || *(buffer + 1) != *(buffer + 4) || *(buffer + 2) != *(buffer + 5))
					{
						*ziel++ = *buffer++;
						*ziel++ = *buffer++;
						*ziel++ = *buffer++;
						x += 3;
						count += 3;
					}
					else
						break;
				}
				*(ziel - count - 1) = count;
				complenp += 2 + count;
			}
		} while(x < w); /* x */
		if(complenp > uncomplen)
		{
			ziel -= complenp;
			buffer -= w;
			left = w;
			k = 0;
			do
			{
				if(left >= 255)
					run = 255;
				else
					run = left;
				*ziel++ = 0x80;
				*ziel++ = (char)run;
				memcpy(ziel, buffer, run);
				ziel += run;
				buffer += run;
				complen += run + 2;
				left -= run;
			} while(++k < strings);
		}
		else
			complen += complenp;

		complenp = 0;

	/* noch v Bytes als SOLID0 dazu */
		if(v)
		{
			*ziel++ = v;
			complen++;
		}
	} while(++y < height);

	return(complen);
} /* write_RGB */


unsigned long write_Plane(char *buffer, char *ziel, unsigned long w, unsigned int height, char BitsPerPixel, unsigned int strings)
{
	char *obuffer, *vbuffer,
		 vrc, v1, count, run, Planes, p, mode;

	unsigned int x, y, k;

	unsigned long pla, plh, plo, complen, complenp, uncomplen, left;


	Planes = BitsPerPixel;

	plh = (unsigned long)height * w;		/* Hîhe einer Plane in Byte */

	complen = 0;
	complenp = 0;
	uncomplen = (unsigned long)strings * 2L + w;

	obuffer = buffer;

	y = 0;
	do
	{
		plo = y * w; /* Offset vom Planeanfang in Bytes */

	/* vrc mîglich? */
		vrc = 1;
		while(y + vrc < height && vrc < 255)
		{
			p = 0;
			do
			{
				vbuffer = obuffer + p * plh + plo;

				if(memcmp(vbuffer, vbuffer + w, w) != 0)
					goto whileoff;
			} while(++p < Planes);
			plo += w;
			vrc++;
		}

whileoff:
		if(vrc > 1)			/* Bringt's der if? */
		{
			y += vrc - 1;
			plo = y * w;	/* Offset vom Planeanfang in Bytes */

			*ziel++ = 0x0;
			*ziel++ = 0x0;
			*ziel++ = 0xff;
			*ziel++ = vrc;
			complen += 4;
		}

		for(p = 0; p < Planes; p++) /* Planes */
		{
			pla = plh * p;	/* Abstand dieser Plane vom Bildanfang */
			buffer = obuffer + pla + plo; /* Quelladresse der zu kodierenden Scanline */

			x = 0;
			do
			{
				count = 1;

				v1 = *buffer++;
				x++;

				if(v1 == 0x0)
					mode = SOLID0;
				else
					if(v1 == 0xff)
						mode = SOLID1;
					else
						if(x + 1 < w &&
						   *buffer == v1 &&
						   *(buffer + 1) == v1)
							 mode = PATRUN;
						 else
							 mode = BITSTR;

				switch(mode)
				{
					case SOLID0: while(*buffer == 0x0 && x < w && count < 127)
								 {
									 buffer++;
									 x++;
									 count++;
								 }
								 *ziel++ = count;
								 complenp++;
								 break;
					case SOLID1: while(*buffer == 0xff && x < w && count < 127)
								 {
									 buffer++;
									 x++;
									 count++;
								 }
								 *ziel++ = count | 0x80;
								 complenp++;
								 break;
					case PATRUN: while(*buffer == v1 && x < w && count < 255)
								 {
								 	 buffer++;
									 x++;
									 count++;
								 }
								 *ziel++ = 0x0;
								 *ziel++ = count;
								 *ziel++ = v1;
								 complenp += 3;
								 break;
					case BITSTR: *ziel++ = 0x80;
								 ziel++;
								 *ziel++ = v1;
								 while(x < w && count < 255)
								 {
								 	if(x + 3 > w ||
									   !(*buffer == 0x0 && *(buffer + 1) == 0x0 && *(buffer + 2) == 0x0) &&
									   !(*buffer == 0xff && *(buffer + 1) == 0xff && *(buffer + 2) == 0xff) &&
									   !(*buffer == *(buffer + 1) || *buffer == *(buffer + 2)))
									{
										*ziel++ = *buffer++;
										x++;
										count++;
									}
									else
										break;
								 }
								 *(ziel - count - 1) = count;
								 complenp += 2 + count;
								 break;
				}
			} while(x < w);
			if(complenp > uncomplen)
			{
				ziel -= complenp;
				buffer -= w;
				left = w;
				k = 0;
				do
				{
					if(left >= 255)
						run = 255;
					else
						run = left;
					*ziel++ = 0x80;
					*ziel++ = (char)run;
					memcpy(ziel, buffer, run);
					ziel += run;
					buffer += run;
					complen += run + 2;
					left -= run;
				} while(++k < strings);
			}
			else
				complen += complenp;

			complenp = 0;
		}
	} while(++y < height);

	return(complen);
} /* write_Plane */