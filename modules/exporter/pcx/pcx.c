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
/*					PCX Paintbrush File Encoder				*/
/* Version 0.1  --  13.10.97 - 14.10.97						*/
/*	  1-8 (aužer 2 Bit), 24 Bit								*/
/*	  Speichert knapp hinter Graftool die am besten kompri-	*/
/*	  mierten PCX.											*/
/* Version 0.2  --  17.10.98								*/
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
	#include "pcx\en\pcx.rsh"
	#include "pcx\en\pcx.rh"
#else
	#include "pcx\de\pcx.rsh"
	#include "pcx\de\pcx.rh"
#endif


void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);

void write_header(char *ziel, unsigned int width, unsigned int height, long w, char comp, char BitsPerPixel, char *pal);
long write_1Bit(char *ziel, char *buffer, int w, int height);
long write_8and24Bit(char *ziel, char *buffer, int w, int height, char Planes);
long write_3and4Bit(char *ziel, char *buffer, int w, int height, char Planes);
long encode_1Bit(char *ziel, char *buffer, int w, int height);
long encode_3and4Bit(char *ziel, char *buffer, int w, int height, char Planes);
long encode_8and24Bit(char *ziel, char *buffer, int w, int height, char Planes);

/* Dies bastelt direkt ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"PCX Paintbrush",
						0x0020,
						"Christian Eyrich",
						"PCX", "", "", "", "",
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
						1, 3, 4, 8, 24,
						0, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						2					/* More */
						};


/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				PCX Image Format (PCX)				*/
/*		1-8, 24 Bit, unkomprimiert, RLE				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel, *pal, *ppal,
		 BitsPerPixel;
	char wt[] = "PCX Exporter";
	static char comp;

	static int module_id;
	unsigned int width, height, headsize, Button, pallen, cols, w, t;

	unsigned long f_len, memwidth;

	static WINDOW window;
	static OBJECT *win_form;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			/* falls bergeben, Konfig bernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy(&comp, (char *)*(long *)&smurf_struct->event_par[0], 1);
			else
				comp = RLE;

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[PCX_EXPORT];							/* Resourcebaum holen */

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
	
			window.whandlem = 0;				/* evtl. Handle l”schen */
			window.module = module_id;			/* ID in die Fensterstruktur eintragen  */
			window.wnum = 1;					/* Fenster nummer 1...  */
			window.wx = -1;						/* Fenster X-...    	*/
			window.wy = -1;						/* ...und Y-Pos     	*/
			window.ww = win_form->ob_width;		/* Fensterbreite    	*/
			window.wh = win_form->ob_height;	/* Fensterh”he      	*/
			strcpy(window.wtitle, wt);			/* Titel reinkopieren   */
			window.resource_form = win_form;	/* Resource         	*/
			window.picture = NULL;				/* kein Bild			*/ 
			window.editob = 0;					/* erstes Editobjekt	*/
			window.nextedit = 0;				/* n„chstes Editobjekt	*/
			window.editx = 0;

			smurf_struct->wind_struct = &window;  /* und die Fensterstruktur in die Gargamel */

			if(f_module_window(&window) == -1)			/* Gib mir 'n Fenster! */
				smurf_struct->module_mode = M_EXIT;		/* keins mehr da? */
			else 
				smurf_struct->module_mode = M_WAITING;	/* doch? Ich warte... */

			break;

