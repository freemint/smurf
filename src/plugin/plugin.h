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

/*-----------------------------------------------------------------	*/
/*	Strukturdefinitionen fÅr die PlugIn - Schnittstelle				*/
/*	fÅr SMURF Grafikkonverter (der immer mehr zu einem 				*/
/*	Betriebssytem wird...), 21.09.1997 22:55, Olaf Piesche			*/
/*																	*/
/*	Plugins Funktionsweise:											*/
/*		Bei SMURF-Programmstart werden die Plugins hochgeladen und	*/
/*		mit der Message MSTART zum Initialisieren wie Module		*/
/*		aufgerufen.	Dann kînnen sie sich in eine oder mehrere der	*/
/*		Funktionen in den Åbergebenen Strukturen PLUGIN_FUNCTIONS	*/
/*		und ADD_FUNCTIONS einhÑngen. 								*/
/*		Hierzu gibt es zwei Mîglichkeiten: Entweder ersetzt ein 	*/
/*		Plugin eine Funktion komplett, oder es hÑngt sich "hinten"	*/
/*		dran. Vergleichbar mit dem XBRA-Verfahren wird das Plugin	*/
/*		dazu in eine Liste eingetragen, Åber die es nach dem Aufruf	*/
/*		der eigentlichen Funktion aufgerufen wird. Wenn sich		*/
/*		mehrere Module an dieselbe Funktion anhÑngen wollen, werden	*/
/*		sie in der Reihenfolge aufgerufen, die sich aus der 		*/
/*		PrioritÑt in plugin_flag in PLUGIN_DATA ergibt.	Das setzt	*/
/*		Ñuûerst sorgfÑltige Programmierung und Verwaltung voraus!	*/
/*		Der Modulfunktion, die eingehÑngt 							*/
/*		wird, werden immer dieselben Parameter Åbergeben, wie der	*/
/*		eigentlichen Funktion. öber den Parameter plugin_flag kann	*/
/*		das Plugin seine gewÅnschte Position unabhÑngig von seiner	*/
/*		physikalischen Dateiposition angeben, an die es in der 		*/
/*		Branch-Liste fÅr die entsprechende Funktion gerne mîchte.	*/
/*		Bei SMURF-Programmende werden die Plugins nacheinander mit	*/
/*		MTERM aufgerufen, um sich zu deinitialisieren und 			*/
/*		terminieren.												*/
/*	Die dritte der zwei Mîglichkeiten ;-) :							*/
/*		Mit der Message MENU_ENTRY kann sich ein Plugin unter dem	*/
/*		erweiterten Namen ins Smurf-MenÅ "Plugin" eintragen lassen. */
/*		Bei Klick auf den entsprechenden MenÅeintrag wird die		*/
/*		Message PLGSELECTED an das entsprechende Plugin geschickt,	*/
/*		und es kann Loslegen (z.B. Fenster îffnen, etc.). Fenster-	*/
/*		Handling und Events (MBEVT, MWRED) laufen wie bei den 		*/
/*		Modulen ab, beim Fensterschlieûen wird allerdings nicht 	*/
/*		MTERM sondern MWINDCLOSED geschickt.						*/
/*		FIXED_PLUGIN ist das Zeichen fÅr Smurf, daû das Plugin fest */
/*		verdrahtet ist, wie z.B. Multikonvert und Print.			*/
/*																	*/
/*	Die Plugin-Schnittstelle wird nicht îffentlich dokumentiert.	*/
/*	Basta. Das ist einfach zuviel Arbeit fÅr den Programmierer und	*/
/*	vor allem fÅr mich, der ich die Dokumentation dazu schreiben	*/
/*	mÅûte ;-)														*/
/*-----------------------------------------------------------------	*/

#define MAX_PIC	25

#define PLG_INFO_OFFSET 12		/* Abstand der Plugin-Infostruktur vom Textsegementanfang */


/*----- Messages fÅr Plugins von Smurf*/
#define	MWINDCLOSED	257				/* Beim Fensterschlieûen kann nicht MTERM geschickt werden,
										weil das Plugin ja trotzdem weiterlaufen soll.	*/
#define	PLGSELECTED 258
#define PLG_STARTUP	259				/* fÅr die Startup-Kommunikation */

/*----- Messages fÅr Smurf von Plugins*/
#define	MENU_ENTRY		256			/* Plugin will ins Plugin-MenÅ */
#define	FIXED_PLUGIN	257			/* Plugin ist ein festverdrahtetes welches */
#define	INVISIBLE		258			/* Plugin ist nicht fÅr den User sichtbar */




