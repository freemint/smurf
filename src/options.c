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
 * The Initial Developer of the Original Code are
 * Olaf Piesche, Christian Eyrich, Dale Russell and Jîrg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

#include <tos.h>
#include <string.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <multiaes.h>
#include <vdi.h>
#include <ext.h>
#include <math.h>
#include <demolib.h>
#include <errno.h>
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurfine.h"
#include "smurf_f.h"
#include "popdefin.h"
#include "plugin\plugin.h"
#include "olgagems.h"
#include "smurfobs.h"
#include "ext_obs.h"
#include "globdefs.h"

extern int openmode;				/* Dialog neu geîffnet (0) oder buttonevent? (!=0) */
extern SYSTEM_INFO Sys_info;			/* Systemkonfiguration */

extern WINDOW wind_s[25];
extern POP_UP	popups[25];
extern DITHER_MOD_INFO *ditmod_info[10];

extern OBJECT 	*col_pop;					/* Zeiger auf Resource-DITHERPOPUP	*/
extern OBJECT	*colred_popup;
extern WINDOW picture_windows[MAX_PIC];

extern int Radio, SelectedRadio;
extern int Check, SelectedCheck;
extern int Cycle, SelectedCycle;



/* ----------------------------------------------------------------	*/
/* 		Formular	-	OPTIONEN									*/
/* ----------------------------------------------------------------	*/
void f_options(void)
{
	char pstr[3];

	int button=0, popbut, item;
	int oldenv;
	int t;
	static int NPCD, NOUTCOL, NPREVDIT, NPICMANDIT, NMOVEDIT, NAUTOPAL, NAUTODIT;

	OBJECT *infowindow;


	infowindow = Dialog.smurfOpt.tree;

	if(!openmode)
	{
		NPCD = Sys_info.PCD_Defsize;
		NOUTCOL = Sys_info.outcol;
		NPREVDIT = Sys_info.PreviewDither;
		NPICMANDIT = Sys_info.PicmanDither;
		NMOVEDIT = Sys_info.PreviewMoveDither;
		NAUTOPAL = Sys_info.AutoconvPalmode;
		NAUTODIT = Sys_info.AutoconvDither;
	}

	button = Dialog.init(WIND_OPTIONS, OPT_OK);

	if(button != -1)
	{
		switch(button)
		{
			/* Registerkarte: Allgemein */
			case MISC_BUTTON:
					if(infowindow[MISC_BOX].ob_flags&HIDETREE)
					{
						infowindow[MISC_BOX].ob_flags &= ~HIDETREE;
						infowindow[MISC_BUTTON].ob_state |= SHADOWED;
						infowindow[MISC_BUTTON].ob_y = 8;
						infowindow[INTERFACE_BOX].ob_flags |= HIDETREE;
						infowindow[INTERFACE_BUTTON].ob_state &= ~SHADOWED;
						infowindow[INTERFACE_BUTTON].ob_y = 12;
						infowindow[PREVIEWS_BOX].ob_flags |= HIDETREE;
						infowindow[PREVIEWS_BUTTON].ob_state &= ~SHADOWED;
						infowindow[PREVIEWS_BUTTON].ob_y = 12;
						infowindow[CONVERSION_BOX].ob_flags |= HIDETREE;
						infowindow[CONVERS_BUTTON].ob_state &= ~SHADOWED;
						infowindow[CONVERS_BUTTON].ob_y = 12;
						Window.redraw(&wind_s[WIND_OPTIONS],NULL, BUTTONS_BOX, 0);
						Window.redraw(&wind_s[WIND_OPTIONS],NULL, MISC_BOX, 0);
					}
					break;
			
			/* Registerkarte: Interface */				
			case INTERFACE_BUTTON:
					if(infowindow[INTERFACE_BOX].ob_flags&HIDETREE)
					{
						infowindow[INTERFACE_BOX].ob_flags &= ~HIDETREE;
						infowindow[INTERFACE_BUTTON].ob_state |= SHADOWED;
						infowindow[INTERFACE_BUTTON].ob_y = 8;
						infowindow[MISC_BOX].ob_flags |= HIDETREE;
						infowindow[MISC_BUTTON].ob_state &= ~SHADOWED;
						infowindow[MISC_BUTTON].ob_y = 12;
						infowindow[PREVIEWS_BOX].ob_flags |= HIDETREE;
						infowindow[PREVIEWS_BUTTON].ob_state &= ~SHADOWED;
						infowindow[PREVIEWS_BUTTON].ob_y = 12;
						infowindow[CONVERSION_BOX].ob_flags |= HIDETREE;
						infowindow[CONVERS_BUTTON].ob_state &= ~SHADOWED;
						infowindow[CONVERS_BUTTON].ob_y = 12;
						Window.redraw(&wind_s[WIND_OPTIONS],NULL, BUTTONS_BOX, 0);
						Window.redraw(&wind_s[WIND_OPTIONS],NULL, INTERFACE_BOX, 0);
					}
					break;

			/* Registerkarte: PREVIEWS */				
			case PREVIEWS_BUTTON:
					if(infowindow[PREVIEWS_BOX].ob_flags&HIDETREE)
					{
						infowindow[PREVIEWS_BOX].ob_flags &= ~HIDETREE;
						infowindow[PREVIEWS_BUTTON].ob_state |= SHADOWED;
						infowindow[PREVIEWS_BUTTON].ob_y = 8;
						infowindow[INTERFACE_BOX].ob_flags |= HIDETREE;
						infowindow[INTERFACE_BUTTON].ob_state &= ~SHADOWED;
						infowindow[INTERFACE_BUTTON].ob_y = 12;
						infowindow[MISC_BOX].ob_flags |= HIDETREE;
						infowindow[MISC_BUTTON].ob_state &= ~SHADOWED;
						infowindow[MISC_BUTTON].ob_y = 12;
						infowindow[CONVERSION_BOX].ob_flags |= HIDETREE;
						infowindow[CONVERS_BUTTON].ob_state &= ~SHADOWED;
						infowindow[CONVERS_BUTTON].ob_y = 12;
						Window.redraw(&wind_s[WIND_OPTIONS],NULL, BUTTONS_BOX, 0);
						Window.redraw(&wind_s[WIND_OPTIONS],NULL, PREVIEWS_BOX, 0);
					}
					break;

			/* Registerkarte: Wandlung */				
			case CONVERS_BUTTON:
					if(infowindow[CONVERSION_BOX].ob_flags&HIDETREE)
					{
						infowindow[CONVERSION_BOX].ob_flags &= ~HIDETREE;
						infowindow[CONVERS_BUTTON].ob_state |= SHADOWED;
						infowindow[CONVERS_BUTTON].ob_y = 8;
						infowindow[INTERFACE_BOX].ob_flags |= HIDETREE;
						infowindow[INTERFACE_BUTTON].ob_state &= ~SHADOWED;
						infowindow[INTERFACE_BUTTON].ob_y = 12;
						infowindow[MISC_BOX].ob_flags |= HIDETREE;
						infowindow[MISC_BUTTON].ob_state &= ~SHADOWED;
						infowindow[MISC_BUTTON].ob_y = 12;
						infowindow[PREVIEWS_BOX].ob_flags |= HIDETREE;
						infowindow[PREVIEWS_BUTTON].ob_state &= ~SHADOWED;
						infowindow[PREVIEWS_BUTTON].ob_y = 12;
						Window.redraw(&wind_s[WIND_OPTIONS],NULL, BUTTONS_BOX, 0);
						Window.redraw(&wind_s[WIND_OPTIONS],NULL, CONVERSION_BOX, 0);
					}
					break;

		
			/* Auûenfarbe-Popup */
			case OUTCOL:
			case OCOL_CB:
					popbut = f_pop(&popups[POPUP_OUTCOL], 0, button, NULL);
					if(popbut != -1)
					{
						NOUTCOL = popbut - 1;
						infowindow[OCOL_DISP].ob_spec.obspec.interiorcol = popbut-1;
						itoa(popbut-1, pstr, 10);
						strncpy(infowindow[OUTCOL].TextCast, pstr, 2);
					}

					change_object(&wind_s[WIND_OPTIONS], OCOL_DISP, UNSEL, 1);
					f_deselect_popup(&wind_s[WIND_OPTIONS], OUTCOL, OCOL_CB);
					break;
	
			/* Importerliste erstellen */
			case OPT_IMPLIST:
					f_scan_import();
					change_object(&wind_s[WIND_OPTIONS], OPT_IMPLIST, UNSEL, 1);
					break;

			case PCD_DEF_PB:
			case PCD_DEF_CB:
					popbut = f_pop(&popups[POPUP_KODAK], 0, button, NULL);
					if(popbut != -1)
						NPCD = popbut;
					f_deselect_popup(&wind_s[WIND_OPTIONS], PCD_DEF_PB, PCD_DEF_CB);
					break;

			case PREVDIT_PB:
			case PREVDIT_CB:
					for(t = DIT1; t <= DIT5; t++)
					{
						if(!ditmod_info[t - 1] || ditmod_info[t - 1]->pal_mode!=ALLPAL)
							col_pop[t].ob_state |= DISABLED;
						else
							col_pop[t].ob_state &= ~DISABLED;
					}

					popbut = f_pop(&popups[POPUP_PREVDIT], 0, button, NULL);
					if(popbut != -1)
						NPREVDIT = popbut;
					f_deselect_popup(&wind_s[WIND_OPTIONS], PREVDIT_PB, PREVDIT_CB);

					for(t = DIT1; t <= DIT5; t++)
						if(ditmod_info[t - 1]) 
							col_pop[t].ob_state &= ~DISABLED;

					break;

			case PICMANDIT_PB:
			case PICMANDIT_CB:
					for(t = DIT1; t <= DIT5; t++)
					{
						if(!ditmod_info[t - 1] || ditmod_info[t-1]->pal_mode!=ALLPAL)
							col_pop[t].ob_state |= DISABLED;
						else
							col_pop[t].ob_state &= ~DISABLED;
					}

					popbut = f_pop(&popups[POPUP_PICMANDIT], 0, button, NULL);
					if(popbut != -1)
						NPICMANDIT = popbut;
					f_deselect_popup(&wind_s[WIND_OPTIONS], PICMANDIT_PB, PICMANDIT_CB);

					for(t = DIT1; t <= DIT5; t++)
						if(ditmod_info[t - 1]) 
							col_pop[t].ob_state &= ~DISABLED;

					break;

			case MOVE_PREVDIT_PB:
			case MOVE_PREVDIT_CB:
					for(t = DIT1; t <= DIT5; t++)
					{
						if(!ditmod_info[t - 1] || ditmod_info[t-1]->pal_mode!=ALLPAL)
							col_pop[t].ob_state |= DISABLED;
						else
							col_pop[t].ob_state &= ~DISABLED;
					}

					popbut = f_pop(&popups[POPUP_MOVEDIT], 0, button, NULL);
					if(popbut != -1)
						NMOVEDIT = popbut;
					f_deselect_popup(&wind_s[WIND_OPTIONS], MOVE_PREVDIT_PB, MOVE_PREVDIT_CB);

					for(t = DIT1; t <= DIT5; t++)
						if(ditmod_info[t - 1])
							col_pop[t].ob_state &= ~DISABLED;

					break;

			case AUTODIT_PB:
			case AUTODIT_CB:
					popbut = f_pop(&popups[POPUP_AUTODITHER], 0, button, NULL);
					if(popbut != -1)
						NAUTODIT = popbut;
					f_deselect_popup(&wind_s[WIND_OPTIONS], AUTODIT_PB, AUTODIT_CB);

					if(ditmod_info[NAUTODIT-1]->pal_mode == FIXPAL)
					{
						strncpy(infowindow[AUTOPAL_PB].TextCast, colred_popup[CR_FIXPAL].TextCast, 15);
						change_object(&wind_s[WIND_OPTIONS], AUTOPAL_PB, DISABLED, 1);
						change_object(&wind_s[WIND_OPTIONS], AUTOPAL_CB, DISABLED, 1);
						NAUTOPAL = CR_FIXPAL;
					}
					else
					{
						if(Sys_info.AutoconvPalmode != CR_FIXPAL)
							NAUTOPAL = Sys_info.AutoconvPalmode;
						else
							NAUTOPAL = CR_SYSPAL;

						strcpy(infowindow[AUTOPAL_PB].TextCast, colred_popup[NAUTOPAL].TextCast);
						change_object(&wind_s[WIND_OPTIONS], AUTOPAL_PB, ENABLED, 1);
						change_object(&wind_s[WIND_OPTIONS], AUTOPAL_CB, ENABLED, 1);
					}
					break;

			case AUTOPAL_PB:
			case AUTOPAL_CB:
					popbut = f_pop(&popups[POPUP_AUTOPAL], 0, button, NULL);
					if(popbut != -1)
						NAUTOPAL = popbut;
					f_deselect_popup(&wind_s[WIND_OPTIONS], AUTOPAL_PB, AUTOPAL_CB);
					break;
		}			
	}

	oldenv = Sys_info.environment;

	/*
	 * OK oder setzen? -> Einstellungen Åbernehmen
	 */	
	if(button == OPT_OK || button == OPT_SET)
	{
		Sys_info.center_dialog = infowindow[OPT_CENTER].ob_state;
		Sys_info.window_alert = infowindow[OPT_WINDOWALERT].ob_state;
		Sys_info.profi_mode = infowindow[OPT_PROFI].ob_state;
		Sys_info.busybox_oc = infowindow[ORIGINAL_CLIP].ob_state; 
		Sys_info.immed_prevs = infowindow[OPT_PREVS].ob_state; 
		Sys_info.realtime_dither = infowindow[REALTIME_DITHER].ob_state; 

		if(Sys_info.outcol != NOUTCOL)
		{
			Sys_info.outcol = NOUTCOL;
			for(t = 0; t < MAX_PIC; t++)
				if(picture_windows[t].whandlem)
					Window.redraw(&picture_windows[t], NULL, 0, DRAWNOTREE);
		}

		Sys_info.PCD_Defsize = NPCD;
		Sys_info.PreviewDither = NPREVDIT;
		Sys_info.PicmanDither = NPICMANDIT;
		Sys_info.PreviewMoveDither = NMOVEDIT;
		Sys_info.AutoconvDither = NAUTODIT;
		Sys_info.AutoconvPalmode = NAUTOPAL;

		if(infowindow[ENV_STANDARD].ob_state&SELECTED)
			Sys_info.environment = 1;
		else
			if(infowindow[ENV_SILLY].ob_state&SELECTED)
				Sys_info.environment = 2;
			else
				if(infowindow[ENV_THERAPY].ob_state&SELECTED)
					Sys_info.environment = 3;

		if(infowindow[KBEV_JOIN].ob_state&SELECTED)
			Sys_info.keyevents = KBEV_JOIN;
		else
			if(infowindow[KBEV_DELETE].ob_state&SELECTED)
				Sys_info.keyevents = KBEV_DELETE;

		if(button == OPT_OK)
		{
			change_object(&wind_s[WIND_OPTIONS], OPT_OK, UNSEL, 1);
			Dialog.close(WIND_OPTIONS);
		}
	}


	if(button == OPT_SET)
		change_object(&wind_s[WIND_OPTIONS], OPT_SET, UNSEL, 1);

	/*
	 * Env-Config setzen und Fenster redrawen
	 */
	if(Sys_info.environment != oldenv)
	{
		Dialog.smurfOpt.setEnvconfig();
		for(item = 0; item < 20; item++)
			if(wind_s[item].whandlem != 0)
				Window.redraw(&wind_s[item], NULL, 0, 0);
	}

	return;
} /* f_options */


