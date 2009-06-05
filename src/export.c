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
/*#include <multiaes.h>*/
#include <aes.h>
#include <vdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "smurf.h"
#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "popdefin.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "olgagems.h"

#include "smurfobs.h"
#include "ext_obs.h"

/*----- Smurf-Systemspezifisches ----*/
extern	SYSTEM_INFO Sys_info;			/* Systemkonfiguration */
extern	DISPLAY_MODES Display_Opt;
extern	int	obj;						/* Objekt beim loslassen des Buttons */
extern	int active_pic;

/*-------- GUI-Spezifisches -------*/
extern	OBJECT	*menu_tree;
extern	SMURF_PIC *smurf_picture[MAX_PIC];
extern	WINDOW picture_windows[MAX_PIC];
extern	WINDOW wind_s[25];
extern	int picthere;
extern	int klicks;						/* Anzahl Mausklicks beim letzten Buttonevent */
extern	int	key_scancode;				/* Scancode beim letzten Keyboard-Event */
extern	int openmode;					/* Dialog neu geîffnet (0) oder buttonevent? (!=0) */
extern	int key_at_event;
extern	char savepath[257];				/* voller Pfad der zuletzt gespeicherten Datei */

/*--------- Exporterspezifisches ------*/
extern	char *export_modules[100];		/* Pfade fÅr bis zu 100 Export-Module */
extern	char *export_cnfblock[50];
extern	int export_cnflen[50];
extern	char *export_path;				/* Pfad des Export-Modules	*/
extern	int export_depth[8];
extern	int export_format[8];

EXPORT_CONFIG exp_conf;

/*--------	lokale Funktionen	--------------------*/
void init_exmod_info(int mod_index);
int dither_for_export(MOD_ABILITY *mod_abs, int max_expdepth, int dest_format, /*int dest_colsys,*/ SMURF_PIC *converted_pic);
int loadNCT(int loadplanes, SYSTEM_INFO *sysinfo);