/*------------------------------------------------------------------------------
	Struktur fÅr die Funktionen, die von den Plugins ersetzt oder aufgerufen
	werden kînnen. Dies sind sÑmtlich Funktionen aus dem SMURF-
	Hauptprogramm. Zum Ersetzen trÑgt das Plugin einfach die Adresse seiner Funktion
	in den entsprechenden Zeiger der Funktion ein.
	----------------------------------------------------------------------------*/
typedef struct
{
	/*
	*	Funktionen zum Starten der Module 
	*/
	int			(*start_imp_module)(char *modpath, SMURF_PIC *imp_pic);
	EXPORT_PIC* (*start_exp_module)(char *modpath, int message, SMURF_PIC *pic_to_export, BASPAG *exbase, GARGAMEL *sm_struct, int mod_id);
	BASPAG*		(*start_edit_module)(char *modpath, BASPAG *edit_basepage, int mode, int mod_anz, GARGAMEL *smurf_struct);
	BASPAG*		(*start_dither_module)(int mode, int mod_id, DITHER_DATA *ditherdata);
	void 		(*check_and_terminate)(int mode, int module_number);

	/*
	*	Transformers und Ditherroutinen
	*/
	int		(*tfm_std_to_std)(SMURF_PIC *smurf_pic, char dst_depth, char mode);
	int		(*tfm_std_to_pp)(SMURF_PIC *picture, char dst_depth, char mode);
	int		(*tfm_pp_to_std8)(SMURF_PIC *picture, char mode);
	int		(*tfm_8_to_16)(SMURF_PIC *picture, char mode);
	int		(*tfm_8_to_24)(SMURF_PIC *picture, char mode);
	int		(*tfm_16_to_24)(SMURF_PIC *picture, char mode);
	int		(*tfm_24_to_16)(SMURF_PIC *picture, char mode);
	int		(*tfm_bgr_to_rgb)(SMURF_PIC *picture, char mode);
	int		(*tfm_cmy_to_rgb)(SMURF_PIC *picture, char mode);
	int		(*tfm_rgb_to_grey)(SMURF_PIC *picture, char mode);

	int		(*dither_for_export)(MOD_ABILITY *mod_abs, int max_expdepth, int dest_format, SMURF_PIC *converted_pic);
/*	int		(*dither)(SMURF_PIC *dest, SYSTEM_INFO *sys_info, int pic_changed); */
	int 	(*f_convert)(SMURF_PIC *picture, MOD_ABILITY *mod_abs, char modcolform, char mode, char automatic);


	/*
	* Systemfunktionen (Eventhandler u.Ñ.)
	*/
	void	(*f_event)(void);				/* Allg. Event-Funktion */
	int		(*f_handle_message)(void);		/* AES-Message-Handler	*/
	void 	(*f_pic_event)(WINDOW *picwindow, int event_type, int windnum);

	/*
	* Dialogfunktionen
	*/
	void	(*check_windclose)(int windnum);	/* Auswerten von Windowdialog-AbbrÅchen */
	int		(*CallDialog)(int topwin);			/* Dialogdispatcher */
	int		(*init_dialog)(int DialogNumber, int DialogOK);
	void	(*close_dialog)(int windnum);

	void	(*f_module_prefs)(MOD_INFO *infostruct, int mod_id);	/* Modulformular aufrufen */
	void	(*f_mpref_change)(void);								/* Eingabe im Modulformular */
	void 	(*transform_pic)(void);
	void	(*close_window)(int handle);

	/*
	* Resource-Funktionen
	*/
	void	(*init_smurfrsc)(char *rscpath);
	void	(*fix_rsc)(void);				/* RSC-Anpassung an OS, Farbtiefe, AES, etc... */
	void	(*f_init_popups)(void);		/* Alle Popups initialisieren */
	void	(*f_init_sliders)(void);	

	/*
	* Zeichenfunktionen (Blockbox, Fadenkreuz, etc.)
	*/
	void	(*f_draw_blockbox)(WINDOW *window);
	void	(*f_draw_crosshair)(WINDOW *window);		/* Positionierungskreuz in einem Bild zeichnen */


	/*
	* Fileroutinen
	*/
	struct DIRENTRY*	(*build_up_filelist)(char *path, char *ext, int pathlen);
	struct DIRENTRY*	(*insert_entry)(struct DIRENTRY *ende, char *string);
	void				(*destroy_filelist)(struct DIRENTRY *begin);

	/*
	*	Dateihandling mit Fileselector 
	*/
	int		(*file_save)(char *stext, char *buf, long length);
	void	(*file_load)(char *ltext, char **dateien, int mode);
	int		(*f_fsbox)(char *Path, char *fbtext, char selectart);
	char *	(*fload)(char *Path, int header);

	/* 
	*	Bild importieren und was dazugehîrt
	*/
	int		(*f_import_pic)(SMURF_PIC *smurf_picture, char *filename);
	int 	(*f_formhandle)(int picture_to_load, int module_ret, char *namename);
	char *	(*f_do_pcd)(char *Path);

	/*
	*	Konfiguration und Smurf-Dateien
	*/
	void	(*save_config)(void);
	int		(*load_config)(void);

	void	(*destroy_smurfpic)(SMURF_PIC *pic);
} PLUGIN_FUNCTIONS;




