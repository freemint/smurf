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

/*  ------------------------------------------------------------------- */
/*							PLUGIN.C									*/
/*	EnthÑlt die Routinen zum Initialisieren der Plugin-Schnittstelle,	*/
/*	zum Hochfahren der Plugins und zur Kommunikation mit diesen.		*/
/*																		*/
/*	Beim Initialisieren des Plugins mit MSTART wird diesem in seiner
	PLG_DATA - Struktur eine ID zugewiesen. Die niedrigste Plugin-ID
	ist 0x200, um aus der ID einen Index auf Arrays zu machen, muû
	diese also mit 0xff undiert werden. Dies ist nîtig, um bei einem
	Buttonevent an der ID in der Fensterstruktur ein Plugin erkennen
	zu kînnen.
	Die IDs fÅr Exporter beginnen mit 0x100, demnach stehen bis zu 255
	IDs fÅr Exporter zur VerfÅgung, was auch im spÑteren Multithreading-
	Betrieb ausreichen sollte.	
	Beim Initialisieren wird auûerdem anhand der PLUGIN_INFO-Strukturen
	festgestellt, ob dasselbe Plugin schon in Smurf angemeldet ist, die
	Versionsnummern verglichen und nur das neuere Plugin Hochgefahren,
	das alte aber entfernt.                                             */
/* -------------------------------------------------------------------- */

#include <tos.h>
#include <ext.h>
#include <multiaes.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\..\sym_gem.h"
#include "..\mod_devl\import.h"
#include "..\smurfine.h"
#include "..\globdefs.h"
#include "..\smurf_st.h"
#include "..\smurf.h"
#include "..\smurf_f.h"
#include "plugin.h"
#include "..\ext_rsc.h"
#include "..\destruct.h"

#include "..\smurfobs.h"
#include "..\ext_obs.h"

/*
 * lokale Funktionen
 */
void insert_to_menu(OBJECT *menu, int entry, char *string);
int start_plugin(BASPAG *bp, int message, int plg_id, PLUGIN_DATA *data);
void init_structs(void);
int load_plugin(int plugin_number);
void terminate_plugin(int index);
void plugin_startup(int index, int *curr_plugin_entry, char *plg_filename);

extern	struct DIRENTRY *build_up_filelist(char *path, char *ext, int pathlen);
extern	void destroy_filelist(struct DIRENTRY *begin);
extern	SYSTEM_INFO Sys_info;			
extern	OBJECT	*menu_tree;
extern	SERVICE_FUNCTIONS global_services;
extern	long Name_Max;

extern	WINDOW wind_s[25];
extern	WINDOW picture_windows[MAX_PIC];
extern	SMURF_PIC *smurf_picture[MAX_PIC];

extern	char loadpath[257];				/* voller Pfad der zuletzt geladenen Datei */
extern	char savepath[257];				/* voller Pfad der zuletzt gespeicherten Datei */
extern	char commpath[257];			/* voller Pfad der zuletzt Åber ein Protokoll empfangenen Datei */
extern	char DraufschmeissBild;

extern	int picwindthere, dialwindthere, picthere;
extern	int active_pic;

PLUGIN_FUNCTIONS global_replace;
ADD_FUNCTIONS global_call;
PLUGIN_FUNCTIONS global_functions;
SMURF_VARIABLES global_vars;


BASPAG	*plugin_bp[11];
PLUGIN_DATA *plg_data[11];
PLUGIN_INFO *plg_info[11];
int anzahl_plugins;
char *plugin_paths[11];
signed char menu2plugin[128];		/* feste MenÅeintrÑge, in die Plugins eingehÑngt sind */

EXT_MODCONF	*modconfs[20];			/* Strukturen fÅr Modul-Notifying */

int printplug_found=0;

