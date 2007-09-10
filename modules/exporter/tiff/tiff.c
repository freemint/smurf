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

/* Effizienz des LZW-Encoders ist viel zu niedrig.
   Siehe WZ_LZW_SM.TIF vs. WZ_LZW_PS.TIF.
   Photoshop komprimiert 14,5% besser! */

/* =========================================================*/
/*							TIFF Exporter					*/
/* Version 0.1  --  23.04.98 - 24.04.98						*/
/*	  Header und 1 Bit unkomprimiert						*/
/* Version 0.2  --  25.04.98								*/
/*	  8 Bit Graustufen, Palettenbilder und 24 Bit eingebaut	*/
/* Version 0.3  --  02.05.98 und 18.5.98					*/
/*	  Fehler bei 8 Bit Export gefixt. Es wurde der Funktion	*/
/*	  fest zielbuf statt actziel als Ziel Åbergeben.		*/
/* Version 0.4  --  17.10.98								*/
/*	  Fehler in Bedienung behoben. Bei Verlassen per Return	*/
/*	  wurde der Dialog zwar geschlossen, die Config aber	*/
/*	  nicht gespeichert.									*/
/* Version 0.5  --  22.10.2000								*/
/*	  Schande, longborder hatte nie funktioniert (immer 0	*/
/*	  geliefert). D.h. Long-Tags in Intel-TIFF waren fast	*/
/*	  immer ungÅltig.										*/
/*	  Davon abgesehen wurden pStripOffsets und				*/
/*	  pStripByteCounts Åberhaupt nicht in Intelschreibweise	*/
/*	  gespeichert ...										*/
/* Version 0.6  --  4.11.2000								*/
/*	  4 Bit Bilder werden jetzt korrekt geschrieben wenn	*/
/*	  sie aus mehr als einem Streifen bestehen.				*/
/* Version 0.7  --  5.11.2000 - 6.11.2000					*/
/*	  LZW-Kodierung eingebaut								*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <..\..\..\demolib.h>

#define PRG		0

#define ENGLISCH 0

#if ENGLISCH
	#include "tiff\en\tiff.rsh"
	#include "tiff\en\tiff.rh"
#else
	#include "tiff\de\tiff.rsh"
	#include "tiff\de\tiff.rh"
#endif


#define BILEVEL 1
#define GREYSCALE	2
#define PALETTE 3
#define FULLCOLOR 4

#define II 0x4949
#define MM 0x4d4d

#define BYTE 1
#define ASCII 2
#define SHORT 3
#define LONG 4
#define RATIONAL 5

typedef struct
{
	char comp;		/* Kompression */
	char border;	/* Byte Order */
} CONFIG;

#if !PRG
void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);
#else
void *SMalloc(long amount);
int SMfree(void *ptr);
#endif

int (*f_module_window)(WINDOW *mod_window);
void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);

unsigned int write_header(char *ziel, SMURF_PIC *smurf_pic, CONFIG *config, char format, unsigned int headsize, long w);
long writeTIFFdata_bi(char *ziel, char *buffer, unsigned long w, unsigned int height);
long writeTIFFdata_grey(char *ziel, char *buffer, unsigned long w, unsigned int height);
long writeTIFFdata_pal4(char *ziel, char *buffer, unsigned int width, unsigned int realheight, unsigned int height);
long writeTIFFdata_pal8(char *ziel, char *buffer, unsigned long w, unsigned int height);
long writeTIFFdata_rgb(char *ziel, char *buffer, unsigned int width, unsigned int height);
long encode_RLE(char *ziel, char *buffer, unsigned long w, unsigned int height);
long encode_LZW(char *ziel, char *buffer, unsigned long w, unsigned int height, GARGAMEL *smurf_struct);
void getpix_std_line(void *st_pic, void *buf16, int planes, long planelen, int howmany);
unsigned int intborder(unsigned int input);
unsigned long longborder(unsigned long input);
void write_IFDEntry(char *ziel, int *dcount, unsigned int tag, unsigned int type, unsigned long count, unsigned long value, unsigned int *ifds);

