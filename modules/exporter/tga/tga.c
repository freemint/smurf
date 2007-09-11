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
/*						TGA-Format Exporter					*/
/*	Version 0.1 - xx.xx.96									*/
/*	  unkomprimierte TGAs									*/
/*	Version 0.2 - xx.xx.96									*/
/*	  nun auch komprimierte TGAs							*/
/*	Version 0.3 - 09.07.96									*/
/*	  Von Dale Åbernommen und Åberarbeitet.					*/
/*	  Komprimierte TGAs kînnen nun auch von anderen			*/
/*	  Programmen gelesen werden.							*/
/*	Version 0.4 - 12.07.96									*/
/*	  Verbesserte RLE-Komprimierung - unkomprimierbare		*/
/*	  Folgen Åber mehrere Pixel werden zusammengefasst.		*/
/*	Version 0.5 - 03.05.97									*/
/*	  Nochmal umgestrickt und auf meine Programmiernorm		*/
/*	  umgestellt. Unterroutinen in Funktionen gepackt.		*/
/*	  Eigenes Konfigurationsformular eingebaut.				*/
/* Version 0.6  --  12.12.97								*/
/*	  Monochrome TGA werden von anderen Programmen nun		*/
/*	  nicht mehr invertiert angezeigt.						*/
/* Version 0.7  --  17.10.98								*/
/*	  Fehler in Bedienung behoben. Bei Verlassen per Return	*/
/*	  wurde der Dialog zwar geschlossen, die Config aber	*/
/*	  nicht gespeichert.									*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <..\..\..\demolib.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

#define ENGLISCH 0

#if ENGLISCH
	#include "tga\en\tga.rsh"
	#include "tga\en\tga.rh"
#else
	#include "tga\de\tga.rsh"
	#include "tga\de\tga.rh"
#endif


void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);

void write_header(char *ziel, char *comment, unsigned int width, unsigned int height, char comp, char BitsPerPixel);
unsigned long write_1Bit(char *buffer, char *ziel, unsigned int width, unsigned int height);
unsigned long write_8Bit(char *buffer, char *ziel, unsigned int width, unsigned int height);
unsigned long write_16Bit(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height);
unsigned long write_24Bit(char *buffer, char *ziel, unsigned int width, unsigned int height);
unsigned long encode_1Bit(char *buffer, char *ziel, unsigned int width, unsigned int height);
unsigned long encode_8Bit(char *buffer, char *ziel, unsigned int width, unsigned int height);
unsigned long encode_16Bit(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height);
unsigned long encode_24Bit(char *buffer, char *ziel, unsigned int width, unsigned int height);


/* Dies bastelt direct ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/*	Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"TGA Bitmap",
						0x0070,
						"Christian Eyrich, Dale Russell",
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


MOD_ABILITY module_ability = {
						1, 8, 16, 24, 0,
						0, 0, 0,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						6					/* More und Infotext */
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				Targa Bitmap - (TGA)				*/
/*		1, 8 und 24 Bit, unkomprimiert und RLE		*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel, *pal, *ppal, *comment,
		 BitsPerPixel, t;
	char wt[] = "TGA Exporter";
	static char comp;

	static int module_id;
	int headlen, palcols;
	unsigned int width, height, w, Button;

	unsigned long backlen, len;

	static WINDOW window;
	static OBJECT *win_form;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			/* falls Åbergeben, Konfig Åbernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy(&comp, (char *)*(long *)&smurf_struct->event_par[0], 1);
			else
				comp = RLE;

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[TGA_EXPORT];							/* Resourcebaum holen */

			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			smurf_struct->module_mode = M_WAITING;

			break;

		case MMORE:
			/* Ressource aktualisieren */
			if(comp == KEINE)
			{
				win_form[KEINE].ob_state |= SELECTED;
				win_form[RLE].ob_state &= ~SELECTED;
			}
			else
			{
				win_form[KEINE].ob_state &= ~SELECTED;
				win_form[RLE].ob_state |= SELECTED;
			}

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
				memcpy(&comp, (char *)*(long *)&smurf_struct->event_par[0], 1);
			else
				comp = RLE;

			smurf_struct->module_mode = M_WAITING;

			break;

