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
/*							BMP Encoder						*/
/* Version 0.1  --  06.01.98 - 09.01.98						*/
/*	  1, 4, 8 und 24 Bit, unkomprimiert						*/
/* Version 0.2  --  12.01.98								*/
/*	  1, 8 und 24 Bit, komprimiert							*/
/* Version 0.3  --  13.01.98								*/
/*	  Komprimierung Åberarbeitet, beste Kompressionsrate	*/
/*	  aller Programme, auûerdem werden die Bildern jetzt	*/
/*	  auch von den PC-Programmen gelesen.					*/
/* Version 0.4  --  13.01.98								*/
/*	  4 Bit komprimiert, Kompressionsrate ebensogut			*/
/* Version 0.4.5  --  20.01.98								*/
/*	  OberflÑche erweitert (Unmîglichkeiten in der			*/
/*	  Konfiguration im GUI abgefangen).						*/
/* Version 0.5  --  27.08.98								*/
/*	  Probleme mit der Invertierung von 1 Bit Bildern		*/
/*	  behoben. Leider auf Kosten der Modulgrîûe (eigene		*/
/*	  Routinen fÅr 1 Bit Ausgabe).							*/
/* Version 0.6  --  18.09.98								*/
/*	  Erzeugung von FÅllbytes durch encoded runs mit LÑnge	*/
/*	  v am Zeileende entfernt da durch nachfolgenden		*/
/*	  Escapecode 0 unnîtig (und spart nun Zeilen * 2 Bytes	*/
/*	  im Bild ein).											*/	  
/* Version 0.7  --  17.10.98								*/
/*	  Fehler in Bedienung behoben. Bei Verlassen per Return	*/
/*	  wurde der Dialog zwar geschlossen, die Config aber	*/
/*	  nicht gespeichert.									*/
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
	#include "bmp\en\bmp.rsh"
	#include "bmp\en\bmp.rh"
#else
	#include "bmp\de\bmp.rsh"
	#include "bmp\de\bmp.rh"
#endif


#define LIT		1
#define ENC		2

typedef struct
{
	char comp;		/* komprimiert oder nicht */
	char format;	/* Win 2.x, Win 3.x, OS/2 1.x oder OS/2 2.x */
} CONFIG;

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);
void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);

long writeBMPdata1(char *ziel, char *buffer, unsigned long w, unsigned int height);
long writeBMPdata(char *ziel, char *buffer, unsigned long w, unsigned int height);
long writeBMPdata4(char *ziel, char *buffer, unsigned int width, unsigned int height);
long writeBMPdata24(char *ziel, char *buffer, unsigned int width, unsigned int height);
long encodeBMPdata1(char *ziel, char *buffer, unsigned int w, unsigned int height);
long encodeBMPdata(char *ziel, char *buffer, unsigned int w, unsigned int height, char BitsPerPixel);
long encodeBMPdata4(char *ziel, char *buffer, unsigned int width, unsigned int height);
int write_header(char *ziel, unsigned int width, unsigned int height, CONFIG *config, char BitsPerPixel, char *pal, char flag, long f_len);
void getpix_std_line(void *st_pic, void *buf16, int planes, long planelen, int howmany);

/* Dies bastelt direkt ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"BMP",
						0x0070,
						"Christian Eyrich",
						"BMP", "", "", "", "",
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


MOD_ABILITY  module_ability = {
						1, 4, 8, 24, 0,
						0, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						2					/* More */
						};