/* Dies bastelt direkt ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"TIFF",
						0x0070,
						"Christian Eyrich",
						"TIFF", "", "", "", "",
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

char Intel, headwrite;
unsigned long *pStripByteCounts;
unsigned long *pStripOffsets;
unsigned int StripsPerPic, RowsPerStrip;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*			Taged Image File Format (TIFF)			*/
/*		1, 4, 8, 24 Bit, 							*/
/*		unkomprimiert, RLE, Huffman und LZW			*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *actziel, *zielbuf, *ziel, *oziel,
		 BitsPerPixel, format;
	char wt[] = "TIFF Exporter";

	static int module_id;
	unsigned int width, height, heightinv, headsize, Button, t;

	unsigned long f_len, w, ws, datalen;

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
				config.comp = KEINE;
				config.border = MOTO;
			}

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[TIFF_EXPORT];							/* Resourcebaum holen */

			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			smurf_struct->module_mode = M_WAITING;

			break;

		case MMORE:
			/* Ressource aktualisieren */
			win_form[KEINE].ob_state &= ~SELECTED;
			win_form[RLE].ob_state &= ~SELECTED;
			win_form[HUFFMAN].ob_state &= ~SELECTED;
			win_form[LZW].ob_state &= ~SELECTED;
			win_form[config.comp].ob_state |= SELECTED;

			win_form[MOTO].ob_state &= ~SELECTED;
			win_form[INTEL].ob_state &= ~SELECTED;
			win_form[config.border].ob_state |= SELECTED;

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
				config.comp = KEINE;
				config.border = MOTO;
			}

			smurf_struct->module_mode = M_WAITING;

			break;

	/* Buttonevent */
		case MBEVT:
			Button = smurf_struct->event_par[0];

			if(Button == KEINE || Button == RLE || Button == HUFFMAN || Button == LZW)
			{
				config.comp = (char)Button;

				smurf_struct->module_mode = M_WAITING;
			}
			else
				if(Button == MOTO || Button == INTEL)
				{
						config.border = (char)Button;

					smurf_struct->module_mode = M_WAITING;
				}
				else
					if(Button == OK)
					{
						/* Konfig Åbergeben */
						*(long *)&smurf_struct->event_par[0] = (long)&config;
						smurf_struct->event_par[2] = (unsigned int)sizeof(CONFIG);

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

			break;

	/* Keyboardevent */
		case MKEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&config;
				smurf_struct->event_par[2] = (unsigned int)sizeof(CONFIG);

				smurf_struct->module_mode = M_MOREOK;
			}
			else
				smurf_struct->module_mode = M_WAITING;

			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			if(smurf_struct->smurf_pic->depth == 8 && smurf_struct->smurf_pic->col_format == GREY)
				smurf_struct->event_par[0] = GREY;
			else
				smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */
#if !PRG
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;
#endif
	
			buffer = smurf_struct->smurf_pic->pic_data;
	
			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			if(config.border == MOTO)
				Intel = 0;
			else
				Intel = 1;

			switch(BitsPerPixel)
			{
				case 1:	ws = w = (width + 7) / 8;
						break;
				case 4:	w = (width + 1) / 2;
						ws = (width + 7) / 8;
						break;
				case 8: ws = w = width;
						break;
				case 24: ws = w = width * 3L;
						 break;
			}

			if(BitsPerPixel == 1)
				format = BILEVEL;
			if(BitsPerPixel >= 2 && BitsPerPixel <= 8 && smurf_struct->smurf_pic->col_format != GREY)
				format = PALETTE;
			if(BitsPerPixel == 8 && smurf_struct->smurf_pic->col_format == GREY)
				format = GREYSCALE;
			if(BitsPerPixel == 24)
				format = FULLCOLOR;

			headwrite = 0;
			headsize = write_header(NULL, smurf_struct->smurf_pic, &config, format, 0, w);
/*			printf("evaluierter Header: %u\n", headsize); */

			f_len = w * (long)height;
			if(config.comp == RLE)
				f_len += f_len * 10 / 100;				/* plus 10% Sicherheitsreserve */
			else
				if(config.comp == LZW && BitsPerPixel > 1)
					f_len = f_len;						/* sparen geht nicht, dafÅr ist die Effizienz nicht hoch genug */

