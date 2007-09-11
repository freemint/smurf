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
/*			Portable Bitmap Utilities Formats Encoder		*/
/* Version 0.1  --  01.02.97								*/
/*	  1, 8 und 24 Bit binÑr									*/
/* Version 0.2  --  22.03.97								*/
/*	  24 Bit ASCII											*/
/* Version 0.3  --  26.03.97								*/
/*	  1, 8 Bit ASCII										*/
/* Version 0.4  --  05.04.97								*/
/*	  Eigenes Konfigurationsformular eingebaut.				*/
/* Version 0.5  --  27.04.97								*/
/*	  Konfig kann Åbermittelt und empfangen werden.			*/
/* Version 0.6  --  17.10.98								*/
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
	#include "ppm\en\ppm.rsh"
	#include "ppm\en\ppm.rh"
#else
	#include "ppm\de\ppm.rsh"
	#include "ppm\de\ppm.rh"
#endif


void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);

void write_header(char mode, unsigned int width, unsigned int height, unsigned long *f_len, char BitsPerPixel);
void write_ASCIIPic(char *buffer, unsigned int width, unsigned int height, unsigned long w, char BitsPerPixel);
void write_BINPic(char *buffer, unsigned int height, unsigned long w);
void putval(char val);
void putval_1Bit(char val);


/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"PBM Format",
						0x0060,
						"Christian Eyrich",
						"PPM", "PGM", "PBM", "", "",
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
						1, 8, 24, 0, 0,
						0, 0, 0,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						2				/* More */
						};


char *ziel, *oziel;

unsigned int x, width;

