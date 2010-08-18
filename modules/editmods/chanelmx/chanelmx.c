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
/*						Channelmixer						*/
/* Version 0.1  --  13.04.98								*/
/*	  24 Bit, Bilder mÅssen noch gleich groû sein!			*/
/* Version 0.2  --  20.04.98								*/
/*	  Bilder kînnen jetzt auch in unterschiedlicher Grîûe	*/
/*	  vorliegen.											*/
/* Version 0.3  --  27.09.98								*/
/*	  öbermitteln und Empfangen sowie Laden und Speichern	*/
/*	  von Konfigurationen eingebaut.						*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include "..\..\..\src\lib\demolib.h"

#include "chanelmx.rsh"
#include "chanelmx.rh"

#define DEST_PIC	0
#define SRC_PIC		1
#define	TextCast	ob_spec.tedinfo->te_ptext

typedef struct
{
	long version;
	char srcchanel, destchanel;
	int sdeck;
} CONFIG;

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

MOD_INFO module_info = {"Channelmixer",
						0x0020,
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
						2,
						"Zielbild",
						"Quellbild"
						};


MOD_ABILITY  module_ability = {
						24, 0, 0, 0, 0,
						0, 0, 0,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

char srcchanel, destchanel;

int module_id, sdeck;

SLIDER decksl;

static WINDOW window;
static OBJECT *win_form;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Channelmixer					*/
/* Ein Kanal (R, G oder B) wird in einen anderen	*/
/* Kanal eines anderen Bildes eingerechnet (Modus	*/
/* ersetzen mit StÑrke von 1-100).					*/
/*		24 Bit										*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	static char *buffer, *ziel;
	char t, pixval;
	char wt[] = "Chanelmixer";

	unsigned int sx, sy, dsx, dx, ddx, dy, Button, bh, bl, deckung;
	static unsigned int swidth, sheight, dwidth, dheight;

	CONFIG *config;


	service = smurf_struct->services;

/* Wenn das Modul zum ersten Mal gestartet wurde */
	switch(smurf_struct->module_mode)
	{
		case MSTART:
			win_form = rs_trindex[CHANELMIXER];					/* Resourcebaum holen */
	
			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			module_id = smurf_struct->module_number;
			SMfree = smurf_struct->services->SMfree;
			redraw_window = smurf_struct->services->redraw_window;
			mconfLoad = smurf_struct->services->mconfLoad;
			mconfSave = smurf_struct->services->mconfSave;

			window.whandlem = 0;				/* evtl. Handle lîschen */
			window.module = smurf_struct->module_number;	/* ID in die Fensterstruktur eintragen  */
			window.wnum = 1;					/* Fenster nummer 1...  */
			window.wx = -1;						/* Fenster X- ...    	*/
			window.wy = -1;						/* ... und Y-Pos     	*/
			window.ww = win_form->ob_width;		/* Fensterbreite    	*/
			window.wh = win_form->ob_height;	/* Fensterhîhe      	*/
			strcpy(window.wtitle, wt);			/* Titel reinkopieren   */
			window.resource_form = win_form;	/* Resource         	*/
			window.picture = NULL;				/* kein Bild.       	*/ 
			window.editob = 0;					/* erstes Editobjekt	*/
			window.nextedit = 0;				/* nÑchstes Editobjekt	*/
			window.editx = 0;

			smurf_struct->wind_struct = &window;  /* und die Fensterstruktur in die Gargamel */

			/* Defaults bestÅcken */
			srcchanel = QR;
			destchanel = ZR;
			sdeck = 100;
			make_sliders();
			service->set_slider(&decksl, 100);

			if(service->f_module_window(&window) == -1)			/* Gib mir 'n Fenster! */
				smurf_struct->module_mode = M_EXIT;		/* keins mehr da? */
			else 
				smurf_struct->module_mode = M_WAITING;	/* doch? Ich warte... */

			break;

/* Buttonevent */
		case MBEVT:
			Button = smurf_struct->event_par[0];

			if(Button == QR || Button == QG || Button == QB)
			{
				srcchanel = Button;
				smurf_struct->module_mode = M_WAITING;
			}
			else
				if(Button == ZR || Button == ZG || Button == ZB)
				{
					destchanel = Button;
					smurf_struct->module_mode = M_WAITING;
				}
				else
					if(Button == S_S)
						sdeck = (int)service->slider(&decksl);
					else
						if(Button == START)
							smurf_struct->module_mode = M_STARTED;
						else
							if(Button == LOAD)
								load_setting();
							else
								if(Button == SAVE)
									save_setting();

			break;

/* Keyboardevent */
		case MKEVT:
			Button = smurf_struct->event_par[0];

			if(Button == START)
			{
				smurf_struct->module_mode = M_STARTED;
				return;
			}

			break;

		case MPICS:
			switch(smurf_struct->event_par[0])
			{
				case DEST_PIC:	smurf_struct->event_par[0] = 24;
								smurf_struct->event_par[1] = FORM_PIXELPAK;
								smurf_struct->event_par[2] = RGB;
								smurf_struct->module_mode = M_PICTURE;
								break;

				case SRC_PIC:	smurf_struct->event_par[0] = 24;
								smurf_struct->event_par[1] = FORM_PIXELPAK;
								smurf_struct->event_par[2] = RGB;
								smurf_struct->module_mode = M_PICTURE;
								break;

				default:	smurf_struct->module_mode = M_WAITING;
							break;
			}
			return;


		case MPICTURE:
			if(smurf_struct->event_par[0] == DEST_PIC)
			{
				ziel = smurf_struct->smurf_pic->pic_data;
				dwidth = smurf_struct->smurf_pic->pic_width;
				dheight = smurf_struct->smurf_pic->pic_height;
			}
			else
				if(smurf_struct->event_par[0] == SRC_PIC)
				{
					buffer = smurf_struct->smurf_pic->pic_data;
					swidth = smurf_struct->smurf_pic->pic_width;
					sheight = smurf_struct->smurf_pic->pic_height;
				}
		
			smurf_struct->module_mode = M_WAITING;

			break;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */
			busybox = service->busybox;

			deckung = sdeck * 64 / 100;

			if(swidth > dwidth)
			{
				dsx = (swidth - dwidth) * 3;
				ddx = 0;
				swidth = dwidth;
			}
			else
			{
				ddx = (dwidth - swidth) * 3;
				dsx = 0;
				dwidth = swidth;
			}

			if(sheight > dheight)
				sheight = dheight;
			else
				dheight = sheight;

			if((bh = sheight / 10) == 0) 	/* busy-height */
				bh = sheight;
			bl = 0;							/* busy-length */

			if(deckung == 64)
			{
				sy = 0;
				do
				{
					if(!(sy%bh))
					{
						busybox(bl);
						bl += 12;
					}

					sx = 0;
					do
					{
						if(srcchanel == QR)
							pixval = *buffer;
						else
							if(srcchanel == QG)
								pixval = *(buffer + 1);
							else
								pixval = *(buffer + 2);
						buffer += 3;

						if(destchanel == ZR)
							*ziel = pixval;
						else
							if(destchanel == ZG)
								*(ziel + 1) = pixval;
							else
								*(ziel + 2) = pixval;
						ziel += 3;
					} while(++sx < swidth);
					buffer += dsx;
					ziel += ddx;
				} while(++sy < sheight);
			}
			else
			{
				sy = 0;
				do
				{
					if(!(sy%bh))
					{
						busybox(bl);
						bl += 12;
					}

					sx = 0;
					do
					{
						if(srcchanel == QR)
							pixval = *buffer;
						else
							if(srcchanel == QG)
								pixval = *(buffer + 1);
							else
								pixval = *(buffer + 2);
						buffer += 3;

						if(destchanel == ZR)
							*ziel = (char)(((int)pixval * deckung + (int)*ziel * (64 - deckung)) >> 8);
						else
							if(destchanel == ZG)
								*(ziel + 1) = (char)(((int)pixval * deckung + (int)*(ziel + 1) * (64 - deckung)) >> 8);
							else
								*(ziel + 2) = (char)(((int)pixval * deckung + (int)*(ziel + 2) * (64 - deckung)) >> 8);
						ziel += 3;
					} while(++sx < swidth);
					buffer += dsx;
					ziel += ddx;
				} while(++sy < sheight);
			}

/* wie schnell waren wir? */
/*	printf("\n%lu\n", get_timer());
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
		case MTERM:
			smurf_struct->module_mode = M_EXIT;
			break;
	} /* switch */

	return;
}