/* Closer geklickt, Default wieder her */
		case MMORECANC:
			/* falls bergeben, Konfig bernehmen */
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
				/* Konfig bergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&comp;
				smurf_struct->event_par[2] = 1;

				smurf_struct->module_mode = M_MOREOK;
			}
			else
			if(Button == SAVE)
			{
				/* Konfig bergeben */
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
				/* Konfig bergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&comp;
				smurf_struct->event_par[2] = 1;

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

			printf("comp: %d\n", (int)comp);

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			headsize = 0x80;

			if(BitsPerPixel < 8)
			{
				w = (width + 7) / 8;
				memwidth = w * 8;
			}
			else
			{
				w = width;
				memwidth = width;
			}

			f_len = ((long)memwidth * (long)height * BitsPerPixel) >> 3;
			if(comp == RLE)
				f_len += f_len * 15 / 100;				/* plus 15% Sicherheitsreserve */

			if(BitsPerPixel == 8)
				pallen = 769;
			else
				pallen = 0;

			pal = smurf_struct->smurf_pic->palette;

			if((ziel = (char *)SMalloc(headsize + f_len + pallen)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				write_header(ziel, width, height, w, comp, BitsPerPixel, pal);

				if(comp == KEINE)
					switch((int)BitsPerPixel)
					{
						case 1: f_len = write_1Bit(ziel + headsize, buffer, w, height);
								break;
/*						case 2: f_len = write_2Bit(ziel, buffer);
								break; */
						case 3:
						case 4: f_len = write_3and4Bit(ziel + headsize, buffer, w, height, BitsPerPixel);
								break;
						case 8:
						case 24: f_len = write_8and24Bit(ziel + headsize, buffer, w, height, BitsPerPixel >> 3);
								 break;
					}
				else
					switch((int)BitsPerPixel)
					{
						case 1: f_len = encode_1Bit(ziel + headsize, buffer, w, height);
								break;
/*						case 2: f_len = encode_2Bit(ziel, buffer);
								break; */
						case 3:
						case 4: f_len = encode_3and4Bit(ziel + headsize, buffer, w, height, BitsPerPixel);
								break;
						case 8:
						case 24: f_len = encode_8and24Bit(ziel + headsize, buffer, w, height, BitsPerPixel >> 3);
								 break;
					}

				/* Palette */
				if(BitsPerPixel == 8)
				{
					ppal = ziel + headsize + f_len;
					*ppal++ = 0x0c;
					cols = 1 << BitsPerPixel;
					for(t = 0; t < cols; t++)
					{
						*ppal++ = *pal++;
						*ppal++ = *pal++;
						*ppal++ = *pal++;
					}
				}

				exp_pic->pic_data = ziel;
				exp_pic->f_len = headsize + f_len + pallen;
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


/* Muž wegen der Invertierung leider in eigene Funktion */
long write_1Bit(char *ziel, char *buffer, int w, int height)
{
	unsigned int x, y;

	long len = 0;


	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = ~*buffer++;
		} while(++x < w);
	} while(++y < height);

	len = (long)w * (long)height;

	return(len);
} /* write_1Bit */


long write_3and4Bit(char *ziel, char *buffer, int w, int height, char Planes)
{
	char *obuffer,
		 p;

	unsigned int x, y;

	long offset, planelength, len = 0;


	offset = w;
	planelength = (long)w * (long)height;

	obuffer = buffer;

	y = 0;
	do
	{
		p = 0;
		do
		{
			buffer = obuffer + p * planelength;

			x = 0;
			do
			{
				*ziel++ = *buffer++;
			} while(++x < w);
		} while(++p < Planes);
		obuffer += offset;
	} while(++y < height);

	len = (long)w * (long)height * Planes;

	return(len);
} /* write_3and4Bit */


long write_8and24Bit(char *ziel, char *buffer, int w, int height, char Planes)
{
	char *obuffer,
		 p;

	unsigned int x, y;

	long offset, len = 0;


	offset = w * Planes;

	obuffer = buffer;

	y = 0;
	do
	{
		p = 0;
		do
		{
			buffer = obuffer + p;

			x = 0;
			do
			{
				*ziel++ = *buffer;
				buffer += Planes;
			} while(++x < w);
		} while(++p < Planes);
		obuffer += offset;
	} while(++y < height);

	len = (long)w * (long)height * Planes;

	return(len);
} /* write_8and24Bit */


/* Muž wegen der Invertierung leider in eigene Funktion */
long encode_1Bit(char *ziel, char *buffer, int w, int height)
{
	char n, pixel;

	unsigned int x, y;

	long len = 0;


	y = 0;
	do
	{
		x = 0;
		do
		{
			n = 1;
			pixel = ~*buffer++;
			x++;
			while(~*buffer == pixel && n < 0x3f && x < w)
			{
				buffer++;
				x++;
				n++;
			}

			if(n > 1 || (pixel & 0xc0) == 0xc0)
			{
				*ziel++ = (n | 0xc0);
				*ziel++ = pixel;
				len += 2;
			}
			else
			{
				*ziel++ = pixel;
				len++;
			}
		} while(x < w);
	} while(++y < height);

	return(len);
} /* encode_1Bit */


long encode_3and4Bit(char *ziel, char *buffer, int w, int height, char Planes)
{
	char *obuffer,
		 n, p, pixel;

	unsigned int x, y;

	long offset, planelength, len = 0;


	offset = w;
	planelength = (long)w * (long)height;

	obuffer = buffer;

	y = 0;
	do
	{
		p = 0;
		do
		{
			buffer = obuffer + p * planelength;

			x = 0;
			do
			{
				n = 1;
				pixel = *buffer++;
				x++;
				while(*buffer == pixel && n < 0x3f && x < w)
				{
					buffer++;
					x++;
					n++;
				}

				if(n > 1 || (pixel & 0xc0) == 0xc0)
				{
					*ziel++ = (n | 0xc0);
					*ziel++ = pixel;
					len += 2;
				}
				else
				{
					*ziel++ = pixel;
					len++;
				}
			} while(x < w);
		} while(++p < Planes);
		obuffer += offset;
	} while(++y < height);

	return(len);
} /* encode_3and4Bit */


