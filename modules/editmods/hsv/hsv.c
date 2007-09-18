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
/*							HSV's mir						*/
/* Version 0.1  --  30.07.96								*/
/*	  TC-Bilder												*/
/* Version 0.2  --  06.09.96								*/
/*	  Palettenbilder										*/
/* Version 0.3  --  08.09.96								*/
/*	  Nun funktioniert es endlich richtig					*/
/* Version 0.4  --  04.03.98								*/
/*	  ... denkste. Zwei blîde Fehler bei der Berechnung des	*/
/*	  Maximums, bzw. des Minimums.							*/
/* Version 0.5  --  17.03.98								*/
/*	  In eigene Resource umgesetzt.							*/
/* Version 0.6  --  22.04.98 - 23.04.98						*/
/*	  HLS-System hinzugefÅgt.								*/
/* Version 0.7  --  23.04.98								*/
/*	  Relativmodus fÅr Hue jetzt doch eingebaut (nur HLS).	*/
/*	  öberlauf wenn zwei Farbteile den Maximalwert besaûen	*/
/*	  beseitigt.											*/
/* Version 0.8  --  28.08.98								*/
/*	  OberflÑchenkram: Bei Umschaltung zwischen HSV und HSL	*/
/*	  Slider, Beschriftungen und Bedeutungen vertauschen.	*/
/* Version 0.9  --  26.09.98								*/
/*	  öbermitteln und Empfangen sowie Laden und Speichern	*/
/*	  von Konfigurationen eingebaut.						*/
/* Version 1.0  --  5.10.98									*/
/*	  Setsat und setbright in do_HLS werden jetzt richtig-	*/
/*	  herum behandelt.										*/
/* Version 1.1  --  9.9.2000								*/
/*	  Unsinnige "Umschaltung" HSV auf HSV oder HLS auf HLS	*/
/*	  wird unterdrÅckt. Bug der die Umschaltung der			*/
/*	  Aktivierung der Slider fÅr SÑttigung und Luminanz bei	*/
/*	  der Umschaltung zwischen HSV->HLS und HLS->HSV		*/
/*	  verhindete behoben. Auûerdem bewegen sich die			*/
/*	  Relativwerte fÅr SÑttigung und Helligkeit nun im		*/
/*	  Bereich von -100 bis 100.								*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include <math.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <..\..\..\demolib.h>

#define ENGLISCH 0

#if ENGLISCH
	#include "hsv\en\hsv.rsh"
	#include "hsv\en\hsv.rh"
#else
	#include "hsv\de\hsv.rsh"
	#include "hsv\de\hsv.rh"
#endif

#define DEBUG 0
#define	TextCast	ob_spec.tedinfo->te_ptext

typedef struct
{
	long version;
	char mode;
	unsigned long slidcol, slidsat, slidbright;
	char setcol, setsat, setbright;
} CONFIG;

int handle_bevt(unsigned int Button);
int make_sliders(void);
int (*busybox)(int pos);
int	(*SMfree)(void *ptr);
void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);
void *(*mconfLoad)(MOD_INFO *modinfo, int mod_id, char *name);
void (*mconfSave)(MOD_INFO *modinfo, int mod_id, void *confblock, long len, char *name);
SERVICE_FUNCTIONS *service;

void save_setting(void);
void load_setting(void);
void apply_setting(CONFIG *myConfig);
void write_setting(CONFIG *myConfig);

void do_HSV(char *data, unsigned int width, unsigned int height, unsigned long colval, unsigned long satval, unsigned long brightval);
void do_HLS(char *data, unsigned int width, unsigned int height, unsigned long colval, unsigned long lumval, unsigned long satval);

MOD_INFO module_info = {"HSV's mir",
						0x0110,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						"",
						0, 64,
						0, 64,
						0, 64,
						0, 64,
						0, 10,
						0, 10,
						0, 10,
						0, 10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						1
						};

MOD_ABILITY module_ability = {
						2, 4, 7, 8, 24,
						0, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

char setcol = 'a', setsat = ' ', setbright = ' ', mode;

int module_id;

long slidcol = 0, slidsat = 0, slidbright = 0;

SLIDER colsl, satsl, brsl;

static WINDOW window;
static OBJECT *win_form;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*						HSV's mir					*/
/*  Diverse Spielereien im HSV-System.				*/
/*	Recht schîn aber schweinisch langsam.			*/
/*		2 - 8 Bit, 24 Bit							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/

void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 BitsPerPixel, t;
	char wt[] = "HSV's mir";

	int Button;

	unsigned int width, height, colval = 0;
	
	unsigned long satval = 0, brightval = 0;

	CONFIG *config;


	service = smurf_struct->services;

/* Wenn das Modul zum ersten Mal gestartet wurde */
	switch(smurf_struct->module_mode)
	{
		case MSTART:
			win_form = rs_trindex[HSVS_MIR];

			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			module_id = smurf_struct->module_number;
			SMfree = smurf_struct->services->SMfree;
			redraw_window = service->redraw_window;
			mconfLoad = service->mconfLoad;
			mconfSave = service->mconfSave;

			window.whandlem = 0;				/* evtl. Handle lîschen */
			window.module = smurf_struct->module_number;	/* ID in die Fensterstruktur eintragen  */
			window.wnum = 1;					/* Fenster nummer 1...  */
			window.wx = -1;						/* Fenster X- ...    	*/
			window.wy = -1;						/* ... und Y-Pos     	*/
			window.ww = win_form->ob_width;		/* Fensterbreite    	*/
			window.wh = win_form->ob_height;	/* Fensterhîhe      	*/
			strcpy(window.wtitle, wt);			/* Titel reinkopieren   */
			window.resource_form = win_form;	/* Resource         	*/
			window.picture = NULL;				/* kein Bild	      	*/ 
			window.editob = 0;					/* erstes Editobjekt	*/
			window.nextedit = 0;				/* nÑchstes Editobjekt	*/
			window.editx = 0;

			*(char *)&rs_object[CHECK_H].ob_spec.tedinfo = 'a';		/* auf 'a' initialisieren */
			*(char *)&rs_object[CHECK_S].ob_spec.tedinfo = ' ';		/* auf ' ' initialisieren */
			*(char *)&rs_object[CHECK_V].ob_spec.tedinfo = ' ';		/* auf ' ' initialisieren */
			mode = _HSV;

			make_sliders();

			smurf_struct->wind_struct = &window;  /* und die Fensterstruktur in die Gargamel */

			if(smurf_struct->services->f_module_window(&window) == -1)			/* Gib mir 'n Fenster! */
				smurf_struct->module_mode = M_EXIT;		/* keins mehr da? */
			else 
				smurf_struct->module_mode = M_WAITING;	/* doch? Ich warte... */

			break;

		/* Buttonevent */
		case MBEVT:
			smurf_struct->module_mode = handle_bevt(smurf_struct->event_par[0]);
			return;

		/* Keyboardevent */
		case MKEVT:
			Button = smurf_struct->event_par[0];

			if(Button == START)
			{
				smurf_struct->module_mode = M_STARTED;
				return;
			}

			break;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */
			busybox = smurf_struct->services->busybox;

			colval = (unsigned int)slidcol;						/* Absolutwert */

			if(mode == _HSV)
			{
				if(setsat == 'a')
					satval = 1024 * slidsat / 255;					/* Absolutwert */
				else
					satval = (slidsat + 100) * 256 / 100;			/* Relativwert */

				if(setbright == 'a')
					brightval = slidbright;							/* Absolutwert */
				else
					brightval = (slidbright + 100) * 256 / 100;		/* Relativwert */
			}
			else
			{
				if(setsat == 'a')
					satval = 1024 * slidbright / 255;				/* Absolutwert */
				else
					satval = (slidbright + 100) * 256 / 100;		/* Relativwert */

				if(setbright == 'a')
					brightval = slidsat << 2L;						/* Absolutwert */
				else
					brightval = (slidsat + 100) * 256 / 100;		/* Relativwert */
			}

#if DEBUG
			Cur_home();
			printf("colval: %d, satval: %lu, brightval: %lu\n", colval, satval, brightval);
#endif

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			if(BitsPerPixel == 24)
			{
				data = smurf_struct->smurf_pic->pic_data;

				width = smurf_struct->smurf_pic->pic_width;
				height = smurf_struct->smurf_pic->pic_height;
			}
			else
			{
				data = smurf_struct->smurf_pic->palette;

				width = 256;
				height = 1;
			}

			if(mode == _HSV)
				do_HSV(data, width, height, colval, satval, brightval);
			else
				do_HLS(data, width, height, colval, brightval, satval);

/* wie schnell waren wir? */
/*	printf("\n%lu", get_timer());
	getch(); */

			smurf_struct->module_mode = M_PICDONE;
			break;

		case GETCONFIG:	config = smurf_struct->services->SMalloc(sizeof(CONFIG));
						write_setting(config);
						smurf_struct->event_par[0] = (int)((unsigned long)config >> 16);
						smurf_struct->event_par[1] = (int)config;
						smurf_struct->event_par[2] = (int)sizeof(CONFIG);
						smurf_struct->module_mode = M_CONFIG;
						break;

		case CONFIG_TRANSMIT:	config = (CONFIG *)(((unsigned long)smurf_struct->event_par[0] << 16)|((unsigned long)smurf_struct->event_par[1]&0xffff));
								apply_setting(config);
								smurf_struct->module_mode = M_WAITING;
								break;

		/* Mterm empfangen - Speicher freigeben und beenden */
		case MTERM:	smurf_struct->module_mode = M_EXIT;
					break;
	}

	return;
}