/* ------------------------------------------------------------	*/
/*							Bild exportieren  					*/
/*	Handled das Export-Listfeld und ruft, je nach Useraktion	*/
/*	f_export_form oder f_save_pic auf. Die mîglichen Exporter-	*/
/*	farbtiefen und -datenformate werden aus dem evtl. 			*/
/*	gestarteten Modul ausgelesen und in die Felder export_depth	*/
/*	und export_format eingetragen. Auûerdem wird die erweiterte	*/
/*	Konfiguration (Aufrufen, Senden) hier geregelt.				*/
/* ------------------------------------------------------------	*/
void f_export_pic(void)
{
	int mod_num;
	int info;
	int button=0, mod_index, omod_index;
	MOD_ABILITY *export_mabs;
	MOD_INFO *modinfo;
	char *txtbeg;
	OBJECT *exp_form;
	int my_scancode;

	static MOD_ABILITY expmabs;
	extern void	prepare_depthpopup(void);


	my_scancode = key_scancode >> 8;
	info = Dialog.expmodList.tree[EXMOD_INFO].ob_state&SELECTED;
	exp_form = Dialog.expmodList.tree;

	/* aktuelles Listen-Objekt vor énderung ermittelten */
	omod_index = f_listfield((long *)&wind_s[WIND_EXPORT], 0, 0, &Dialog.expmodList.modList);

	if(my_scancode != KEY_DOWN && my_scancode != KEY_UP) 
	{
		button = Dialog.init(WIND_EXPORT, START_EMOD);
		if(obj == DIALOG_EXIT)
		{
			if(!info)
				button = START_EMOD;
			else
				button = EXMOD_INFO_OK;
		}
	}

	/*
	 * selektiertes Listen-Objekt ermitteln
	 * der Index im Feld wird hier zurÅckgegeben, nicht der Objektindex!
	 */
	if(!info)
		mod_index = f_listfield((long *)&wind_s[WIND_EXPORT], button, key_scancode, &Dialog.expmodList.modList);
	else
		mod_index = f_listfield((long *)&wind_s[WIND_EXPORT], 0, 0, &Dialog.expmodList.modList);

	if(key_scancode && my_scancode != KEY_UP && my_scancode != KEY_DOWN || !openmode)
		Window.windSet(wind_s[WIND_EXPORT].whandlem, WF_INFO,
			LONG2_2INT((long)Dialog.expmodList.modList.autolocator), 0, 0);

	/*
	 * wenn gescrollt oder ein anderes Modul selektiert wurde, 
	 * muû der Standardexport-Schalter u.U. selektiert oder deselektiert werden.
	 */
	if(omod_index != mod_index)
	{
		if(Sys_info.defaultExporter != mod_index && IsSelected(exp_form[EXPORT_STANDARD]))
		   change_object(&wind_s[WIND_EXPORT], EXPORT_STANDARD, UNSEL, 1);
		else
			if(Sys_info.defaultExporter == mod_index && !IsSelected(exp_form[EXPORT_STANDARD]))
			   change_object(&wind_s[WIND_EXPORT], EXPORT_STANDARD, SELECTED, 1);

		/*
		 * Options-Button enablen bzw. disablen
		 */
		mod_num = give_free_module();
		mod_num |= 0x100;						/* als Exporter kennzeichnen */
		module.smStruct[mod_num&0xFF] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
		memset(module.smStruct[mod_num&0xFF], 0x0, sizeof(GARGAMEL));
		export_mabs = (MOD_ABILITY *)module.comm.startExport(export_modules[mod_index], MQUERY, NULL, module.bp[mod_num&0xFF], module.smStruct[mod_num&0xFF], mod_num);
	
		if((export_mabs->ext_flag)&0x02)
			change_object(&wind_s[WIND_EXPORT], EXPORT_OPTIONS, ENABLED, 1);
		else
			change_object(&wind_s[WIND_EXPORT], EXPORT_OPTIONS, DISABLED, 1);

		module.smStruct[mod_num&0xFF]->module_mode = M_EXIT;		/* Modulende "simulieren" */
		check_and_terminate(MTERM, mod_num&0xFF);
	}

	/*
	 * Infodialog ein- oder ausschalten
	 */
	if(button == EXMOD_INFO && mod_index != -1 && openmode && Dialog.expmodList.modNames[mod_index] != NULL && strlen(Dialog.expmodList.modNames[mod_index]) != 0)
	   Dialog.expmodList.infoOn(mod_index);
	else
	if(info && button == EXMOD_INFO_OK && openmode)
	{
		Dialog.expmodList.infoOff();
		Window.redraw(&wind_s[WIND_EXPORT], NULL, 0, 0);
	}
	/*
	 * Standard-Exporter festlegen
	 * Der Standard-Exporter wird bei "Sichern" aufgerufen.
	 */
	else
	if(button == EXPORT_STANDARD)
	{
		if(IsSelected(exp_form[EXPORT_STANDARD]))
		{
			Sys_info.defaultExporter = mod_index;
			menu_tree[FILE_SAVE].ob_state &= ~DISABLED;
		}
		else
		{
			Sys_info.defaultExporter = -1;
			menu_tree[FILE_SAVE].ob_state |= DISABLED;
		}
	}
	/*
	 * Optionsdialog îffnen
	 * Das Modul wird normal gestartet. Terminierung bei OK im Optionsfenster 
	 * oder dessen Schlieûen wird vorgenommen, wenn der Exporter laut 
	 * EXPORT_CONFIG exp_conf nicht lÑuft (also, wenn das Exportformular nicht mit
	 * diesem Exporter offen ist).
	 */
	else
	if(!info && button == EXPORT_OPTIONS)
	{
		mod_num = give_free_module();
		mod_num |= 0x100;					/* als Exporter kennzeichnen */
		module.smStruct[mod_num&0xFF] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
		memset(module.smStruct[mod_num&0xFF], 0x0, sizeof(GARGAMEL));

		module.comm.startExport(export_modules[mod_index], MQUERY, NULL, module.bp[mod_num&0xFF], module.smStruct[mod_num&0xFF], mod_num);

		txtbeg = module.bp[exp_conf.export_mod_num&0xFF]->p_tbase;
		modinfo	= (MOD_INFO *)*((MOD_INFO **)(txtbeg + MOD_INFO_OFFSET));		/* Zeiger auf Modulinfostruktur */
		
		if(export_cnfblock[mod_index] == NULL)
		{
			export_cnflen[mod_index] = 0;
			export_cnfblock[mod_index] = load_from_modconf(modinfo, "", &export_cnflen[mod_index], 'SXMD');
		}

		if(export_cnfblock[mod_index] != NULL)
		{
			*((long *)module.smStruct[mod_num&0xFF]->event_par) = (long)export_cnfblock[mod_index];
			module.smStruct[mod_num&0xFF]->event_par[2] = export_cnflen[mod_index];
		}
			

		export_path = export_modules[mod_index];
		module.bp[mod_num&0xFF] = (BASPAG *)module.comm.startExport(export_path, MSTART, smurf_picture[active_pic], module.bp[mod_num&0xFF], module.smStruct[mod_num&0xFF], mod_num);
		module.comm.startExport(export_path, MMORE, smurf_picture[active_pic], module.bp[mod_num&0xFF], module.smStruct[mod_num&0xFF], mod_num);

		change_object(&wind_s[WIND_EXPORT], EXPORT_OPTIONS, UNSEL, 1);
	}
	/*
	 * Exporter starten
	 */
	else
	if(!info && button == START_EMOD || (klicks == 2 && button >= EMODULE1 && button <= EMODULE9) || my_scancode == SCAN_RETURN || my_scancode == SCAN_ENTER)
	{
		change_object(&wind_s[WIND_EXPORT], START_EMOD, UNSEL, 1);

		if(!picthere)
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NOPIC_TO_SAVE].TextCast, NULL, NULL, NULL, 1);
		else
		{
			/*
			 * freie Modulstruktur ermitteln
			 */
			mod_num = give_free_module();
			if(mod_num == -1)
			{
				Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);
				return;
			}
			
			mod_num |= 0x100;				/* als Exporter kennzeichnen */
			exp_conf.export_mod_num = mod_num;

			module.smStruct[exp_conf.export_mod_num&0xFF] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
			memset(module.smStruct[exp_conf.export_mod_num&0xFF], 0x0, sizeof(GARGAMEL));

			/*
			 * Modul analysieren
			 */
			export_path = export_modules[mod_index];
			export_mabs = (MOD_ABILITY *)module.comm.startExport(export_path, MQUERY, NULL, module.bp[mod_num&0xFF], module.smStruct[mod_num&0xFF], mod_num);
			memcpy(&expmabs, export_mabs, sizeof(MOD_ABILITY));

			txtbeg = module.bp[exp_conf.export_mod_num&0xFF]->p_tbase;
			modinfo	= (MOD_INFO *)*((MOD_INFO **)(txtbeg + MOD_INFO_OFFSET));		/* Zeiger auf Modulinfostruktur */

			export_depth[0] = expmabs.depth1;
			export_depth[1] = expmabs.depth2;
			export_depth[2] = expmabs.depth3;
			export_depth[3] = expmabs.depth4;
			export_depth[4] = expmabs.depth5;
			export_depth[5] = expmabs.depth6;
			export_depth[6] = expmabs.depth7;
			export_depth[7] = expmabs.depth8;
			
			export_format[0] = expmabs.form1;
			export_format[1] = expmabs.form2;
			export_format[2] = expmabs.form3;
			export_format[3] = expmabs.form4;
			export_format[4] = expmabs.form5;
			export_format[5] = expmabs.form6;
			export_format[6] = expmabs.form7;
			export_format[7] = expmabs.form8;

			prepare_depthpopup();

			check_and_terminate(MTERM, exp_conf.export_mod_num&0xFF);	/* Modul wieder terminieren */

			/* 
			 * und einen MSTART schicken (nur beim ersten Aufruf des Exporters)
			 * Dann kann sich das Modul initialisieren, z.B. Rsrc-fix u.Ñ.
			 */
			module.smStruct[exp_conf.export_mod_num&0xFF] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
			memset(module.smStruct[exp_conf.export_mod_num&0xFF], 0x0, sizeof(GARGAMEL));
			
			if(export_cnfblock[mod_index] == NULL)
			{
				export_cnflen[mod_index] = 0;
				export_cnfblock[mod_index] = load_from_modconf(modinfo, "", &export_cnflen[mod_index], 'SXMD');
			}

			if(export_cnfblock[mod_index] != NULL)
			{
				*((long *)module.smStruct[exp_conf.export_mod_num&0xFF]->event_par) = (long)export_cnfblock[mod_index];
				module.smStruct[exp_conf.export_mod_num&0xFF]->event_par[2] = export_cnflen[mod_index];
			}

			module.bp[exp_conf.export_mod_num&0xFF] = (BASPAG *)start_exp_module(export_path, MSTART,
													  smurf_picture[active_pic], module.bp[exp_conf.export_mod_num&0xFF],
													  module.smStruct[exp_conf.export_mod_num&0xFF], exp_conf.export_mod_num);


			openmode = 0;
			obj = -1;
			if(key_at_event == KEY_ALT)
			{
				openmode = 0;
				f_export_formular();
			}
			else 
			{
				exp_conf.exp_dither = -1;       /* signalisiert, daû die Autokonversion gÅltig ist */
				f_save_pic(&expmabs);
			}
		}
	}

	return;
} /* f_export_pic */