/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				BMP Image Format (BMP)				*/
/*		Windows 2.x, Windows 3.x, OS/2 1.x,			*/
/*		OS/2 2.x,									*/
/*		1, 4, 8, 24 Bit, 							*/
/*		unkomprimiert, RLE4 und RLE8)				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel,
		 BitsPerPixel;
	char wt[] = "BMP Exporter";

	static int module_id;
	unsigned int width, height, headsize, Button, pallen, t;

	unsigned long f_len, w, memwidth;

	static WINDOW window;
	static OBJECT *win_form;

	static CONFIG config;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			/* falls Åbergeben, Konfig Åbernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy((char *)&config, (char *)*(long *)&smurf_struct->event_par[0], sizeof(CONFIG));
			else
			{
				config.format = WIN3;
				config.comp = KEINE;
			}

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[BMP_EXPORT];							/* Resourcebaum holen */

			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			smurf_struct->module_mode = M_WAITING;

			break;

		case MMORE:
			/* Ressource aktualisieren */
			win_form[WIN12].ob_state &= ~SELECTED;
			win_form[WIN3].ob_state &= ~SELECTED;
			win_form[OS2_1].ob_state &= ~SELECTED;
			win_form[OS2_2T1].ob_state &= ~SELECTED;
			win_form[OS2_2T2].ob_state &= ~SELECTED;
			win_form[config.format].ob_state |= SELECTED;

			if(config.comp == KEINE)
			{
				win_form[KEINE].ob_state |= SELECTED;
				win_form[RLE].ob_state &= ~SELECTED;
			}
			else
			{
				win_form[KEINE].ob_state &= ~SELECTED;
				win_form[RLE].ob_state |= SELECTED;
			}

			/* Zwangskorrektur weil Win 1.x/2.x und OS/2 1.x keine Kompression unterstÅtzten */
			if(config.format == WIN12 || config.format == OS2_1)
			{
				win_form[KEINE].ob_state |= SELECTED;
				win_form[RLE].ob_state &= ~SELECTED;

				win_form[KEINE].ob_state |= DISABLED;
				win_form[RLE].ob_state |= DISABLED;
			}
			else
				if(config.format == WIN3 || config.format == OS2_2T1 || config.format == OS2_2T2)
				{
					win_form[KEINE].ob_state &= ~DISABLED;
					win_form[RLE].ob_state &= ~DISABLED;
				}

			f_module_window = smurf_struct->services->f_module_window;	/* Windowfunktion */

			redraw_window = smurf_struct->services->redraw_window;		/* Redrawfunktion */
	
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
				config.format = WIN3;
				config.comp = KEINE;
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
				smurf_struct->event_par[2] = sizeof(CONFIG);

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
				if(Button == WIN12 || Button == WIN3 || Button == OS2_1 || Button == OS2_2T1 || Button == OS2_2T2)
					config.format = (char)Button;
				else
					if(Button == KEINE || Button == RLE)
						config.comp = (char)Button;

				/* Zwangskorrektur weil Win 1.x/2.x und OS/2 1.x keine Kompression unterstÅtzten */
				if(config.format == WIN12 || config.format == OS2_1)
				{
					win_form[KEINE].ob_state |= SELECTED;
					win_form[RLE].ob_state &= ~SELECTED;

					win_form[KEINE].ob_state |= DISABLED;
					win_form[RLE].ob_state |= DISABLED;

					redraw_window(&window, NULL, COMP_BOX, 0);
				}
				else
					if(config.format == WIN3 || config.format == OS2_2T1 || config.format == OS2_2T2)
					{
						win_form[KEINE].ob_state &= ~DISABLED;
						win_form[RLE].ob_state &= ~DISABLED;

						redraw_window(&window, NULL, COMP_BOX, 0);
					}

				smurf_struct->module_mode = M_WAITING;
			}

			break;

	/* Keyboardevent */
		case MKEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&config;
				smurf_struct->event_par[2] = sizeof(CONFIG);

				smurf_struct->module_mode = M_MOREOK;
			}
			else
				smurf_struct->module_mode = M_WAITING;

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
	
			buffer = smurf_struct->smurf_pic->pic_data;
	
			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			/* Zwangskorrektur weil Win 1.x/2.x nur 1 Bit beherrscht */
			if(config.format == WIN12 && BitsPerPixel > 1)
				config.format = WIN3;

			/* Zwangskorrektur weil Win 1.x/2.x und OS/2 1.x keine Kompression unterstÅtzten */
			if((config.format == WIN12 || config.format == OS2_1) && config.comp == RLE)
				config.comp = KEINE;

			switch(config.format)
			{
				case WIN12:	headsize = 10;
							break;
				case WIN3:	headsize = 14 + 40;
							break;
				case OS2_1:	headsize = 14 + 12;
							break;
				case OS2_2T1:	headsize = 14 + 64;			/* OS/2 2.x Variante 1 */
								break;
				case OS2_2T2:	headsize = 14 + 14 + 40;	/* OS/2 2.x Variante 2 */
								break;
			}

			if(BitsPerPixel <= 8)
				pallen = 1024;
			else
				pallen = 0;

			switch(BitsPerPixel)
			{
				case 1:	w = (width + 7) / 8;
						break;
				case 4:	w = (width + 1) / 2;
						break;
				case 8: w = width;
						break;
				case 24: w = width * 3L;
						 break;
			}

			memwidth = ((w + 3) / 4) * 4;

			f_len = memwidth * (long)height;
			if(config.comp == RLE)
				f_len += f_len * 10 / 100;				/* plus 10% Sicherheitsreserve */

			if((ziel = (char *)SMalloc(headsize + pallen + f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				f_len = write_header(ziel, width, height, &config, BitsPerPixel, smurf_struct->smurf_pic->palette, 0, 0);

/*				printf("headerlen: %lu\n", f_len); */

				if(config.comp == KEINE)
					switch((int)BitsPerPixel)
					{
						case 1:	f_len += writeBMPdata1(ziel + f_len, buffer, w, height);
								break;
						case 8:	f_len += writeBMPdata(ziel + f_len, buffer, w, height);
								break;
						case 24: f_len += writeBMPdata24(ziel + f_len, buffer, width, height);
								 break;
						case 4:	f_len += writeBMPdata4(ziel + f_len, buffer, width, height);
								break;
					}
				else
					switch((int)BitsPerPixel)
					{
						case 1: f_len += encodeBMPdata1(ziel + f_len, buffer, width, height);
								break;
						case 8:	
						case 24: f_len += encodeBMPdata(ziel + f_len, buffer, width, height, BitsPerPixel);
								 break;
						case 4: f_len += encodeBMPdata4(ziel + f_len, buffer, width, height);
								break;
					}

				/* erst jetzt bekannte Headerwerte nachtragen */
				write_header(ziel, width, height, &config, BitsPerPixel, smurf_struct->smurf_pic->palette, 1, f_len);

/*				printf("f_len: %lu\n", f_len); */

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


/* BMP mit 1 Bit schreiben - muû wegen Invertierung leider in eine eigene Funktion */
long writeBMPdata1(char *ziel, char *buffer, unsigned long w, unsigned int height)
{
	char *oziel,
		 v;

	unsigned int x, y;


	oziel = ziel;

	buffer += (unsigned long)(height - 1) * w;

	v = (char)(((w + 3) / 4) * 4 - w);

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = ~*buffer++;
		} while(++x < w);
		ziel += v;
		buffer -= w + w;
	} while(++y < height);

	return(ziel - oziel);
} /* writeBMPdata1 */


/* BMP mit 8 Bit schreiben */
long writeBMPdata(char *ziel, char *buffer, unsigned long w, unsigned int height)
{
	char *oziel,
		 v;

	unsigned int x, y;


	oziel = ziel;

	buffer += (unsigned long)(height - 1) * w;

	v = (char)(((w + 3) / 4) * 4 - w);

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *buffer++;
		} while(++x < w);
		ziel += v;
		buffer -= w + w;
	} while(++y < height);

	return(ziel - oziel);
} /* writeBMPdata */


