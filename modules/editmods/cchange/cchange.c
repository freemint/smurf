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
/*						Colorchange-Modul					*/
/* Version 0.1  --  18.05.98								*/
/*	  24 Bit												*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <..\..\..\demolib.h>

#include "cchange.rsh"
#include "cchange.rh"

typedef struct
{
	unsigned int index;
	char r;
	char g;
	char b;
} PIXEL;

void do_24Bit(SMURF_PIC *smurf_pic, PIXEL *dst, PIXEL *src);
void do_16Bit(SMURF_PIC *smurf_pic, PIXEL *dst, PIXEL *src);
void do_pal(SMURF_PIC *smurf_pic, PIXEL *dst, PIXEL *src);
void evaluate_color(SMURF_PIC *smurf_pic, PIXEL *dst, PIXEL *src, unsigned int sx, unsigned int sy, unsigned int dx, unsigned int dy, int type);
void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);
int setpix_std_line(char *buf, char *dest, int depth, long planelen, int howmany);

void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);

MOD_INFO module_info = {"Farbe „ndern",
						0x0010,
						"Christian Eyrich",
						"", "", "", "", "",
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
						0,64,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						2, 4, 7, 8, 16,
						24, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

static WINDOW window;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Colorchange						*/
/*	Žndert eine per Fadenkreuz angegebene Farbe in	*/
/*	eine andere ebenfalls per Fadenkreuz anzugebende*/
/*	Farbe.											*/
/*		1-8, 16 und 24 Bit 							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char t, BitsPerPixel;
	char wt[] = "Colorchange";
	static char Color;
	
	int Button;
	static unsigned int dx, dy, sx, sy;
	
	static PIXEL dst, src;
	static OBJECT *win_form;
	

/* wie schnell sind wir? */
/*	init_timer(); */

	switch(smurf_struct->module_mode)
	{
		/* Wenn das Modul zum ersten Mal gestartet wurde */
		case MSTART:	win_form = rs_trindex[COL_CHANGE];				/* Resourcebaum holen */
	
						/* Resource umbauen */
						for(t = 0; t < NUM_OBS; t++)
							rsrc_obfix(&rs_object[t], 0);

						redraw_window = smurf_struct->services->redraw_window;		/* Redrawfunktion */

						window.whandlem = 0;				/* evtl. Handle l”schen */
						window.module = smurf_struct->module_number;	/* ID in die Fensterstruktur eintragen  */
						window.wnum = 1;					/* Fenster nummer 1...  */
						window.wx = -1;						/* Fenster X- ...    	*/
						window.wy = -1;						/* ... und Y-Pos     	*/
						window.ww = win_form->ob_width;		/* Fensterbreite    	*/
						window.wh = win_form->ob_height;	/* Fensterh”he      	*/
						strcpy(window.wtitle, wt);			/* Titel reinkopieren   */
						window.resource_form = win_form;	/* Resource         	*/
						window.picture = NULL;				/* kein Bild.       	*/ 
						window.editob = 0;					/* erstes Editobjekt	*/
						window.nextedit = 0;				/* n„chstes Editobjekt	*/
						window.editx = 0;

						smurf_struct->wind_struct = &window;  /* und die Fensterstruktur in die Gargamel */

						/* Defaults bestcken */
						Color = DST;
						sx = smurf_struct->smurf_pic->pic_width / 2;
						sy = smurf_struct->smurf_pic->pic_height / 2;

						dx = smurf_struct->smurf_pic->pic_width / 2;
						dy = smurf_struct->smurf_pic->pic_height / 2;

						if(smurf_struct->services->f_module_window(&window) == -1)			/* Gib mir 'n Fenster! */
							smurf_struct->module_mode = M_EXIT;		/* keins mehr da? */
						else 
							smurf_struct->module_mode = M_WAITING;	/* doch? Ich warte... */

						break;

		/* Buttonevent */
		case MBEVT:	Button = smurf_struct->event_par[0];

					printf("Button: %u\n", Button);
					if(Button == DST || Button == SRC)
					{
						evaluate_color(smurf_struct->smurf_pic, &dst, &src, sx, sy, dx, dy, Button);
						Color = Button;
						smurf_struct->module_mode = M_WAITING;
					}
					else
						if(Button == START)
							smurf_struct->module_mode = M_STARTED;

					break;

		/* Keyboardevent */
		case MKEVT:	Button = smurf_struct->event_par[0];

					if(Button == START)
						smurf_struct->module_mode = M_STARTED;

					break;

		/*---- Smurf fragt: Fadenkreuz? */
		case MCROSSHAIR:	smurf_struct->event_par[0] = 1;
							smurf_struct->event_par[1] = 0;
							smurf_struct->module_mode = M_CROSSHAIR;

							break;

		/*--- Smurf fragt: und die Start-Koordinaten? */
		case MCH_DEFCOO:	if(Color == DST)
							{
								smurf_struct->event_par[0] = dx;
								smurf_struct->event_par[1] = dy;
							}
							else
							{
								smurf_struct->event_par[0] = sx;
								smurf_struct->event_par[1] = sy;
							}

							smurf_struct->module_mode = M_CHDEFCOO;

							break;

		/*--- Smurf sagt: hier sind die eingestellten Koordinaten */
		case MCH_COORDS:	if(Color == DST)
							{
								dx = smurf_struct->event_par[0];
								dy = smurf_struct->event_par[1];
							}
							else
							{
								sx = smurf_struct->event_par[0];
								sy = smurf_struct->event_par[1];
							}

							smurf_struct->module_mode = M_WAITING;

							break;

		case MEXEC:		BitsPerPixel = smurf_struct->smurf_pic->depth;

						if(BitsPerPixel == 24)
							do_24Bit(smurf_struct->smurf_pic, &dst, &src);
						else
							if(BitsPerPixel == 16)
								do_16Bit(smurf_struct->smurf_pic, &dst, &src);
							else
								if(BitsPerPixel <= 8)
									do_pal(smurf_struct->smurf_pic, &dst, &src);

		/* wie schnell waren wir? */
		/*	printf("%lu\n", get_timer());
			getch(); */

						smurf_struct->module_mode = M_DONEEXIT;

						break;

		/* Mterm empfangen - Speicher freigeben und beenden */
		case MTERM:	smurf_struct->module_mode = M_EXIT;

					break;
	}

	return;
}


