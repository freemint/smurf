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
/*				Degas (Elite)-Image Encoder					*/
/* Version 0.1  --  28.03.96								*/
/*	  Prototyp meiner Exporter, P?1 - P?3					*/
/* Version 0.2  --  29.07.97								*/
/*	  Nachdem die 0.1 nie lief, komplette šberarbeitung des	*/
/*	  Exporters.											*/
/* Version 0.3  --  17.10.98								*/
/*	  Fehler in Bedienung behoben. Bei Verlassen per Return	*/
/*	  wurde der Dialog zwar geschlossen, die Config aber	*/
/*	  nicht gespeichert.									*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <..\..\..\demolib.h>

#define ENGLISCH 0

#if ENGLISCH
	#include "pix\en\pix.rsh"
	#include "pix\en\pix.rh"
#else
	#include "pix\de\pix.rsh"
	#include "pix\de\pix.rh"
#endif


void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_module_window)(WINDOW *mod_window);

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Degas Elite",
						0x0030,
						"Christian Eyrich",
						"PI1","PI2","PI3","PC1","PC2",
						"PC3","","","","",
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
						0,10
						};


MOD_ABILITY  module_ability = {
						1, 2, 4, 0, 0,
						0, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						2					/* More */
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				Degas-(Elite) Komprimierer			*/
/*		1, 2, 4 Bit, keine und RLE					*/
/* Gr”žere und kleinere Bildern als Degas zul„žt	*/
/* werden selbst„ndig abgeschnitten und aufgeblasen.*/
/* Leider ist es beim Aufblasen nicht immer m”glich,*/
/* den Rand weiž zu machen, da Weiž weder auf Index */
/* 0 (siehe memset()) liegen, noch berhaupt in der */
/* Bildpalette vorhanden sein muž.					*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/

EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *obuffer, *ziel, *oziel, *ppal,
		 Planes, BitsPerPixel, res, p, t;
	char expmessag[21];
	char wt[] = "Degas Exporter";
	static char comp;

	static int module_id;
	unsigned int *pal, i, n, x, y, bv, zv, w, v1, v2,
				 width, height, runheight, Button;

	unsigned long src_pos = 0, dst_pos = 0, plh, plo, f_len;

	typedef struct
	{
		char comp;
		char res;
		unsigned int pal[16];
	} head;

	head *pix_header;

	static WINDOW window;
	static OBJECT *win_form;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			/* falls bergeben, Konfig bernehmen */
			if(*(long *)&smurf_struct->event_par[0] != 0)
				memcpy(&comp, (char *)*(long *)&smurf_struct->event_par[0], 1);
			else
				comp = KEINE;

			module_id = smurf_struct->module_number;

			win_form = rs_trindex[PIX_EXPORT];							/* Resourcebaum holen */

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
			window.picture = NULL;				/* kein Bild.       	*/ 
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

	/* Extender wird vom Smurf erfragt */
		case MEXTEND:
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			switch((int)BitsPerPixel)
			{
				case 1:	smurf_struct->event_par[0] = 3;
						break;
				case 2:	smurf_struct->event_par[0] = 2;
						break;
				case 4:	smurf_struct->event_par[0] = 1;
						break;
			}

			if(comp == RLE)
				smurf_struct->event_par[0] += 3;

			smurf_struct->module_mode = M_EXTEND;
			
			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
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
			obuffer = buffer;

			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			switch(smurf_struct->smurf_pic->depth)
			{
				case 1:	width = 640;
						height = 400;
						Planes = 1;
						res = 2;
						break;
				case 2:	width = 640;
						height = 200;
						Planes = 2;
						res = 1;
						break;
				case 4:	width = 320;
						height = 200;
						Planes = 4;
						res = 0;
						break;
				default:break;
			}

			f_len = width / 8 * height * Planes;

			if((ziel = SMalloc(sizeof(head) + f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				oziel = ziel;
				memset(ziel, 0x0, sizeof(head) + f_len);

				pix_header = (head *)ziel;

				if(comp == KEINE)
					pix_header->comp = 0x0;
				else
					pix_header->comp = 0x80;

				pix_header->res = res;

				strcpy(expmessag, "Degas ");
				if(comp == KEINE)
					strcat(expmessag, "PI?");
				else
					strcat(expmessag, "PC?");
				strcpy(smurf_struct->smurf_pic->format_name, expmessag);

				ziel += sizeof(head);

				if(comp == KEINE)						/* unkomprimiert */
				{
					w = (smurf_struct->smurf_pic->pic_width + 7) / 8;
					if(w > width / 8)
						w = width / 8;
					/* wieviel Bildrest berspringen? */
					if(smurf_struct->smurf_pic->pic_width > width)
						bv = (smurf_struct->smurf_pic->pic_width + 7) / 8 - width / 8;
					else
						bv = 0;
					/* wieviel Zielrest berspringen? */
					if(width > smurf_struct->smurf_pic->pic_width)
						zv = width / 8 - (smurf_struct->smurf_pic->pic_width + 7) / 8;
					else
						zv = 0;

					runheight = smurf_struct->smurf_pic->pic_height;
					if(runheight > height)
						runheight = height;

					p = 0;
					do /* Planes */
					{
						ziel = oziel + sizeof(head) + (p << 1);	/* Zieladresse der kodierten Scanline */

						y = 0;
						do /* height */
						{
							x = 0;
							do /* width */
							{
								*ziel++ = *buffer++;
								x++;
								if(x < w)
								{
									*ziel++ = *buffer++;
									x++;
								}
								ziel += (Planes - 1) << 1;	/* Zieladresse der kodierten Scanline */
							} while(x < w); /* x */
							buffer += bv;				/* Bild berspringen */
							ziel += zv * Planes;		/* Ziel berspringen */
						} while(++y < runheight); /* y */
					} while(++p < Planes); /* p */
				}
				else									/* komprimiert */
				{
					plh = w * height; /* H”he einer Plane */
					dst_pos = p;						/* Zieladresse der dekodierten Scanline */

					y = 0;
					do /* height */
					{
						plo = y * w; /* Offset vom Planeanfang in Bytes */

						p = 0;
						do /* Plane */
						{
							src_pos = p * plh + plo;
							x = 0;
							do /* width */
							{
								v1 = buffer[src_pos++];
								if((v1 & 0x80) == 0x80)
								{
									n = (0x101 - v1);
									v2 = buffer[src_pos++];
									for(i = 0; i < n; i++)
										ziel[dst_pos++] = v2;
									x += n;
								}
								else
								{
									for (i = 0; i < v1 + 1; i++)
										ziel[dst_pos++] = buffer[src_pos++];
									x += (v1 + 1);
								}
							} while(x < w); /* x */
						} while(++p < Planes); /* p */
					} while(++y < height); /* y */
				} /* comp? */

				buffer = obuffer;
				ziel = oziel;

				exp_pic->pic_data = ziel;
				exp_pic->f_len = sizeof(head) + f_len;

				pal = pix_header->pal;
				ppal = smurf_struct->smurf_pic->palette;

				for(i = 16; i > 0; i--)
				{
					*pal = ((unsigned int)*ppal++ >> 5) << 8;
					*pal |= ((unsigned int)*ppal++ >> 5) << 4;
					*pal++ |= (unsigned int)*ppal++ >> 5;
				}
			} /* Malloc */

/* wie schnell waren wir? */
/*  printf("%lu", get_timer);
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