/*------------------ Plugins suchen und eintragen ------------------*/
/* Der Ordner MODULES\\PLUGIN wird nach Files mit der Extension	*/
/*	.PLG durchsucht.												*/
/* Eine verkettete Fileliste (DIRENTRY) wird aufgebaut				*/
/* ----------------------------------------------------------------	*/
void scan_plugins(void)
{
	char *dummy=NULL;
	char *editpath;										/* voller Modulpfad, Original */
	char *edit_path;									/* voller Modulpfad, editable */
	char *swapstr, alert[256];
	char newpath[256], *oldpath, *dot_pos;
	char *textseg_begin;
	int pathlen, t, install_flag, install_index, curr_plugin_entry;
	PLUGIN_INFO *curr_info, *info;
	PLUGIN_INFO installed_infos[20];
	long mod_magic;

	long ProcLen;
	long temp, lback;

	struct DIRENTRY *actual, *filelist;

	extern	long get_proclen(BASPAG *baspag);

	
	for(t=0; t<11; t++)	plg_data[t]=NULL;
	
	memset(menu2plugin, -1, 128);
	init_structs();			/* Plugin-Strukturen aufbauen */

	/*---- Pfade vorbereiten ----*/
	editpath = calloc(1, strlen(Sys_info.standard_path) + strlen("\\modules\\plugin\\") + 1);
	strcpy(editpath, Sys_info.standard_path);
	strcat(editpath, "\\modules\\plugin\\");

	Name_Max = get_maxnamelen(editpath);

	pathlen = (unsigned int)(strlen(editpath) + Name_Max);
	edit_path = calloc(1, pathlen + 1);
	swapstr = calloc(1, pathlen + 1);

	filelist = build_up_filelist(editpath, "plg", pathlen);

	anzahl_plugins = 0;
	curr_plugin_entry = 0;
	actual = filelist;

	while(actual != NULL)
	{
		/*
		 * Plugin laden und Basepage ermitteln 
		 */
		strcpy(edit_path, editpath);
		strcat(edit_path, actual->modname);

		temp = Pexec(3, edit_path, NULL, NULL);
		if(temp < 0)
		{
			strcpy(alert, "[1][Fehler in File|");
			strcat(alert, actual->modname);
			strcat(alert, "|im Ordner|");
			strcat(alert, "\\modules\\plugin\\!]");
			strcat(alert, "[ OK ]");
			form_alert(1, alert);
		}
		else
		{	
			plugin_bp[anzahl_plugins] = (BASPAG *)temp;

			mod_magic = get_modmagic(plugin_bp[anzahl_plugins]);		/* Zeiger auf Magic (muû 'PLGN' sein!) */
			if(mod_magic != 'SPLG')
			{
				strcpy(alert, "[1][ File ");
				strcat(alert, actual->modname);
				strcat(alert, " | im Ordner |");
				strcat(alert, " \\modules\\plugin\\ |");
				strcat(alert, " ist kein Smurf-Plugin! ][ OK ]");
				form_alert(1, alert);
			}
			
			/*
			 * Plugin zur Initialisierung aufrufen 
			 */
			else		
			{
				/*---- LÑnge des gesamten Tochterprozesses ermitteln */
				ProcLen = get_proclen(plugin_bp[anzahl_plugins]);
				Mshrink(0, plugin_bp[anzahl_plugins], ProcLen);		/* Speicherblock verkÅrzen */
				plugin_bp[anzahl_plugins]->p_hitpa = (void *)((long)plugin_bp[anzahl_plugins] + ProcLen);

				lback = Pexec(4, 0L, (char *)plugin_bp[anzahl_plugins], 0L);
				if(lback < 0L)
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);

				textseg_begin = plugin_bp[anzahl_plugins]->p_tbase;			/* Textsegment-Startadresse holen */

				install_index = anzahl_plugins;
				install_flag = 1;
				
				curr_info = (PLUGIN_INFO *)*((PLUGIN_INFO **)(textseg_begin + PLG_INFO_OFFSET));

				if(curr_info->for_smurf_version != SMURF_VERSION)
				{
					strcpy(alert, "[2][Das Plugin \"");
					strcat(alert, curr_info->name);
					strcat(alert, "\"|ist nicht fÅr diese Smurf-Version|gedacht! Da das zu ");
					strcat(alert, "Problemen|fÅhren kann, wird das Plugin terminiert.][ OK ]");
					form_alert(1, alert);
					install_flag = 0;
				}
				else
				{
					/*
					 * ist das gleiche Plugin (andere Version?) schon angemeldet?
					 */
					for(t = 0; t < anzahl_plugins; t++)
					{
						info = &installed_infos[t];
						if(strcmp(curr_info->name, info->name) == 0)
						{
							if(curr_info->plugin_version == info->plugin_version)		/* gleiche Version */
								install_flag = 0;
							else
							{
								strcpy(alert, "[2][Das Plugin \"");
								strcat(alert, info->name);
								strcat(alert, "\"|ist zweimal in unterschiedlichen|Versionen installiert! ");
								strcat(alert, "Nur das|neuere wird eingebunden. Soll das|Ñltere Plugin abgemeldet werden?][ Ja | Nein ]");
								if(form_alert(1, alert) == 1)
								{
									if(curr_info->plugin_version > info->plugin_version)	/* ist das jetzige neuer als das vorher angemeldete? */
										oldpath = plugin_paths[t];
									else
										oldpath = edit_path;

									strcpy(newpath, oldpath);
									dot_pos = strrchr(newpath, '.');
									strcpy(dot_pos + 1, "plx");
									Frename(0, oldpath, newpath);
								}
	
								if(curr_info->plugin_version > info->plugin_version)	/* ist das jetzige neuer als das vorher angemeldete? */
								{
									free(plg_info[t]->name);
									free(plg_info[t]);
									free(plugin_paths[t]);
									plugin_bp[t] = plugin_bp[install_index];
									plugin_bp[install_index] = NULL;
									install_index = t;
								}
								else
									install_flag = 0;
							}

							break;
						}
					}
				}
		
				/*
				 * ggfs. Plugin anmelden
				 */
				if(install_flag == 1)
				{
					plg_info[install_index] = malloc(sizeof(PLUGIN_INFO));
					memcpy(plg_info[install_index], curr_info, sizeof(PLUGIN_INFO));
					plg_info[install_index]->name = malloc(strlen(curr_info->name) + 1);
					strcpy(plg_info[install_index]->name, curr_info->name);
					memcpy(&installed_infos[install_index], plg_info[install_index], sizeof(PLUGIN_INFO));
					
					plugin_paths[install_index] = malloc(strlen(edit_path) + 1);
					memset(plugin_paths[install_index], 0x0, strlen(edit_path) + 1);
					strcpy(plugin_paths[install_index], edit_path);
				
					plg_data[install_index] = malloc(sizeof(PLUGIN_DATA));
					memset(plg_data[install_index], 0x0, sizeof(PLUGIN_DATA));

					/* Startup-Kommunikation */
					plugin_startup(install_index, &curr_plugin_entry, actual->modname);
					
					if(strcmp(curr_info->name, "GDOS Print-Plugin") == 0)
						printplug_found = 1;
					
					if(!curr_info->resident)
						terminate_plugin(install_index);

					/* nur hochzÑhlen, wenn wirklich ein neues eingetragen */
					/* und nicht nur ausgetauscht wurde */
					if(!(install_index == t && install_index != anzahl_plugins))
						anzahl_plugins++;
				}
				/*
				 *
				 */
				else
				{
/*					Pexec(102, dummy, plugin_bp[anzahl_plugins], ""); */
					SMfree(plugin_bp[anzahl_plugins]->p_env);
					SMfree(plugin_bp[anzahl_plugins]);
					plugin_bp[anzahl_plugins] = NULL;
				}
			}
		}

		actual = actual->next;
	}

	destroy_filelist(filelist);

	free(edit_path);
	free(editpath);
	free(swapstr);

	return;
} /* scan_plugins */