void f_set_envconfig(void)
{
	switch(Sys_info.environment)
	{
		case 1:	Radio=RADIO; SelectedRadio=RADIO_SEL;
				Check=CHECK; SelectedCheck=CHECK_SEL;
				Cycle=CYCLE; SelectedCycle=CYCLE_SEL;
				if(Sys_info.OS & MATSCHIG)
				{
					Cycle=CYCLE_MAGX;
					SelectedCycle=CYCLE_SEL_MAGX;
				}
				break;
		case 2:	Radio=RADIO2; SelectedRadio=RADIO2_SEL;
				Check=CHECKICN2; SelectedCheck=CHECKICN2_SEL;
				Cycle=CYCLE_2; SelectedCycle=CYCLE_2_SEL;
				if(Sys_info.OS & MATSCHIG)
				{
					Cycle=CYCLE_MAGX;
					SelectedCycle=CYCLE_SEL_MAGX;
				}
				break;
		case 3:	Radio=RADIO3; SelectedRadio=RADIO3_SEL;
				Check=CHECKICN3; SelectedCheck=CHECKICN3_SEL;
				Cycle=CYCLE_3; SelectedCycle=CYCLE_3_SEL;
				if(Sys_info.OS & MATSCHIG)
				{
					Cycle=CYCLE_MAGX;
					SelectedCycle=CYCLE_SEL_MAGX;
				}
				break;
	}

	return;
} /* f_set_envconfig */