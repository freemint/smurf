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

#include "tos.h"
#include "multiaes.h"
#include "vdi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "smurf.h"
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "destruct.h"

#include "smurfobs.h"
#include "ext_obs.h"


extern	SMURF_PIC *smurf_picture[MAX_PIC];
extern	WINDOW picture_windows[MAX_PIC];
extern	WINDOW	wind_s[25];
extern	int active_pic;

extern	char *edit_modules[100];		/* Pfade fÅr bis zu 100 Edit-Module */

extern	SYSTEM_INFO	Sys_info;			/* Systemkonfiguration */
extern	int	openmode;					/* Dialog neu geîffnet (0) oder buttonevent? (!=0) */
extern	int	key_scancode;				/* Scancode beim letzten Keyboard-Event */
extern	int	klicks;						/* Anzahl Mausklicks beim letzten Buttonevent */

extern	int	obj;
extern	int picthere;

extern	long sx1, sx2, sx3, sx4;	/* Maxima */
extern	long sn1, sn2, sn3, sn4;	/* Minima */
extern	CROSSHAIR	position_markers[20];		/* Positionsmarker fÅr die Editmodule */
extern	signed char module_pics[21][7];


/* ----------------------------------------------------------------	*/
/* ------------------------ EDIT-MODUL-WINDOW ----------------------*/
/* ----------------------------------------------------------------	*/
void f_edit_pop(void)
{
	char alert[128];
	char *mpath;
	char *textseg_begin;

	int button = 0, back = 0;
	int mod_num = 0, mod_index;
	int oldwid, oldhgt, oldtop;
	int info = 0;
	int pic_needed;
	int my_scancode;
	int message;

	MOD_INFO *module_info;
	MOD_INFO *mod_info;
	MOD_ABILITY *mod_abs;
	GARGAMEL *garg_st;
	SMURF_PIC *picture_to_handle;

	extern DISPLAY_MODES Display_Opt;


	my_scancode = (key_scancode&0xff00) >> 8;

	info = Dialog.emodList.tree[INFO_MODULE].ob_state&SELECTED;

	/*
	 * Neuinitialisierung des Dialogs
	 */
	if(my_scancode != KEY_UP && my_scancode != KEY_DOWN)
	{
		button = Dialog.init(WIND_MODULES, START_MODULE);

		if(obj == DIALOG_EXIT)
		{
			if(!info)
				button = START_MODULE;
			else
				button = MOD_INFO_OK;
		}
	}

	/*
	 * selektiertes Eintrags-Objekt ermitteln! 
	 */
	if(!info)
		mod_index = f_listfield((long*)&wind_s[WIND_MODULES], button, key_scancode, &Dialog.emodList.modList);
	else
		mod_index = f_listfield((long*)&wind_s[WIND_MODULES], 0, 0, &Dialog.emodList.modList);

	if(key_scancode && my_scancode != KEY_UP && my_scancode != KEY_DOWN || !openmode)
		Window.windSet(wind_s[WIND_MODULES].whandlem, WF_INFO, LONG2_2INT((long)Dialog.emodList.modList.autolocator), 0,0);

	/*
	 * "Info" angeklickt?
	 */
	if(button == INFO_MODULE && mod_index != -1 && openmode && Dialog.emodList.modNames[mod_index]!=NULL && strlen(Dialog.emodList.modNames[mod_index]) != 0)
		Dialog.emodList.infoOn(mod_index);
	else
		if(info && button == MOD_INFO_OK && openmode)
			Dialog.emodList.infoOff();

	/*
	 * Modul wurde aufgerufen
	 */
	if((button == START_MODULE || (klicks == 2 && button >= MODULE1 && button <= MODULE9)) && !info &&
	   Dialog.emodList.modNames[mod_index] != NULL && strlen(Dialog.emodList.modNames[mod_index]) != 0)
	{
		mpath = edit_modules[mod_index];					/* Modulpfad holen */
		change_object(&wind_s[WIND_MODULES], START_MODULE, UNSEL, 1);

		/*
		 * Modulinformationen holen
		 */
		module.smStruct[20] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
		memset(module.smStruct[20], 0x0, sizeof(GARGAMEL));
		if((mod_abs = (MOD_ABILITY *)module.comm.startEdit(mpath, module.bp[20], MQUERY, 20, module.smStruct[20])) == NULL)
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[EMOD_START_ERR].TextCast, NULL, NULL, NULL, 1);

		textseg_begin = module.bp[20]->p_tbase;
		mod_info = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));
	
		pic_needed = mod_info->how_many_pix;
		module.smStruct[20]->module_mode=M_EXIT;
		check_and_terminate(MTERM, 20);

		if(!picthere && pic_needed > 0)
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_PIC_LOADED].TextCast, NULL, NULL, NULL, 1);
		else
		{
			/*
			 * freie Modulstruktur ermitteln
			 */
			mod_num = give_free_module();
			module.smStruct[mod_num] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
		
			if(mpath != NULL)
			{
				if(picthere || pic_needed)
				{
					picture_to_handle = smurf_picture[active_pic];
					if(picture_to_handle->block)
						picture_to_handle = picture_to_handle->block;
						
					oldwid = picture_to_handle->pic_width;
					oldhgt = picture_to_handle->pic_height;
				}

				garg_st = module.smStruct[mod_num];
				garg_st->module_mode = 127;
				garg_st->module_number = mod_num;
				garg_st->smurf_pic = picture_to_handle;
				
				Dialog.busy.reset(0, Dialog.emodList.modNames[mod_index]);
				module.bp[mod_num] = module.comm.startEdit(mpath, module.bp[mod_num], MSTART, mod_num, garg_st);
				
				if(module.bp[mod_num]==NULL)
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[EMOD_START_ERR].TextCast, NULL, NULL, NULL, 1);
				
				f_handle_modmessage(module.smStruct[mod_num]);

				Dialog.busy.ok();
				Dialog.busy.dispRAM();

				/*
				 * Modul will sofort gestartet werden?
				 */
				if(module.smStruct[mod_num]->module_mode == M_STARTED)
				{
					/* Sicherheitsabfrage */
					if(Sys_info.profi_mode != SELECTED)
					{
						strcpy(alert, "[2][");
						strcat(alert, Dialog.emodList.modNames[mod_index]);
						strcat(alert, "|ausfÅhren?][ Ja | Nein ]");
						
						if(form_alert(1, alert) == 2)
						{
							module.comm.startEdit(edit_modules[mod_num], module.bp[mod_num], MTERM, mod_num, module.smStruct[mod_num]);
							check_and_terminate(module.smStruct[mod_num]->module_mode, mod_num);
							return;
						}
					}
					
					Window.windGet(0, WF_TOP, &oldtop, 0, 0, 0);
					Window.topNow(&wind_s[WIND_BUSY]);
					Window.redraw(&wind_s[WIND_BUSY], NULL, 0, 0);
					
					/*
					 * Textsegment-Startadresse holen
					 */
					textseg_begin = module.bp[mod_num]->p_tbase;
					mod_info = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));
					mod_abs = *((MOD_ABILITY **)(textseg_begin + MOD_ABS_OFFSET));

					back = 0;
					
					if(picthere || pic_needed)
					{
						if(mod_info->how_many_pix > 1)
							back = f_give_pics(mod_info, mod_abs, mod_num);
						else
							f_convert(picture_to_handle, mod_abs, RGB, SAME, 0);
					}
					
					/*
					 * Bilder Åbergeben, -> karierte Flagge fÅrs Modul
					 */
					if(back == 0)
					{
						Dialog.busy.reset(0, mod_info->mod_name);
						module.comm.startEdit(edit_modules[mod_num], module.bp[mod_num], MEXEC, mod_num, module.smStruct[mod_num]);
						Dialog.busy.ok();
					}

					/*
					 * Modul ist fertig - neu dithern, etc.
					 */
					if((module.smStruct[mod_num]->module_mode == M_DONEEXIT || module.smStruct[mod_num]->module_mode == M_PICDONE) && (picthere || pic_needed))
					{
						/* damit bei BildÑnderungen im Fall von Median-Cut */
						/* auch die Palette neu berechnet wird und sowieso */
						if(picture_to_handle->local_nct)
						{
							SMfree(picture_to_handle->local_nct);
							picture_to_handle->local_nct = NULL;
						}

						if(!Sys_info.realtime_dither)
							f_dither(picture_to_handle, &Sys_info, 1, NULL, &Display_Opt);

						if(picture_to_handle->pic_width != oldwid ||  picture_to_handle->pic_height != oldhgt)
							imageWindow.clipPicwin(&picture_windows[active_pic]);

						/*
						 * kompletter Redraw wenn kein RT-Dither oder kein Block
						 */
						if(Sys_info.realtime_dither && picture_to_handle->changed == 255)
						{
							Window.redraw(&picture_windows[active_pic], NULL, 0, DRAWNOTREE|DRAWNOBLOCK|BLOCK_ONLY);
							Window.redraw(&picture_windows[active_pic], NULL, 0, BLOCK_ONLY);
						}
						else
							Window.redraw(&picture_windows[active_pic], NULL, 0, 0);

						f_pic_changed(&picture_windows[active_pic], 1);
					}
					/*
					 * Modul wartet - Infostruktur auslesen
					 */
					else
						if(module.bp[mod_num] != NULL && module.smStruct[mod_num]->module_mode == M_WAITING)
						{
							textseg_begin = (char*)module.bp[mod_num]->p_tbase;				/* Zeiger auf Modulinfostruktur */
							module_info = (MOD_INFO*)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));
							sn1 = module_info->smin1;
							sx1 = module_info->smax1;
							sn2 = module_info->smin2;
							sx2 = module_info->smax2;
							sn3 = module_info->smin3;
							sx3 = module_info->smax3;
							sn4 = module_info->smin4;
							sx4 = module_info->smax4;
						}

					Dialog.busy.ok();
					Window.topHandle(oldtop);
				}

				/*
				 * Modul hat einen Dialog angefordert und wartet
				 */
				else
				if(module.smStruct[mod_num]->module_mode == M_WAITING)
				{
					module.comm.startEdit(mpath, module.bp[mod_num], MCROSSHAIR, mod_num, garg_st);
					if(module.smStruct[mod_num]->module_mode == M_CROSSHAIR)
					{
						position_markers[mod_num].anzahl = module.smStruct[mod_num]->event_par[0];
						position_markers[mod_num].mod_pic[0] = module.smStruct[mod_num]->event_par[1];
						module.smStruct[mod_num]->module_mode = M_WAITING;

						/* Defaultkoordinaten erfragen */
						module.comm.startEdit("", module.bp[mod_num], MCH_DEFCOO, mod_num, garg_st);
						if(module.smStruct[mod_num]->module_mode == M_CHDEFCOO)
						{
							position_markers[mod_num].xpos[0] = module.smStruct[mod_num]->event_par[0];
							position_markers[mod_num].ypos[0] = module.smStruct[mod_num]->event_par[1];
						}

						if(mod_info->how_many_pix == 1 && position_markers[mod_num].anzahl >= 1)
						{
							position_markers[mod_num].mod_pic[0] = -2;			/* aktives Bild */
							position_markers[mod_num].smurfpic[0] = active_pic;	/* aktives Bild */
							module_pics[mod_num][0] = active_pic;
							imageWindow.drawCrosshair(&picture_windows[active_pic]);
						}
					}

					if(module.smStruct[mod_num]->wind_struct != &wind_s[WIND_MODFORM])
						transmitConfig(module.bp[mod_num], module.smStruct[mod_num]);
				}
				
				/*
				 * ggfs. Modul terminieren und alle anderen Module Åber BildÑnderung informieren
				 */
				 message = module.smStruct[mod_num]->module_mode;
				 
				if(message == M_MODERR)
					module.comm.checkTerminate(MTERM, mod_num);
				else
					module.comm.checkTerminate(message, mod_num);

				if(message == M_PICDONE || message == M_DONEEXIT)
					module.comm.inform(MPIC_UPDATE, smurf_picture[active_pic]);
			}

		}
	}

	return;
}


