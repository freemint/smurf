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
/*#include <multiaes.h>*/
#include <aes.h>
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
extern DISPLAY_MODES Display_Opt;

extern WINDOW wind_s[25];
extern POP_UP	popups[25];
extern DITHER_MOD_INFO *ditmod_info[10];

extern OBJECT 	*col_pop;					/* Zeiger auf Resource-DITHERPOPUP	*/
extern OBJECT	*colred_popup;

extern int fix_red[256], fix_blue[256], fix_green[256];


/* ****************************************************************	*/
/*				Formular DARSTELLUNG / OPTIONEN						*/
/*		Die Einstellungen werden bis zur öbernahme mit Setzen oder	*/
/*		OK in der Sys_info zwischengespeichert und bei Setzen oder	*/
/*		OK in die Display_options Åbernommen.						*/
/* ****************************************************************	*/
void f_display_opt(void)
{
	char *pal_loadpath;
	static char filepal_name[32]="lade palette";

	int button = 0, t;
	int popback;
	int olditem, ditmode;

	OBJECT *dopt_tree;

	extern char *load_palfile(char *path, int *red, int *green, int *blue, int max_cols);
	extern int picwindthere;
	extern WINDOW picture_windows[MAX_PIC];


	dopt_tree = wind_s[WIND_DOPT].resource_form;

	if(!openmode)
		Dialog.dispOpt.updateWindow(1, 1);

	button = Dialog.init(WIND_DOPT, DISPLAY_OK);

	/* Farbtiefen-Radiobutton wurde angeklickt */
	if(button == DITHER_24 || button == DITHER_8 || button == DITHER_4)
		Dialog.dispOpt.updateWindow(0, 1);

	/* Dithering einstellen */
	else
	if(button == DITHER_POPBUT || button == DITHER_CB)
	{
		if(dopt_tree[DITHER_24].ob_state&SELECTED)
			ditmode = Sys_info.dither24;
		else
			if(dopt_tree[DITHER_8].ob_state&SELECTED)
				ditmode = Sys_info.dither8;
			else
				if(dopt_tree[DITHER_4].ob_state&SELECTED)
					ditmode = Sys_info.dither4;
	
		olditem=popups[POPUP_DITHER].item;
		popups[POPUP_DITHER].item = ditmode;
		
		popback = f_pop(&popups[POPUP_DITHER], 0, button, NULL);

		popups[POPUP_DITHER].item = olditem;
		
		if(popback > 0)
		{
			if(dopt_tree[DITHER_4].ob_state&SELECTED)
			{
				Sys_info.dither4 = popback;
				if(ditmod_info[popback - 1]->pal_mode == FIXPAL)
					Sys_info.pal4 = CR_FIXPAL;
				else
					if(Sys_info.pal4 == CR_FIXPAL)
						Sys_info.pal4 = CR_SYSPAL;
			}
			else
			if(dopt_tree[DITHER_8].ob_state&SELECTED) 
			{
				Sys_info.dither8 = popback;
				if(ditmod_info[popback - 1]->pal_mode == FIXPAL)
					Sys_info.pal8 = CR_FIXPAL;
				else
					if(Sys_info.pal8 == CR_FIXPAL)
						Sys_info.pal8 = CR_SYSPAL;
			}
			else
			if(dopt_tree[DITHER_24].ob_state&SELECTED)
			{
				Sys_info.dither24 = popback;
				if(ditmod_info[popback - 1]->pal_mode == FIXPAL)
					Sys_info.pal24 = CR_FIXPAL;
				else
					if(Sys_info.pal24 == CR_FIXPAL)
						Sys_info.pal24 = CR_SYSPAL;
			}

			Dialog.dispOpt.updateWindow(0,1);
		}
		else
			f_deselect_popup(&wind_s[WIND_DOPT], DITHER_POPBUT, DITHER_CB);
	}
	else
	if(button == COLRED_POPBUT || button == COLRED_CB)
	{
		if(dopt_tree[DITHER_24].ob_state&SELECTED)
			ditmode = Sys_info.pal24;
		else
			if(dopt_tree[DITHER_8].ob_state&SELECTED)
				ditmode = Sys_info.pal8;
			else
				if(dopt_tree[DITHER_4].ob_state&SELECTED)
					ditmode = Sys_info.pal4;

		olditem=popups[POPUP_COLRED].item;
		popups[POPUP_COLRED].item=ditmode;

		strcpy(colred_popup[CR_FILEPAL].TextCast, filepal_name);

		popback=f_pop(&popups[POPUP_COLRED], 0, button, NULL);

		popups[POPUP_COLRED].item = olditem;

		if(popback > 0)
		{
			if(dopt_tree[DITHER_4].ob_state&SELECTED)
				Sys_info.pal4 = popback;
			else
				if(dopt_tree[DITHER_8].ob_state&SELECTED)
					Sys_info.pal8 = popback;
			else
				if(dopt_tree[DITHER_24].ob_state&SELECTED)
					Sys_info.pal24 = popback;

			Dialog.dispOpt.updateWindow(0, 1);
		}
		else
			f_deselect_popup(&wind_s[WIND_DOPT], COLRED_POPBUT, COLRED_CB);
	}
	/*--------- feste Palette laden -------*/
	else
	if(button == LOAD_PAL)
	{
		pal_loadpath = load_palfile(Sys_info.standard_path, fix_red, fix_green, fix_blue, Sys_info.Max_col+1);
		if(pal_loadpath != NULL)
		{
			strcpy(filepal_name, strrchr(pal_loadpath, '\\')+1);
			strcpy(colred_popup[CR_FILEPAL].TextCast, filepal_name);
			strcpy(dopt_tree[COLRED_POPBUT].TextCast, filepal_name);
		}

		change_object(&wind_s[WIND_DOPT], COLRED_POPBUT, UNSEL, 1);
		change_object(&wind_s[WIND_DOPT], LOAD_PAL, UNSEL, 1);
	}


	/*---------------------- Einstellungen Åbernehmen */
	if(button == DISPLAY_OK || button == DISPLAY_SET)
	{
		Display_Opt.dither_24 = Sys_info.dither24;
		Display_Opt.dither_8 = Sys_info.dither8;
		Display_Opt.dither_4 = Sys_info.dither4;

		Display_Opt.syspal_4 = Sys_info.pal4;
		Display_Opt.syspal_8 = Sys_info.pal8;
		Display_Opt.syspal_24 = Sys_info.pal24;

		if(Dialog.dispOpt.tree[PAL_MOUSE].ob_state&SELECTED)
			Display_Opt.palette_mode = PAL_MOUSE;
		else
			if(Dialog.dispOpt.tree[PAL_TOPWIN].ob_state&SELECTED)
				Display_Opt.palette_mode = PAL_TOPWIN;
			else
				if(Dialog.dispOpt.tree[PAL_SYSTEM].ob_state&SELECTED)
					Display_Opt.palette_mode = PAL_SYSTEM;
				
		Sys_info.Event_Timer = atoi(Dialog.dispOpt.tree[PAL_TIMER].TextCast);

		/* ist unbedingt nîtig da bei 0 im Sys_info.Event_Timer zum Ruckeln */
		/* bei Fenstermoves und seltsamerweise zum WIND_CLOSE beim Verschieben */
		/* des Displayoptions-Fensters fÅhrt */
		if(Sys_info.Event_Timer == 0)
			Sys_info.Event_Timer = 1;
	
		if(button == DISPLAY_OK)
		{
			Dialog.dispOpt.updateWindow(1, 0);
			change_object(&wind_s[WIND_DOPT], DISPLAY_OK, UNSEL, 1);
			
			Dialog.close(WIND_DOPT);
		}

		if(Sys_info.realtime_dither)
		{
			Dialog.busy.reset(0, "Updating display");
			for(t = 0; t <= picwindthere; t++)
			{
				Dialog.busy.draw(t * 128 / (picwindthere + 1));
				Window.redraw(&picture_windows[t], NULL, 0, DRAWNOTREE);
			}

			Dialog.busy.ok();
		}
	}

	if(button == DISPLAY_SET)
		change_object(&wind_s[WIND_DOPT], DISPLAY_SET, UNSEL, 1);

	return;
} /* f_display_opt */