/* save_file -----------------------------------------------------
	Lîst das Speichern des aktiven Bildes mit dem Defaultexporter
	mittels f_save_pic() aus. Ein ôffnen des Exportdialoges steht
	hier (noch) nicht zur VerfÅgung.
	Der Modulkonfigurationsblock wird vor dem Aufruf von f_save_pic()
	an das Modul geschickt.
	---------------------------------------------------------------*/
void save_file(void)
{
	int mod_num, t;
	MOD_INFO *modinfo;
	MOD_ABILITY *export_mabs, expmabs;
	char *txtbeg, module_name[30];

	if(Sys_info.defaultExporter==-1)
		return;

	if(!picthere)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NOPIC_TO_SAVE].TextCast, NULL, NULL, NULL, 1);
	else
	{
		/*
		 * freie Modulstruktur ermitteln
		 */
		mod_num = give_free_module();
		if(mod_num==-1)
		{
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);
			return;
		}
		
		mod_num |= 0x100;							/* als Exporter kennzeichnen */
		exp_conf.export_mod_num = mod_num;

		module.smStruct[exp_conf.export_mod_num&0xFF] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
		memset(module.smStruct[exp_conf.export_mod_num&0xFF], 0x0, sizeof(GARGAMEL));
		export_path = export_modules[Sys_info.defaultExporter];

		/*
		 * Modul analysieren
		 */
		export_mabs = (MOD_ABILITY *)module.comm.startExport(export_path, MQUERY, NULL, module.bp[mod_num&0xFF], module.smStruct[mod_num&0xFF], mod_num);
		memcpy(&expmabs, export_mabs, sizeof(MOD_ABILITY));

		txtbeg = module.bp[exp_conf.export_mod_num&0xFF]->p_tbase;
		modinfo	= (MOD_INFO *)*((MOD_INFO **)(txtbeg + MOD_INFO_OFFSET));		/* Zeiger auf Modulinfostruktur */
		strncpy(module_name, modinfo->mod_name, 30);

		if(key_at_event&KEY_ALT)
		{
			export_depth[0]=expmabs.depth1;
			export_depth[1]=expmabs.depth2;
			export_depth[2]=expmabs.depth3;
			export_depth[3]=expmabs.depth4;
			export_depth[4]=expmabs.depth5;
			export_depth[5]=expmabs.depth6;
			export_depth[6]=expmabs.depth7;
			export_depth[7]=expmabs.depth8;
			
			export_format[0]=expmabs.form1;
			export_format[1]=expmabs.form2;
			export_format[2]=expmabs.form3;
			export_format[3]=expmabs.form4;
			export_format[4]=expmabs.form5;
			export_format[5]=expmabs.form6;
			export_format[6]=expmabs.form7;
			export_format[7]=expmabs.form8;

			prepare_depthpopup();
		}

		check_and_terminate(MTERM, exp_conf.export_mod_num&0xFF);	/* Modul wieder terminieren */
		module.smStruct[exp_conf.export_mod_num&0xFF] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
		memset(module.smStruct[exp_conf.export_mod_num&0xFF], 0x0, sizeof(GARGAMEL));
		
		/*
		 * Exporternummer suchen, um den Konfigurationsblock zu ermitteln
		 * eigentlich mÅûte der Index des Blocks gleich mod_index, also
		 * Sys_info.defaultExporter sein, das muû noch geprÅft werden.
		 */
		for(t = 0; t < Dialog.expmodList.anzahl; t++)
		{
			if(strncmp(module_name, Dialog.expmodList.modNames[t], strlen(module_name)) == 0)
				break;
		} 

		if(export_cnfblock[t] == NULL)
		{
			export_cnflen[t] = 0;
			export_cnfblock[t] = load_from_modconf(modinfo, "", &export_cnflen[t], 'SXMD');
		}

		if(export_cnfblock[t] != NULL)
		{
			*((long *)module.smStruct[exp_conf.export_mod_num&0xFF]->event_par) = (long)export_cnfblock[t];
			module.smStruct[exp_conf.export_mod_num&0xFF]->event_par[2] = export_cnflen[t];
		}

		module.bp[exp_conf.export_mod_num&0xFF] = (BASPAG *)start_exp_module(export_path, 
												  MSTART, smurf_picture[active_pic], 
												  module.bp[exp_conf.export_mod_num&0xFF], 
												  module.smStruct[exp_conf.export_mod_num&0xFF], 
												  exp_conf.export_mod_num);

		obj = -1;
		if(key_at_event&KEY_ALT)
		{
			openmode = 0;
			f_export_formular();
		}
		else
		{
			exp_conf.exp_dither = -1;       /* signalisiert, daû die Autokonversion gÅltig ist */
			f_save_pic(&expmabs);
		}
	}

	return;
} /* save_file */