/* BMP mit 4 Bit schreiben - muû leider wegen 2 Pixel in ein Byte in eigene Funktion */
long writeBMPdata4(char *ziel, char *buffer, unsigned int width, unsigned int height)
{
	char *oziel, *pixbuf, *line,
		 v;

	unsigned int x, y;

	unsigned long planelength, w, realwidth;


	oziel = ziel;

	realwidth = (width + 7) / 8;
	buffer += (unsigned long)(height - 1) * realwidth;

	w = (width + 1) / 2;
	v = (char)(((w + 3) / 4) * 4 - w);

	planelength = realwidth * (unsigned long)height;

	pixbuf = SMalloc(width + 7);

	y = 0;
	do
	{
		memset(pixbuf, 0x0, width);
		getpix_std_line(buffer, pixbuf, 4, planelength, width);
		line = pixbuf;

		x = 0;
		do
		{
			*ziel = *line++ << 4;
			*ziel++ |= *line++;
		} while(++x < w);
		ziel += v;
		buffer -= realwidth;
	} while(++y < height);

	SMfree(pixbuf);

	return(ziel - oziel);
} /* writeBMPdata4 */


/* BMP mit 24 Bit schreiben - muû leider wegen BGR statt RGB in eigene Funktion */
long writeBMPdata24(char *ziel, char *buffer, unsigned int width, unsigned int height)
{
	char *oziel,
		 v;

	unsigned int x, y;

	unsigned long realwidth;


	oziel = ziel;

	realwidth = width * 3L;
	buffer += (unsigned long)(height - 1) * realwidth;

	v = (char)(((realwidth + 3) / 4) * 4 - realwidth);		/* ZeilenlÑnge auf Vielfaches von 4 */

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *(buffer++ + 2);
			*ziel++ = *buffer++;
			*ziel++ = *(buffer++ - 2);
		} while(++x < width);
		ziel += v;
		buffer -= realwidth + realwidth;
	} while(++y < height);

	return(ziel - oziel);
} /* writeBMPdata24 */


