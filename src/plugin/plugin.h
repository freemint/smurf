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
 * Olaf Piesche, Christian Eyrich, Dale Russell and J�rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/*-----------------------------------------------------------------	*/
/*	Strukturdefinitionen f�r die PlugIn - Schnittstelle				*/
/*	f�r SMURF Grafikkonverter (der immer mehr zu einem 				*/
/*	Betriebssytem wird...), 21.09.1997 22:55, Olaf Piesche			*/
/*																	*/
/*	Plugins Funktionsweise:											*/
/*		Bei SMURF-Programmstart werden die Plugins hochgeladen und	*/
/*		mit der Message MSTART zum Initialisieren wie Module		*/
/*		aufgerufen.	Dann k�nnen sie sich in eine oder mehrere der	*/
/*		Funktionen in den �bergebenen Strukturen PLUGIN_FUNCTIONS	*/
/*		und ADD_FUNCTIONS einh�ngen. 								*/
/*		Hierzu gibt es zwei M�glichkeiten: Entweder ersetzt ein 	*/
/*		Plugin eine Funktion komplett, oder es h�ngt sich "hinten"	*/
/*		dran. Vergleichbar mit dem XBRA-Verfahren wird das Plugin	*/
/*		dazu in eine Liste eingetragen, �ber die es nach dem Aufruf	*/
/*		der eigentlichen Funktion aufgerufen wird. Wenn sich		*/
/*		mehrere Module an dieselbe Funktion anh�ngen wollen, werden	*/
/*		sie in der Reihenfolge aufgerufen, die sich aus der 		*/
/*		Priorit�t in plugin_flag in PLUGIN_DATA ergibt.	Das setzt	*/
/*		�u�erst sorgf�ltige Programmierung und Verwaltung voraus!	*/
/*		Der Modulfunktion, die eingeh�ngt 							*/
/*		wird, werden immer dieselben Parameter �bergeben, wie der	*/
/*		eigentlichen Funktion. �ber den Parameter plugin_flag kann	*/
/*		das Plugin seine gew�nschte Position unabh�ngig von seiner	*/
/*		physikalischen Dateiposition angeben, an die es in der 		*/
/*		Branch-Liste f�r die entsprechende Funktion gerne m�chte.	*/
/*		Bei SMURF-Programmende werden die Plugins nacheinander mit	*/
/*		MTERM aufgerufen, um sich zu deinitialisieren und 			*/
/*		terminieren.												*/
/*	Die dritte der zwei M�glichkeiten ;-) :							*/
/*		Mit der Message MENU_ENTRY kann sich ein Plugin unter dem	*/
/*		erweiterten Namen ins Smurf-Men� "Plugin" eintragen lassen. */
/*		Bei Klick auf den entsprechenden Men�eintrag wird die		*/
/*		Message PLGSELECTED an das entsprechende Plugin geschickt,	*/
/*		und es kann Loslegen (z.B. Fenster �ffnen, etc.). Fenster-	*/
/*		Handling und Events (MBEVT, MWRED) laufen wie bei den 		*/
/*		Modulen ab, beim Fensterschlie�en wird allerdings nicht 	*/
/*		MTERM sondern MWINDCLOSED geschickt.						*/
/*		FIXED_PLUGIN ist das Zeichen f�r Smurf, da� das Plugin fest */
/*		verdrahtet ist, wie z.B. Multikonvert und Print.			*/
/*																	*/
/*	Die Plugin-Schnittstelle wird nicht �ffentlich dokumentiert.	*/
/*	Basta. Das ist einfach zuviel Arbeit f�r den Programmierer und	*/
/*	vor allem f�r mich, der ich die Dokumentation dazu schreiben	*/
/*	m��te ;-)														*/
/*-----------------------------------------------------------------	*/

#define MAX_PIC	25

#define PLG_INFO_OFFSET 12		/* Abstand der Plugin-Infostruktur vom Textsegementanfang */


/*----- Messages f�r Plugins von Smurf*/
#define	MWINDCLOSED	257				/* Beim Fensterschlie�en kann nicht MTERM geschickt werden,
										weil das Plugin ja trotzdem weiterlaufen soll.	*/
#define	PLGSELECTED 258
#define PLG_STARTUP	259				/* f�r die Startup-Kommunikation */

/*----- Messages f�r Smurf von Plugins*/
#define	MENU_ENTRY		256			/* Plugin will ins Plugin-Men� */
#define	FIXED_PLUGIN	257			/* Plugin ist ein festverdrahtetes welches */
#define	INVISIBLE		258			/* Plugin ist nicht f�r den User sichtbar */