/*-----------------------------------------------------------------	*/
/*							f_save_pic								*/
/*	Handled das Aufrufen der Dither-, Convert- und Codierungsfunk-	*/
/*	tionen fÅrs Exportieren mit dem Modul export_mabs.				*/
/*	Zu exportierendes Bild ist immer smurf_picture[active_pic]		*/
/*-----------------------------------------------------------------	*/
int f_save_pic(MOD_ABILITY *export_mabs)
{
	char savepathback[257], *name, *save_ext, *expext;
	char module_name[30], *txtbeg;
	char titleString[64];
	char *picture;

	int max_expdepth, dest_format;
	int ext_number;
	int dest_colsys;
	int old_picdepth, t;
	char str[10];

	long len,whlen;

	EXPORT_PIC *pic_to_save;
	SMURF_PIC *pic_to_export;
	SMURF_PIC *converted_pic;
	MOD_INFO *modinfo;
	GARGAMEL	*exp_gstruct;
	BASPAG		*exp_bp;
	WINDOW		*saved_window;

	extern	char *picnames[100];			/* BILDMANAGER: Namen fÅr bis zu 100 Bilder */


	exp_gstruct = module.smStruct[exp_conf.export_mod_num&0xFF];
	exp_bp = module.bp[exp_conf.export_mod_num&0xFF];

	/*
	 * Maximale Farbtiefe des Exporters und zugehîriges Datenformat ermitteln
	 */
	max_expdepth = export_mabs->depth1;
	dest_format = export_mabs->form1;

	if(export_mabs->depth2 > max_expdepth) 
	{
		max_expdepth=export_mabs->depth2;
		dest_format=export_mabs->form2;
	}
	if(export_mabs->depth3 > max_expdepth) 	
	{
		max_expdepth=export_mabs->depth3;
		dest_format=export_mabs->form3;
	}
	if(export_mabs->depth4 > max_expdepth) 	
	{
		max_expdepth=export_mabs->depth4;
		dest_format=export_mabs->form4;
	}
	if(export_mabs->depth5 > max_expdepth) 	
	{
		max_expdepth=export_mabs->depth5;
		dest_format=export_mabs->form5;
	}
	if(export_mabs->depth6 > max_expdepth) 	
	{
		max_expdepth=export_mabs->depth6;
		dest_format=export_mabs->form6;
	}
	if(export_mabs->depth7 > max_expdepth) 	
	{
		max_expdepth=export_mabs->depth7;
		dest_format=export_mabs->form7;
	}
	if(export_mabs->depth8 > max_expdepth) 	
	{
		max_expdepth=export_mabs->depth8;
		dest_format=export_mabs->form8;
	}
	
	exp_conf.exp_depth = max_expdepth;
	exp_conf.exp_form = dest_format;
	
	pic_to_export = smurf_picture[active_pic];

	/*
	 * Bild konvertieren / dithern
	 */		
	if(export_mabs != NULL)
	{
		/*----------- neue SMURF_PIC generieren -------------*/
		converted_pic = SMalloc(sizeof(SMURF_PIC));
		memcpy(converted_pic, pic_to_export, sizeof(SMURF_PIC));
		converted_pic->palette = malloc(1025);
		memcpy(converted_pic->palette, pic_to_export->palette, 1025);
		converted_pic->screen_pic = NULL;

		/* --- Maximale Farbtiefe des Xporters ausreichend? */
		/* sollte eigentlich mit kleinster Exporterfarbtiefe verglichen werden */
		if(pic_to_export->depth > max_expdepth)
		{
			if(pic_to_export->depth == 24 && max_expdepth == 16)
			{
				if(Dialog.winAlert.openAlert("Beim Export in 16 Bit wird das Bild an FarbqualitÑt verlieren! Trotzdem in 16 Bit exportieren?", "Nein", " Ja ", NULL, 1) == 1)
					return(-2);
				else
					tfm_24_to_16(converted_pic, NEW);
			}
			else
			{
				/*
				 * Farbsystem holen
				 */
				old_picdepth = pic_to_export->depth;
				pic_to_export->depth = max_expdepth;

				start_exp_module(export_path, MCOLSYS, pic_to_export, exp_bp, exp_gstruct, exp_conf.export_mod_num);
				if(exp_gstruct->module_mode == M_COLSYS)
					dest_colsys = exp_gstruct->event_par[0];

				pic_to_export->depth = old_picdepth;

				if(max_expdepth == 8 && dest_colsys == GREY)
				{
					if(Dialog.winAlert.openAlert("Das Bild muû fÅr den Export in 8 Bit nach Graustufen gewandelt werden! Sind Sie damit einverstanden?", "Nein", " Ja ", NULL, 1) == 1)
						return(-2);

					if(f_convert(converted_pic, export_mabs, GREY, NEW, 0) != 0)
						return(-2);
				}
				else
					if(max_expdepth < 16 && max_expdepth < pic_to_export->depth)
					{
						if(exp_conf.exp_dither == -1)
						{
							exp_conf.exp_dither = Sys_info.AutoconvDither;

							if(max_expdepth == 1)
								exp_conf.exp_colred = CR_SYSPAL;
							else
								exp_conf.exp_colred = Sys_info.AutoconvPalmode;
						}

						if(dither_for_export(export_mabs, max_expdepth, dest_format, converted_pic) != 0)
							return(-2);
					}
			}
		}
		else
			if(f_convert(converted_pic, export_mabs, 255, NEW, 0) != 0)
				return(-2);
	}

	/*	
	 * Extension ermitteln, Exporter aufrufen 
	 */
	module.comm.startExport(export_path, MEXTEND, converted_pic, exp_bp, exp_gstruct, exp_conf.export_mod_num);
	txtbeg = exp_bp->p_tbase;
	modinfo	= (MOD_INFO *)*((MOD_INFO **)(txtbeg + MOD_INFO_OFFSET));		/* Zeiger auf Modulinfostruktur */

	if(exp_gstruct->module_mode == M_EXTEND) 
		ext_number = exp_gstruct->event_par[0] - 1;						/* Da 0 der erste ist */
	else
		ext_number = 0;

	expext = modinfo->ext[ext_number];

	/*
	 * Speichername zusammenbasteln 
	 */
	strncpy(module_name, modinfo->mod_name, 28);
	strcpy(savepathback, savepath);					/* Backup vom Savepath */
	strcpy(strrchr(savepath, '\\') + 1, strrchr(smurf_picture[active_pic]->filename, '\\') + 1);
	strlwr(expext);

	if((save_ext = strrchr(savepath, '.')) != NULL && save_ext > strrchr(savepath, '\\'))
		strcpy(save_ext + 1, expext);		/* Extender gegen neuen ersetzen */
	else
	{
		strcat(savepath, ".");				/* Punkt und */
		strncat(savepath, expext, 4);		/* neuen Extender an Filenamen ohne Extender hÑngen */
	}

	Dialog.busy.reset(0, "codiere Bild...");

	pic_to_save = module.comm.startExport(export_path, MEXEC, converted_pic, exp_bp, exp_gstruct, exp_conf.export_mod_num);

	saved_window = &picture_windows[active_pic];

	if(exp_gstruct->module_mode == M_MEMORY)
	{
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NOMEM_EXPORT].TextCast, NULL, NULL, NULL, 1);
		strcpy(savepath, savepathback);
	}
	else
	if(exp_gstruct->module_mode == M_PICERR)
	{
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NOMEM_PICERR].TextCast, NULL, NULL, NULL, 1);
		strcpy(savepath, savepathback);
	}
	else
	if(exp_gstruct->module_mode == M_DONEEXIT)
	{
		picture = (char *)pic_to_save->pic_data;
		len = pic_to_save->f_len;
		f_set_syspal();
		Dialog.busy.reset(0, "speichere Bild...");

		strcpy(titleString, expext);
		strcat(titleString, " speichern");
		if(file_save(titleString, picture, len) == TRUE)
		{
			if(strcmp(smurf_picture[active_pic]->filename, savepath) != 0)
				if(Dialog.winAlert.openAlert("Neuen Namen Åbernehmen?", "Nein", " Ja ", NULL, 1) == 2)
				{
					Comm.renameOLGA(smurf_picture[active_pic]->filename, savepath);
					strncpy(smurf_picture[active_pic]->filename, savepath, 257);

					memset(saved_window->wtitle, 0x0, 41);
					itoa(smurf_picture[active_pic]->pic_width, str, 10);
					strncpy(saved_window->wtitle, str, 5);
					strncat(saved_window->wtitle, "*", 1);
					itoa(smurf_picture[active_pic]->pic_height, str, 10);
					strncat(saved_window->wtitle, str, 5);

					whlen = (int)strlen(saved_window->wtitle);
					strncat(saved_window->wtitle, "            ", 12 - whlen);

/*					strncat(saved_window->wtitle, strrchr(savepath, '\\') + 1, 40); */
					if((name = strrchr(savepath, '\\')) != NULL)
						name++;
					else
						name = savepath;
					strcat(saved_window->wtitle, shorten_name(name, 41 - (char)strlen(saved_window->wtitle)));
					Window.windSet(saved_window->whandlem, WF_NAME, LONG2_2INT((long)saved_window->wtitle), 0, 0);

					t = 0;
					while(Dialog.picMan.picmanList[t] != active_pic)
						t++;
					picnames[t] = saved_window->wtitle + 12;
					f_listfield((long *)Dialog.picMan.window, REDRAW, 0, &Dialog.picMan.pictureList);
				}

			if(saved_window->wtitle[11] == '*')
				Comm.updateOLGA(savepath);

			imageWindow.toggleAsterisk(saved_window, 0);
			Comm.updateDhst(savepath);
		}
		else
			strcpy(savepath, savepathback);

		/* if, damit bei raw-Export nicht das Original flîten geht ... */
		if(pic_to_save->pic_data != pic_to_export->pic_data)	
		{
			if(SMfree(pic_to_save->pic_data) != 0)
				Dialog.winAlert.openAlert(Dialog.winAlert.alerts[EXP_MFREE_ERR].TextCast, NULL, NULL, NULL, 1);
		}

		if(SMfree(pic_to_save) != 0)
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[EXP_MFREE_ERR].TextCast, NULL, NULL, NULL, 1);
	}

	/* Muû sein, da bei Export ohne Konvertierung converted_pic noch aufs Original zeigt! */
	if(converted_pic->pic_data != pic_to_export->pic_data)
		SMfree(converted_pic->pic_data);

	free(converted_pic->palette);
	SMfree(converted_pic);

	/*
	 * Exportformular zu, Modul terminieren und tschÅû
	 */
	Dialog.close(FORM_EXPORT);
	check_and_terminate(MTERM, exp_conf.export_mod_num&0xFF);
	Dialog.busy.ok();

	return(0);
} /* f_save_pic */


