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
/*						GIF-Format Exporter					*/
/*	Version 0.1 - 07.09.97									*/
/*	  Skelett wird geschrieben								*/
/*	Version 0.2 - 15.8.98									*/
/*	  GIF wird korrekt aber ohne Kompression geschrieben	*/
/*	Version 0.3 - 2.10.98-3.10.98							*/
/*	  LZW-Kompression eingebaut								*/
/*	Version 0.4 - 17.10.98									*/
/*	  LZW-Kompressor Åberarbeitet und etwas beschleunigt;	*/
/*	  Transparenz ermîglicht.								*/
/*	Version 0.5 - 22.10.98									*/
/*	  Verarbeitung von 1-7 Bit Bildern ist jetzt mîglich.	*/
/*	Version 0.6 - 28.10.2000								*/
/*	  Bilder kînnen auch interlaced gespeichert werden.		*/
/*	Version 0.7 - 4.11.2000									*/
/*	  Speicherplatzverbrauch reduziert (im Falle von Bildern*/
/*	  < 8 Bit sogar drastisch). Vorher war immer noch die	*/
/*	  Berechnung auf Grund des "Expansionsalgorithmus"		*/
/*	  (8 auf 9 Bit pro Pixel auch bei 1-7 Bit) zum Testen	*/
/*	  der ersten Version drinnen.							*/
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

#define ENGLISCH	0

#if ENGLISCH
	#include "gif\en\gif.rsh"
	#include "gif\en\gif.rh"
#else
	#include "gif\de\gif.rsh"
	#include "gif\de\gif.rh"
#endif


#define _LSCRDES	 7			/* ist wegen des AuffÅllens der Strukturen */
#define _IMAGE_DES	10			/* von Compilerseite her leider nîtig */

typedef struct
{
	char typ;			/* 87a oder 89a */
	char interlace;		/* interlaced oder nicht */
	int transparent;	/* transparenter Index */
	char tindexok;		/* transparenter Index gÅltig */
} CONFIG;

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);
void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);
void (*reset_busybox)(int lft, char *string);

char *write_header(char *file, char typ);
char *write_lscrdes(char *file, char *pal, unsigned int width, unsigned int height, char BitsPerPixel);
char *write_graphctrl_ext(char *file, CONFIG *config, int delay);
char *write_comment_ext(char *file, CONFIG *config, char *comment);
char *write_image_descriptor(char *file, char *pal, unsigned int width, unsigned int height, char BitsPerPixel, char interlace);
void interlace(unsigned int height, CONFIG *config);
char *encode_lzw_17bit(char *buffer, char *ziel, unsigned int width, unsigned int height, char BitsPerPixel);
char *encode_lzw_8bit(char *buffer, char *ziel, unsigned int width, unsigned int height);


/* Dies bastelt direct ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/*	Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"GIF",
						0x0070,
						"Christian Eyrich",
						"GIF","","","","",
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
						1, 2, 3, 4, 5,
						6, 7, 8,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_PIXELPAK,
						6					/* More + Infotext */
						};