void do_HSV(char *data, unsigned int width, unsigned int height, unsigned long colval, unsigned long satval, unsigned long brightval)
{
	char min, max, delta, k;

	int h, hnew, r, g, b;
	unsigned int i, j, bh, bl;

	unsigned long s, v;


	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */

	hnew = (int)((colval << 10) / 60);		/* colval ist max. 360 also ist hnew max. 6144 */

#if DEBUG
printf("colval: %lu, hnew: %d\n", colval, hnew);
getch();
#endif


	for(i = 0; i < height; i++)
	{
		if(!(i%bh))
		{
			busybox(bl);
			bl += 12;
		}

		for(j = 0; j < width; j++)
		{
			/* Werte einlesen */
		    r = (int)*data;
		    g = (int)*(data + 1);
		    b = (int)*(data + 2);

			/* RGB -> HSV */
			/* Maximum finden */
			if(r >= g)
				if(r >= b)
					max = r;
				else
					max = b;
			else
				if(g >= b)
					max = g;
				else
					max = b;

			/* Minimum finden */
			if(r < g)
				if(r < b)
					min = r;
				else
					min = b;
			else
				if(g < b)
					min = g;
				else
					min = b;

#if DEBUG
printf("r: %u, g: %u, b: %u, max: %u, min: %u\n", (unsigned int)r, (unsigned int)g, (unsigned int)b, (unsigned int)max, (unsigned int)min);
getch();
#endif

			v = max;

			/* Ist s == 0, ist h undefiniert -> Schleife braucht nicht betreten werden */
			if(max)
			{
				delta = max - min;

				s = ((unsigned long)delta << 10L) / max;

			    if(r == max)
			        h = (int)(((long)(g - b) << 10L) / delta);				/* Farbe ist zwischen Gelb und Magenta */
			    else
			        if(g == max)
			            h = 2048 + (int)(((long)(b - r) << 10L) / delta);	/* Farbe ist zwischen Cyan und Gelb */
			        else
		                h = 4096 + (int)(((long)(r - g) << 10L) / delta);	/* Farbe ist zwischen Magenta und Cyan */

				if(h >= 0)
				{
					if(h == 6144)											/* 360 Grad */
						h = 0;
				}
				else
					h += 6144;												/* 360 Grad */
			}
			else
			{
				s = 0;
				h = 0;
			}

#if DEBUG
printf("h: %d, s: %lu, v: %u\n", h, s, (unsigned int)v);
#endif					

			if(s || setsat != ' ')
			{					
				if(setcol == 'a')
					h = hnew;
				else
					if(setcol == 'r')
					{
						h += hnew;
						if(h > 6144)							/* Åber 360 Grad */
							h -= 6144;							/* 360 Grad runter */
					}

				if(setsat == 'a')
					s = satval;
				else
					if(setsat == 'r')
					{
						/* Prozentwert auf s */
					    s = (s * satval) >> 8;
					    if(s > 1024)
					    	s = 1024;
					}

				if(setbright == 'a')
					v = brightval;
				else
					if(setbright == 'r')
					{
						/* Prozentwert auf v */
					    v = (v * brightval) >> 8;
					    if(v > 255)
					    	v = 255;
					}

				k = (char)(h >> 10);							/* durch 60 Grad */
				h &= 1023;										/* %1024 entspr. %60 Grad */

#if DEBUG
printf("h: %d, k: %d, s: %lu, v: %u\n\n", h, (int)k, s, (unsigned int)v);
#endif

				/* HSV -> RGB */
			    switch(k)
			    {
					case 6:
					case 0:	r = v;
							g = (v * (1048576L - (s * (1024 - h)))) >> 20;
							b = (v * (1024 - s)) >> 10;
			                break;
					case 1: r = (v * (1048576L - (s * h))) >> 20;
							g = v;
							b = (v * (1024 - s)) >> 10;
			                break;
					case 2: r = (v * (1024 - s)) >> 10;
							g = v;
			                b = (v * (1048576L - (s * (1024 - h)))) >> 20;
			                break;
					case 3: r = (v * (1024 - s)) >> 10;
			                g = (v * (1048576L - (s * h))) >> 20;
			                b = v;
			                break;
					case 4:	r = (v * (1048576L - (s * (1024 - h)))) >> 20;
							g = (v * (1024 - s)) >> 10;
			                b = v;
			                break;
					case 5:	r = v;
							g = (v * (1024 - s)) >> 10;
			                b = (v * (1048576L - (s * h))) >> 20;
			                break;
			    }

#if DEBUG
printf("r: %u, g: %u, b: %u\n", (unsigned int)r, (unsigned int)g, (unsigned int)b);
getch();
#endif

		/* Werte zurÅckschreiben */
			    *data++ = (char)r;
			    *data++ = (char)g;
			    *data++ = (char)b;
			}
			else
				data += 3;
		} /* j */
	} /* i */

	return;
} /* do_HSV */