/*			printf("Speicherblock f_len: %lu, headsize: %u\n", f_len, headsize); */

			if((ziel = (char *)SMalloc(headsize + f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				oziel = ziel;
				memset(ziel, 0x0, headsize + f_len);

				if(config.comp != KEINE)
				{
					if((zielbuf = (char *)Malloc(32770L)) == 0)			/* Zwischenpuffer */
					{
						SMfree(ziel);
						smurf_struct->module_mode = M_MEMORY;
						return(exp_pic);
					}

					actziel = zielbuf;
				}

				headwrite = 1;
				f_len = write_header(ziel, smurf_struct->smurf_pic, &config, format, headsize, w);
/*				printf("geschriebener Header: %lu\n", f_len); */
				ziel += f_len;

				heightinv = height;
				do
				{
					*pStripOffsets++ = longborder(f_len);

					if(heightinv < RowsPerStrip)
						RowsPerStrip = heightinv;

					if(config.comp == KEINE)		/* wieder auf aktuelles ziel synchronisieren */
						actziel = ziel;

					switch(format)
					{
						case BILEVEL:	datalen = writeTIFFdata_bi(actziel, buffer, w, RowsPerStrip);
										break;
						case GREYSCALE:	datalen = writeTIFFdata_grey(actziel, buffer, w, RowsPerStrip);
										break;
						case PALETTE:	if(BitsPerPixel == 4)
											datalen = writeTIFFdata_pal4(actziel, buffer, width, height, RowsPerStrip);
										else
											datalen = writeTIFFdata_pal8(actziel, buffer, w, RowsPerStrip);
										break;
						case FULLCOLOR:	datalen = writeTIFFdata_rgb(actziel, buffer, width, RowsPerStrip);
										break;
					}

/*					printf("vor Komp. datalen: %lu\n", datalen); */

					if(config.comp == RLE)
						datalen = encode_RLE(ziel, zielbuf, w, RowsPerStrip);
					else
						if(config.comp == LZW)
							datalen = encode_LZW(ziel, zielbuf, w, RowsPerStrip, smurf_struct);

/*					printf("nach Komp. datalen: %lu\n", datalen);
					getch(); */

					heightinv -= RowsPerStrip;

					f_len += datalen;
					ziel += datalen;

					buffer += ws * RowsPerStrip;
					*pStripByteCounts++ = longborder(datalen);
				} while(--StripsPerPic);

				ziel = oziel;

/*				printf("f_len: %lu\n", f_len); */

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


/* TIFF mit 1 Bit schreiben */
long writeTIFFdata_bi(char *ziel, char *buffer, unsigned long w, unsigned int height)
{
	char *oziel;

	unsigned int x, y;


	oziel = ziel;

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *buffer++;
		} while(++x < w);
	} while(++y < height);

	return(ziel - oziel);
} /* writeTIFFdata_bi */


/* TIFF mit 8 Bit Graustufen schreiben */
long writeTIFFdata_grey(char *ziel, char *buffer, unsigned long w, unsigned int height)
{
	char *oziel;

	unsigned int x, y;


	oziel = ziel;

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *buffer++;
		} while(++x < w);
	} while(++y < height);

	return(ziel - oziel);
} /* writeTIFFdata_grey */


/* TIFF 4 Bit Palettenbilder schreiben - muû leider wegen 2 Pixel in ein Byte in eigene Funktion */
long writeTIFFdata_pal4(char *ziel, char *buffer, unsigned int width, unsigned int realheight, unsigned int height)
{
	char *oziel, *pixbuf, *line;

	unsigned int x, y;

	unsigned long planelength, w, realwidth;


	oziel = ziel;

	realwidth = (width + 7) / 8;
	w = (width + 1) / 2;

	planelength = realwidth * (unsigned long)realheight;

	pixbuf = SMalloc(width + 7);

	y = 0;
	do
	{
		memset(pixbuf, 0x0, width);
		getpix_std_line(buffer, pixbuf, 4, planelength, width);
		buffer += realwidth;
		line = pixbuf;

		x = 0;
		do
		{
			*ziel++ = (*line++ << 4) | *line++;
		} while(++x < w);
	} while(++y < height);

	SMfree(pixbuf);

	return(ziel - oziel);
} /* writeTIFFdata_pal4 */


/* TIFF 8 Bit Palettenbilder schreiben */
long writeTIFFdata_pal8(char *ziel, char *buffer, unsigned long w, unsigned int height)
{
	char *oziel;

	unsigned int x, y;


	oziel = ziel;

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *buffer++;
		} while(++x < w);
	} while(++y < height);

	return(ziel - oziel);
} /* writeTIFFdata_pal8 */