/* Buttonevent */
		case MBEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&comp;
				smurf_struct->event_par[2] = 1;

				smurf_struct->module_mode = M_MOREOK;
			}
			else
			if(Button == SAVE)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&comp;
				smurf_struct->event_par[2] = 1;

				smurf_struct->module_mode = M_CONFSAVE;
			}
			else
			{
				if(Button == KEINE || Button == RLE)
					comp = (char)Button;

				smurf_struct->module_mode = M_WAITING;
			}

			break;

	/* Keyboardevent */
		case MKEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&comp;
				smurf_struct->event_par[2] = 1;

				smurf_struct->module_mode = M_WAITING;
			}
			else
				smurf_struct->module_mode = M_MOREOK;

			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

		/* Vielleicht ist noch eine 256 Byte-Tabelle nîtig mit der monochrome
		   Bilder invertiert werden kînnen. Die bisher abgespeicherten werden
		   nÑmlich invertiert angezeigt. */

		/* Zeiger initialisieren */
			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));
			buffer = smurf_struct->smurf_pic->pic_data;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			switch(BitsPerPixel)
			{
				case 1: w = (width + 15) / 16;
						len = (long)w * 2L * (long)height;
						break;
				case 4: w = (width + 3) / 4;
						len = (long)w * 2L * (long)height;
						break;
				default: len = (long)(((long)width * (long)height * (long)BitsPerPixel) >> 3L);
						 break;
			}

			if(BitsPerPixel == 4 || BitsPerPixel == 8)
				palcols = 0x01 << BitsPerPixel;
			else
				palcols = 0;

		/* Hier wird die LÑnge des Kommentars ermittelt */
		/* Der Kommentar sollte lt. Buch nullterminiert sein! */
			comment = smurf_struct->smurf_pic->infotext;
		/* HeaderlÑnge (0x12) + KommentarlÑnge */
			headlen = 0x12 + (int)strlen(comment) + 1;
		/* len: BildlÑnge, palcols: Anzahl Palettenfarben */
			if((ziel = (char *)SMalloc(((long)headlen + len + (long)palcols * 3) + 16384)) == NULL)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(FALSE);
			}

			write_header(ziel, comment, width, height, comp, BitsPerPixel);

			if(BitsPerPixel > 1 && BitsPerPixel < 16)
			{
				pal = smurf_struct->smurf_pic->palette;
				ppal = ziel + headlen;
				for(palcols = 0; palcols < 256; palcols++)
				{
					*ppal++ = *(pal++ + 2);		/* B */
					*ppal++ = *pal++;			/* G */
					*ppal++ = *(pal++ - 2);		/* R */
				}
		
				headlen += 768;
			}

			if(comp == KEINE)			/* unkomprimiert */
			{
				if(BitsPerPixel == 1)
					write_1Bit(buffer, ziel + headlen, width, height);
				else
					if(BitsPerPixel == 8)
						write_8Bit(buffer, ziel + headlen, width, height);
					else
						if(BitsPerPixel == 16)
							write_16Bit((unsigned int *)buffer, (unsigned int *)(ziel + headlen), width, height);
						else
							if(BitsPerPixel == 24)
								write_24Bit(buffer, ziel + headlen, width, height);

				backlen = len;
			}
			else						/* komprimiert */
				if(BitsPerPixel == 1)
					backlen = encode_1Bit(buffer, ziel + headlen, width, height);
				else
					if(BitsPerPixel == 8)
						backlen = encode_8Bit(buffer, ziel + headlen, width, height);
					else
						if(BitsPerPixel == 16)
							backlen = encode_16Bit((unsigned int *)buffer, (unsigned int *)(ziel + headlen), width, height);
						else
							if(BitsPerPixel == 24)
								backlen = encode_24Bit(buffer, ziel + headlen, width, height);

			exp_pic->pic_data = ziel;
			exp_pic->f_len = backlen + headlen;
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



/* TGA-Headerstruktur schreiben */
void write_header(char *ziel, char *comment, unsigned int width, unsigned int height, char comp, char BitsPerPixel)
{
	ziel[0] = strlen(comment) + 1;		/* LÑnge des Kommentarfeldes */
	ziel[2] = 0x0;

	if(comp == RLE)
		ziel[2] |= 0x08;				/* Komprimierung */

	if(BitsPerPixel == 4 || BitsPerPixel == 8)
	{
		ziel[1] = 1;					/* mit Farbpalette */
		ziel[2] |= 0x01;				/* Palettenbild */
		ziel[3] = 0;					/* Index des 1. Paletteneintr. */
		ziel[4] = 0;					/* im Intelformat */

		if(BitsPerPixel == 4)
		{
			ziel[5] = 0x30;				/* Anzahl PaletteneintrÑge */
			ziel[6] = 0x00;				/* im Intel-Format; 48 */
		}
		else
		{
			ziel[5] = 0x00;				/* Anzahl PaletteneintrÑge */
			ziel[6] = 0x01;				/* im Intel-Format; 256 */
		}

		ziel[7] = 24;					/* Bits pro Paletteneintrag */
	}
	else
	{
		ziel[1] = 0;					/* ohne Farbpalette */

		if(BitsPerPixel == 1)
			ziel[2] |= 0x03;			/* s/w-Bild */
		else
			ziel[2] |= 0x02;			/* RGB-Bild */

		ziel[3] = 0;					/* Index des 1. Paletteneintr. */
		ziel[4] = 0;					/* im Intelformat */
		ziel[5] = 0;					/* Anzahl PaletteneintrÑge */
		ziel[6] = 0;					/* im Intel-Format */
		ziel[7] = 0;					/* Bits pro Paletteneintrag */
	}

	ziel[8] = 0;						/* x-pos im */
	ziel[9] = 0;						/* Intel-Format */
	ziel[0x0a] = 0;						/* y-pos im*/
	ziel[0x0b] = 0;						/* Intel-Format */
	ziel[0x0c] = (width&0x00ff);		/* width im */
	ziel[0x0d] = (width&0xff00) >> 8;	/* Intel-Format */
	ziel[0x0e] = (height&0x00ff);		/* height im */
	ziel[0x0f] = (height&0xff00) >> 8;	/* Intel-Format */
	ziel[0x10] = BitsPerPixel;			/* Farbtiefe */
	
	ziel[0x11] = 0x20;					/* Bildattribute: Aufbau */
										/* von oben nach unten */
	strcpy(ziel + 0x12, comment);		/* Kommentar */

	return;
} /* write_header */