void do_HLS(char *data, unsigned int width, unsigned int height, unsigned long colval, unsigned long lumval, unsigned long satval)
{
	char min, max, delta;

	int r, g, b;
	unsigned int i, j, sum, bh, bl;

	long h, hnew, hue;
	long s, l, m1, m2;


	if((bh = height / 10) == 0) 	/* busy-height */
		bh = height;
	bl = 0;							/* busy-length */


	hnew = colval << 10;

	if(lumval > 1020)
		lumval = 1020;

	for(i = 0; i < height; i++)
	{
		if(!(i%bh))
		{
			busybox(bl);
			bl += 12;
		}
		
		for(j = 0; j < width; j++)
		{
			/* Werte einlesen */
		    r = (int)*data;
		    g = (int)*(data + 1);
		    b = (int)*(data + 2);

			/* RGB -> HLS */
			/* Maximum finden */
			if(r >= g)
				if(r >= b)
					max = r;
				else
					max = b;
			else
				if(g >= b)
					max = g;
				else
					max = b;

			/* Minimum finden */
			if(r < g)
				if(r < b)
					min = r;
				else
					min = b;
			else
				if(g < b)
					min = g;
				else
					min = b;

#if DEBUG
printf("r: %u, g: %u, b: %u, max: %u, min: %u\n", (unsigned int)r, (unsigned int)g, (unsigned int)b, (unsigned int)max, (unsigned int)min);
getch();
#endif

			sum = max + min;

			l = (long)sum << 1L;		/* bei Weiû ist l == 1, also 1020 */

			/* Ist max == min, ist s == 0 und ist h undefiniert -> Schleife braucht nicht betreten werden */
			if(max == min)
			{
				s = 0;
				h = 0;
			}
			else
			{
				delta = max - min;

				s = (l <= 510) ? (((unsigned long)delta << 10L) / sum) : (((unsigned long)delta << 10L) / (510 - sum));

			    if(r == max)
			        h = ((long)(g - b) << 10L) / delta;				/* Farbe ist zwischen Gelb und Magenta */
			    else
			        if(g == max)
			            h = 2048 + ((long)(b - r) << 10L) / delta;	/* Farbe ist zwischen Cyan und Gelb */
			        else
		                h = 4096 + ((long)(r - g) << 10L) / delta;	/* Farbe ist zwischen Magenta und Cyan*/

				h *= 60L;

				if(h >= 0)
				{
					if(h == 368640L)
						h = 0;
				}
				else
					h += 368640L;
			}

#if DEBUG
printf("h: %ld, l: %ld, s: %ld\n", h, l, s);
#endif					

			if(s || setsat != ' ')
			{					
				if(setcol == 'a')
					h = hnew;
				else
					if(setcol == 'r')
					{
						h += hnew;
						if(h > 368640L)
							h -= 368640L;
					}

				if(setbright == 'a')
					s = satval;
				else
					if(setbright == 'r')
					{
						/* Prozentwert auf s */
					    s = (s * satval) >> 8;
					    if(s > 1024)
					    	s = 1024;
					}

				if(setsat == 'a')
					l = lumval;
				else
					if(setsat == 'r')
					{
						/* Prozentwert auf l */
					    l = (l * lumval) >> 8;
					    if(l > 1020)
					    	l = 1020;
					}

#if DEBUG
printf("h: %ld, l: %ld, s: %ld\n", h, l, s);
#endif

				/* HLS -> RGB */
				if(s == 0)
					r = g = b = l >> 2;
				else
				{
					m2 = (l <= 510) ? (l * (1024 + s)) : (((l + s) << 10) - l * s);
					m1 = ((2 * l) << 10) - m2;

#if DEBUG
printf("m1: %ld, m2: %ld\n", m1, m2);
#endif

					hue = h + 122880L;
					if(hue > 368640L)
						hue -= 368640L;
					if(hue < 61440L)
						r = (m1 + (m2 - m1) / 61440L * hue + 500) >> 12;
					else
						if(hue < 184320L)
							r = m2 >> 12;
						else
							if(hue < 245760L)
								r = (m1 + (m2 - m1) / 61440L * (245760L - hue) + 500) >> 12;
							else
								r = m1 >> 12;

					hue = h;
					if(hue < 61440L)
						g = (m1 + (m2 - m1) / 61440L * hue + 500) >> 12;
					else
						if(hue < 184320L)
							g = m2 >> 12;
						else
							if(hue < 245760L)
								g = (m1 + (m2 - m1) / 61440L * (245760L - hue) + 500) >> 12;
							else
								g = m1 >> 12;

					hue = h - 122880L;
					if(hue < 0)
						hue += 368640L;
					if(hue < 61440L)
						b = (m1 + (m2 - m1) / 61440L * hue + 500) >> 12;
					else
						if(hue < 184320L)
							b = m2 >> 12;
						else
							if(hue < 245760L)
								b = (m1 + (m2 - m1) / 61440L * (245760L - hue) + 500) >> 12;
							else
								b = m1 >> 12;
				}
								

#if DEBUG
printf("r: %u, g: %u, b: %u\n\n", (unsigned int)r, (unsigned int)g, (unsigned int)b);
getch();
#endif

				/* Werte zurÅckschreiben */
			    *data++ = (char)r;
			    *data++ = (char)g;
			    *data++ = (char)b;
			}
			else
				data += 3;
		} /* j */
	} /* i */

	return;
} /* do_HLS */



