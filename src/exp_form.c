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

/**************************************************************	*/
/*																*/
/*							EXP_FORM.C							*/
/*																*/
/*	Handling des kompletten Export-Dialogs. Ruft ggfs. die		*/
/*	Ditherroutinen, die convert-Routs und save_pic auf.			*/
/*																*/
/**************************************************************	*/

#include <tos.h>
#include <multiaes.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurfine.h"
#include "smurf_f.h"
#include "popdefin.h"
#include "popdefin.h"

#include "globdefs.h"

#include "smurfobs.h"
#include "ext_obs.h"


/*------------- GUI-Kram --------------*/
extern OBJECT	*export_form;				/* Export-Formular	*/
extern OBJECT 	*col_pop;					/* Zeiger auf Resource-DITHERPOPUP	*/
extern OBJECT	*colred_popup;
extern	WINDOW	wind_s[25];
extern	POP_UP	popups[25];
extern	int	openmode;				/* Dialog neu geîffnet (0) oder buttonevent? (!=0) */
extern	int	obj;					/* Objekt beim loslassen des Buttons */
extern	OBJECT	*exp_dp_popup;
extern	OBJECT	*colred_popup;

extern	SMURF_PIC	*smurf_picture[MAX_PIC];
extern	DITHER_MOD_INFO *ditmod_info[10];
extern	int	active_pic;
extern	int	key_scancode;			/* Scancode beim letzten Keyboard-Event */

extern	int	export_depth[8];
extern	int	export_format[8];
extern	char *export_path;			/* Pfad des Export-Modules	*/
extern	char *export_modules[100];		/* Pfade fÅr bis zu 100 Export-Module */
extern	SYSTEM_INFO Sys_info;			/* Systemkonfiguration */

extern	long 	f_len;					/* LÑnge des letzten geladenen Files */

extern	EXPORT_CONFIG exp_conf;

MOD_ABILITY export_mod_ability;
int dest_colsys;

/*------------- lokale Funktionen --------------*/
int best_depth(int desired_depth);
void get_picform(MOD_ABILITY *dest_mabs, int ed_button);
void act_exdither_popups(void);