/* BMP mit 1 Bit kodieren - muû wegen Invertierung leider in eine eigene Funktion */
long encodeBMPdata1(char *ziel, char *buffer, unsigned int width, unsigned int height)
{
	char *oziel, *linebuf, *olinebuf,
		 pixel, counter, nextenc;

	unsigned int x, y, xx;

	unsigned long w;


	w = (width + 7) / 8;

	oziel = ziel;

	buffer += (unsigned long)(height - 1) * w;

	linebuf = (char *)SMalloc(w + 1);
	olinebuf = linebuf;

	y = 0;
	do
	{
		linebuf = olinebuf;
		memcpy(linebuf, buffer, w);
		buffer += w;

		x = 0;
		do
		{
			counter = 1;
			pixel = *linebuf++;
			x++;

			xx = x;
			while(pixel == *linebuf && counter < 0xff && xx < w)
			{
				linebuf++;
				xx++;
				counter++;
			}

			if(counter >= 2)						/* es lohnt sich! */
			{
				*ziel++ = counter;
				*ziel++ = ~pixel;
				x = xx;
			}       
			else									/* ... aber leider nicht! */
			{
				*ziel++ = 0;
				ziel++;
				*ziel++ = ~pixel;

				do
				{
					nextenc = 1;

					while(*linebuf != *(linebuf + 1) && counter < 0xff && xx < w)
					{
						*ziel++ = ~*linebuf++;
						xx++;
						counter++;
					}

					while(*linebuf == *(linebuf + 1) && nextenc < 0x5 && counter < 0xff && xx < w)
					{
						*ziel++ = ~*linebuf++;
						xx++;
						counter++;
						nextenc++;
					}
				} while(nextenc < 5 && counter < 0xff && xx < w);

				if(nextenc > 4)
				{
					counter -= nextenc - 1;
					xx -= nextenc - 1;
					linebuf -= nextenc - 1;
					ziel -= nextenc - 1;
				}

				if(counter < 3)
				{
					if(counter == 1)
					{
						ziel -= 3;
						*ziel++ = 1;
						*ziel++ = ~pixel;
					}
					else
					{
						ziel -= 4;
						*ziel++ = 1;
						*ziel++ = ~pixel;
						*ziel++ = 1;
						*ziel++ = ~*(linebuf - 1);
					}
				}
				else
				{
					*(ziel - counter - 1) = counter;
					if(counter&1)				/* wordalignement */
						*ziel++ = 0;
				}

				x = xx;
			}
		} while(x < w);

		/* Ende-der-Zeile-Escape */
		*ziel++ = 0;
		*ziel++ = 0;

		buffer -= w + w;
	} while(++y < height);

	/* Ende-der-Grafik-Escape */
	*ziel++ = 0;
	*ziel++ = 1;

	return(ziel - oziel);
} /* encodeBMPdata1 */