/* TIFF 24 Bit Fullcolorbilder schreiben */
long writeTIFFdata_rgb(char *ziel, char *buffer, unsigned int width, unsigned int height)
{
	char *oziel;

	unsigned int x, y;


	oziel = ziel;

	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = *buffer++;
			*ziel++ = *buffer++;
			*ziel++ = *buffer++;
		} while(++x < width);
	} while(++y < height);

	return(ziel - oziel);
} /* writeTIFFdata_rgb */



/* TIFF mit 1, 8 und 24 Bit PackBits kodieren */
long encode_RLE(char *ziel, char *buffer, unsigned long w, unsigned int height)
{
	char *oziel,
		 pixel, counter, nextenc;

	unsigned int x, y, xx;


	oziel = ziel;

	y = 0;
	do
	{
		x = 0;
		do
		{
			counter = 1;
			pixel = *buffer++;
			x++;

			xx = x;
			while(*buffer == pixel && counter < 0x7f && xx < w)
			{
				buffer++;
				xx++;
				counter++;
			}

			if(counter >= 2)						/* es lohnt sich! */
			{
				*ziel++ = 0x101 - counter;
				*ziel++ = pixel;
				x = xx;
			}       
			else									/* ... aber leider nicht! */
			{
				ziel++;								/* Header fÅr Literal Run schreiben */
				*ziel++ = pixel;

				do
				{
					nextenc = 1;

					while(*buffer != *(buffer + 1) && counter < 0x7f && xx < w)
					{
						*ziel++ = *buffer++;
						xx++;
						counter++;
					}

					while(*buffer == *(buffer + 1) && nextenc < 4 && counter < 0x7f && xx < w)
					{
						*ziel++ = *buffer++;
						xx++;
						counter++;
						nextenc++;
					}
				} while(nextenc < 4 && counter < 0x7f && xx < w);

				if(nextenc > 3)
				{
					counter -= nextenc - 1;
					xx -= nextenc - 1;
					buffer -= nextenc - 1;
					ziel -= nextenc - 1;
				}
/*
				if(counter < 4)
				{
					if(counter == 1)
					{
						ziel -= 2;
						*ziel++ = 1;
						*ziel++ = pixel;
					}
					else
					{
						ziel -= 4;
						*ziel++ = 1;
						*ziel++ = pixel;
						*ziel++ = 1;
						*ziel++ = *(buffer - 1);
					}
				}
				else */
					*(ziel - counter - 1) = counter - 1;

				x = xx;
			}
		} while(x < w);
	} while(++y < height);

	return(ziel - oziel);
} /* encode_RLE */