/*-----------------------------------------------------------------------------
	Struktur fÅr Funktionen, an die sich Plugins anhÑngen kînnen. In die Zeiger
	mÅssen von den Plugins die Adressen der eigenen Funktionen eingetragen werden.
	Die Parameteranzahl und -Typ dÅrfen sich nicht unterscheiden, egal ob die 
	Parameter benîtigt werden oder nicht, sonst gibt's Stackrprobleme!
	----------------------------------------------------------------------------*/
typedef struct
{
	/*------- Funktionen zum Starten der Module ----------------*/
	int			(*start_imp_module)(char *modpath, SMURF_PIC *imp_pic);
	EXPORT_PIC*	(*start_exp_module)(char *modpath, int message, SMURF_PIC *pic_to_export, BASPAG *exbase, GARGAMEL *sm_struct, int mod_id);
	BASPAG*		(*start_edit_module)(char *modpath, BASPAG *edit_basepage, int mode, int mod_anz, GARGAMEL *smurf_struct);
	BASPAG*		(*start_dither_module)(int mode, int mod_id, DITHER_DATA *ditherdata);
} ADD_FUNCTIONS;




/*----------------------------------------------------------------------
	Struktur fÅr Smurf-Variablen, zum Zugriff oder éndern durch Plugins.
	------------------------------------------------------------------*/
typedef struct
{
	/*------------------ die komplette Resource -------------------*/
	OBJECT 	*info_window;				/* Smurf-Info	*/
	OBJECT 	*form_pop;					/* Farbtiefen-Popup	*/
	OBJECT 	*edit_pop;					/* 	*/
	OBJECT 	*col_pop;					/* Dithermodul-popup	*/
	OBJECT 	*display_opt;				/* Display Options*/
	OBJECT	*pic_form;					/* Pic-Window-Formular	*/
	OBJECT	*pic_info_form;				/* Pic-Info-Formular	*/
	OBJECT	*options_form;				/* Smurf-Options-Formular	*/
	OBJECT	*alert_form;				/* WindAlert-Formular	*/
	OBJECT	*module_form;				/* Modul-Einstell-Formular	*/
	OBJECT	*busy_window;				/* Busywindow-Formular	*/
	OBJECT	*modules_window;			/* Edit-Modullisten-Formular	*/
	OBJECT	*newpic_window;				/* Neues-Bild-Formular	*/
	OBJECT	*exmod_window;				/* Expormodul-Formular */
	OBJECT	*form_picmanager;			/* Bild-Manager		*/
	OBJECT	*export_form;				/* Export-Formular	*/
	OBJECT	*exp_dp_popup;				/* Farbtiefen fÅr Export	*/
	OBJECT	*picorder_popup;			/* Bildreihenfolge-Popup	*/
	OBJECT	*colred_popup;				/* Palettenmodus-Popup */
	OBJECT	*blockpopup;				/* Blockpopup */
	OBJECT	*mod_information;			/* Edit-Modulinformation - Dialog */
	OBJECT	*exmod_info;				/* Exportmodul - Info */
	OBJECT	*image_order_window;		/* Bildreihenfolge-Window */
	OBJECT	*transform_window;			/* Bild wandeln */
	OBJECT	*blocktype_window;			/* Block eingeben */

	OBJECT	*menu_tree;					/* MenÅleiste */
	OBJECT 	*u_tree;					/* Radiobutton/Checkbox-Formular	*/
	OBJECT	*alerts;					/* Alert-Strings */

	char *loadpath;				/* voller Pfad der zuletzt geladenen Datei (256 Bytes) */
	char *savepath;				/* voller Pfad der zuletzt gespeicherten Datei (256 Bytes) */
	char *commpath;				/* voller Pfad der zuletzt Åber ein Protokoll empfangenen Datei (256 Bytes)*/
	char *DraufschmeissBild;	/* D&D-Flag (1 Byte!) */

	SYSTEM_INFO *Sys_info;			/* Systemkonfiguration */

	WINDOW *wind_s;
	WINDOW *picture_windows;
	SMURF_PIC * *smurf_picture;
	GARGAMEL * *smurf_struct;			/* öbergabestrukturen der Edit-Module */
	BASPAG * *edit_bp;

	int *picwindthere, *dialwindthere, *picthere;
	int *active_pic;
	
	LIST_FIELD *export_list;
	LIST_FIELD *edit_list;
	LIST_FIELD *picture_list;
	POP_UP *pop_ups;

	DITHER_MOD_INFO * *ditmod_info;

	EXPORT_CONFIG *exp_conf;
	int	anzahl_importmods, anzahl_editmods, anzahl_exporter, anzahl_dithermods;

	char * *edit_modules;			/* Pfade fÅr bis zu 100 Edit-Module */
	char * *edit_module_names;		/* Namen fÅr bis zu 100 Edit-Module */
	char * *export_modules;			/* Pfade fÅr bis zu 100 Export-Module */
	char * *export_module_names;	/* Namen fÅr bis zu 100 Export-Module */

	char * *export_cnfblock;		/* Konfigurationsblîcke fÅr die Exporter */
	int	* export_cnflen;			/* LÑnge des jeweiligen Blockes */

	int *mouse_xpos,*mouse_ypos,*mouse_button,*klicks,
		*obj,*key_scancode,*key_ascii,*key_at_event;
} SMURF_VARIABLES;