int make_sliders(void)
{
	decksl.regler = S_S;
	decksl.schiene = S_F; 
	decksl.rtree = win_form;
	decksl.txt_obj = S_E;
	decksl.min_val = 1;
	decksl.max_val = 100;
	decksl.window = &window;

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
	srcchanel = myConfig->srcchanel;
	destchanel = myConfig->destchanel;

	win_form[ZR].ob_state &= ~SELECTED;
	win_form[ZG].ob_state &= ~SELECTED;
	win_form[ZB].ob_state &= ~SELECTED;
	win_form[destchanel].ob_state |= SELECTED;

	win_form[QR].ob_state &= ~SELECTED;
	win_form[QG].ob_state &= ~SELECTED;
	win_form[QB].ob_state &= ~SELECTED;
	win_form[srcchanel].ob_state |= SELECTED;

	redraw_window(&window, NULL, DEST_BOX, 1);
	redraw_window(&window, NULL, SRC_BOX, 1);

	service->set_slider(&decksl, myConfig->sdeck);

	return;
} /* apply_setting */


void write_setting(CONFIG *myConfig)
{
	myConfig->version = 0x0030;

	myConfig->srcchanel = srcchanel;
	myConfig->destchanel = destchanel;

	myConfig->sdeck = sdeck;

	return;
} /* write_setting */