/*------------------------------------------------------------------------------
	Struktur f�r die Funktionen, die von den Plugins ersetzt oder aufgerufen
	werden k�nnen. Dies sind s�mtlich Funktionen aus dem SMURF-
	Hauptprogramm. Zum Ersetzen tr�gt das Plugin einfach die Adresse seiner Funktion
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
	* Systemfunktionen (Eventhandler u.�.)
	*/
	void	(*f_event)(void);				/* Allg. Event-Funktion */
	int		(*f_handle_message)(void);		/* AES-Message-Handler	*/
	void 	(*f_pic_event)(WINDOW *picwindow, int event_type, int windnum);

	/*
	* Dialogfunktionen
	*/
	void	(*check_windclose)(int windnum);	/* Auswerten von Windowdialog-Abbr�chen */
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
	*	Bild importieren und was dazugeh�rt
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
	Struktur f�r Funktionen, an die sich Plugins anh�ngen k�nnen. In die Zeiger
	m�ssen von den Plugins die Adressen der eigenen Funktionen eingetragen werden.
	Die Parameteranzahl und -Typ d�rfen sich nicht unterscheiden, egal ob die 
	Parameter ben�tigt werden oder nicht, sonst gibt's Stackrprobleme!
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
	Struktur f�r Smurf-Variablen, zum Zugriff oder �ndern durch Plugins.
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
	OBJECT	*exp_dp_popup;				/* Farbtiefen f�r Export	*/
	OBJECT	*picorder_popup;			/* Bildreihenfolge-Popup	*/
	OBJECT	*colred_popup;				/* Palettenmodus-Popup */
	OBJECT	*blockpopup;				/* Blockpopup */
	OBJECT	*mod_information;			/* Edit-Modulinformation - Dialog */
	OBJECT	*exmod_info;				/* Exportmodul - Info */
	OBJECT	*image_order_window;		/* Bildreihenfolge-Window */
	OBJECT	*transform_window;			/* Bild wandeln */
	OBJECT	*blocktype_window;			/* Block eingeben */

	OBJECT	*menu_tree;					/* Men�leiste */
	OBJECT 	*u_tree;					/* Radiobutton/Checkbox-Formular	*/
	OBJECT	*alerts;					/* Alert-Strings */

	char *loadpath;				/* voller Pfad der zuletzt geladenen Datei (256 Bytes) */
	char *savepath;				/* voller Pfad der zuletzt gespeicherten Datei (256 Bytes) */
	char *commpath;				/* voller Pfad der zuletzt �ber ein Protokoll empfangenen Datei (256 Bytes)*/
	char *DraufschmeissBild;	/* D&D-Flag (1 Byte!) */

	SYSTEM_INFO *Sys_info;			/* Systemkonfiguration */

	WINDOW *wind_s;
	WINDOW *picture_windows;
	SMURF_PIC * *smurf_picture;
	GARGAMEL * *smurf_struct;			/* �bergabestrukturen der Edit-Module */
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

	char * *edit_modules;			/* Pfade f�r bis zu 100 Edit-Module */
	char * *edit_module_names;		/* Namen f�r bis zu 100 Edit-Module */
	char * *export_modules;			/* Pfade f�r bis zu 100 Export-Module */
	char * *export_module_names;	/* Namen f�r bis zu 100 Export-Module */

	char * *export_cnfblock;		/* Konfigurationsbl�cke f�r die Exporter */
	int	* export_cnflen;			/* L�nge des jeweiligen Blockes */

	int *mouse_xpos,*mouse_ypos,*mouse_button,*klicks,
		*obj,*key_scancode,*key_ascii,*key_at_event;
} SMURF_VARIABLES;



/*--------------------------------------------------------------------
	Struktur f�r die Daten�bergabe und Kommunikation mit Plugins
	Die Funktionszeiger in den Strukturen replace und call sind von Smurf
	s�mtlich initialisiert, das Modul mu� nur noch die Funktionen,
	die es einh�ngen will, in die entsprechenden Zeiger eintragen. 
	Wichtig: Nur das �ndern, wo auch wirklich was eingeh�ngt werden soll!
	F�r alle call-Funktionen gilt die Branch-Position in plugin_flag.
	Bei MSTART mu� vom Plugin die �bergebene Struktur in den Variablen
	plugin_name und ggfs. plugin_flag initialisiert werden.
	-----------------------------------------------------------------------*/
typedef struct
{
	WINDOW *wind_struct;					/* evtl. Fenster des Plugins */
	SERVICE_FUNCTIONS	*services;			/* die �blichen Dienstfunktionen... */

	/* vom Plugin �nderbarer Kram: */
	PLUGIN_FUNCTIONS	*replace;			/* Funktionen zum Ersetzen */
	ADD_FUNCTIONS		*call;				/* Funktionen zum Dranh�ngen */
	SMURF_VARIABLES		*smurf_vars;		/* Smurf-Variablen f�rs Modul */

	/* nur f�r Lesezugriff durch Plugin: */
	PLUGIN_FUNCTIONS	*smurf_functions;	/* Smurf-Funktionen zum Aufrufen f�rs Plugin */


	/* Kommunikation und Information f�r Smurf */
	int message;					/* Message */
	int event_par[10];				/* Beim Event betroffenes Objekt und weitere Daten*/
	int mousex,mousey;				/* Mauspos beim Event 			*/
	int klicks;						/* Mausklicks beim Event 		*/

	char	plugin_name[25];		/* erweiterter Plugin-Name f. Men�titel */
	int		plugin_flag;			/* Bit 0+1: Gew�nschte Position in evtl.
										Branch-Liste:	%_0: egal,
										sonst			%01: ganz vorne,
														%11: ganz hinten
									falls sich mehrere Plugins an dieselbe
									Funktion anh�ngen wollen. */
	int id;
	struct moduleObj *module_object;
} PLUGIN_DATA;



/*--------------------------------------------------------------
	Diese Struktur h�ngt im Plugin-Header und erm�glicht folgendes:
	�ber for_smurf_version kann Smurf einfach feststellen, ob ein Plugin �berhaupt 
	f�r ihn gedacht ist. �ber name und plugin_version kann er feststellen, ob ein 
	Plugin zweimal in unterschiedlichen	Versionsnummern vorhanden ist, und ggfs. 
	das veraltete gleich abmelden. Diese beiden Punkte sind hier extrem wichtig, weil
	Plugins direkt in Smurf eingreifen. 
	Au�erdem wird es eine Dienstfunktion geben, mit der ein Plugin alle anderen aktiven 
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