/* TIFF-Headerstruktur schreiben */
unsigned int write_header(char *ziel, SMURF_PIC *smurf_pic, CONFIG *config, char format, unsigned int headsize, long w)
{
	char *pal,
		 BitsPerPixel;

	int i, cols, dcount, bps;
	unsigned int *ppal,
				 comp, ifds = 0, pallen;

	unsigned long PalOffset, StripOffsets, StripByteCounts, help;


	BitsPerPixel = smurf_pic->depth;

	if(config->comp == KEINE)
		comp = 1;
	else
		if(config->comp == HUFFMAN)
			comp = 2;
		else
			if(config->comp == LZW)
				comp = 5;
			else
				if(config->comp == RLE)
					comp = (unsigned int)32773L;

	cols = 1 << BitsPerPixel;
	if(BitsPerPixel > 1 && BitsPerPixel <= 8 && smurf_pic->col_format != GREY)
		pallen = 6 * cols;
	else
		pallen = 0;

	if(headwrite)			/* nicht bei Pass 1 ausfÅhren */
	{
		if(config->border == MOTO)
			*(unsigned int *)ziel = MM;
		else
			*(unsigned int *)ziel = II;

		*(unsigned int *)(ziel + 2) = intborder(42);

		*(unsigned long *)(ziel + 4) = longborder(28);

		strcpy(ziel + 8, "Smurf TIFF-Exporter");
	}

	dcount = 30;
	write_IFDEntry(ziel, &dcount, 0xfe, LONG, 1, 0, &ifds);						/* New Subfile Type */
	write_IFDEntry(ziel, &dcount, 0x100, SHORT, 1, smurf_pic->pic_width, &ifds);		/* Bildbreite */
	write_IFDEntry(ziel, &dcount, 0x101, SHORT, 1, smurf_pic->pic_height, &ifds);		/* Bildhîhe */
	if(BitsPerPixel >= 1 && BitsPerPixel <= 8)
	{
		bps = 0;
		write_IFDEntry(ziel, &dcount, 0x102, SHORT, 1, BitsPerPixel, &ifds);	/* BitsPerSample */
	}
	if(BitsPerPixel == 24)
	{
		bps = 6;		
		help = headsize - bps - pallen;
		write_IFDEntry(ziel, &dcount, 0x102, SHORT, 3, help, &ifds);			/* BitsPerSample */
		if(headwrite)			/* nicht bei Pass 1 ausfÅhren */
		{
			*(unsigned int *)(ziel + help) = intborder(0x08);
			*(unsigned int *)(ziel + help + 2) = intborder(0x08);
			*(unsigned int *)(ziel + help + 4) = intborder(0x08);
		}
	}
	
	write_IFDEntry(ziel, &dcount, 0x103, SHORT, 1, comp, &ifds);				/* Kompression */

	switch(format)
	{
		case BILEVEL:	write_IFDEntry(ziel, &dcount, 0x106, SHORT, 1, 0, &ifds);	/* min is white */
						break;
		case GREYSCALE:	write_IFDEntry(ziel, &dcount, 0x106, SHORT, 1, 1, &ifds);	/* min is black */
						break;
		case PALETTE:	write_IFDEntry(ziel, &dcount, 0x106, SHORT, 1, 3, &ifds);	/* palette color image */
						break;
		case FULLCOLOR:	write_IFDEntry(ziel, &dcount, 0x106, SHORT, 1, 2, &ifds);	/* RGB full color image */
						break;
	}

	RowsPerStrip = (unsigned int)(32768L / w);	/* 32 KB pro Streifen durch Bytes pro Zeile */
	if(RowsPerStrip > smurf_pic->pic_height)
		RowsPerStrip = smurf_pic->pic_height;
	StripsPerPic = (smurf_pic->pic_height + RowsPerStrip - 1) / RowsPerStrip;	/* mit Rundung */

/*	printf("RowsPerStrip: %u, StripsPerPic: %u\n", RowsPerStrip, StripsPerPic); */

	if(StripsPerPic > 1)
		StripOffsets = headsize - StripsPerPic * 8L - bps - pallen;
	else
		StripOffsets = headsize;
	pStripOffsets = (unsigned long *)(ziel + StripOffsets);
	write_IFDEntry(ziel, &dcount, 0x111, LONG, StripsPerPic, StripOffsets, &ifds);	/* StripOffsets */

	if(BitsPerPixel == 24)
		write_IFDEntry(ziel, &dcount, 0x115, SHORT, 1, 3, &ifds);				/* SamplesPerPixel */
	write_IFDEntry(ziel, &dcount, 0x116, SHORT, 1, RowsPerStrip, &ifds);		/* RowsPerStrip */

	if(StripsPerPic > 1)
		StripByteCounts = headsize - StripsPerPic * 4L - bps - pallen;
	else
		StripByteCounts = dcount + 8;
	pStripByteCounts = (unsigned long *)(ziel + StripByteCounts);				/* Adresse merken */
	write_IFDEntry(ziel, &dcount, 0x117, LONG, StripsPerPic, StripByteCounts, &ifds);	/* StripByteCounts */

	if(StripsPerPic == 1)
		help = headsize - 16 - bps - pallen;
	else
		help = headsize - 16 - StripsPerPic * 8L - bps - pallen;
	write_IFDEntry(ziel, &dcount, 0x11a, RATIONAL, 1, help, &ifds);		/* x-resolution */
	if(headwrite)
	{
		*(unsigned long *)(ziel + help) = longborder(0x48);				/* 72 */
		*(unsigned long *)(ziel + help + 4) = longborder(0x01);			/* durch 1 */
	}
	write_IFDEntry(ziel, &dcount, 0x11b, RATIONAL, 1, help + 8, &ifds);	/* y-resolution */
	if(headwrite)
	{
		*(unsigned long *)(ziel + help + 8) = longborder(0x48);			/* 72 */
		*(unsigned long *)(ziel + help + 12) = longborder(0x01);		/* durch 1 */
	}
	write_IFDEntry(ziel, &dcount, 0x128, SHORT, 1, 2, &ifds);			/* dpi */

	/* öbertragen der Palette */
	if(BitsPerPixel > 1 && BitsPerPixel <= 8 && smurf_pic->col_format != GREY)
	{
		PalOffset = headsize - pallen;
		write_IFDEntry(ziel, &dcount, 0x140, SHORT, 3 * cols, PalOffset, &ifds);	/* ColorMap */

		if(headwrite)			/* Nicht bei Pass 1 ausfÅhren */
		{
			pal = smurf_pic->palette;
			ppal = (unsigned int *)(ziel + PalOffset);

			for(i = 0; i < cols; i++)
			{
				*ppal = *pal++ << 8;
				*(ppal + cols) = *pal++ << 8;
				*(ppal++ + cols + cols) = *pal++ << 8;
			}
		}
	}

	if(headwrite)			/* nicht bei Pass 1 ausfÅhren */
		*(unsigned long *)(ziel + dcount) = longborder(0x0);			/* Offset of next IFD */

	if(headwrite)			/* nicht bei Pass 1 ausfÅhren */
		*(unsigned int *)(ziel + 28) = intborder(ifds);					/* Number of entrys in this IFD */

	if(StripsPerPic == 1)
		return(dcount + 4 + bps + 16 + pallen);
	else
		return(dcount + 4 + bps + 16 + StripsPerPic * 8L + pallen);
} /* write_header */


