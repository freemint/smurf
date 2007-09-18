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
/*						Chanel No.5							*/
/* Version 0.1  --  26.01.97								*/
/*	  1-8, 24 Bit											*/
/* Version 0.2  --  30.01.97								*/
/*	  16 Bit - Achtung, wegen g von 6 auf 5 Bits nicht		*/
/*	  verlustfrei!											*/
/* Version 0.3  --  15.03.97								*/
/*	  Eigene Ressource										*/
/* Version 0.4  --  27.09.98								*/
/*	  šbermitteln und Empfangen von Konfigurationen ein-	*/
/*	  gebaut.												*/
/* Version 0.5  --  21.08.2000								*/
/*	  Laden und speichern von Konfigurationen eingebaut.	*/
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

#define ENGLISCH 0

#if ENGLISCH
	#include "chanel5\en\chanel5.rsh"
	#include "chanel5\en\chanel5.rh"
#else
	#include "chanel5\de\chanel5.rsh"
	#include "chanel5\de\chanel5.rh"
#endif

typedef struct
{
	long version;
	char Folge;
} CONFIG;

int	(*SMfree)(void *ptr);
void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);
void *(*mconfLoad)(MOD_INFO *modinfo, int mod_id, char *name);
void (*mconfSave)(MOD_INFO *modinfo, int mod_id, void *confblock, long len, char *name);
SERVICE_FUNCTIONS *service;

void save_setting(void);
void load_setting(void);
void apply_setting(CONFIG *myConfig);
void write_setting(CONFIG *myConfig);

MOD_INFO module_info = {"Chanel No.5",
						0x0050,
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


MOD_ABILITY  module_ability = {
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


char Folge;

int module_id;

static WINDOW window;
static OBJECT *win_form;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Chanel No.5						*/
/*		1-8, 16 und 24 Bit							*/
/* Und wieder ein Modul mit so einem sch”nen Namen.	*/
/* Hiermit darf der User die drei Farbkan„le R, G	*/
/* und B eines Bildes verteilen wie er will.		*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data,
		 t, pixval, BitsPerPixel;
	char wt[] = "Chanel No.5";

	unsigned int *data16, pixval16, width, height, Button;

	unsigned long length;

	CONFIG *config;


	service = smurf_struct->services;

/* Wenn das Modul zum ersten Mal gestartet wurde */
	switch(smurf_struct->module_mode)
	{
		case MSTART:
			win_form = rs_trindex[CHANEL5];					/* Resourcebaum holen */
	
			/* Resource umbauen */
			for(t = 0; t < NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);

			module_id = smurf_struct->module_number;
			SMfree = smurf_struct->services->SMfree;
			redraw_window = smurf_struct->services->redraw_window;
			mconfLoad = service->mconfLoad;
			mconfSave = service->mconfSave;

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

			Folge = _RBG;						/* Default bestcken */

			if(smurf_struct->services->f_module_window(&window) == -1)			/* Gib mir 'n Fenster! */
				smurf_struct->module_mode = M_EXIT;		/* keins mehr da? */
			else 
				smurf_struct->module_mode = M_WAITING;	/* doch? Ich warte... */

			break;

/* Buttonevent */
		case MBEVT:
			Button = smurf_struct->event_par[0];

			if(Button >= _RBG && Button <= _BGR)
			{
				Folge = (char)Button;
				smurf_struct->module_mode = M_WAITING;
				return;
			}
			else
				if(Button == START)
				{
					smurf_struct->module_mode = M_STARTED;
					return;
				}
				else
					if(Button == MCONF_LOAD)
						load_setting();
					else
						if(Button == MCONF_SAVE)
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

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */

			BitsPerPixel = smurf_struct->smurf_pic->depth;

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			if(BitsPerPixel != 16)
			{
				if(BitsPerPixel == 24)
				{
					data = smurf_struct->smurf_pic->pic_data;

					length = (unsigned long)width * (unsigned long)height;
				}
				else
				{
					data = smurf_struct->smurf_pic->palette;

					length = 256L;
				}

				switch(Folge)
				{
					case _RBG:	do
								{
									data++;
									pixval = *data;
									*data++ = *(data + 1);
									*data++ = pixval;
								} while(--length);
								break;
					case _GRB:	do
								{
									pixval = *data;
									*data++ = *(data + 1);
									*data++ = pixval;
									data++;
								} while(--length);
								break;
					case _GBR:	do
								{
									pixval = *data;
									*data++ = *(data + 1);
									*data++ = *(data + 1);
									*data++ = pixval;
								} while(--length);
								break;
					case _BRG:	do
								{
									pixval = *data;
									*data++ = *(data + 2);
									*(data++ + 1) = *data;
									*(data++ - 1) = pixval;
								} while(--length);
								break;
					case _BGR:	do
								{
									pixval = *data;
									*data++ = *(data + 2);
									data++;
									*data++ = pixval;
								} while(--length);
								break;
					default: break;
				}
			}
			else
			{
				data16 = (unsigned int *)smurf_struct->smurf_pic->pic_data;

				length = (unsigned long)width * (unsigned long)height;

				switch(Folge)
				{
					case _RBG:	do
								{
									pixval16 = *data16;
									*data16++ = (pixval16 & 0xf800) | ((pixval16 & 0x7c0) >> 6) | ((pixval16 & 0x1f) << 6);
								} while(--length);
								break;
					case _GRB:	do
								{
									pixval16 = *data16;
									*data16++ = ((pixval16 & 0xf800) >> 5) | ((pixval16 & 0x7c0) << 5) | (pixval16 & 0x1f);
								} while(--length);
								break;
					case _GBR:	do
								{
									pixval16 = *data16;
									*data16++ = ((pixval16 & 0xf800) >> 11) | ((pixval16 & 0x7c0) << 5) | ((pixval16 & 0x1f) << 6);
								} while(--length);	
								break;
					case _BRG:	do
								{
									pixval16 = *data16;
									*data16++ = ((pixval16 & 0xf800) >> 5) | ((pixval16 & 0x7c0) >> 6) | ((pixval16 & 0x1f) << 11);
								} while(--length);	
								break;
					case _BGR:	do
								{
									pixval16 = *data16;
									*data16++ = ((pixval16 & 0xf800) >> 11) | (pixval16 & 0x7e0) | ((pixval16 & 0x1f) << 11);
								} while(--length);	
								break;
					default: break;
				}
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
	Folge = myConfig->Folge;

	win_form[_RBG].ob_state &= ~SELECTED;
	win_form[_GRB].ob_state &= ~SELECTED;
	win_form[_GBR].ob_state &= ~SELECTED;
	win_form[_BRG].ob_state &= ~SELECTED;
	win_form[_BGR].ob_state &= ~SELECTED;
	win_form[Folge].ob_state |= SELECTED;

	redraw_window(&window, NULL, FORMAT_BOX, 1);

	return;
} /* apply_setting */


void write_setting(CONFIG *myConfig)
{
	myConfig->version = 0x0050;

	myConfig->Folge = Folge;

	return;
} /* write_setting */