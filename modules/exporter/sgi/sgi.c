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
/*						SGI Image File Encoder				*/
/* Version 0.1  --  03.03.97								*/
/*	  8, 24 Bit												*/
/* Version 0.2  --  03.05.97								*/
/*	  Eigenes Konfigurationsformular eingebaut.				*/
/* Version 0.3  --  17.10.98								*/
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
	#include "sgi\en\sgi.rsh"
	#include "sgi\en\sgi.rh"
#else
	#include "sgi\de\sgi.rsh"
	#include "sgi\de\sgi.rh"
#endif

#define	TextCast	ob_spec.tedinfo->te_ptext


void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);

void write_header(char *ziel, char *comment, unsigned int width, unsigned int height, char comp, char ColorMap, char Planes, char Dim);
unsigned long write_1bpc(char *ziel, char *buffer, unsigned int width, unsigned int height, char Planes);
unsigned long encode_1bpc(char *ziel, char *buffer, unsigned int width, unsigned int height, unsigned long tablen, char Planes);
int switch_orient(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel);


/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"SGI Image File",
						0x0030,
						"Christian Eyrich",
						"SGI", "RLE", "", "", "",
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
						8, 24, 0, 0, 0,
						0, 0, 0,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						6					/* More + Infotext */
						};


/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				SGI Image Format (SGI)				*/
/*		8, 24 Bit, unkomprimiert, RLE				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel, *oziel,
		 BitsPerPixel, Planes, Dim, ColorMap, t;
	char wt[] = "SGI Exporter";
	static char comp;

	static int module_id;
	unsigned int width, height, headsize, Button;

	unsigned long tablen, f_len;

	static WINDOW window;
	static OBJECT *win_form, *alerts;

	switch(smurf_struct->module_mode)
	{
		case MSTART:
			/* falls Åbergeben, Konfig Åbernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy(&comp, (char *)*(long *)&smurf_struct->event_par[0], 1);
			else
				comp = RLE;

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[SGI_EXPORT];							/* Resourcebaum holen */
			alerts = rs_trindex[ALERT_STRINGS];							/* Resourcebaum holen */

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

				smurf_struct->module_mode = M_MOREOK;
			}
			else
				smurf_struct->module_mode = M_WAITING;

			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			if(smurf_struct->smurf_pic->depth == 8)
				smurf_struct->event_par[0] = GREY;
			else
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

			/* 1 = BW, ditherd, colormapped; 3 = RGB; 4 = RGB + alpha channel */
			if(BitsPerPixel == 24)
				Planes = 3;
			else
				Planes = 1;

			if(BitsPerPixel == 24)
				Dim = 3;
			else
				Dim = 2;

			/* 0x00 - Normal pixel values: 1 Channel Greyscale, 3 Channels RGB */
			/* 0x01 - Dithered Images: (8 BPP 3+3+2) - obsolete */
			/* 0x02 - single Channel Images - obsolete */
			/* 0x03 - color Map */
			ColorMap = 0;

			headsize = 0x200;
			f_len = ((long)width * (long)height * Planes * BitsPerPixel) >> 3;

			if(comp == RLE)
				tablen = (long)height * (long)Planes * 4 * 2;
			else
				tablen = 0;

			if((ziel = (char *)SMalloc(headsize + f_len + tablen)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				oziel = ziel;

				write_header(ziel, smurf_struct->smurf_pic->infotext, width, height, comp, ColorMap, Planes, Dim);

				if(comp == KEINE)
				{
					if(switch_orient(buffer, width, height, BitsPerPixel) != 0)
						smurf_struct->services->f_alert(alerts[ORIENTCORR].TextCast, NULL, NULL, NULL, 1);
					f_len = write_1bpc(oziel, buffer, width, height, Planes);
				}
				else
					f_len = encode_1bpc(oziel, buffer, width, height, tablen, Planes);

				ziel = oziel;

				exp_pic->pic_data = ziel;
				exp_pic->f_len = headsize + f_len;
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

/* Kodiert Bilder mit einem und drei Bytes pro Pixel */
unsigned long write_1bpc(char *ziel, char *buffer, unsigned int width, unsigned int height, char Planes)
{
	char *obuffer,
		 p;

	unsigned int x, y;

	unsigned long len;


	obuffer = buffer;
	ziel += 0x200;

	len = 0;

	p = 0;
	do
	{
		buffer = obuffer + p;

		y = height;
		while(y--)				/* y absteigend, damit die Tabelle von unten her geschrieben wird - */
		{						/* so wird ein flip bei komprimierten Bildern ÅberflÅssig */
			x = 0;
			do /* width */
			{
				x++;
				*ziel++ = *buffer;
				buffer += Planes;

			} while(x < width); /* x */

			len += width;
		} /* y */
	} while(++p < Planes);

	return(len);
} /* write_1bpc */

/* Kodiert Bilder mit einem und drei Bytes pro Pixel */
unsigned long encode_1bpc(char *ziel, char *buffer, unsigned int width, unsigned int height, unsigned long tablen, char Planes)
{
	char *zielo, *obuffer,
		 pixel, p, counter;

	unsigned int x, xx, y;

	unsigned long *starttab, *lengthtab,
				  len, llen, tablen3;


	obuffer = buffer;
	zielo = ziel;
	ziel += 0x200;

	starttab = (unsigned long *)ziel;
	lengthtab = starttab + (height * Planes);
	ziel += tablen;

	llen = len = 0;

	p = 0;
	do
	{
		tablen3 = height * p;

		buffer = obuffer + p;

		y = height;
		while(y--)				/* y absteigend, damit die Tabelle von unten her geschrieben wird - */
		{						/* so wird ein flip bei komprimierten Bildern ÅberflÅssig */
			starttab[tablen3 + y] = (unsigned long)(ziel - zielo);

			x = 0;
			do /* width */
			{
				counter = 1;
				pixel = *buffer;
				buffer += Planes;
				x++;

				xx = x;
				while(*buffer == pixel && counter < 0x7f && xx < width)
				{
					buffer += Planes;
					xx++;
					counter++;
				}

				if(counter > 1)					/* es lohnt sich ... */
				{
					*ziel++ = counter;
					*ziel++ = pixel;
					len += 2;
				}       
				else							/* ... leider nicht! */
				{
					ziel++;
					*ziel++ = pixel;
					while((*buffer != *(buffer + Planes) || *buffer != *(buffer + Planes + Planes)) && counter < 0x7f && xx < width)
					{
						*ziel++ = *buffer;
						buffer += Planes;
						counter++;
						xx++;
					}
					*(ziel - counter - 1) = 0x80 | counter;
					len += counter + 1;
				}
				x = xx;
			} while(x < width); /* x */

			lengthtab[tablen3 + y] = len - llen;
			llen = len;
		} /* y */
	} while(++p < Planes);

	*ziel++ = 0;				/* Zuletzt Null-Run */
	len++;
	len += tablen;

	return(len);
} /* encode_1bpc */


/* éndert die Orientierung der Bilder */
int switch_orient(char *buffer, unsigned int width, unsigned int height, char BitsPerPixel)
{
	char *buffero, *bufferu, *temp;

	unsigned int t;

	unsigned long w, len, planelength;


	w = (unsigned long)width;	

	len = (w * BitsPerPixel) >> 3;
	planelength = len * height;

	if((temp = Malloc(len)) == 0)
		return(-1);

	buffero = buffer;
	bufferu = buffero + planelength - len;

	t = height >> 1;
	while(t--)
	{
		memcpy(temp, buffero, len);
		memcpy(buffero, bufferu, len);
		memcpy(bufferu, temp, len);

		buffero += len;
		bufferu -= len;
	}

	Mfree(temp);

	return(0);
}


/* SGI-Headerstruktur schreiben */
void write_header(char *ziel, char *comment, unsigned int width, unsigned int height, char comp, char ColorMap, char Planes, char Dim)
{
	*((unsigned int *)ziel)++ = 0x01da;					/* Magic */

	if(comp == KEINE)
		*ziel++ = 0;									/* Compression */
	else
		*ziel++ = 1;									/* Compression */

	/* 2 Bpc wird es nicht geben, da es keineswegs 16 Bit-Bilder sind, sondern weiterhin */
	/* Bilder mit drei KanÑlen, jedoch 16 Bit Genauigkeit pro RGB-Kanal ... */
	*ziel++ = 1;										/* Bpc */
	*((unsigned int *)ziel)++ = (unsigned int)Dim;		/* Dimension */

	*((unsigned int *)ziel)++ = width;					/* XSize */
	*((unsigned int *)ziel)++ = height;					/* YSize */
	*((unsigned int *)ziel)++ = (unsigned int)Planes;	/* ZSize */

	*((unsigned long *)ziel)++ = 0x0;					/* PixMin */
	*((unsigned long *)ziel)++ = 0xff;					/* PixMax */

	memset(ziel, 0x0, 4);								/* Dummy */
	ziel += 4;

	strncpy(ziel, comment, 79);							/* Image Name */
	ziel[79] = '\0';
	ziel += 80;

	*((unsigned long *)ziel)++ = ColorMap;				/* ColorMap */
	memset(ziel, 0x0, 404);								/* Not used */
	ziel += 404;

	return;
} /* write_header */