void do_24Bit(SMURF_PIC *smurf_pic, PIXEL *dst, PIXEL *src)
{
	char *data,
		 dr, dg, db, sr, sg, sb;

	unsigned int x, y, width, height;


	width = smurf_pic->pic_width;
	height = smurf_pic->pic_height;
	data = smurf_pic->pic_data;

	dr = dst->r;
	dg = dst->g;
	db = dst->b;

	sr = src->r;
	sg = src->g;
	sb = src->b;

	y = 0;
	do
	{
		x = 0;
		do
		{
			if(*data == dr && *(data + 1) == dg && *(data + 2) == db)
			{
				*data++ = sr;
				*data++ = sg;
				*data++ = sb;
			}
			else
				data += 3;
		} while(++x < width);
	} while(++y < height);

	return;
} /* do_24Bit */


void do_16Bit(SMURF_PIC *smurf_pic, PIXEL *dst, PIXEL *src)
{
	unsigned int *data16,
				 x, y, width, height, dest, source;


	width = smurf_pic->pic_width;
	height = smurf_pic->pic_height;
	data16 = (unsigned int *)smurf_pic->pic_data;

	dest = dst->index;

	source = src->index;

	y = 0;
	do
	{
		x = 0;
		do
		{
			if(*data16 == dest)
				*data16++ = source;
			else
				data16++;
		} while(++x < width);
	} while(++y < height);

	return;
} /* do_16Bit */


