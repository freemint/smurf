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
/*					Atari STAD Grafik (PAC)					*/
/* Version 0.1  --  18.11.98								*/
/*	  1 Bit, ID-, Pack- und Special-Byte noch fest			*/
/* Version 0.2  --  20.11.98								*/
/*	  ID-, Pack- und Special-Byte werden jetzt aufwendig	*/
/*	  mit Hilfe eines Histogramms ausgesucht. 				*/
/*	  Kommt jetzt auch mit Sourcebildern anderer Grîûen als	*/
/*	  640*400 zurecht.										*/
/* Version 0.3  --  22.11.98								*/
/*	  Jetzt auch vertikale (pm86) Kompression.				*/
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
	#include "pac\en\pac.rsh"
	#include "pac\en\pac.rh"
#else
	#include "pac\de\pac.rsh"
	#include "pac\de\pac.rh"
#endif


#define	DEBUG	0

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);

void write_header(char *ziel, char comp, unsigned int ID_Byte, unsigned int Pack_Byte, unsigned int Special_Byte);
unsigned long encode_pM85(char *buffer, char *ziel, unsigned int sheight, unsigned int sw);
unsigned long encode_pM86(char *buffer, char *ziel, unsigned int sheight, unsigned int sw);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"STAD-Grafik",
						0x0030,
						"Christian Eyrich",
						"PAC", "", "", "", "",
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
						1, 0, 0, 0, 0,
						0, 0, 0,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0
						};