int handle_bevt(unsigned int Button)
{
	int ret = M_WAITING, temp;

	
	switch(Button)
	{
		case START:	ret = M_STARTED;
					break;

		case S4_S:	slidcol = (long)service->slider(&colsl);
					break;
		case S5_S:	slidsat = (long)service->slider(&satsl);
					break;
		case S6_S:	slidbright = (long)service->slider(&brsl);
					break;

		case CHECK_H:	if(setcol == ' ')
							setcol = 'a';
						else
							if(setcol == 'a')
								setcol = 'r';
							else
								setcol = ' ';
						*(char *)&rs_object[CHECK_H].ob_spec.tedinfo = setcol;
						redraw_window(&window, NULL, CHECK_H, 0);
						make_sliders();
						service->set_slider(&colsl, slidcol);
						break;

		case CHECK_S:/*	printf("slidsat: %lu\n", slidsat); */
						if(setsat == ' ')
							setsat = 'a';
						else
							if(setsat == 'a')
								setsat = 'r';
							else
								setsat = ' ';
						if(setsat == 'a')
							/* von relativ nach absolut umrechnen, inklusive aufrunden */
							slidsat = ((slidsat + 100) * 255 + 100) / 200;
						else
							if(setsat == 'r')
								/* von absolut nach relativ umrechnen, inklusive aufrunden */
								slidsat = (slidsat * 200 + 127) / 255 - 100;

/*						printf("slidsat: %lu\n", slidsat); */
						*(char *)&rs_object[CHECK_S].ob_spec.tedinfo = setsat;
						redraw_window(&window, NULL, CHECK_S, 0);
						make_sliders();
						service->set_slider(&satsl, slidsat);
						break;

		case CHECK_V:/*	printf("slidbright: %lu\n", slidbright); */
						if(setbright == ' ')
							setbright = 'a';
						else
							if(setbright == 'a')
								setbright = 'r';
							else
								setbright = ' ';
						if(setbright == 'a')
							/* von relativ nach absolut umrechnen, inklusive aufrunden */
							slidbright = ((slidbright + 100) * 255 + 100) / 200;
						else
							if(setbright == 'r')
								/* von absolut nach relativ umrechnen, inklusive aufrunden */
								slidbright = (slidbright * 200 + 127) / 255 - 100;

/*						printf("slidbright: %lu\n", slidbright); */
						*(char *)&rs_object[CHECK_V].ob_spec.tedinfo = setbright;
						redraw_window(&window, NULL, CHECK_V, 0);
						make_sliders();
						service->set_slider(&brsl, slidbright);
						break;
		case _HSV:		
		case _HLS:		if(mode != Button)
						{						
							mode = Button;

							if(Button == _HSV)
							{
								strcpy(rs_object[COMPONENT2].ob_spec.tedinfo->te_ptext, "S");
								strcpy(rs_object[COMPONENT3].ob_spec.tedinfo->te_ptext, "V");
							}
							else
							{
								strcpy(rs_object[COMPONENT2].ob_spec.tedinfo->te_ptext, "L");
								strcpy(rs_object[COMPONENT3].ob_spec.tedinfo->te_ptext, "S");
							}

							redraw_window(&window, NULL, COMPONENT2, 0);
							redraw_window(&window, NULL, COMPONENT3, 0);

							temp = slidsat;
							slidsat = slidbright;
							slidbright = temp;

							temp = setsat;
							setsat = setbright;
							setbright = temp;

							make_sliders();

							*(char *)&rs_object[CHECK_S].ob_spec.tedinfo = setsat;
							redraw_window(&window, NULL, CHECK_S, 0);
							*(char *)&rs_object[CHECK_V].ob_spec.tedinfo = setbright;
							redraw_window(&window, NULL, CHECK_V, 0);
							service->set_slider(&satsl, slidsat);
							service->set_slider(&brsl, slidbright);
						}
						break;
		case MCONF_LOAD:	load_setting();
							break;

		case MCONF_SAVE:	save_setting();
							break;
	}
	
	return(ret);
} /* handle_bevt */