/* leider wird eine eigene Funktion benîtigt weil monochrome Bilder */
/* invertiert werden mÅssen */
unsigned long write_1Bit(char *buffer, char *ziel, unsigned int width, unsigned int height)
{
	char v;

	unsigned int x, y;

	unsigned long w;


	w = (width + 7) / 8;
	v = (char)(((width + 15) / 16) * 2 - w);

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = ~*buffer++;
		} while(++x < w);

		if(v)
			*ziel++ = 0xff;
	} while(++y < height);

	return(0);
} /* write_1Bit */


unsigned long write_8Bit(char *buffer, char *ziel, unsigned int width, unsigned int height)
{
	unsigned int x, y;


	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *buffer++;
		} while(++x < width);
	} while(++y < height);

	return(0);
} /* write_8Bit */


unsigned long write_16Bit(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height)
{
	unsigned int x, y, pixel16;


/* wie schnell sind wir? */
/*	init_timer(); */

	buffer = (unsigned int *)buffer;
	y = 0;
	do
	{
		x = 0;
		do
		{
			pixel16 = *buffer++;
		/* Vorsicht: rrrrrggg|ggabbbbb nach arrrrrgg|gggbbbbb und dann noch nach Intel ... */
			*ziel++ = swap_word(((pixel16&0xffc0) >> 1) | (pixel16&0x001f));
		} while(++x < width);
	} while(++y < height);

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */

	return(0);
} /* write_16Bit */


unsigned long write_24Bit(char *buffer, char *ziel, unsigned int width, unsigned int height)
{
	unsigned int x, y;


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
	} while(++y < height);

	return(0);
} /* write_24Bit */


/* leider wird eine eigene Funktion benîtigt weil monochrome Bilder */
/* invertiert werden mÅssen */
unsigned long encode_1Bit(char *buffer, char *ziel, unsigned int width, unsigned int height)
{
	char pixel, v;

	unsigned int x, xx, y, counter;

	unsigned long w, dlen;


	w = (width + 7) / 8;
	v = (char)(((width + 15) / 16) * 2 - w);

	dlen = 0L;

	y = 0;
	do
	{
		x = 0;
		do
		{
			counter = 0x80;
			pixel = *buffer++;
			x++;

			xx = x;
			while(*buffer == pixel && counter < 0xff && xx < w)
			{
				buffer++;
				xx++;
				counter++;
			}

			if(counter > 0x80)			/* es lohnt sich! */
			{
				*ziel++ = counter;
				*ziel++ = ~pixel;
				x = xx;
				dlen += 2;  
			}       
			else						/* ... aber leider nicht! */
			{
				ziel++;
				*ziel++ = ~pixel;
				counter = 1;
				while(*buffer != *(buffer + 1) && counter < 0x80 && xx < w)
				{
					*ziel++ = ~*buffer++;
					xx++;
					counter++;
				}
				*(ziel - counter - 1) = counter - 1;
				x = xx;
				dlen += counter + 1;
			}
		} while(x < w);
	/* Primitivlîsung, einfach mal unkomprimiert hinhÑngen */
		if(v)
		{
			*ziel++ = 0;
			*ziel++ = 0xff;
			dlen += 2;
		}
	} while(++y < height);

	return(dlen);
} /* encode_1Bit */