/* void start_plugin(...) -----------------------------------------------------
	Zum Aufrufen eines im Speicher befindlichen Plugins (BASBAG *bp).
	Die Message <message> wird Åbergeben. Handling ansonsten wie bei Editmodulen
	---------------------------------------------------------------------------*/
int start_plugin(BASPAG *bp, int message, int plg_id, PLUGIN_DATA *data)
{
	void (*plg_main)(PLUGIN_DATA *data);

	char *textseg_begin;

	int out_msg, index, curr_notify_index;

	long mod_magic;


	textseg_begin = bp->p_tbase;			/* Textsegment-Startadresse holen */
	mod_magic = get_modmagic(bp);			/* Zeiger auf Magic (muû 'PLGN' sein!) */
	index = plg_id&0xFF;

	if(mod_magic == 'SPLG')
	{
		/* PluginData fertigmachen */
		data->services = &global_services;
		data->replace = &global_replace;
		data->smurf_functions = &global_functions;
		data->smurf_vars = &global_vars;
		data->call = &global_call;
		data->message = message;
		data->module_object = &module;

		if(message == MSTART)
			data->id = plg_id;				/* ID eintragen (nur beim Hochfahren) */

		plg_main = (VOID_FUNCTION)((char *)textseg_begin + MAIN_FUNCTION_OFFSET);
		if(plg_main != NULL)
			plg_main(data);

		/*
		 * 	RÅckgabe-Messages
		 *	Jetzt werden Sachen wie z.B. WINDEVENTS gehandlet. Dazu wird nach einer
		 *	RÅckgabemessage != M_WAITING entsprechend reagiert und solange DONE geschickt,
		 *	bis ein M_WAITING vom Plugin zurÅckkommt, dieses also endlich zufrieden ist. ;-)
		 */
		out_msg = data->message;

		if(out_msg == M_MEMORY)
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_MEM].TextCast, NULL, NULL, NULL, 1);
		else
		if(out_msg == M_EXIT)
		{
			terminate_plugin(index);
			return(M_TERMINATED);
		}
		else
		if(message != MSTART && message != MTERM && out_msg != M_WAITING)
		{
			if(modconfs[index] == NULL)
			{
				modconfs[index] = malloc(sizeof(EXT_MODCONF));
				memset(modconfs[index], 0x0, sizeof(EXT_MODCONF));
			}
			
			curr_notify_index = 0;
			
			do
			{
				switch(out_msg)
				{
					case MBEVT:
					case MKEVT:
					case WINDEVENTS:
					case ALL_MSGS:	modconfs[index]->notify_types[curr_notify_index] = out_msg;
									modconfs[index]->id = data->id;
									break;
				}

				if(out_msg == MBEVT || out_msg == MKEVT)
					modconfs[index]->windhandle = data->event_par[0];

				curr_notify_index++;
				
				data->message = DONE;

				plg_main(data);
				out_msg = data->message;
			} while(out_msg != M_WAITING);
		}
	}
	else
	{
		Dialog.winAlert.openAlert("Fehler beim Aufrufen des Plugins!", NULL, NULL, NULL, 1);
		return(-1);
	}

	return(0);
} /* start_plugin */