/********************************************************************/
/********************************************************************/
/*						Bild fÅr Export dithern						*/
/*	Es wird eine SYSTEM_INFO-Struktur gefÑlscht und die zum Export	*/
/*	nîtigen Daten eingehÑngt, gleiches gilt fÅr DISPLAY_MODES.		*/
/*	Dithermodule / Konverter werden aufgerufen, Palette Åbertragen, */
/*	etc.															*/
/********************************************************************/
/********************************************************************/
int dither_for_export(MOD_ABILITY *mod_abs, int max_expdepth, int dest_format, SMURF_PIC *converted_pic)
{
	extern int export_dither_dispatcher(SMURF_PIC *dest, SYSTEM_INFO *sys_info, DISPLAY_MODES *display, 
								int *fixpal_red, int *fixpal_green, int *fixpal_blue);
	char *dest_pal, dest_depth, dest_form;
	int t;
	SYSTEM_INFO	Cheat_sysinfo;
	DISPLAY_MODES new_display;


	dest_depth = max_expdepth;
	dest_form = dest_format;


	/*----------- Sysinfo zum Dithern generieren -----------*/
	memcpy(&Cheat_sysinfo, &Sys_info, sizeof(SYSTEM_INFO));

	/*------------------------ Sind wir in einer anderen Farbtiefe (muû eine andere NCT geladen werden)? */
	if(exp_conf.exp_colred == CR_SYSPAL && dest_depth != Sys_info.bitplanes)
	{
		if(loadNCT(dest_depth, &Cheat_sysinfo) != 0)
		{
			Dialog.winAlert.openAlert("Kein NCT-File fÅr die gewÑhlte Farbtiefe gefunden. Dithering unmîglich.", NULL, NULL, NULL, 1);
			return(-1);
		}
	}

	Cheat_sysinfo.bitplanes = dest_depth;
	Cheat_sysinfo.Max_col = (1 << dest_depth) - 1;

	/*---------------- Neue Display_modes zurechtbescheiûen */
	memcpy(&new_display, &Display_Opt, sizeof(DISPLAY_MODES));

	new_display.syspal_24 = exp_conf.exp_colred;
	new_display.syspal_8 = exp_conf.exp_colred;
	new_display.syspal_4 = exp_conf.exp_colred;
	new_display.dither_24 = exp_conf.exp_dither;
	new_display.dither_8 = exp_conf.exp_dither;
	new_display.dither_4 = exp_conf.exp_dither;

	export_dither_dispatcher(converted_pic, &Cheat_sysinfo, &new_display, 
		exp_conf.exp_fix_red, exp_conf.exp_fix_green, exp_conf.exp_fix_blue);

	/*---------------- Bilddaten aus dem MFDB Åbertragen */
	converted_pic->pic_data = converted_pic->screen_pic->fd_addr;
	converted_pic->depth = Cheat_sysinfo.bitplanes;
	converted_pic->format_type = dest_form;

	/* und gleich den nun unnîtigen MFDB freigeben */
	free(converted_pic->screen_pic);

	/*---------------- Palette Åbertragen */
	dest_pal = (converted_pic->palette);
	for(t = 0; t < 256; t++)
	{
		*dest_pal++ = (char)converted_pic->red[t];
		*dest_pal++ = (char)converted_pic->grn[t];
		*dest_pal++ = (char)converted_pic->blu[t];
	}

	/*---------------- Und jetzt noch konvertieren, dann sind wir fertig. */
	f_convert(converted_pic, mod_abs, 255, SAME, 0);

	/*---------------- alles aus der Cheat_sysinfo wegwerfen, -----*/
	/*----------	was in der load_nct angefordert wurde ----*/
	if(exp_conf.exp_colred == CR_SYSPAL && dest_depth != Sys_info.bitplanes)
	{
		free(Cheat_sysinfo.red);
		free(Cheat_sysinfo.grn);
		free(Cheat_sysinfo.blu);
		free(Cheat_sysinfo.plane_table);
		SMfree(Cheat_sysinfo.nc_table);
	}

	return(0);
} /* dither_for_export */