unsigned long encode_8Bit(char *buffer, char *ziel, unsigned int width, unsigned int height)
{
	char pixel;

	unsigned int x, xx, y, counter;

	unsigned long dlen;


	dlen = 0L;

	y = 0;
	do
	{
		x = 0;
		do
		{
			counter = 0x80;
			pixel = *buffer++;
			x++;

			xx = x;
			while(*buffer == pixel && counter < 0xff && xx < width)
			{
				buffer++;
				xx++;
				counter++;
			}

			if(counter > 0x80)			/* es lohnt sich! */
			{
				*ziel++ = counter;
				*ziel++ = pixel;
				x = xx;
				dlen += 2;  
			}       
			else						/* ... aber leider nicht! */
			{
				ziel++;
				*ziel++ = pixel;
				counter = 1;
				while(*buffer != *(buffer + 1) && counter < 0x80 && xx < width)
				{
					*ziel++ = *buffer++;
					xx++;
					counter++;
				}
				*(ziel - counter - 1) = counter - 1;
				x = xx;
				dlen += counter + 1;
			}
		} while(x < width);
	} while(++y < height);

	return(dlen);
} /* encode_8Bit */


unsigned long encode_16Bit(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height)
{
	unsigned int pixel16;

	unsigned int x, xx, y, counter;

	unsigned long dlen;


	buffer = (unsigned int *)buffer;
	dlen = 0L;

	y = 0;
	do
	{
		x = 0;
		do
		{
			counter = 0x80;
			pixel16 = *buffer++;
			x++;

			xx = x;
			while(*buffer == pixel16 && counter < 0xff && xx < width)
			{
				buffer++;
				xx++;
				counter++;
			}

			if(counter > 0x80)			/* es lohnt sich! */
			{
				*((char *)ziel)++ = counter;
			/* Vorsicht: rrrrrggg|ggabbbbb nach arrrrrgg|gggbbbbb und dann noch nach Intel ... */
				*ziel++ = swap_word(((pixel16&0xffc0) >> 1) | (pixel16&0x001f));
				x = xx;
				dlen += 3;
			}   
			else						/* ... aber leider nicht! */
			{
				((char *)ziel)++;
			/* Vorsicht: rrrrrggg|ggabbbbb nach arrrrrgg|gggbbbbb und dann noch nach Intel ... */
				*ziel++ = swap_word(((pixel16&0xffc0) >> 1) | (pixel16&0x001f));
				counter = 1;
				while(*buffer != *(buffer + 1) && counter < 0x80 && xx < width)
				{
				/* Vorsicht: rrrrrggg|ggabbbbb nach arrrrrgg|gggbbbbb und dann noch nach Intel ... */
					*ziel++ = swap_word(((*buffer&0xffc0) >> 1) | (*buffer++&0x001f));
					xx++;
					counter++;
				}
				*(((char *)ziel) - (counter * 2) - 1) = counter - 1;
				x = xx;
				dlen += (counter * 2) + 1;
			}
		} while(x < width);
	} while(++y < height);

	return(dlen);
} /* encode_16Bit */


unsigned long encode_24Bit(char *buffer, char *ziel, unsigned int width, unsigned int height)
{
	char pixel[3];

	unsigned int x, xx, y, counter;

	unsigned long dlen;


	dlen = 0L;

	y = 0;
	do
	{
		x = 0;
		do
		{
			counter = 0x80;
			pixel[0] = *buffer++;
			pixel[1] = *buffer++;
			pixel[2] = *buffer++;
			x++;

			xx = x;
			while(counter < 0xff && xx < width)
			{
				if(*buffer != pixel[0] || *(buffer + 1) != pixel[1] || *(buffer + 2) != pixel[2])
					break;
				else
				{
					buffer += 3;
					xx++;
					counter++;
				}
			}

			if(counter > 0x80)			/* es lohnt sich! */
			{
				*ziel++ = counter;
				*ziel++ = pixel[2];
				*ziel++ = pixel[1];
				*ziel++ = pixel[0];
				x = xx;
				dlen += 4;
			}
			else						/* ... aber leider nicht! */
			{
				ziel++;
				*ziel++ = pixel[2];
				*ziel++ = pixel[1];
				*ziel++ = pixel[0];
				counter = 1;
				while(counter < 0x80 && xx < width)
				{
					if(*buffer != *(buffer + 3) || *(buffer + 1) != *(buffer + 4) || *(buffer + 2) != *(buffer + 5))
					{
						*ziel++ = *(buffer++ + 2);
						*ziel++ = *buffer++;
						*ziel++ = *(buffer++ - 2);
						xx++;
						counter++;
					}
					else
						break;
				}
				*(ziel - (counter * 3) - 1) = counter - 1;
				x = xx;
				dlen += (counter * 3) + 1;
			}
		} while(x < width);
	} while(++y < height);

	return(dlen);
} /* encode_24Bit */