void emod_info_on(int mod_index)
{
	char str[10], *filename;
	char *textseg_begin;

	MOD_INFO *info_mi;
	MOD_ABILITY *info_mabs;


	module.smStruct[20] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
	memset(module.smStruct[20], 0x0, sizeof(GARGAMEL));
	if((info_mabs = (MOD_ABILITY *)module.comm.startEdit(edit_modules[mod_index], module.bp[20], MQUERY, 20, module.smStruct[20])) == NULL)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[EMOD_START_ERR].TextCast, NULL, NULL, NULL, 1);

	textseg_begin = module.bp[20]->p_tbase;
	info_mi = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));

	strncpy(Dialog.emodList.infoTree[I_MODNAME].TextCast, Dialog.emodList.modNames[mod_index], 27);

	BCD2string(Dialog.emodList.infoTree[I_MODVER].TextCast, info_mi->version);

	strcpy(Dialog.emodList.infoTree[I_MODAUTOR].TextCast, "");
	strncat(Dialog.emodList.infoTree[I_MODAUTOR].TextCast, info_mi->autor, 27);

	filename = strrchr(edit_modules[mod_index], '\\');
	strncpy(Dialog.emodList.infoTree[I_MODPATH].TextCast, filename+1, 12);
		
	strcpy(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, "");

	if(info_mabs->depth1 > 0)
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, itoa(info_mabs->depth1, str, 10), 2);
	if(info_mabs->depth2 > 0)
	{
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, itoa(info_mabs->depth2, str, 10), 2);
	}
	if(info_mabs->depth3 > 0)
	{
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, itoa(info_mabs->depth3, str, 10), 2);
	}
	if(info_mabs->depth4 > 0)
	{
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, itoa(info_mabs->depth4, str, 10), 2);
	}
	if(info_mabs->depth5 > 0)
	{
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, itoa(info_mabs->depth5, str, 10), 2);
	}
	if(info_mabs->depth6 > 0)
	{
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, itoa(info_mabs->depth6, str, 10), 2);
	}
	if(info_mabs->depth7 > 0)
	{
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, itoa(info_mabs->depth7, str, 10), 2);
	}
	if(info_mabs->depth8 > 0)
	{
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.emodList.infoTree[I_MODDEPTH].TextCast, itoa(info_mabs->depth8, str, 10), 2);
	}

	module.smStruct[20]->module_mode = M_EXIT;					/* Modulende "simulieren" */
	check_and_terminate(MTERM, 20);
	wind_s[WIND_MODULES].resource_form=Dialog.emodList.infoTree;

	Dialog.emodList.infoTree[0].ob_x = wind_s[WIND_MODULES].wx;
	Dialog.emodList.infoTree[0].ob_y = wind_s[WIND_MODULES].wy;

	Window.redraw(&wind_s[WIND_MODULES], NULL, 0, 0);

	return;
} /* emod_info_on */


/* emod_info_off --------------------------------------------------	
	Ausschalten der Editmodul-Info im Listfenster
	---------------------------------------------------------------	*/
void emod_info_off(void)
{
	/*
	 * ist die Info Åberhaupt eingeschaltet?
	 */
	if(wind_s[WIND_MODULES].resource_form == Dialog.emodList.infoTree)
	{
		change_object(&wind_s[WIND_MODULES], MOD_INFO_OK, UNSEL, 0);
		wind_s[WIND_MODULES].resource_form = Dialog.emodList.tree;

		Dialog.emodList.tree[0].ob_x = wind_s[WIND_MODULES].wx;
		Dialog.emodList.tree[0].ob_y = wind_s[WIND_MODULES].wy;

		change_object(&wind_s[WIND_MODULES], INFO_MODULE, UNSEL, 0);
	}

	Window.redraw(&wind_s[WIND_MODULES], NULL, 0, 0);

	return;
} /* emod_info_off */