int *lacetab;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					GIF - Exporter (GIF)			*/
/*		1-8 Bit, LZW								*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel, *comment,
		 *file, *current_file,
		 BitsPerPixel, t;
	char wt[] = "GIF Exporter";

	static int module_id;
	unsigned int width, height, headlen, Button;

	unsigned long f_len, length;

	static WINDOW window;
	static OBJECT *win_form;
	static CONFIG config;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			reset_busybox = smurf_struct->services->reset_busybox;

			/* falls Åbergeben, Konfig Åbernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy((char *)&config, (char *)*(long *)&smurf_struct->event_par[0], sizeof(CONFIG));
			else
			{
				config.typ = GIF87A;
				config.interlace = 0;
				config.transparent = 0;
				config.tindexok = 0;
			}

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[GIF_EXPORT];			/* Resourcebaum holen */

			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			smurf_struct->module_mode = M_WAITING;

			break;

		case MMORE:
			/* Ressource aktualisieren */
			if(config.interlace)
				win_form[LINE_ARRANGE].ob_state |= SELECTED;
			else
				win_form[LINE_ARRANGE].ob_state &= ~SELECTED;

			if(config.tindexok)
			{
				win_form[TINDEX].ob_state &= ~DISABLED;
				win_form[TINDEXOK].ob_state |= SELECTED;
			}
			else
			{
				win_form[TINDEX].ob_state |= DISABLED;
				win_form[TINDEXOK].ob_state &= ~SELECTED;
			}

			if(config.typ == GIF87A)
			{
				win_form[GIF87A].ob_state |= SELECTED;
				win_form[GIF89A].ob_state &= ~SELECTED;
				win_form[TINDEX].ob_state |= DISABLED;
				win_form[TINDEXOK].ob_state |= DISABLED;
			}
			else
			{
				win_form[GIF87A].ob_state &= ~SELECTED;
				win_form[GIF89A].ob_state |= SELECTED;
				if(config.tindexok)
					win_form[TINDEX].ob_state &= ~DISABLED;
				win_form[TINDEXOK].ob_state &= ~DISABLED;
			}

			itoa(config.transparent, win_form[TINDEX].ob_spec.tedinfo->te_ptext, 10);

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
			window.editob = TINDEX;				/* erstes Editobjekt	*/
			window.nextedit = TINDEX;			/* nÑchstes Editobjekt	*/
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
				config.typ = GIF87A;
				config.interlace = 0;
				config.transparent = 0;
				config.tindexok = 0;
			}

			smurf_struct->module_mode = M_WAITING;

			break;

	/* Buttonevent */
		case MBEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				config.transparent = atoi(win_form[TINDEX].ob_spec.tedinfo->te_ptext);
				if(config.transparent < 0 || config.transparent > 255)
				{
					form_alert(1, "[3][Wrong transparent index][OK]");
					smurf_struct->module_mode = M_WAITING;
					break;
				}

				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&config;
				smurf_struct->event_par[2] = (int)sizeof(CONFIG);

				smurf_struct->module_mode = M_MOREOK;
			}
			else
			if(Button == SAVE)
			{
				config.transparent = atoi(win_form[TINDEX].ob_spec.tedinfo->te_ptext);

				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&config;
				smurf_struct->event_par[2] = (unsigned int)sizeof(CONFIG);

				smurf_struct->module_mode = M_CONFSAVE;
			}
			else
			{
				if(Button == LINE_ARRANGE)
						config.interlace ^= 1;
					else				
						if(Button == GIF87A || Button == GIF89A)
						{
							config.typ = (char)Button;

							if(config.typ == GIF87A)
							{
								win_form[TINDEX].ob_state |= DISABLED;
								win_form[TINDEXOK].ob_state |= DISABLED;
							}
							else
							{
								if(config.tindexok)
									win_form[TINDEX].ob_state &= ~DISABLED;
								win_form[TINDEXOK].ob_state &= ~DISABLED;
							}

							redraw_window(&window, NULL, TINDEX_BOX, 0);
						}
						else
							if(Button == TINDEXOK)
							{
								config.tindexok ^= 1;

								if(config.tindexok)
									win_form[TINDEX].ob_state &= ~DISABLED;
								else
									win_form[TINDEX].ob_state |= DISABLED;

								redraw_window(&window, NULL, TINDEX, 0);
							}

				smurf_struct->module_mode = M_WAITING;
			}

			break;

	/* Keyboardevent */
		case MKEVT:
			Button = smurf_struct->event_par[0];

			if(Button == OK)
			{
				config.transparent = atoi(win_form[TINDEX].ob_spec.tedinfo->te_ptext);
				if(config.transparent < 0 || config.transparent > 255)
				{
					form_alert(1, "[3][Wrong transparent index][OK]");
					smurf_struct->module_mode = M_WAITING;
					break;
				}

				/* Konfig Åbergeben */
				*(long *)&smurf_struct->event_par[0] = (long)&config;
				smurf_struct->event_par[2] = (int)sizeof(CONFIG);

				smurf_struct->module_mode = M_MOREOK;
			}
			else
				smurf_struct->module_mode = M_WAITING;

			break;

	/* Extender wird vom Smurf erfragt */
		case MEXTEND:
			smurf_struct->event_par[0] = 1;

			smurf_struct->module_mode = M_EXTEND;
			
			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			/* Zeiger initialisieren */
			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));
			buffer = smurf_struct->smurf_pic->pic_data;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			BitsPerPixel = smurf_struct->smurf_pic->depth;
			comment = smurf_struct->smurf_pic->infotext;

			/* Header + Screen Descriptor + globale Farbtabelle + Graphic Control Extension + */
			/* Comment Extension + Image Descriptor + Trailer */
			headlen = 6 + _LSCRDES + (int)(1 << BitsPerPixel) * 3 + 8 + (4 + (int)strlen(comment) + 1) + _IMAGE_DES + 1;

			if(BitsPerPixel == 8)
				length = (long)width * (long)height * 2 / 3;		/* sollte an sich ausreichen */
			else
				length = (long)((width + 7) / 8 * BitsPerPixel) * (long)height;

			file = ziel = (char *)SMalloc(headlen + length/* + (length / 250) * 2*/);
			if(ziel == NULL)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}