int make_sliders(void)
{
	colsl.regler = S4_S;
	colsl.schiene = S4_F; 
	colsl.rtree = win_form;
	colsl.txt_obj = S4_E;
	colsl.min_val = 0;
	colsl.max_val = 359;
	colsl.window = &window;

	if(setcol == ' ')
	{
		colsl.rtree[S4_F].ob_state |= DISABLED;
		colsl.rtree[S4_S].ob_state |= DISABLED;
		colsl.rtree[S4_E].ob_state |= DISABLED;
	}
	else
	{
		colsl.rtree[S4_F].ob_state &= ~DISABLED;
		colsl.rtree[S4_S].ob_state &= ~DISABLED;
		colsl.rtree[S4_E].ob_state &= ~DISABLED;
	}

	satsl.regler = S5_S;
	satsl.schiene = S5_F; 
	satsl.rtree = win_form;
	satsl.txt_obj = S5_E;
	if(setsat == 'a')
	{
		satsl.min_val = 0;
		satsl.max_val = 255;
	}
	else
	{
		satsl.min_val = -100;
		satsl.max_val = 100;
	}
	satsl.window = &window;

	if(setsat == ' ')
	{
		satsl.rtree[S5_F].ob_state |= DISABLED;
		satsl.rtree[S5_S].ob_state |= DISABLED;
		satsl.rtree[S5_E].ob_state |= DISABLED;
	}
	else
	{
		satsl.rtree[S5_F].ob_state &= ~DISABLED;
		satsl.rtree[S5_S].ob_state &= ~DISABLED;
		satsl.rtree[S5_E].ob_state &= ~DISABLED;
	}

	brsl.regler = S6_S;
	brsl.schiene = S6_F; 
	brsl.rtree = win_form;
	brsl.txt_obj = S6_E;
	if(setbright == 'a')
	{
		brsl.min_val = 0;
		brsl.max_val = 255;
	}
	else
	{
		brsl.min_val = -100;
		brsl.max_val = 100;
	}
	brsl.window = &window;

	if(setbright == ' ')
	{
		brsl.rtree[S6_F].ob_state |= DISABLED;
		brsl.rtree[S6_S].ob_state |= DISABLED;
		brsl.rtree[S6_E].ob_state |= DISABLED;
	}
	else
	{
		brsl.rtree[S6_F].ob_state &= ~DISABLED;
		brsl.rtree[S6_S].ob_state &= ~DISABLED;
		brsl.rtree[S6_E].ob_state &= ~DISABLED;
	}

	return(0);
} /* make_sliders */