char ID_Byte, Pack_Byte, Special_Byte;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				Atari STAD Grafik (PAC)				*/
/*		1 Bit, RLE in zwei Richtungen				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *ziel, *oziel,
		 t, highest, lowest;
	char wt[] = "STAD Exporter";
	static char comp;

	static int module_id;
	unsigned int sw, sheight, Button;

	unsigned long *histo,
				  headsize, len, f_len, highestcount, lowestcount, x;

	static WINDOW window;
	static OBJECT *win_form;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			/* falls Åbergeben, Konfig Åbernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy(&comp, (char *)*(long *)&smurf_struct->event_par[0], 1);
			else
				comp = VERT;

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[PAC_EXPORT];							/* Resourcebaum holen */

			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			smurf_struct->module_mode = M_WAITING;

			break;

		case MMORE:
			/* Ressource aktualisieren */
			if(comp == VERT)
			{
				win_form[VERT].ob_state |= SELECTED;
				win_form[HOR].ob_state &= ~SELECTED;
			}
			else
			{
				win_form[VERT].ob_state &= ~SELECTED;
				win_form[HOR].ob_state |= SELECTED;
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
				comp = VERT;

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
				if(Button == VERT || Button == HOR)
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
/* wie schnell sind wir? */
/*	init_timer(); */
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;	

			headsize = 7;

			sw = (smurf_struct->smurf_pic->pic_width + 7) / 8;
			sheight = smurf_struct->smurf_pic->pic_height;

			buffer = smurf_struct->smurf_pic->pic_data;
	
			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			if((ziel = (char *)SMalloc(headsize + 32000L)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				oziel = ziel;

				f_len = 0;

				memset(ziel, 0x0, headsize + 32000L);

				/* "Histogramm" aufbauen */
				histo = (unsigned long *)calloc(1024, 1);

				len = (long)sw * (long)sheight;
				x = 0;
				do
				{
					histo[buffer[x]]++;
				} while(++x < len);

				if(sw < 80)								/* Quellbreite < Zielbreite -> Ziel mit Weiû auffÅllen */
					histo[0] += (long)(80 - sw) * sheight;
				if(sheight < 400)						/* Quellhîhe < Zielhîhe -> Ziel mit Weiû auffÅllen */
					histo[0] += (long)(400 - sheight) * 80;

				/* seltenstes und hÑufigestes Byte herausfinden */
				x = 0;
				highestcount = 0;
				lowestcount = 255;
				do
				{
					if(histo[x] >= highestcount)
					{
						highest = x;
						highestcount = histo[x];
					}
					else
						if(histo[x] <= lowestcount)
						{
							lowest = x;
							lowestcount = histo[x];
						}
				} while(++x < 256);

				Pack_Byte = highest;
				ID_Byte = lowest;

				/* zweitseltenstes herausfinden */
				x = 0;
				lowestcount = 255;
				do
				{
					if(histo[x] <= lowestcount && x != ID_Byte)
					{
						lowest = x;
						lowestcount = histo[t];
					}
				} while(++x < 256);

				Special_Byte = lowest;

				free(histo);

				write_header(ziel, comp, ID_Byte, Pack_Byte, Special_Byte);
				ziel += headsize;

				if(comp == HOR)
					f_len = encode_pM85(buffer, ziel, sheight, sw);
				else
					f_len = encode_pM86(buffer, ziel, sheight, sw);

				ziel = oziel;

				smurf_struct->smurf_pic->pic_width = 640;
				smurf_struct->smurf_pic->pic_height = 400;

				f_len += headsize;
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


/* horizontale Kompression */
unsigned long encode_pM85(char *buffer, char *ziel, unsigned int sheight, unsigned int sw)
{
	char counter, pixel;

	unsigned int x, xx, y, dw, dheight, minwidth, minheight;

	unsigned long len, dlen;


	dw = 80;
	dheight = 400;

	if(sw < dw)
		minwidth = sw;
	else
		minwidth = dw;

	if(sheight < dheight)
		minheight = sheight;
	else
		minheight = dheight;

	dlen = 0L;

	y = 0;
	do
	{
		x = 0;
		do
		{
			counter = 0;
			pixel = *buffer++;
			x++;

			xx = x;
			while(*buffer == pixel && counter < 0xff && xx < minwidth)
			{
				buffer++;
				xx++;
				counter++;
			}

			if(counter > 1 ||										/* es lohnt sich */
			   pixel == ID_Byte || pixel == Special_Byte)			/* oder ID- oder Spezial-Byte muû */
			{														/* direkt abgespeichert werden */
				if(pixel == Pack_Byte)
				{
					*ziel++ = ID_Byte;
					*ziel++ = counter;
					dlen += 2;
				}
				else
				{
					*ziel++ = Special_Byte;
					*ziel++ = pixel;
					*ziel++ = counter;
					dlen += 3;
				}

				x = xx;
			}       
			else												/* lohnt sich leider nicht! */
			{
				do
				{
					*ziel++ = pixel;
					dlen++;
				} while(counter--);

				while((*buffer != *(buffer + 1) || *buffer != *(buffer + 2)) && *buffer != ID_Byte && *buffer != Special_Byte && xx < minwidth)
				{
					*ziel++ = *buffer++;
					xx++;
					dlen++;
				}

				x = xx;
			}
		} while(x < minwidth);

		if(sw < dw)								/* Quellbreite < Zielbreite -> Ziel mit Weiû auffÅllen */
		{
			if(Pack_Byte == 0x0)
			{
				*ziel++ = ID_Byte;
				*ziel++ = dw - sw - 1;
				dlen += 2;
			}
			else
			{
				*ziel++ = Special_Byte;
				*ziel++ = 0x0;
				*ziel++ = dw - sw - 1;
				dlen += 3;
			}
		}
		else									/* Quellbreite >= Zielbreite -> ÅberzÑhlige Quellpixel Åbergehen */
			buffer += sw - dw;
	} while(++y < minheight);

	if(sheight < dheight)						/* Quellhîhe < Zielhîhe -> Ziel mit Weiû auffÅllen */
	{
/*
		do
		{
			if(Pack_Byte == 0x0)
			{
				*ziel++ = ID_Byte;
				*ziel++ = 79;
				dlen += 2;
			}
			else
			{
				*ziel++ = Special_Byte;
				*ziel++ = 0x0;
				*ziel++ = 79;
				dlen += 3;
			}
		} while(++y < dheight);
*/
		len = (long)(dheight - sheight) * 80L;

		do
		{
			if(Pack_Byte == 0x0)
			{
				*ziel++ = ID_Byte;
				*ziel++ = 255;
				dlen += 2;
			}
			else
			{
				*ziel++ = Special_Byte;
				*ziel++ = 0x0;
				*ziel++ = 255;
				dlen += 3;
			}

			len -= 256;
		} while(len >= 255);

		if(Pack_Byte == 0x0)
		{
			*ziel++ = ID_Byte;
			*ziel++ = len - 1;
			dlen += 2;
		}
		else
		{
			*ziel++ = Special_Byte;
			*ziel++ = 0x0;
			*ziel++ = len - 1;
			dlen += 3;
		}
	}


	return(dlen);
} /* encode_pM85 */


/* vertikale Kompression */
unsigned long encode_pM86(char *buffer, char *ziel, unsigned int sheight, unsigned int sw)
{
	char *obuffer,
		 counter, pixel;

	unsigned int x, y, yy, dw, dheight, minwidth, minheight;

	unsigned long len, dlen;


	dw = 80;
	dheight = 400;

	if(sw < dw)
		minwidth = sw;
	else
		minwidth = dw;

	if(sheight < dheight)
		minheight = sheight;
	else
		minheight = dheight;

	obuffer = buffer;

	dlen = 0L;

	x = 0;
	do
	{
		buffer = obuffer + x;

		y = 0;
		do
		{
			counter = 0;
			pixel = *buffer;
			buffer += sw;
			y++;

			yy = y;
			while(*buffer == pixel && counter < 0xff && yy < minheight)
			{
				buffer += sw;
				yy++;
				counter++;
			}

			if(counter > 1 ||										/* es lohnt sich */
			   pixel == ID_Byte || pixel == Special_Byte)			/* oder ID- oder Spezial-Byte muû */
			{														/* direkt abgespeichert werden */
				if(pixel == Pack_Byte)
				{
					*ziel++ = ID_Byte;
					*ziel++ = counter;
					dlen += 2;
				}
				else
				{
					*ziel++ = Special_Byte;
					*ziel++ = pixel;
					*ziel++ = counter;
					dlen += 3;
				}

				y = yy;
			}       
			else												/* lohnt sich leider nicht! */
			{
				do
				{
					*ziel++ = pixel;
					dlen++;
				} while(counter--);

				while((*buffer != *(buffer + sw) || *buffer != *(buffer + sw + sw)) && *buffer != ID_Byte && *buffer != Special_Byte && yy < minheight)
				{
					*ziel++ = *buffer;
					buffer += sw;
					yy++;
					dlen++;
				}

				y = yy;
			}
		} while(y < minheight);

		if(sheight < dheight)					/* Quellhîhe < Zielhîhe -> Ziel mit Weiû auffÅllen */
		{
			if(Pack_Byte == 0x0)
			{
				*ziel++ = ID_Byte;
				*ziel++ = dheight - sheight - 1;
				dlen += 2;
			}
			else
			{
				*ziel++ = Special_Byte;
				*ziel++ = 0x0;
				*ziel++ = dheight - sheight - 1;
				dlen += 3;
			}
		}
	} while(++x < minwidth);

	if(sw < dw)									/* Quellbreite < Zielbreite -> Ziel mit Weiû auffÅllen */
	{
		len = (long)(dw - sw) * 400L;

		do
		{
			if(Pack_Byte == 0x0)
			{
				*ziel++ = ID_Byte;
				*ziel++ = 255;
				dlen += 2;
			}
			else
			{
				*ziel++ = Special_Byte;
				*ziel++ = 0x0;
				*ziel++ = 255;
				dlen += 3;
			}

			len -= 256;
		} while(len >= 255);

		if(Pack_Byte == 0x0)
		{
			*ziel++ = ID_Byte;
			*ziel++ = len - 1;
			dlen += 2;
		}
		else
		{
			*ziel++ = Special_Byte;
			*ziel++ = 0x0;
			*ziel++ = len - 1;
			dlen += 3;
		}
	}


	return(dlen);
} /* encode_pM86 */


void write_header(char *ziel, char comp, unsigned int ID_Byte, unsigned int Pack_Byte, unsigned int Special_Byte)
{
	if(comp == HOR)
		*(unsigned long *)ziel = 'pM85';
	else
		*(unsigned long *)ziel = 'pM86';

	*(ziel + 4)	= ID_Byte;
	*(ziel + 5)	= Pack_Byte;
	*(ziel + 6)	= Special_Byte;

	return;
} /* write_header */