/*			printf("est headlen: %u, Malloc: %lu\n", headlen, headlen + length/* + (length / 250) * 2*/); */

			current_file = write_header(file, config.typ);
			current_file = write_lscrdes(current_file, smurf_struct->smurf_pic->palette, width, height, BitsPerPixel);
			current_file = write_graphctrl_ext(current_file, &config, 0);
			/* Bildblock */
			current_file = write_comment_ext(current_file, &config, comment);
			current_file = write_image_descriptor(current_file, smurf_struct->smurf_pic->palette, width, height, BitsPerPixel, config.interlace);
/*			printf("real header: %lu\n", current_file - file); */


			lacetab = (int *)malloc(height * sizeof(int));
			interlace(height, &config);

			if(BitsPerPixel == 8)
				current_file = encode_lzw_8bit(buffer, current_file, width, height);
			else
				current_file = encode_lzw_17bit(buffer, current_file, width, height, BitsPerPixel);

			if(current_file == NULL)
			{
				SMfree(file);
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			/* Trailer */
			*current_file++ = 0x3b;

			free(lacetab);

			f_len = current_file - file;

/*			printf("f_len: %lu\n", f_len); */

			Mshrink(0, ziel, f_len);
			exp_pic->pic_data = ziel;
			exp_pic->f_len = f_len;
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


/*-------------------------------------------------------------	*/
/*							Header schreiben					*/
/*-------------------------------------------------------------	*/
char *write_header(char *file, char typ)
{
	if(typ == GIF87A)
		strncpy(file, "GIF87a", 6);
	else
		strncpy(file, "GIF89a", 6);

	return(file + 6);
} /* write_header */


/*-------------------------------------------------------------	*/
/*		Graphics Control Extension Block schreiben  [optional]	*/
/*-------------------------------------------------------------	*/
char *write_graphctrl_ext(char *file, CONFIG *config, int delay)
{
	/* Wird momentan nur geschrieben wenn Transparent benîtigt wird */
	/* und auch verfÅgbar ist (nur GIF89a) */
	/* SpÑter auch bei Animationen */
	if(config->tindexok && config->typ == GIF89A)	/* Graphics Control Extension */	
	{
		*file = 0x21;					/* Extension Introducer */
		*(file + 1) = 0xf9;				/* Comment Label */
		*(file + 2) = 4;				/* remain fields size */
		/* Packed Fields: 0 = Transparente Farbe? */
		/*				  1 = User Input Flag */
		/*				  2-4 = Disposal Method */
		/*				  5-7 = reserviert */
		*(file + 3) = (0 << 4)|(0 << 1)|(config->tindexok&0x01);
		*(unsigned int *)(file + 4) = swap_word(delay);	/* Delay Time */
		*(file + 6) = (char)config->transparent;		/* Transparent Index */
		*(file + 7) = 0;								/* Block Terminator */

		return(file + 3 + *(file + 2) + 1);
	}
	else
		return(file);
} /* write_graphctrl_ext */


/*-------------------------------------------------------------	*/
/*			Comment Extension Block schreiben  [optional]		*/
/*-------------------------------------------------------------	*/
char *write_comment_ext(char *file, CONFIG *config, char *comment)
{
	if(strlen(comment) && config->typ == GIF89A)		/* Comment Extension */
	{
		*file = 0x21;					/* Extension Introducer */
		*(file + 1) = 0xfe;				/* Comment Label */
		*(file + 2) = strlen(comment) + 1; /* KommentarlÑnge */
		strcpy(file + 3, comment); 		/* Kommentar (nullterminiert) */
		*(file + 3 + *(file + 2)) = 0;	/* Block Terminator */

		return(file + 3 + *(file + 2) + 1);
	}
	else
		return(file);

} /* write_comment_ext */


/*-------------------------------------------------------------	*/
/*				Logical Screen Descriptor schreiben				*/
/*-------------------------------------------------------------	*/
char *write_lscrdes(char *file, char *pal, unsigned int width, unsigned int height, char BitsPerPixel)
{
	unsigned int coltab_size = 0;

	*(unsigned int *)file = swap_word(width);			/* Breite */
	*(unsigned int *)(file + 2) = swap_word(height);	/* Hîhe */
	/* Packed Fields: 0-2 = Bits pro Pixel - 1 */
	/*				  3 = 0 Sortierung nach Farbe */
	/*				  4-6 = Bits pro Farbe - 1 (immer 7) */
	/*				  7 = 1 globale Farbtabelle vorhanden */
	*(file + 4) = (1 << 7)|(7 << 4)|(0 << 3)|(BitsPerPixel - 1);
	*(file + 5) = 0;									/* Hintergrundfarbe */
	*(file + 6) = 0;									/* PixelseitenverhÑltnis */
	
	/* Global Color Table? */
	if(*(file + 4) & 0x80)
	{
		coltab_size = 1 << BitsPerPixel;

		memcpy(file + _LSCRDES, pal, 3 * coltab_size);

#if DEBUG
		printf("\n  Size: %d", coltab_size);
#endif
	}

	return(file + _LSCRDES + 3 * coltab_size);
} /* write_lscrdes */


/*-------------------------------------------------------------	*/
/*			Local Image Descriptor schreiben   [min. 1]			*/
/*-------------------------------------------------------------	*/
char *write_image_descriptor(char *file, char *pal, unsigned int width, unsigned int height, char BitsPerPixel, char interlace)
{
	unsigned int coltab_size = 0;


	*file = 0x2c;
	*(unsigned int *)(file + 1) = 0;					/* X-Pos */
	*(unsigned int *)(file + 3) = 0;					/* Y-Pos */
	*(unsigned int *)(file + 5) = swap_word(width);		/* Breite */
	*(unsigned int *)(file + 7) = swap_word(height);	/* Hîhe */
	/* Packed Fields: 0-2 = Bits pro Pixel - 1 */
	/*				  3-4 reserviert */
	/*				  5 = 0 Sortierung nach Farbe */
	/*				  6 = interlace */
	/*				  7 = 0 lokale Farbtabelle nicht vorhanden */
	*(file + 9) = (0 << 7)|(interlace << 6)|(0 << 5)|(BitsPerPixel - 1);

	/* local color table */
	if(*(file + 9) & 0x80)
	{
		coltab_size = 1 << BitsPerPixel;

		memcpy(file + _LSCRDES, pal, 3 * coltab_size);
#if DEBUG
		printf("\n  Size: %d", coltab_size);
#endif
	}

	return(file + _IMAGE_DES + 3 * coltab_size);
} /* write_image_descriptor */


/* Sortiert ein Bild von normaler Zeilenreihenfolge */
/* in place in interlaced Zeilenreihenfolge um */
void interlace(unsigned int height, CONFIG *config)
{
	int k, l;


/*
#if TIMER
/* wie schnell sind wir? */
	init_timer();
#endif
*/

	if(config->interlace)
	{
		reset_busybox(128, "interlace GIF");

		for(l = 0, k = 0; l < height; l += 8, k++)
			lacetab[k] = l;
		for(l = 4; l < height; l += 8, k++)
			lacetab[k] = l;
		for(l = 2; l < height; l += 4, k++)
			lacetab[k] = l;
		for(l = 1; l < height; l += 2, k++)
			lacetab[k] = l;
	}
	else
		for(k = 0; k < height; k++)
			lacetab[k] = k;


/*
#if TIMER
/* wie schnell waren wir? */
	printf("\nZeit: %lu", get_timer());
	getch();
#endif
*/

	return;
} /* interlace */


/*
/*-------------- Main-Function fÅr GIF-Programm zum Debuggen */
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

	filehandle = (int)Fcreate("K:\\AMBER_.GIF", 0);
	Fwrite(filehandle, exp_pic->f_len, exp_pic->pic_data);
	Fclose(filehandle);

	return;	
}
*/