/*--------------------------------------------------------------------
	Struktur fÅr die DatenÅbergabe und Kommunikation mit Plugins
	Die Funktionszeiger in den Strukturen replace und call sind von Smurf
	sÑmtlich initialisiert, das Modul muû nur noch die Funktionen,
	die es einhÑngen will, in die entsprechenden Zeiger eintragen. 
	Wichtig: Nur das Ñndern, wo auch wirklich was eingehÑngt werden soll!
	FÅr alle call-Funktionen gilt die Branch-Position in plugin_flag.
	Bei MSTART muû vom Plugin die Åbergebene Struktur in den Variablen
	plugin_name und ggfs. plugin_flag initialisiert werden.
	-----------------------------------------------------------------------*/
typedef struct
{
	WINDOW *wind_struct;					/* evtl. Fenster des Plugins */
	SERVICE_FUNCTIONS	*services;			/* die Åblichen Dienstfunktionen... */

	/* vom Plugin Ñnderbarer Kram: */
	PLUGIN_FUNCTIONS	*replace;			/* Funktionen zum Ersetzen */
	ADD_FUNCTIONS		*call;				/* Funktionen zum DranhÑngen */
	SMURF_VARIABLES		*smurf_vars;		/* Smurf-Variablen fÅrs Modul */

	/* nur fÅr Lesezugriff durch Plugin: */
	PLUGIN_FUNCTIONS	*smurf_functions;	/* Smurf-Funktionen zum Aufrufen fÅrs Plugin */


	/* Kommunikation und Information fÅr Smurf */
	int message;					/* Message */
	int event_par[10];				/* Beim Event betroffenes Objekt und weitere Daten*/
	int mousex,mousey;				/* Mauspos beim Event 			*/
	int klicks;						/* Mausklicks beim Event 		*/

	char	plugin_name[25];		/* erweiterter Plugin-Name f. MenÅtitel */
	int		plugin_flag;			/* Bit 0+1: GewÅnschte Position in evtl.
										Branch-Liste:	%_0: egal,
										sonst			%01: ganz vorne,
														%11: ganz hinten
									falls sich mehrere Plugins an dieselbe
									Funktion anhÑngen wollen. */
	int id;
	struct moduleObj *module_object;
} PLUGIN_DATA;



/*--------------------------------------------------------------
	Diese Struktur hÑngt im Plugin-Header und ermîglicht folgendes:
	Åber for_smurf_version kann Smurf einfach feststellen, ob ein Plugin Åberhaupt 
	fÅr ihn gedacht ist. öber name und plugin_version kann er feststellen, ob ein 
	Plugin zweimal in unterschiedlichen	Versionsnummern vorhanden ist, und ggfs. 
	das veraltete gleich abmelden. Diese beiden Punkte sind hier extrem wichtig, weil
	Plugins direkt in Smurf eingreifen. 
	Auûerdem wird es eine Dienstfunktion geben, mit der ein Plugin alle anderen aktiven 
	Plugins abfragen kann. Damit kann ein Plugin sich individuell auf andere einstellen 
	und sogar andere Plugins aufrufen (!).
	Somit ist diese Schnittstelle die flexibelste von allen, und das alles mit dieser
	kleinen Struktur...
	-----------------------------------------------------------------*/
typedef struct plginfo
{
	char *name;
	int	plugin_version;
	int for_smurf_version;
	char resident;			/* Resident halten oder nicht? */
} PLUGIN_INFO;




/* Funktionsprototypen */
int start_plugin(BASPAG *bp, int message, int plg_id, PLUGIN_DATA *data);