/* BMP mit 8 und 24 Bit kodieren */
long encodeBMPdata(char *ziel, char *buffer, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *oziel, *linebuf, *olinebuf,
		 pixel, counter, nextenc;

	unsigned int x, y, xx;

	unsigned long w;


	switch(BitsPerPixel)
	{
		case 1:	w = (width + 7) / 8;
					break;
		case 8: w = width;
				break;
		case 24: w = width * 3L;
				 break;
	}

	oziel = ziel;

	buffer += (unsigned long)(height - 1) * w;

	linebuf = (char *)SMalloc(w + 1);
	olinebuf = linebuf;

	y = 0;
	do
	{
		linebuf = olinebuf;
		if(BitsPerPixel == 24)
		{
			x = 0;
			do
			{
				*linebuf++ = *(buffer++ + 2);
				*linebuf++ = *buffer++;
				*linebuf++ = *(buffer++ - 2);
			} while(++x < width);
			linebuf = olinebuf;
		}
		else
		{
			memcpy(linebuf, buffer, w);
			buffer += w;
		}

		x = 0;
		do
		{
			counter = 1;
			pixel = *linebuf++;
			x++;

			xx = x;
			while(pixel == *linebuf && counter < 0xff && xx < w)
			{
				linebuf++;
				xx++;
				counter++;
			}

			if(counter >= 2)						/* es lohnt sich! */
			{
				*ziel++ = counter;
				*ziel++ = pixel;
				x = xx;
			}       
			else									/* ... aber leider nicht! */
			{
				*ziel++ = 0;
				ziel++;
				*ziel++ = pixel;

				do
				{
					nextenc = 1;

					while(*linebuf != *(linebuf + 1) && counter < 0xff && xx < w)
					{
						*ziel++ = *linebuf++;
						xx++;
						counter++;
					}

					while(*linebuf == *(linebuf + 1) && nextenc < 0x5 && counter < 0xff && xx < w)
					{
						*ziel++ = *linebuf++;
						xx++;
						counter++;
						nextenc++;
					}
				} while(nextenc < 5 && counter < 0xff && xx < w);

				if(nextenc > 4)
				{
					counter -= nextenc - 1;
					xx -= nextenc - 1;
					linebuf -= nextenc - 1;
					ziel -= nextenc - 1;
				}

				if(counter < 3)
				{
					if(counter == 1)
					{
						ziel -= 3;
						*ziel++ = 1;
						*ziel++ = pixel;
					}
					else
					{
						ziel -= 4;
						*ziel++ = 1;
						*ziel++ = pixel;
						*ziel++ = 1;
						*ziel++ = *(linebuf - 1);
					}
				}
				else
				{
					*(ziel - counter - 1) = counter;
					if(counter&1)				/* wordalignement */
						*ziel++ = 0;
				}

				x = xx;
			}
		} while(x < w);

		/* Ende-der-Zeile-Escape */
		*ziel++ = 0;
		*ziel++ = 0;

		buffer -= w + w;
	} while(++y < height);

	/* Ende-der-Grafik-Escape */
	*ziel++ = 0;
	*ziel++ = 1;

	return(ziel - oziel);
} /* encodeBMPdata */