/**********************************************************************	*/
/*								Export-Dialog							*/
/**********************************************************************	*/
void f_export_formular()
{
	int pop_button, button, enablemode;
	int old_display_obj, old_cycle;
	OBJECT *old_display_tree, *exp_form;
	int ed_pop;
	int bdepth, bestdepth;
	int old_picdepth;
	MOD_INFO *export_modinfo;
	MOD_ABILITY *embs;
	char *textseg_begin;
	static char	export_filepal_name[32] = "feste Palette";
	int exp_index;
	char *pal_loadpath;

	extern char *load_palfile(char *path, int *red, int *green, int *blue, int max_cols);


	exp_index = exp_conf.export_mod_num&0xFF;

	exp_form = wind_s[FORM_EXPORT].resource_form;

	Dialog.init(FORM_EXPORT, START_EXPORT);

	/*
	 * Exportdialog neu geîffnet oder neu initialisieren?
	 */
	if(!openmode || openmode == 2)
	{
		memset(&export_mod_ability, 0x0, sizeof(MOD_ABILITY));
	
		/*
		 * vorinitialisieren des Farbtiefenbuttons
		 */
		bestdepth = best_depth(smurf_picture[active_pic]->depth);
		
		if(bestdepth == 24)
			bdepth = EXP_D24;
		else
			if(bestdepth == 16)
				bdepth = EXP_D16;
			else
				if(bestdepth == 8)
					bdepth = EXP_D8;
				else
					if(bestdepth == 4)
						bdepth = EXP_D4;
					else
						if(bestdepth == 2)
							bdepth = EXP_D2;
						else
							if(bestdepth == 1)
								bdepth = EXP_D1;
		
		popups[POPUP_EXP_DEPTH].item = bdepth;
		strncpy(exp_form[EXP_DEPTH].TextCast, exp_dp_popup[bdepth].TextCast, 6);
		ed_pop = bdepth;
		exp_conf.exp_colred = CR_SYSPAL;
		
		/*
		 * Farbsystem holen
		 */
		old_picdepth = smurf_picture[active_pic]->depth;
		smurf_picture[active_pic]->depth = bestdepth;
		module.comm.startExport(export_path, MCOLSYS, smurf_picture[active_pic], module.bp[exp_index], module.smStruct[exp_index], exp_conf.export_mod_num);
		if(module.smStruct[exp_index]->module_mode == M_COLSYS)
			dest_colsys = module.smStruct[exp_index]->event_par[0];
		smurf_picture[active_pic]->depth = old_picdepth;

		/*
		 * und die Ditherpoups ein/abschalten
		 */
		get_picform(&export_mod_ability, bdepth);
		act_exdither_popups();
	
		/*
		 * Formulartitel setzen
		 */
		textseg_begin = module.bp[exp_index]->p_tbase;		/* Textsegment-Startadresse holen */
		export_modinfo = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));
		embs = (MOD_ABILITY *)*((MOD_ABILITY **)(textseg_begin + MOD_ABS_OFFSET));
		strcpy(wind_s[FORM_EXPORT].wtitle, export_modinfo->mod_name);

		Window.windSet(wind_s[FORM_EXPORT].whandlem, WF_NAME, 
			LONG2_2INT((long)wind_s[FORM_EXPORT].wtitle), 0, 0);

		/*
		 * More-Button dis-/enablen
		 */
		if((embs->ext_flag)&0x02)
			change_object(&wind_s[FORM_EXPORT], EXPORT_MORE, ENABLED, 0);
		else
			change_object(&wind_s[FORM_EXPORT], EXPORT_MORE, DISABLED, 0);

		button = 0;
	}
	else
		button = obj;

	if(obj == DIALOG_EXIT)
		button = START_EXPORT;

	switch(button)
	{
		case EXP_DEPTH:
		case EXP_DEPTHC:	ed_pop = f_pop(&popups[POPUP_EXP_DEPTH], 0, button, NULL);
							f_deselect_popup(&wind_s[FORM_EXPORT], EXP_DEPTH, EXP_DEPTHC);
							if(ed_pop > 0)
							{
								get_picform(&export_mod_ability, ed_pop);

								/*
								 * Farbsystem holen
								 */
								old_picdepth = smurf_picture[active_pic]->depth;
								smurf_picture[active_pic]->depth = export_mod_ability.depth1;
								module.comm.startExport(export_path, MCOLSYS, smurf_picture[active_pic], module.bp[exp_index], module.smStruct[exp_index], exp_conf.export_mod_num);
								if(module.smStruct[exp_index]->module_mode == M_COLSYS) 
									dest_colsys = module.smStruct[exp_index]->event_par[0];
								smurf_picture[active_pic]->depth = old_picdepth;

								/*
								 * Dither-Popups ein/ausschalten
								 */
								act_exdither_popups();

								if(exp_conf.exp_dither>0 && export_mod_ability.depth1<=smurf_picture[active_pic]->depth)
								{
									if(ditmod_info[exp_conf.exp_dither-1]->pal_mode==FIXPAL || dest_colsys==GREY)
									{
										exp_conf.exp_colred = CR_FIXPAL;
										strncpy(exp_form[EXP_COLRED].TextCast, colred_popup[exp_conf.exp_colred].TextCast, 15);
										enablemode=DISABLED;
									}
									else 
									{
										exp_conf.exp_colred = CR_SYSPAL;
										strncpy(exp_form[EXP_COLRED].TextCast, colred_popup[exp_conf.exp_colred].TextCast, 15);
										enablemode=ENABLED;
									}

									change_object(&wind_s[FORM_EXPORT], EXP_COLRED, enablemode, 1);
									change_object(&wind_s[FORM_EXPORT], EXP_COLREDC, enablemode, 1);
								}
							}

							break;
	
		case EXP_DITHER:
		case EXP_DITHERC:	old_display_tree = popups[POPUP_DITHER].display_tree;
							old_display_obj = popups[POPUP_DITHER].display_obj;
							old_cycle = popups[POPUP_DITHER].Cyclebutton;
							popups[POPUP_DITHER].display_tree = exp_form;
							popups[POPUP_DITHER].display_obj = EXP_DITHER;
							popups[POPUP_DITHER].Cyclebutton = EXP_DITHERC;
							pop_button = f_pop(&popups[POPUP_DITHER], 0, button, NULL);
							popups[POPUP_DITHER].display_tree = old_display_tree;
							popups[POPUP_DITHER].display_obj = old_display_obj;
							popups[POPUP_DITHER].Cyclebutton = old_cycle;
							f_deselect_popup(&wind_s[FORM_EXPORT], EXP_DITHER, EXP_DITHERC);

							if(pop_button > 0)
							{
								exp_conf.exp_dither = pop_button;

								if(ditmod_info[exp_conf.exp_dither-1]->pal_mode==FIXPAL) 
								{
									exp_conf.exp_colred = CR_FIXPAL;
									strncpy(exp_form[EXP_COLRED].TextCast, colred_popup[exp_conf.exp_colred].TextCast, 15);
									enablemode=DISABLED;
								}
								else 
								{
									exp_conf.exp_colred = CR_SYSPAL;
									popups[POPUP_COLRED].item=CR_SYSPAL;
									strncpy(exp_form[EXP_COLRED].TextCast, colred_popup[exp_conf.exp_colred].TextCast, 15);
									enablemode = ENABLED;
								}
								change_object(&wind_s[FORM_EXPORT], EXP_COLRED, enablemode, 1);
								change_object(&wind_s[FORM_EXPORT], EXP_COLREDC, enablemode, 1);
								change_object(&wind_s[FORM_EXPORT], LOAD_EXPPAL, DISABLED, 1);
							}
							break;
	
		case EXP_COLRED:
		case EXP_COLREDC:	old_display_tree = popups[POPUP_COLRED].display_tree;
							old_display_obj = popups[POPUP_COLRED].display_obj;
							old_cycle = popups[POPUP_COLRED].Cyclebutton;
							popups[POPUP_COLRED].display_tree = exp_form;
							popups[POPUP_COLRED].display_obj = EXP_COLRED;
							popups[POPUP_COLRED].Cyclebutton = EXP_COLREDC;

							strcpy(colred_popup[CR_FILEPAL].TextCast, export_filepal_name);
							pop_button = f_pop(&popups[POPUP_COLRED], 0, button, NULL);

							if(pop_button != -1) 
								exp_conf.exp_colred=pop_button;
							popups[POPUP_COLRED].display_tree = old_display_tree;
							popups[POPUP_COLRED].display_obj = old_display_obj;
							popups[POPUP_COLRED].Cyclebutton = old_cycle;
							f_deselect_popup(&wind_s[FORM_EXPORT], EXP_COLRED, EXP_COLREDC);

							if(exp_conf.exp_colred == CR_FILEPAL)
								change_object(&wind_s[FORM_EXPORT], LOAD_EXPPAL, ENABLED, 1);
							else
								change_object(&wind_s[FORM_EXPORT], LOAD_EXPPAL, DISABLED, 1);
							break;

		case EXPORT_MORE:	change_object(&wind_s[FORM_EXPORT], EXPORT_MORE, UNSEL, 1);
							module.comm.startExport(export_path, MMORE, smurf_picture[active_pic], module.bp[exp_index], module.smStruct[exp_index], exp_conf.export_mod_num);
							break;

		case START_EXPORT:	change_object(&wind_s[FORM_EXPORT], START_EXPORT, UNSEL, 1);
							f_save_pic(&export_mod_ability);
							break;
		
		case LOAD_EXPPAL:	pal_loadpath = load_palfile(Sys_info.standard_path, exp_conf.exp_fix_red, exp_conf.exp_fix_green, exp_conf.exp_fix_blue, (1<<export_mod_ability.depth1));
							if(pal_loadpath != NULL)
							{
								strcpy(export_filepal_name, strrchr(pal_loadpath, '\\')+1);
								strcpy(colred_popup[CR_FILEPAL].TextCast, export_filepal_name);
								strcpy(exp_form[EXP_COLRED].TextCast, export_filepal_name);
							}
							change_object(&wind_s[FORM_EXPORT], EXP_COLRED, UNSEL, 1);
							change_object(&wind_s[FORM_EXPORT], LOAD_EXPPAL, UNSEL, 1);
							break;
	}

	return;
} /* f_export_formular */