/*--------------------------------------------------------------------
	Plugin-MenÅeintrag wurde gewÑhlt (wird von MENU.C aus aufgerufen) 
	-----------------------------------------------------------------*/
void call_plugin(int menuentry)
{
	int plugin_number, msgbuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/*
	if(menuentry >= PLUGIN1 && menuentry <= PLUGIN10)
		plugin_number = menuentry - PLUGIN1;
	else
		plugin_number = menu2plugin[menuentry];
*/
	plugin_number = menu2plugin[menuentry];

	if(plugin_number >= 0)
	{
		/*
		 * rausfinden, ob das Plugin schon lÑuft, und ggfs. nach vorne holen
		 */
		if(plg_data[plugin_number] != NULL)
		{
			if(plg_data[plugin_number]->wind_struct != NULL && 
			   plg_data[plugin_number]->wind_struct->whandlem != -1)
				Window.top(plg_data[plugin_number]->wind_struct->whandlem);
			else
			if(modconfs[plugin_number]->windhandle != 0)
			{
				msgbuf[0] = WM_TOPPED;
				msgbuf[1] = Sys_info.app_id;
				msgbuf[3] = modconfs[plugin_number]->windhandle;
				memcpy(plg_data[plugin_number]->event_par, msgbuf, 16);
				start_plugin(plugin_bp[plugin_number], AES_MESSAGE, plugin_number|0x200, plg_data[plugin_number]);
			}

			return;
		}

		/*
		 *	oder wirklich neu starten?
		 */
		if(!plg_info[plugin_number]->resident)			/* wenn nichtresident, Plugin nachladen */
		{
			if(load_plugin(plugin_number) < 0)
				return;
			plg_data[plugin_number] = malloc(sizeof(PLUGIN_DATA));
			memset(plg_data[plugin_number], 0x0, sizeof(PLUGIN_DATA));
			start_plugin(plugin_bp[plugin_number], MSTART, plugin_number|0x200, plg_data[plugin_number]);
		}

		start_plugin(plugin_bp[plugin_number], PLGSELECTED, plugin_number|0x200, plg_data[plugin_number]);

		/* Plugin lÑuft noch und kein M_WAITING? -> wieder weg damit. */
		if(plg_data[plugin_number] != NULL && plg_data[plugin_number]->message != M_WAITING)
			terminate_plugin(plugin_number);
	}

	return;
} /* call_plugin */


/* load_plugin -------------------------------------------------------
	FÑhrt das Plugin mit der Nummer plugin_number hoch. 
	plugin_paths[plugin_number] muû entsprechend gefÅllt sein!
	Diese Funktion wird z.B. benutzt, um nichtresidente Plugins 
	nachzuladen.
	-----------------------------------------------------------------*/