/*-----------------------------------------------------------------	*/
/*						init_exmod_info								*/
/*	Baut den Dialog fÅr die Exportmodulinfo des Modules mod_index	*/
/*	aus dem Exporter-Listfeld auf									*/
/*-----------------------------------------------------------------	*/
void init_exmod_info(int mod_index)
{
	char *textseg_begin;
	char str[10], *filename;

	int mod_num;

	MOD_INFO *info_mi;
	MOD_ABILITY *info_mabs;


	mod_num = give_free_module();
	mod_num |= 0x100;						/* als Exporter kennzeichnen */
	module.smStruct[mod_num&0xFF] = (GARGAMEL *)malloc(sizeof(GARGAMEL));
	memset(module.smStruct[mod_num&0xFF], 0x0, sizeof(GARGAMEL));
	if((info_mabs = (MOD_ABILITY *)module.comm.startExport(export_modules[mod_index], MQUERY, NULL, module.bp[mod_num&0xFF], module.smStruct[mod_num&0xFF], mod_num)) == NULL)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[EMOD_START_ERR].TextCast, NULL, NULL, NULL, 1);

	textseg_begin = (char *)module.bp[mod_num&0xFF]->p_tbase;
	info_mi = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));

	strncpy(Dialog.expmodList.infoTree[EI_MODNAME].TextCast, Dialog.expmodList.modNames[mod_index], 27);

	BCD2string(Dialog.expmodList.infoTree[EI_MODVER].TextCast, info_mi->version);

	strcpy(Dialog.expmodList.infoTree[EI_MODAUTOR].TextCast, "");
	strncat(Dialog.expmodList.infoTree[EI_MODAUTOR].TextCast, info_mi->autor, 27);

	filename=strrchr(export_modules[mod_index], '\\');
	strncpy(Dialog.expmodList.infoTree[EI_MODPATH].TextCast, filename+1, 12);
		
	strcpy(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, "");

	if(info_mabs->depth1 > 0)
	{
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, itoa(info_mabs->depth1, str, 10), 2);
	}
	if(info_mabs->depth2 > 0)
	{
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, itoa(info_mabs->depth2, str, 10), 2);
	}
	if(info_mabs->depth3 > 0)
	{
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, itoa(info_mabs->depth3, str, 10), 2);
	}
	if(info_mabs->depth4 > 0)
	{
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, itoa(info_mabs->depth4, str, 10), 2);
	}
	if(info_mabs->depth5 > 0)
	{
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, itoa(info_mabs->depth5, str, 10), 2);
	}
	if(info_mabs->depth6 > 0)
	{
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, itoa(info_mabs->depth6, str, 10), 2);
	}
	if(info_mabs->depth7 > 0)
	{
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, itoa(info_mabs->depth7, str, 10), 2);
	}
	if(info_mabs->depth8 > 0)
	{
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, ", ", 2);
		strncat(Dialog.expmodList.infoTree[EI_MODDEPTH].TextCast, itoa(info_mabs->depth8, str, 10), 2);
	}