/**********************************************************************	*/
/*							get_picform									*/
/*		Setzt in dest_mabs das zu ed_button (Farbtiefe) passende		*/
/*		Datenformat und die Farbtiefe aus src_mabs.						*/
/**********************************************************************	*/
void get_picform(MOD_ABILITY *dest_mabs, int ed_button)
{
	if(ed_button == EXP_D24)
		dest_mabs->depth1 = 24;
	else
		if(ed_button == EXP_D16)
			dest_mabs->depth1 = 16;
		else
			if(ed_button == EXP_D8)
				dest_mabs->depth1 = 8;
			else
				if(ed_button == EXP_D4)
					dest_mabs->depth1 = 4;
				else
					if(ed_button == EXP_D2)
						dest_mabs->depth1 = 2;
					else
						if(ed_button == EXP_D1)
							dest_mabs->depth1 = 1;

	if(export_depth[0] == dest_mabs->depth1)
		dest_mabs->form1 = export_format[0];
	if(export_depth[1] == dest_mabs->depth1)
		dest_mabs->form1 = export_format[1];
	if(export_depth[2] == dest_mabs->depth1)
		dest_mabs->form1 = export_format[2];
	if(export_depth[3] == dest_mabs->depth1)
		dest_mabs->form1 = export_format[3];
	if(export_depth[4] == dest_mabs->depth1)
		dest_mabs->form1 = export_format[4];
	if(export_depth[5] == dest_mabs->depth1)
		dest_mabs->form1 = export_format[5];
	if(export_depth[6] == dest_mabs->depth1)
		dest_mabs->form1 = export_format[6];
	if(export_depth[7] == dest_mabs->depth1)
		dest_mabs->form1 = export_format[7];

	exp_conf.exp_depth = dest_mabs->depth1;
	exp_conf.exp_form = dest_mabs->form1;

	return;
} /* get_picform */