void load_setting(void)
{
	char name[33];

	CONFIG *myConfig;


	memset(name, 0x0, 33);

	if((myConfig = mconfLoad(&module_info, module_id, name)) != NULL)
	{
		apply_setting(myConfig);	
		SMfree(myConfig);
	}

	return;
} /* load_setting */


void save_setting(void)
{
	char name[33];

	CONFIG myConfig;


	write_setting(&myConfig);
	
	memset(name, 0x0, 33);
	mconfSave(&module_info, module_id, &myConfig, sizeof(CONFIG), name);

	return;
} /* save_setting */


void apply_setting(CONFIG *myConfig)
{
	mode = myConfig->mode;

	setcol = myConfig->setcol;
	setsat = myConfig->setsat;
	setbright = myConfig->setbright;

	slidcol = myConfig->slidcol;
	slidsat = myConfig->slidsat;
	slidbright = myConfig->slidbright;

	make_sliders();

	*(char *)&rs_object[CHECK_H].ob_spec.tedinfo = setcol;
	redraw_window(&window, NULL, CHECK_H, 0);
	service->set_slider(&colsl, slidcol);

	*(char *)&rs_object[CHECK_S].ob_spec.tedinfo = setsat;
	redraw_window(&window, NULL, CHECK_S, 0);
	service->set_slider(&satsl, slidsat);

	*(char *)&rs_object[CHECK_V].ob_spec.tedinfo = setbright;
	redraw_window(&window, NULL, CHECK_V, 0);
	service->set_slider(&brsl, slidbright);

	win_form[_HSV].ob_state &= ~SELECTED;
	win_form[_HLS].ob_state &= ~SELECTED;
	win_form[mode].ob_state |= SELECTED;
	redraw_window(&window, NULL, SYSTEM_BOX, 0);

	return;
} /* apply_setting */


void write_setting(CONFIG *myConfig)
{
	myConfig->version = 0x0090;

	myConfig->mode = mode;

	myConfig->setcol = setcol;
	myConfig->setsat = setsat;
	myConfig->setbright = setbright;

	myConfig->slidcol = slidcol;
	myConfig->slidsat = slidsat;
	myConfig->slidbright = slidbright;

	return;
} /* write_setting */