int load_plugin(int plugin_number)
{
	char alert[128];
	long temp, lback, ProcLen, mod_magic;

	extern	long get_proclen(BASPAG *baspag);

	
	Dialog.busy.reset(128, "Lade Plugin");

	temp = Pexec(3, plugin_paths[plugin_number], NULL, NULL);
	if(temp < 0)
	{
		strcpy(alert, "[1][Fehler beim Nachladen|");
		strcat(alert, "des Plugins!]");
		strcat(alert, "[ Oh ]");
		form_alert(1, alert);
		return(-1);
	}
	else
	{	
		plugin_bp[plugin_number] = (BASPAG *)temp;

		/*---- LÑnge des gesamten Tochterprozesses ermitteln */
		ProcLen = get_proclen(plugin_bp[plugin_number]);
		Mshrink(0, plugin_bp[plugin_number], ProcLen);		/* Speicherblock verkÅrzen */
		plugin_bp[plugin_number]->p_hitpa = (void *)((long)plugin_bp[plugin_number] + ProcLen);

		lback = Pexec(4, 0L, (char *)plugin_bp[plugin_number], 0L);
		if(lback < 0L)
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);

		mod_magic = get_modmagic(plugin_bp[plugin_number]);							/* Zeiger auf Magic (muû 'PLGN' sein!) */

		if(mod_magic != 'SPLG')
		{
			strcpy(alert, "[1][Plugin-typ ist nicht");
			strcat(alert, " |registriert!]");
			strcat(alert, "[ Au weia ]");
			form_alert(1, alert);
			return(-1);
		}
	}

	return(0);
} /* load_plugin */


/* plugin_startup ----------------------------------------------------------
	Handled die Startup-Kommunikation mit dem Plugin.
	------------------------------------------------------------------------*/
void plugin_startup(int index, int *curr_plugin_entry, char *plg_filename)
{
	char alert[128];

	int message, menuentry;


	start_plugin(plugin_bp[index], MSTART, index|0x200, plg_data[index]);
	
	message = plg_data[index]->message;

	if(message != MENU_ENTRY && message != FIXED_PLUGIN && message != INVISIBLE)
	{
		strcpy(alert, "[1][Plugin|");
		strcat(alert, plg_filename);
		strcat(alert, "|initialisiert sich nicht!][ Oh ]");
		form_alert(1, alert);
		return;
	}

	/*
	 *	Plugin installieren (MenÅeintrag, unsichtbar, etc.)
	 */
	if(message == MENU_ENTRY)
	{
		if(plg_data[index]->event_par[0] == 0)			/* ... im Plugin-MenÅ */
		{
			menuentry = PLUGIN1 + *curr_plugin_entry;
			menu2plugin[menuentry] = index;
			(*curr_plugin_entry)++;
		}
		else
		{
			menuentry = plg_data[index]->event_par[0];	/* oder einen anderen */
			menu2plugin[menuentry] = index;
		}

		insert_to_menu(menu_tree, menuentry, plg_data[index]->plugin_name);
	}

	/*
	 *	So. Und jetzt die Startup-Konfiguration des Plugins...
	 *	Es kommt also nur einmal PLG_STARTUP, der Rest wird Åber
	 *	RÅckgabemessages bzw. die in-Message DONE geregelt. Diese wird nach PLG_STARTUP
	 *	mit RÅckgabe != M_WAITING wie gehabt solange geschickt, bis ein M_WAITING kommt.
	 */	
	start_plugin(plugin_bp[index], PLG_STARTUP, index, plg_data[index]);

	return;	
} /* plugin_startup */


/*---------------------------------------------------------------------------
	Plugin terminieren: mit MTERM aufrufen und Speicher freigeben.
	-------------------------------------------------------------------------*/
void terminate_plugin(int index)
{
	char *dummy = NULL;


	start_plugin(plugin_bp[index], MTERM, index, plg_data[index]);
/*	Pexec(102, dummy, plugin_bp[index], ""); */

	SMfree(plugin_bp[index]->p_env);
	SMfree(plugin_bp[index]);
	plugin_bp[index] = NULL;
	free(plg_data[index]);
	plg_data[index] = NULL;
	free(modconfs[index]);
	modconfs[index] = NULL;

	return;
} /* terminate_plugin */