/* BMP mit 4 Bit kodieren - muû leider wegen 2 Pixel in ein Byte in eigene Funktion */
long encodeBMPdata4(char *ziel, char *buffer, unsigned int width, unsigned int height)
{
	char *oziel, *linebuf, *olinebuf, *pixbuf, *line,
		 pixel, counter, nextenc;

	unsigned int x, y, xx;

	unsigned long planelength, w, realwidth;


	oziel = ziel;

	realwidth = (width + 7) / 8;
	buffer += (unsigned long)(height - 1) * realwidth;

	w = (width + 1) / 2;

	linebuf = (char *)SMalloc(w + 1);
	olinebuf = linebuf;

	planelength = realwidth * (unsigned long)height;

	pixbuf = SMalloc(width + 7);

	y = 0;
	do
	{
		linebuf = olinebuf;
		memset(pixbuf, 0x0, width);
		getpix_std_line(buffer, pixbuf, 4, planelength, width);
		line = pixbuf;

		x = 0;
		do
		{
			*linebuf = *line++ << 4;
			*linebuf++ |= *line++;
		} while(++x < w);
		linebuf = olinebuf;

		x = 0;
		do
		{
			counter = 1;
			pixel = *linebuf++;
			x++;

			xx = x;
			while(pixel == *linebuf && counter < 0x7f && xx < w)
			{
				linebuf++;
				xx++;
				counter++;
			}

			if(counter >= 2)						/* es lohnt sich! */
			{
				*ziel++ = counter << 1;
				*ziel++ = pixel;
				x = xx;
			}       
			else									/* ... aber leider nicht! */
			{
				*ziel++ = 0;
				ziel++;
				*ziel++ = pixel;

				do
				{
					nextenc = 1;

					while(*linebuf != *(linebuf + 1) && counter < 0x7f && xx < w)
					{
						*ziel++ = *linebuf++;
						xx++;
						counter++;
					}

					while(*linebuf == *(linebuf + 1) && nextenc < 0x5 && counter < 0x7f && xx < w)
					{
						*ziel++ = *linebuf++;
						xx++;
						counter++;
						nextenc++;
					}
				} while(nextenc < 5 && counter < 0x7f && xx < w);

				if(nextenc > 4)
				{
					linebuf -= nextenc - 1;
					ziel -= nextenc - 1;
					xx -= nextenc - 1;
					counter -= nextenc - 1;
				}

				if(counter < 3)
				{
					if(counter == 1)
					{
						ziel -= 3;
						*ziel++ = 2;
						*ziel++ = pixel;
					}
					else
					{
						ziel -= 4;
						*ziel++ = 2;
						*ziel++ = pixel;
						*ziel++ = 2;
						*ziel++ = *(linebuf - 1);
					}
				}
				else
				{
					*(ziel - counter - 1) = counter << 1;
					if(counter&1)				/* wordalignement */
						*ziel++ = 0;
				}

				x = xx;
			}
		} while(x < w);
		/* Ende-der-Zeile-Escape */
		*ziel++ = 0;
		*ziel++ = 0;

		buffer -= realwidth;
	} while(++y < height);

	/* Ende-der-Grafik-Escape */
	*ziel++ = 0;
	*ziel++ = 1;

	SMfree(pixbuf);

	return(ziel - oziel);
} /* encodeBMPdata4 */