/* Schreibt je nach Byteformat Motorola oder Intel ints */
unsigned int intborder(unsigned int input)
{
	if(Intel)
		return(swap_word(input));
	else
		return(input);
} /* intborder */


/* Schreibt je nach Byteformat Motorola oder Intel longs */
unsigned long longborder(unsigned long input)
{
	unsigned long output;


	if(Intel)
	{
		output = (unsigned long)swap_word((unsigned int)(input&0xffff)) << 16;
		output |= swap_word((unsigned int)(input >> 16));
		return(output);
	}
	else
		return(input);
} /* longborder */


void write_IFDEntry(char *ziel, int *dcount, unsigned int tag, unsigned int type, unsigned long count, unsigned long value, unsigned int *ifds)
{
	if(headwrite)			/* Nicht bei Pass 1 ausfÅhren */
	{
		ziel += *dcount;

		*(unsigned int *)ziel = intborder(tag);
		*(unsigned int *)(ziel + 2) = intborder(type);
		*(unsigned long *)(ziel + 4) = longborder(count);

		if(type == BYTE)
			*(ziel + 8) = (char)value;
		else
			if(type == SHORT && count == 1)
				*(unsigned int *)(ziel + 8) = intborder((unsigned int)value);
			else
				if(type == LONG || type == RATIONAL || count > 1)
					*(unsigned long *)(ziel + 8) = longborder(value);
	}

	*dcount += 12;
	(*ifds)++;

	return;
} /* write_IFDEntry */


#if PRG
void main(void)
{
	int filehandle;
	GARGAMEL sm_struct;
	SMURF_PIC pic;
	char *file;
	long dummy, len;
	EXPORT_PIC *exp_pic;

	dummy = Fopen("K:\\AMBER.RAW", FO_READ);
	if(dummy >= 0)
		filehandle = (int)dummy;
	else
		return;

	len = Fseek(0L, filehandle, 2);
	Fseek(0L, filehandle, 0L);
	
	file = Malloc(len);

	Fread(filehandle, len, file);
	Fclose(filehandle);

	pic.pic_width = 320;
	pic.pic_height = 200;
	pic.depth = 8;
	sm_struct.smurf_pic = &pic;

	sm_struct.module_mode = MSTART;
	exp_pic = exp_module_main(&sm_struct);
	pic.pic_data = file + 768;
	pic.palette = (char *)malloc(1024);
	memcpy(pic.palette, file, 768);
	sm_struct.module_mode = MEXEC;
	exp_pic = exp_module_main(&sm_struct);

	Mfree(file);

	filehandle = (int)Fcreate("K:\\AMBER_.TIF", 0);
	Fwrite(filehandle, exp_pic->f_len, exp_pic->pic_data);
	Fclose(filehandle);

	return;	
}


void *SMalloc(long amount)
{
	char *buffer;


	buffer = (char *)Malloc(amount);
	memset(buffer, 0x0, amount);

	return(buffer);
}


/* --- Funktion zum Freigeben von Speicher + Kontrolle ------- */
int SMfree(void *ptr)
{
	if(ptr == NULL)
		return(-1);

	Mfree(ptr);

	return(0);
}
#endif