/*------------------------------------------------------------------*/
/*							best_depth								*/
/* suchen der qualitativ bestmîglichen Farbtiefe aus den 			*/
/*	export_depth[n] eines Moduls und der gewÅnschten Farbtiefe		*/
/*	desired_depth.													*/
/*------------------------------------------------------------------*/
int best_depth(int desired_depth)
{
	int t;


	for(t = 0; t < 8 && export_depth[t] != 0; t++)
	{
		if(export_depth[t] >= desired_depth)
			break;
	}

	if(export_depth[t] == 0) 
		t--;

	return(export_depth[t]);
} /* best_depth */


/*----------------------------------------------------------------	*/
/*			Ditherpopups im Exportformular aktualisieren 			*/
/*	Baut die Popups um und redrawed sie, je nach den Einstellungen	*/
/*	und den Spezifikationen des Exporters.							*/
/*----------------------------------------------------------------	*/
void act_exdither_popups(void)
{
	int enablemode;

	
	if(export_mod_ability.depth1 > smurf_picture[active_pic]->depth ||
	   dest_colsys == GREY)
	{
		if(dest_colsys == GREY)
		{
			strcpy(export_form[EXP_DITHER].TextCast, "Graustufen");
			strcpy(export_form[EXP_COLRED].TextCast, "feste Palette");
		}
		else
		{
			strcpy(export_form[EXP_DITHER].TextCast, "-");
			strcpy(export_form[EXP_COLRED].TextCast, "-");
		}

		enablemode = DISABLED;
	}
	else
	{
		strcpy(export_form[EXP_DITHER].TextCast, col_pop[exp_conf.exp_dither].TextCast);
		strcpy(export_form[EXP_COLRED].TextCast, colred_popup[exp_conf.exp_colred].TextCast);

		enablemode = ENABLED;
	}

	change_object(&wind_s[FORM_EXPORT], EXP_DITHER, enablemode, 1);
	change_object(&wind_s[FORM_EXPORT], EXP_DITHERC, enablemode, 1);
	change_object(&wind_s[FORM_EXPORT], EXP_COLRED, enablemode, 1);
	change_object(&wind_s[FORM_EXPORT], EXP_COLREDC, enablemode, 1);

	return;
} /* act_exdither_popups */


/* prepare_depthpopup -------------------------------
	Bereitet das Farbtiefenpopup im Exportdialog vor:
	Die Buttons werden nach den EintrÑgen in export_depth[8]
	en- oder disabled geschaltet.
	-------------------------------------------------*/
void prepare_depthpopup(void)
{
	int t, ob;

	extern OBJECT *exp_dp_popup;				/* Farbtiefenpopup fÅr Export	*/
	extern int export_depth[8];

	
	/*
	 * Depth-Popup vorbereiten
	 */
	ob = EXP_D1;
	for(t = 0; t <= 6; t++)
	{
		exp_dp_popup[ob].ob_state |= DISABLED;
		ob++;
	}

	for(t = 0; t < 8; t++)
	{
		switch(export_depth[t])
		{
			case 24:	exp_dp_popup[EXP_D24].ob_state &= ~DISABLED;		break;
			case 16:	exp_dp_popup[EXP_D16].ob_state &= ~DISABLED;		break;
			case 8:		exp_dp_popup[EXP_D8].ob_state &= ~DISABLED;		break;
			case 4:		exp_dp_popup[EXP_D4].ob_state &= ~DISABLED;		break;
			case 2:		exp_dp_popup[EXP_D2].ob_state &= ~DISABLED;		break;
			case 1:		exp_dp_popup[EXP_D1].ob_state &= ~DISABLED;		break;
		}
	}

	return;
} /* prepare_depthpopup */