/*------------------------ Display-Options Popups updaten. -----------------*/
/* mode: 0=aus Sysinfo (nicht Åbernommene Werte)							*/
/* 		 1=aus Display_Opt (Åbernommene Werte)								*/
void f_update_dwindow(int mode, int redraw)
{
	int dit4, dit8, dit24;
	int pal4, pal8, pal24;
	int rpal, rdit=0;

	OBJECT *dopt_tree;


	if(mode == 0)
	{
		dit4 = Sys_info.dither4;
		dit8 = Sys_info.dither8;
		dit24 = Sys_info.dither24;
		pal4 = Sys_info.pal4;
		pal8 = Sys_info.pal8;
		pal24 = Sys_info.pal24;
	}
	else
		if(mode == 1)
		{
			dit4 = Display_Opt.dither_4;
			dit8 = Display_Opt.dither_8;
			dit24 = Display_Opt.dither_24;
			pal4 = Display_Opt.syspal_4;
			pal8 = Display_Opt.syspal_8;
			pal24 = Display_Opt.syspal_24;
		}

	dopt_tree = wind_s[WIND_DOPT].resource_form;

	if(dopt_tree[DITHER_4].ob_state&SELECTED)
	{
		rdit = dit4;
		rpal = pal4;
	}
	else
		if(dopt_tree[DITHER_8].ob_state&SELECTED)
		{
			rdit = dit8;
			rpal = pal8;
		}
		else
			if(dopt_tree[DITHER_24].ob_state&SELECTED)
			{
				rdit = dit24;
				rpal = pal24;
			}

	strncpy(dopt_tree[DITHER_POPBUT].TextCast, col_pop[rdit].TextCast, 15);
	strncpy(dopt_tree[COLRED_POPBUT].TextCast, colred_popup[rpal].TextCast, 15);

	if(rpal == CR_FILEPAL)
		change_object(&wind_s[WIND_DOPT], LOAD_PAL, ENABLED, 1);
	else
		change_object(&wind_s[WIND_DOPT], LOAD_PAL, DISABLED, 1);


	if(rpal == CR_FIXPAL)
	{
		change_object(&wind_s[WIND_DOPT], COLRED_POPBUT, DISABLED, 0);
		change_object(&wind_s[WIND_DOPT], COLRED_CB, DISABLED, 0);
	}
	else
	{
		change_object(&wind_s[WIND_DOPT], COLRED_POPBUT, ENABLED, 0);
		change_object(&wind_s[WIND_DOPT], COLRED_CB, ENABLED, 0);
	}

	if(redraw)
	{
		f_deselect_popup(&wind_s[WIND_DOPT], DITHER_POPBUT, DITHER_CB);
		f_deselect_popup(&wind_s[WIND_DOPT], COLRED_POPBUT, COLRED_CB);
	}

	return;
} /* f_update_dwindow */