/*
	/*---- Schlumpfine - Versionsnummer auslesen ----*/
	v_byte=*(textseg_begin+MOD_ABS_OFFSET+6);				/* Versionsnummer */
	itoa((int)v_byte, str, 16);								/* Hex-Basis (0x0100 = Version 01.00) */
	strncpy(Dialog.expmodList.infoTree[EI_SMURFVER].TextCast, str, 2);
	strncat(Dialog.expmodList.infoTree[EI_SMURFVER].TextCast, ".", 1);
	v_byte=*(textseg_begin+MOD_ABS_OFFSET+7);				/* Unterversionsnummer */
	itoa((int)v_byte, str, 16);
	strncat(Dialog.expmodList.infoTree[EI_SMURFVER].TextCast, str, 2);
*/

	module.smStruct[mod_num&0xFF]->module_mode = M_EXIT;			/* Modulende "simulieren" */
	check_and_terminate(MTERM, mod_num&0xFF);
	wind_s[WIND_EXPORT].resource_form = Dialog.expmodList.infoTree;

	Dialog.expmodList.infoTree[0].ob_x = wind_s[WIND_EXPORT].wx;
	Dialog.expmodList.infoTree[0].ob_y = wind_s[WIND_EXPORT].wy;

	Window.redraw(&wind_s[WIND_EXPORT], NULL, 0, 0);

	return;
}