/* BMP-Headerstruktur schreiben */
int write_header(char *ziel, unsigned int width, unsigned int height, CONFIG *config, char BitsPerPixel, char *pal, char flag, long f_len)
{
	char *oziel, *ppal, 
		 comp;

	int i, cols, DatenOffset;


	oziel = ziel;

	if(config->comp == RLE)
		if(BitsPerPixel == 4)
			comp = 2;			/* RLE4 */
		else
			comp = 1;			/* RLE8 */
	else
		comp = 0;				/* keine */

	cols = 1 << BitsPerPixel;

	switch(config->format)
	{
		case WIN12:	*(unsigned int *)ziel = swap_word(0x0);
					*(unsigned int *)(ziel + 0x02) = swap_word(width);
					*(unsigned int *)(ziel + 0x04) = swap_word(height);
					*(ziel + 0x08) = 1;
					*(ziel + 0x09) = BitsPerPixel;
/*					printf("Win 1.x/2.x\n"); */
					ziel += 0x0a;
					break;
		case WIN3:	*(unsigned int *)ziel = 0x424d;
					if(flag)
					{
						*(unsigned int *)(ziel + 0x02) = swap_word((unsigned int)(f_len & 0xffff));
						*(unsigned int *)(ziel + 0x04) = swap_word((unsigned int)(f_len >> 16));
					}
					/* Reserved1 (ab 0x06) und Reserved2 (ab 0x08) gleich 0 */
					DatenOffset = 0x0e + 0x28 + cols * 4;
					*(unsigned int *)(ziel + 0x0a) = swap_word(DatenOffset & 0xffff);
					*(unsigned int *)(ziel + 0x0c) = swap_word(DatenOffset >> 16);

					ziel += 0x0e;
					*(unsigned int *)ziel = swap_word(0x28 & 0xffff);
					*(unsigned int *)(ziel + 0x02) = swap_word(0x28 >> 16);
					*(unsigned int *)(ziel + 0x04) = swap_word(width);
					*(unsigned int *)(ziel + 0x08) = swap_word(height);
					*(unsigned int *)(ziel + 0x0c) = swap_word(1);
					*(unsigned int *)(ziel + 0x0e) = swap_word(BitsPerPixel);
					*(unsigned int *)(ziel + 0x10) = swap_word(comp & 0xffff);
					*(unsigned int *)(ziel + 0x12) = swap_word(comp >> 16);
					if(flag)
					{
						*(unsigned int *)(ziel + 0x14) = swap_word((unsigned int)((f_len - DatenOffset) & 0xffff));
						*(unsigned int *)(ziel + 0x16) = swap_word((unsigned int)((f_len - DatenOffset) >> 16));
					}
					/* horizontale (ab 0x18) und vertikale (ab 0x1c) Auflîsung gleich 0 */
					*(unsigned int *)(ziel + 0x20) = swap_word(cols & 0xffff);	/* colors used */
					*(unsigned int *)(ziel + 0x22) = swap_word(cols >> 16);
					*(unsigned long *)(ziel + 0x24) = 0L;						/* significant colors */
					ziel += 0x28;
/*					printf("Win 3.x\n"); */
					break;
		case OS2_1:	*(unsigned int *)ziel = 0x424d;
					if(flag)
					{
						*(unsigned int *)(ziel + 0x02) = swap_word((unsigned int)(f_len & 0xffff));
						*(unsigned int *)(ziel + 0x04) = swap_word((unsigned int)(f_len >> 16));
					}
					/* Reserved1 (ab 0x06) und Reserved2 (ab 0x08) gleich 0 */
					DatenOffset = 0x0e + 0x0c + cols * 3;
					*(unsigned int *)(ziel + 0x0a) = swap_word(DatenOffset & 0xffff);
					*(unsigned int *)(ziel + 0x0c) = swap_word(DatenOffset >> 16);

					ziel += 0x0e;
					*(unsigned int *)ziel = swap_word(0x0c & 0xffff);
					*(unsigned int *)(ziel + 0x02) = swap_word(0x0c >> 16);
					*(unsigned int *)(ziel + 0x04) = swap_word(width);
					*(unsigned int *)(ziel + 0x06) = swap_word(height);
					*(unsigned int *)(ziel + 0x08) = swap_word(1);
					*(unsigned int *)(ziel + 0x0a) = swap_word(BitsPerPixel);
					ziel += 0x0c;
/*					printf("OS/2 1.x\n"); */
					break;
		case OS2_2T1:	*(unsigned int *)ziel = 0x424d;
						if(flag)
						{
							*(unsigned int *)(ziel + 0x02) = swap_word((unsigned int)(f_len & 0xffff));
							*(unsigned int *)(ziel + 0x04) = swap_word((unsigned int)(f_len >> 16));
						}
						/* Reserved1 (ab 0x06) und Reserved2 (ab 0x08) gleich 0 */
						DatenOffset = 0x0e + 0x40 + cols * 4;
						*(unsigned int *)(ziel + 0x0a) = swap_word(DatenOffset & 0xffff);
						*(unsigned int *)(ziel + 0x0c) = swap_word(DatenOffset >> 16);

						ziel += 0x0e;
						*(unsigned int *)ziel = swap_word(0x40 & 0xffff);
						*(unsigned int *)(ziel + 0x02) = swap_word(0x40 >> 16);
						*(unsigned int *)(ziel + 0x04) = swap_word(width);
						*(unsigned int *)(ziel + 0x08) = swap_word(height);
						*(unsigned int *)(ziel + 0x0c) = swap_word(1);
						*(unsigned int *)(ziel + 0x0e) = swap_word(BitsPerPixel);
						*(unsigned int *)(ziel + 0x10) = swap_word(comp & 0xffff);
						*(unsigned int *)(ziel + 0x12) = swap_word(comp >> 16);
						if(flag)
						{
							*(unsigned int *)(ziel + 0x14) = swap_word((unsigned int)((f_len - DatenOffset) & 0xffff));
							*(unsigned int *)(ziel + 0x16) = swap_word((unsigned int)((f_len - DatenOffset) >> 16));
						}
						/* horizontale (ab 0x18) und vertikale (ab 0x1c) Auflîsung gleich 0 */
						*(unsigned int *)(ziel + 0x20) = swap_word(cols & 0xffff);
						*(unsigned int *)(ziel + 0x22) = swap_word(cols >> 16);
						*(unsigned int *)(ziel + 0x24) = swap_word(cols & 0xffff);
						*(unsigned int *)(ziel + 0x26) = swap_word(cols >> 16);
						/* Einheiten fÅr die Auflîsung (ab 0x28) gleich 0 */
						/* Reserviert (ab 0x2a) gleich 0 */
						*(unsigned int *)(ziel + 0x2c) = swap_word(0);	/* steht auf dem Kopf */
						/* Rendering (ab 0x2e), Size1 (ab 0x30) und Size2 (ab 0x34) gleich 0 */
						/* ColorEncoding (ab 0x38) gleich 0 */
						/* Identifier (ab 0x3c) gleich 0 */
						ziel += 0x40;
/*						printf("OS/2 2.x Typ 1\n"); */
						break;
		case OS2_2T2:	*(unsigned int *)ziel = 0x4241;
						*(unsigned int *)(ziel + 0x02) = swap_word(0x0e & 0xffff);
						*(unsigned int *)(ziel + 0x04) = swap_word(0x0e >> 16);
						/* Offset des nÑchsten BAFH (ab 0x06) gleich 0 */
						*(unsigned int *)(ziel + 0x0a) = swap_word(72 & 0xffff);
						*(unsigned int *)(ziel + 0x0c) = swap_word(72 >> 16);

						ziel += 0x0e;
						*(unsigned int *)ziel = 0x424d;
						if(flag)
						{
							*(unsigned int *)(ziel + 0x02) = swap_word((unsigned int)(f_len & 0xffff));
							*(unsigned int *)(ziel + 0x04) = swap_word((unsigned int)(f_len >> 16));
						}
						/* Reserved1 (ab 0x06) und Reserved2 (ab 0x08) gleich 0 */
						DatenOffset = 0x0e + 0x0e + 0x28 + cols * 4;
						*(unsigned int *)(ziel + 0x0a) = swap_word(DatenOffset & 0xffff);
						*(unsigned int *)(ziel + 0x0c) = swap_word(DatenOffset >> 16);

						ziel += 0x0e;
						*(unsigned int *)ziel = swap_word(0x28 & 0xffff);
						*(unsigned int *)(ziel + 0x02) = swap_word(0x28 >> 16);
						*(unsigned int *)(ziel + 0x04) = swap_word(width);
						*(unsigned int *)(ziel + 0x08) = swap_word(height);
						*(unsigned int *)(ziel + 0x0c) = swap_word(1);
						*(unsigned int *)(ziel + 0x0e) = swap_word(BitsPerPixel);
						*(unsigned int *)(ziel + 0x10) = swap_word(comp & 0xffff);
						*(unsigned int *)(ziel + 0x12) = swap_word(comp >> 16);
						if(flag)
						{
							*(unsigned int *)(ziel + 0x14) = swap_word((unsigned int)((f_len - DatenOffset) & 0xffff));
							*(unsigned int *)(ziel + 0x16) = swap_word((unsigned int)((f_len - DatenOffset) >> 16));
						}
						/* horizontale (ab 0x18) und vertikale (ab 0x1c) Auflîsung gleich 0 */
						*(unsigned int *)(ziel + 0x20) = swap_word(cols & 0xffff);
						*(unsigned int *)(ziel + 0x22) = swap_word(cols >> 16);
						*(unsigned int *)(ziel + 0x24) = swap_word(cols & 0xffff);
						*(unsigned int *)(ziel + 0x26) = swap_word(cols >> 16);
						ziel += 0x28;
/*						printf("OS/2 2.x Typ 2\n"); */
						break;
	}

	ppal = ziel;

	/* öbertragen der Palette */
	if(BitsPerPixel < 16 && config->format != WIN12)
	{
		if(BitsPerPixel == 1)
		{
			*ppal++ = 0;
			*ppal++ = 0;
			*ppal++ = 0;
			if(config->format != OS2_1)
				ppal++;
			*ppal++ = 255;
			*ppal++ = 255;
			*ppal++ = 255;
			if(config->format != OS2_1)
				ppal++;
		}
		else
			for(i = 0; i < cols; i++)
			{
				*ppal++ = *(pal++ + 2);
				*ppal++ = *pal++;
				*ppal++ = *(pal++ - 2);
				if(config->format != OS2_1)
					ppal++;
			}
	}

	return((int)(ppal - oziel));
} /* write_header */