long encode_8and24Bit(char *ziel, char *buffer, int w, int height, char Planes)
{
	char *obuffer,
		 n, p, pixel;

	unsigned int x, y;

	long offset, len = 0;


	offset = w * Planes;

	obuffer = buffer;

	y = 0;
	do
	{
		p = 0;
		do
		{
			buffer = obuffer + p;

			x = 0;
			do
			{
				n = 1;
				pixel = *buffer;
				buffer += Planes;
				x++;
				while(*buffer == pixel && n < 0x3f && x < w)
				{
					buffer += Planes;
					x++;
					n++;
				}

				if(n > 1 || (pixel & 0xc0) == 0xc0)
				{
					*ziel++ = (n | 0xc0);
					*ziel++ = pixel;
					len += 2;
				}
				else
				{
					*ziel++ = pixel;
					len++;
				}
			} while(x < w);
		} while(++p < Planes);
		obuffer += offset;
	} while(++y < height);

	return(len);
} /* encode_8and24Bit */


/* PCX-Headerstruktur schreiben */
void write_header(char *ziel, unsigned int width, unsigned int height, long w, char comp, char BitsPerPixel, char *pal)
{
	char *ppal, version;

	int i, cols;


	memset(ziel, 0x0, 0x80);							/* Header initialisieren */

	*ziel++ = 0x0a;										/* Magic */

	/* Version */
	if(BitsPerPixel == 1)
		version = 0;									/* langt fr s/w */
	else
		if(BitsPerPixel > 1 && BitsPerPixel < 8)
			version = 2;
		else
			version = 5;

	*ziel++ = version;

	if(comp == KEINE)
		*ziel++ = 0;									/* Compression */
	else
		*ziel++ = 1;									/* Compression */

	/* BitsPerPixel per Plane */
	if(BitsPerPixel == 1 || BitsPerPixel == 3 || BitsPerPixel == 4)
		*ziel++ = 1;
	else
		if(BitsPerPixel == 2)
			*ziel++ = 2;
		else
			if(BitsPerPixel == 8 || BitsPerPixel == 24)
				*ziel++ = 8;

	*((unsigned int *)ziel)++ = swap_word(0);			/* XStart */
	*((unsigned int *)ziel)++ = swap_word(0);			/* YStart */
	*((unsigned int *)ziel)++ = swap_word(width - 1);	/* XEnd */
	*((unsigned int *)ziel)++ = swap_word(height - 1);	/* YEnd */

	*((unsigned int *)ziel)++ = swap_word(width);		/* HRes */
	*((unsigned int *)ziel)++ = swap_word(height);		/* VRes */


	/* Palette */
	if(BitsPerPixel <= 4)
	{
		ppal = ziel;
	
		if(BitsPerPixel == 1)
		{
			ppal[0] = 255;
			ppal[1] = 255;
			ppal[2] = 255;
			ppal[3] = 0;
			ppal[4] = 0;
			ppal[5] = 0;
		}
		else
		{
			cols = 1 << BitsPerPixel;
			for(i = 0; i < cols; i++)
			{
				*ppal++ = *pal++;
				*ppal++ = *pal++;
				*ppal++ = *pal++;
			}
		}
	}

	ziel += 0x30;

	*ziel++ = 0x0;										/* Reserved1 */

	/* Anzahl Planes */
	if(BitsPerPixel == 1 || BitsPerPixel == 2 || BitsPerPixel == 8)
		*ziel++ = 1;
	else
		if(BitsPerPixel == 3 || BitsPerPixel == 4)
			*ziel++ = BitsPerPixel;
		else
			if(BitsPerPixel == 24)
				*ziel++ = 3;

	*((unsigned int *)ziel)++ = swap_word(w);			/* Bytes per Line */

	*((unsigned int *)ziel)++ = swap_word(1);			/* Palette Type */
		
	if(version > 4)
	{
		*((unsigned int *)ziel)++ = swap_word(width);	/* horizontal Screensize */
		*((unsigned int *)ziel)++ = swap_word(height);	/* vertikal Screensize */
	}	

	return;
} /* write_header */