unsigned long in;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*			PBMPlus Picture Format (PPM)			*/
/*		1, 8, 24 Bit, unkomprimiert,				*/
/*		Speicherform binÑr und ASCII				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer,
		 BitsPerPixel, zeichen, t;
	char wt[] = "PBMPlus Exporter";
	static char mode;

	static int module_id;
	unsigned int height, headsize, ret, Button;

	unsigned long w, memwidth, f_len;

	static WINDOW window;
	static OBJECT *win_form;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			/* falls Åbergeben, Konfig Åbernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy(&mode, (char *)*(long *)&smurf_struct->event_par[0], 1);
			else
				mode = BIN;

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[PPM_EXPORT];							/* Resourcebaum holen */

			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			smurf_struct->module_mode = M_WAITING;

			break;

		case MMORE:
			/* Ressource aktualisieren */
			if(mode == BIN)
			{
				win_form[BIN].ob_state |= SELECTED;
				win_form[ASCII].ob_state &= ~SELECTED;
			}
			else
			{
				win_form[BIN].ob_state &= ~SELECTED;
				win_form[ASCII].ob_state |= SELECTED;
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
				memcpy(&mode, (char *)*(long *)&smurf_struct->event_par[0], 1);
			else
				mode = BIN;

			smurf_struct->module_mode = M_WAITING;

			break;

	/* Buttonevent */
		case MBEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&mode;
				smurf_struct->event_par[2] = 1;

				smurf_struct->module_mode = M_MOREOK;
			}
			else
			if(Button == SAVE)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&mode;
				smurf_struct->event_par[2] = 1;

				smurf_struct->module_mode = M_CONFSAVE;
			}
			else
			{
				if(Button == BIN || Button == ASCII)
					mode = (char)Button;

				smurf_struct->module_mode = M_WAITING;
			}

			break;

	/* Keyboardevent */
		case MKEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&mode;
				smurf_struct->event_par[2] = 1;

				smurf_struct->module_mode = M_MOREOK;
			}
			else
				smurf_struct->module_mode = M_WAITING;

			break;

	/* Extender wird vom Smurf erfragt */
		case MEXTEND:
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			switch((int)BitsPerPixel)
			{
				case 1:	smurf_struct->event_par[0] = 3;
						break;
				case 8:	smurf_struct->event_par[0] = 2;
						break;
				case 24:smurf_struct->event_par[0] = 1;
						break;
			}
			smurf_struct->module_mode = M_EXTEND;
			
			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			if(smurf_struct->smurf_pic->depth == 8)
				smurf_struct->event_par[0] = GREY;
			else
				smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

	/* Und losexportieren */
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

			if(BitsPerPixel == 1)
			{
				w = (unsigned long)(width + 7) / 8;
				memwidth = w * 8;
			}
			else
			{
				if(BitsPerPixel == 24)
					w = (unsigned long)width * 3;
				else
					w = (unsigned long)width;
				memwidth = (unsigned long)width;
			}

			headsize = 3 + 6 + 6 + 4;

			if(mode == ASCII)			/* ASCII */
			{
				if(BitsPerPixel == 1)
				{
					/* 4 Werte passen in eine Zeile zu 70 Stellen */
					/* ret Returnzeichen zusÑtzlich werden benîtigt */
					ret = ((w + 3) / 4) * 2;
					zeichen = 2;
	
					f_len = ((long)zeichen * width + ret) * (long)height;
				}
				else
				{
					/* 17 Werte passen in eine Zeile zu 70 Stellen */
					/* ret Returnzeichen zusÑtzlich werden benîtigt */
					ret = ((w + 16) / 17) * 2;
					zeichen = 4;
	
					f_len = ((long)zeichen * w + ret) * (long)height;
				}
			}
			else		/* binÑr */
				f_len = (memwidth * (long)height * (long)BitsPerPixel) >> 3;

			if((ziel = (char *)SMalloc(headsize + f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				oziel = ziel;

				write_header(mode, width, height, &f_len, BitsPerPixel);			/* Header schreiben */

				if(mode == ASCII)
					write_ASCIIPic(buffer, width, height, w, BitsPerPixel);
				else
					write_BINPic(buffer, height, w);

				ziel = oziel;

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
	} /* switch */

	return(NULL);
}


/* Schreibt den PPM-Header */
void write_header(char mode, unsigned int width, unsigned int height, unsigned long *f_len, char BitsPerPixel)
{
	char dummy[6],
		 slen;


	/* Header schreiben */
	*ziel++ = 'P';
	if(BitsPerPixel == 1)
		if(mode == ASCII)
			*ziel++ = '1';
		else
			*ziel++ = '4';
	else
		if(BitsPerPixel == 8)
			if(mode == ASCII)
				*ziel++ = '2';
			else
				*ziel++ = '5';
		else
			if(mode == ASCII)
				*ziel++ = '3';
			else
				*ziel++ = '6';
	*ziel++ = 0x0a;
	*f_len += 3;

	itoa(width, dummy, 10);
	slen = (char)strlen(dummy);
	strncpy(ziel, dummy, slen);
	ziel += slen;
	*f_len += slen;

	*ziel++ = ' ';

	(*f_len)++;
	itoa(height, dummy, 10);
	strncpy(ziel, dummy, slen);
	slen = (char)strlen(dummy);
	ziel += slen;
	*f_len += slen;

	if(BitsPerPixel != 1)
	{
		*ziel++ = 0x0a;
		(*f_len)++;
		strncpy(ziel, "255", 3);
		ziel += 3;
		*f_len += 3;
	}
	*ziel++ = 0x0a;
	(*f_len)++;

	return;
} /* write_header */


/* Schreibt ein ASCII-Bild */
void write_ASCIIPic(char *buffer, unsigned int width, unsigned int height, unsigned long w, char BitsPerPixel)
{
	unsigned int y;


	if(BitsPerPixel == 1)
	{
		y = 0;
		do
		{
			x = 0;
			do
			{
				in = 0;			/* maximale ZeilenlÑnge 4 Pixel */
				do
				{
					putval_1Bit(*buffer++);
				} while(x < width && in < 64);
				*ziel++ = '\r';
				*ziel++ = '\n';
			} while(x < width);
		} while(++y < height);
	}
	else
	{
		y = 0;
		do
		{
			x = 0;
			do
			{
				in = 0;			/* maximale ZeilenlÑnge 17 Pixel */
				do
				{
					putval(*buffer++);
				} while(++x < w && in < 68);
				*ziel++ = '\r';
				*ziel++ = '\n';
			} while(x < w);
		} while(++y < height);
	}

	return;
} /* write_ASCIIPic */


/* Schreibt ein binÑres Bild */
void write_BINPic(char *buffer, unsigned int height, unsigned long w)
{
	unsigned int y;


	y = 0;
	do
	{
		memcpy(ziel, buffer, w);
		buffer += w;
		ziel += w;
	} while(++y < height);

	return;
} /* write_BINPic */


void putval(char val)
{
	char pixval, addval, prevnotspace = 0;


	if((pixval = (val / 100)) != 0)
	{
		*ziel++ = pixval + '0';
		prevnotspace = 1;
	}
	else
		*ziel++ = ' ';

	addval = pixval * 100;

	if((pixval = (val / 10 - pixval * 10)) != 0 || prevnotspace)
		*ziel++ = pixval + '0';
	else
		*ziel++ = ' ';

	addval += pixval * 10;

	*ziel++ = (val - addval) + '0';

	*ziel++ = ' ';

	in += 4;

	return;
} /* putval */



void putval_1Bit(char val)
{
	char pixval, bits;


	bits = 7;
	do
	{
		pixval = (val >> bits) & 0x01;

		*ziel++ = pixval + '0';
		*ziel++ = ' ';

		x++;
	} while(bits-- && x < width);

	in += 16;

	return;
} /* putval1 */