/* insert_to_menu ---------------------------------------------
	HÑngt den String string ins PulldownmenÅ menu in den Eintrag
	entry als MenÅeintrag ein. string darf nicht lÑnger als der
	ursprÅngliche MenÅeintrag sein!
	-----------------------------------------------------------*/
void insert_to_menu(OBJECT *menu, int entry, char *string)
{
	char *dest_str;
	char shortcut[6];
	long string_len;


	dest_str = menu[entry].ob_spec.free_string;

	strcpy(shortcut, dest_str + strlen(dest_str) - 4);

	string_len = strlen(string);

	strncpy(dest_str + 2, string, string_len);
	
	menu_ienable(menu, entry, 1);

	return;
} /* insert_to_menu */


/*---------------------------------------------------------------
	Initialisieren der Strukturen fÅr Funktionen und Variablen
	--------------------------------------------------------------*/
void init_structs(void)
{
	extern	POP_UP	popups[25];	

	extern	char *edit_modules[100];		/* Pfade fÅr bis zu 100 Edit-Module */
	extern	char *export_modules[100];		/* Pfade fÅr bis zu 100 Export-Module */

	extern	char *export_cnfblock[50];		/* Konfigurationsblîcke fÅr die Exporter */
	extern	int	export_cnflen[50];			/* LÑnge des jeweiligen Blockes */
	extern	int	anzahl_importmods, anzahl_dithermods;
	extern	EXPORT_CONFIG exp_conf;				/* laufender Exporter */

	extern	int dither_for_export(MOD_ABILITY *mod_abs, int max_expdepth, int dest_format, SMURF_PIC *converted_pic);
	extern	DITHER_MOD_INFO *ditmod_info[10];

	extern int mouse_xpos, mouse_ypos, mouse_button, klicks, key_scancode, key_ascii, key_at_event, obj;


	/*---------- Globale Funktionsstruktur zum Zugriff fÅr die Module */
	global_functions.start_imp_module = start_imp_module;
	global_functions.start_exp_module = start_exp_module;
	global_functions.start_edit_module = start_edit_module;
	global_functions.start_dither_module = start_dither_module;
	global_functions.check_and_terminate = check_and_terminate;

	global_functions.f_event = f_event;
	global_functions.f_handle_message = f_handle_message;
	global_functions.f_pic_event = f_pic_event;

	global_functions.check_windclose = check_windclose;
	global_functions.CallDialog = CallDialog;
	global_functions.init_dialog = init_dialog;
	global_functions.close_dialog = close_dialog;
	global_functions.close_window = Window.close;

	global_functions.f_module_prefs = f_module_prefs;
	global_functions.f_mpref_change = f_mpref_change;
	global_functions.transform_pic = transform_pic;

	global_functions.tfm_std_to_std = tfm_std_to_std;
	global_functions.tfm_std_to_pp = tfm_std_to_pp;
	global_functions.tfm_pp_to_std8 = tfm_pp_to_std8;
	global_functions.tfm_8_to_16 = tfm_8_to_16;
	global_functions.tfm_8_to_24 = tfm_8_to_24;
	global_functions.tfm_16_to_24 = tfm_16_to_24;
	global_functions.tfm_24_to_16 = tfm_24_to_16;
	global_functions.tfm_bgr_to_rgb =  tfm_bgr_to_rgb;
	global_functions.tfm_cmy_to_rgb = tfm_cmy_to_rgb;
	global_functions.tfm_rgb_to_grey = tfm_rgb_to_grey;

	global_functions.f_convert = f_convert;
	global_functions.dither_for_export = dither_for_export;
/*	global_functions.dither = f_dither; */

	global_functions.init_smurfrsc = init_smurfrsc;
	global_functions.fix_rsc = fix_rsc;
	global_functions.f_init_popups = f_init_popups;
	global_functions.f_init_sliders = f_init_sliders;

	global_functions.f_draw_blockbox = f_draw_blockbox;
	global_functions.f_draw_crosshair = f_draw_crosshair;

	global_functions.build_up_filelist = build_up_filelist;
	global_functions.insert_entry = insert_entry;
	global_functions.destroy_filelist = destroy_filelist;

	global_functions.file_save = file_save;
	global_functions.file_load = file_load;
	global_functions.f_fsbox = f_fsbox;

	global_functions.fload = fload;

	global_functions.f_import_pic = f_import_pic;
	global_functions.f_formhandle = f_formhandle;
	global_functions.f_do_pcd = f_do_pcd;

	global_functions.save_config = save_config;
	global_functions.load_config = load_config;

	global_functions.destroy_smurfpic = destroy_smurfpic;

	/*---------- Globale Variablenstruktur zum Zugriff fÅr die Module */
	global_vars.info_window =info_window;
	global_vars.form_pop =form_pop;
	global_vars.edit_pop =edit_pop;
	global_vars.col_pop =col_pop;
	global_vars.display_opt = Dialog.dispOpt.tree;
	global_vars.pic_form =pic_form;
	global_vars.pic_info_form =pic_info_form;
	global_vars.options_form = Dialog.smurfOpt.tree;
	global_vars.alert_form =alert_form;
	global_vars.module_form =module_form;
	global_vars.busy_window = Dialog.busy.busyTree;
	global_vars.modules_window = Dialog.emodList.tree;
	global_vars.newpic_window =newpic_window;
	global_vars.exmod_window = Dialog.expmodList.tree;
	global_vars.form_picmanager = Dialog.picMan.tree;
	global_vars.export_form =export_form;
	global_vars.exp_dp_popup =exp_dp_popup;
	global_vars.picorder_popup =picorder_popup;
	global_vars.colred_popup =colred_popup;
	global_vars.blockpopup =blockpopup;
	global_vars.mod_information = Dialog.emodList.infoTree;
	global_vars.exmod_info =exmod_info;
	global_vars.image_order_window =image_order_window;
	global_vars.transform_window =transform_window;
	global_vars.blocktype_window =blocktype_window;

	global_vars.menu_tree = menu_tree;
	global_vars.u_tree = u_tree;
	global_vars.alerts = Dialog.winAlert.alerts;

	global_vars.loadpath = loadpath;
	global_vars.savepath = savepath;
	global_vars.commpath = commpath;
	global_vars.DraufschmeissBild = &DraufschmeissBild;

	global_vars.Sys_info = &Sys_info;

	global_vars.wind_s = wind_s;
	global_vars.picture_windows = picture_windows;
	global_vars.smurf_picture = &smurf_picture[0];
	global_vars.smurf_struct = &module.smStruct[0];
	global_vars.edit_bp = &module.bp[0];
	
	global_vars.picwindthere = &picwindthere;
	global_vars.dialwindthere = &dialwindthere;
	global_vars.picthere = &picthere;
	global_vars.active_pic = &active_pic;
	
	global_vars.export_list = &Dialog.expmodList.modList;
	global_vars.edit_list = &Dialog.emodList.modList;
	global_vars.picture_list = &Dialog.picMan.pictureList;

	global_vars.ditmod_info = &ditmod_info[0];
	
	global_vars.pop_ups = popups;

	global_vars.exp_conf = &exp_conf;
	global_vars.anzahl_exporter = Dialog.expmodList.anzahl;		/* Anzahl an Export-Modulen */
	global_vars.anzahl_editmods = Dialog.emodList.anzahl;		/* Anzahl an Edit-Modulen */
	global_vars.anzahl_importmods = anzahl_importmods;	/* Anzahl an Import-Modulen */
	global_vars.anzahl_dithermods = anzahl_dithermods;	/* Anzahl an Dither-Modulen */

	global_vars.edit_modules = &edit_modules[0];				/* Pfade fÅr bis zu 100 Edit-Module */
	global_vars.edit_module_names = &Dialog.emodList.modNames[0];		/* Namen fÅr bis zu 100 Edit-Module */

	global_vars.export_modules = &export_modules[0];			/* Pfade fÅr bis zu 100 Export-Module */
	global_vars.export_module_names = &Dialog.expmodList.modNames[0];	/* Namen fÅr bis zu 100 Export-Module */

	global_vars.export_cnfblock = &export_cnfblock[0];		/* Konfigurationsblîcke fÅr die Exporter */

	global_vars.export_cnflen = export_cnflen;			/* LÑnge des jeweiligen Blockes */

	global_vars.mouse_xpos = &mouse_xpos;
	global_vars.mouse_ypos = &mouse_ypos;
	global_vars.mouse_button = &mouse_button;
	global_vars.klicks = &klicks;
	global_vars.key_scancode = &key_scancode;
	global_vars.key_ascii = &key_ascii;
	global_vars.key_at_event = &key_at_event;
	global_vars.obj = &obj;
	
	return;
} /* init_structs */