void do_pal(SMURF_PIC *smurf_pic, PIXEL *dst, PIXEL *src)
{
	char *data, *pixbuf, *line,
		 BitsPerPixel, dindex, sindex;

	unsigned int x, y, width, height;

	long planelength;


	width = smurf_pic->pic_width;
	height = smurf_pic->pic_height;
	BitsPerPixel = smurf_pic->depth;
	data = smurf_pic->pic_data;

	planelength = (long)(width + 7) / 8 * (long)height;

	dindex = (char)dst->index;

	sindex = (char)src->index;

	if(smurf_pic->format_type == FORM_STANDARD)
		pixbuf = Malloc(width + 7);

	y = 0;
	do
	{
		if(smurf_pic->format_type == FORM_STANDARD)
		{
			getpix_std_line(data, pixbuf, BitsPerPixel, planelength, width);
			line = pixbuf;
		}
		else
			line = data;

		x = 0;
		do
		{
			if(*line == dindex)
				*line++ = sindex;
			else
				line++;
		} while(++x < width);

		if(smurf_pic->format_type == FORM_STANDARD)
			data += setpix_std_line(pixbuf, data, BitsPerPixel, planelength, width);
		else
			data += width;
	} while(++y < height);

	Mfree(pixbuf);

	return;
} /* do_pal */


void evaluate_color(SMURF_PIC *smurf_pic, PIXEL *dst, PIXEL *src, unsigned int sx, unsigned int sy, unsigned int dx, unsigned int dy, int type)
{
	char *data, *pal, *palette;

	unsigned int *data16,
				 width;

	long pos;


	width = smurf_pic->pic_width;
	data16 = (unsigned int *)data = smurf_pic->pic_data;

	if(smurf_pic->depth == 24)
	{
		pos = (long)sy * (long)width * 3L + (long)sx * 3L;
		printf("scoords: %u/%u, pos: %ld\n", sx, sy, pos);
		src->r = (char)*(data + pos);
		src->g = (char)*(data + pos + 1);
		src->b = (char)*(data + pos + 2);

		pos = (long)dy * (long)width * 3L + (long)dx * 3L;
		printf("dcoords: %u/%u, pos: %ld\n", dx, dy, pos);
		dst->r = (char)*(data + pos);
		dst->g = (char)*(data + pos + 1);
		dst->b = (char)*(data + pos + 2);
	}
	else
		if(smurf_pic->depth == 16)
		{
			pos = (long)sy * (long)width * 2L + (long)sx * 2L;
			src->index = *(data16 + pos);
			src->r = (src->index & 0xf800) >> 8;
			src->g = (src->index & 0x7e0) >> 3;
			src->b = (src->index & 0x1f) << 3;

			pos = (long)dy * (long)width * 2L + (long)dx * 2L;
			dst->index = *(data16 + pos);
			dst->r = (dst->index & 0xf800) >> 8;
			dst->g = (dst->index & 0x7e0) >> 3;
			dst->b = (dst->index & 0x1f) << 3;
		}
		else
			if(smurf_pic->depth <= 8)
			{
				palette = smurf_pic->palette;

				pos = (long)sy * (long)width + (long)sx;
				src->index = *(data + pos);
				pal = palette + src->index * 3L;
				src->r = (char)*pal++;
				src->g = (char)*pal++;
				src->b = (char)*pal++;

				pos = (long)dy * (long)width + (long)dx;
				src->index = *(data + pos);
				pal = palette + dst->index * 3L;
				dst->r = (char)*pal++;
				dst->g = (char)*pal++;
				dst->b = (char)*pal++;
			}

	if(type == SRC)
	{
		itoa(dst->r, rs_object[EDIT_SR].ob_spec.tedinfo->te_ptext, 10);
		itoa(dst->g, rs_object[EDIT_SG].ob_spec.tedinfo->te_ptext, 10);
		itoa(dst->b, rs_object[EDIT_SB].ob_spec.tedinfo->te_ptext, 10);
		redraw_window(&window, NULL, SRAHMEN, 0);
	}
	else
	{
		itoa(src->r, rs_object[EDIT_DR].ob_spec.tedinfo->te_ptext, 10);
		itoa(src->g, rs_object[EDIT_DG].ob_spec.tedinfo->te_ptext, 10);
		itoa(src->b, rs_object[EDIT_DB].ob_spec.tedinfo->te_ptext, 10);
		redraw_window(&window, NULL, DRAHMEN, 0);
	}

	return;
} /* evaluate_color */