/*------------------ Ausschalten der Exportmodul-Info -------------*/
void exmod_info_off(void)
{
	/*
	 * ist die Info Åberhaupt eingeschaltet?
	 */
	if(wind_s[WIND_EXPORT].resource_form == Dialog.expmodList.infoTree)
	{
		change_object(&wind_s[WIND_EXPORT], EXMOD_INFO_OK, UNSEL, 0);
		wind_s[WIND_EXPORT].resource_form = Dialog.expmodList.tree;

		Dialog.expmodList.tree[0].ob_x = wind_s[WIND_EXPORT].wx;
		Dialog.expmodList.tree[0].ob_y = wind_s[WIND_EXPORT].wy;

		change_object(&wind_s[WIND_EXPORT], EXMOD_INFO, UNSEL, 0);
	}

	return;
}


/*-----------------------------------------------------------------	*/
/*	NCT-File, Bintable und Palette fÅr Export in Bildschirmfremden	*/
/*	Farbtiefen temporÑr einladen.									*/
/*	Da z.B. im 4Bit-Screenmodus keine Palette und Umsetzungs-		*/
/*	tabellen vorhanden sind, werden diese sofern vorhanden eingela-	*/
/*	den. D.h., sie mÅssen, damit Systempaletten-Export in anderen	*/
/*	Farbtiefen mîglich ist, erstmal erzeugt werden. 				*/
/*	Unschîn, geht aber nicht anders.								*/
/*	nc_table, plane_table, red, grn und blu werden neu angefordert!	*/
/*-----------------------------------------------------------------	*/
int loadNCT(int loadplanes, SYSTEM_INFO *sysinfo)
{
	char tablename[256], bpstring[4];
	char *palbuf;

	int attrib = 0, maxc, t;
	int *palette;


	Dialog.busy.reset(0, "lade NCT");

	if(loadplanes > 8)
		return(0);

	/* Tabellen-Filename zusammenbasteln */
	strcpy(tablename, Sys_info.home_path);
	strcat(tablename, "\\smp.");
	itoa(loadplanes, bpstring, 10);
	strcat(tablename, bpstring);

	maxc = 1 << loadplanes;
	
	/* Mit einem Trick testen, ob die Datei vorhanden ist */
	if(Fattrib(tablename, 0, attrib) >= 0)
	{
		if((palbuf = fload(tablename, 0)) == NULL)
			return(-1);

		sysinfo->nc_table = SMalloc(32800L);
		memcpy(sysinfo->nc_table, palbuf + maxc * 6 + 256, 32768L);

		sysinfo->plane_table = malloc(256);
		memcpy(sysinfo->plane_table, palbuf + maxc * 6, 256);
		palette = (int *)palbuf;
		
		sysinfo->red = malloc(256 * 2);
		sysinfo->grn = malloc(256 * 2);
		sysinfo->blu = malloc(256 * 2);

		for(t = 0; t < 256; t++)
		{
			sysinfo->pal_red[t] = palette[t * 3];
			sysinfo->pal_green[t] = palette[t * 3 + 1];
			sysinfo->pal_blue[t] = palette[t * 3 + 2];
			sysinfo->red[t] = (int)(palette[t * 3] * 31L / 1000L);
			sysinfo->grn[t] = (int)(palette[t * 3 + 1] * 31L / 1000L);
			sysinfo->blu[t] = (int)(palette[t * 3 + 2] * 31L / 1000L);
		}

		return(0);
	}
	else
		return(-1);
}