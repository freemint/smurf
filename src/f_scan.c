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
 * Olaf Piesche, Christian Eyrich, Dale Russell and J”rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/******************************************************************	*/
/*						SMURF Grafikkonverter						*/
/*	Alle Funktionen zum Generieren der Modul-Listen in der globalen	*/
/*	SYSTEM_INFO-Struktur.											*/
/*																	*/
/*	Ausgelagert und umgestellt auf systemunabh„ngige Filefunktionen	*/
/*	16.03.97 - 19.03.97 Christian									*/
/*																	*/
/******************************************************************	*/


#include <tos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include <ext.h>
/*#include <multiaes.h>*/
#include <aes.h>
#include <errno.h>

#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "smurf.h"
#include "globdefs.h"

#include "smurfobs.h"
#include "ext_obs.h"

extern SYSTEM_INFO Sys_info;
extern IMPORT_LIST Import_list;

extern char *export_modules[100];		/* Pfade fr bis zu 100 Export-Module */
extern char *edit_modules[100];			/* Pfade fr bis zu 100 Edit-Module */

extern int anzahl_importmods;			/* Anzahl an Import-Modulen */
extern int anzahl_dithermods;			/* Anzahl an Dither-Modulen */

extern BASPAG *Dithermod_Basepage[10];		/* Basepages fr Dithermodule */
extern DITHER_MOD_INFO *ditmod_info[10];

extern OBJECT *col_pop;					/* Zeiger auf Resource-DITHERPOPUP	*/

int files_read;
extern long Name_Max;

void save_extensions(MOD_INFO *module_info);
void save_import_list(void);

extern long get_proclen(BASPAG *baspag);

/* ----------------------------------------------------------------	*/
/*----------------- Edit-Module suchen und eintragen ---------------*/
/* Der Ordner MODULES\\EDIT wird nach Files mit der Extension		*/
/* .SEM durchsucht. Die Edit-Module sind in Gruppen unterteilt, 	*/
/* fr die repr„sentativ die Unterordner stehen, in dem sie sich	*/
/* befinden.                                                        */
/* ----------------------------------------------------------------	*/
void f_scan_edit(void)
{
	char *editpath;										/* voller Modulpfad, Original */
	char *edit_path;									/* voller Modulpfad, editable */
	char *swapstr, alert[128];
	char *textseg_begin;
	char edstring[64], strn[4];
	char *dummy = NULL;
	extern void set_startupdial(char *string);
	long mod_magic;

	int t, tt, biggest, pathlen;

	long ProcLen, entrlen;
	long temp, lback;

	BASPAG *edit_baspag;
	MOD_INFO *module_info;

	struct DIRENTRY *filelist, *actual;


	/*---- Pfade vorbereiten ----*/
	editpath = calloc(1, strlen(Sys_info.standard_path) + strlen("\\modules\\edit\\") + 1);
	strcpy(editpath, Sys_info.standard_path);
	strcat(editpath, "\\modules\\edit\\");

	Name_Max = get_maxnamelen(editpath);

	pathlen = (int)(strlen(editpath) + Name_Max);
	edit_path = (char *)calloc(1, pathlen + 1);
	swapstr = (char *)calloc(1, pathlen + 1);

	filelist = build_up_filelist(editpath, "sem", pathlen);

	Dialog.emodList.anzahl = 0;

	actual = filelist;
	while(actual != NULL)
	{
		/*
		 * Modul laden und Basepage ermitteln
		 */
		strcpy(edit_path, editpath);
		strcat(edit_path, actual->modname);

		temp = Pexec(3, edit_path, NULL, NULL);
		if(temp < 0)
		{
			if(temp == EACCDN)
			{
				strcpy(alert, "[1][Modul|");
				strcat(alert, actual->modname);
				strcat(alert, "|besitzt keine Berechtigung|");
				strcat(alert, "ausgefhrt zu werden|(Exec-Flag?)!]");
				strcat(alert, "[ OK ]");
			}
			else
			{
				strcpy(alert, "[1][Fehler in File|");
				strcat(alert, actual->modname);
				strcat(alert, "|im Ordner|");
				strcat(alert, "\\modules\\edit\\!]");
				strcat(alert, "[ OK ]");
			}

			form_alert(1, alert);
		}
		else
		{	
			edit_baspag = (BASPAG *)temp;

			mod_magic = get_modmagic(edit_baspag);			/* Zeiger auf Magic (muž 'SEMD' sein!) */
			if(mod_magic != 'SEMD')
			{
				strcpy(alert, "[1][Datei ");
				strcat(alert, actual->modname);
				strcat(alert, "|im Ordner|");
				strcat(alert, "\\modules\\edit\\|");
				strcat(alert, "ist kein Editmodul!][ OK ]");
				form_alert(1, alert);
			}
			else
			{
				/*
				 * L„nge des gesamten Tochterprozesses ermitteln
				 */
				ProcLen = get_proclen(edit_baspag);
				Mshrink(0, edit_baspag, ProcLen);		/* Speicherblock verkrzen */
				edit_baspag->p_hitpa = (void *)((long)edit_baspag + ProcLen);

				lback = Pexec(4, 0L, (char *)edit_baspag, 0L);
				if(lback < 0L)
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);

				textseg_begin = edit_baspag->p_tbase;			/* Textsegment-Startadresse holen */

				module_info = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));

				/*
				 * Modul eintragen
				 */
				edit_modules[Dialog.emodList.anzahl] = malloc(pathlen + 1);		/* keine variable L„nge wegen Sortierung! */
				strcpy(edit_modules[Dialog.emodList.anzahl], edit_path);

				Dialog.emodList.modNames[Dialog.emodList.anzahl] = (char *)calloc(1, 29);
				strncpy(Dialog.emodList.modNames[Dialog.emodList.anzahl], module_info->mod_name, 28);

				entrlen = strlen(Dialog.emodList.modNames[Dialog.emodList.anzahl]);
				if(entrlen < 28)
					strncat(Dialog.emodList.modNames[Dialog.emodList.anzahl], "                           ", 28 - entrlen);

				Dialog.emodList.anzahl++;

				/*---- gescante Module im Startupdialog hochz„hlen */
				if(!(Dialog.emodList.anzahl&3))
				{
					strcpy(edstring, itoa(Dialog.emodList.anzahl, strn, 10));
					strcat(edstring, " Editmodule");
					set_startupdial(edstring);
				}
			}

/*			Pexec(102, dummy, edit_baspag, 0L); */
			SMfree(edit_baspag->p_env);
			SMfree(edit_baspag);
		}

		actual = actual->next;
	}

	strcpy(edstring, itoa(Dialog.emodList.anzahl, strn, 10));
	strcat(edstring, " Editmodule");
	set_startupdial(edstring);

	/*---------------- Liste alphabetisch sortieren -------------*/
	for(t = 0; t < Dialog.emodList.anzahl; t++)
	{
		biggest = t;
		for(tt = t + 1; tt < Dialog.emodList.anzahl; tt++)
			if(stricmp(Dialog.emodList.modNames[tt], Dialog.emodList.modNames[biggest]) < 0)
				biggest = tt;

		strncpy(swapstr, Dialog.emodList.modNames[t], 29);
		strncpy(Dialog.emodList.modNames[t], Dialog.emodList.modNames[biggest], 29);
		strncpy(Dialog.emodList.modNames[biggest], swapstr, 29);
		strcpy(swapstr, edit_modules[t]);
		strcpy(edit_modules[t], edit_modules[biggest]);
		strcpy(edit_modules[biggest], swapstr);
	}

	destroy_filelist(filelist);

	free(swapstr);
	free(edit_path);
	free(editpath);

	return;
} /* f_scan_edit */


/* ----------------------------------------------------------------	*/
/*----------------- Import-Module suchen und eintragen -------------*/
/*	Durchsucht den Ordner MODULES\\IMPORT nach allen Importmodulen	*/
/*	und tr„gt diese mit deren untersttzten Extensionen in die ent- */
/*	sprechenden Arrays ein. Beim Bild-Laden wird dann zuerst diese	*/
/*	Liste nach der Extension des zu ladenden Files durchsucht und 	*/
/*	alle Importer aufgerufen, die diese Extension untersttzen.		*/
/*	Fruchtet dies nicht, werden die Importer der Reihe nach			*/
/*	aufgerufen, falls die Extension des Files nicht stimmt.			*/
/* ----------------------------------------------------------------	*/
void f_scan_import(void)
{
	char *importpath;									/* voller Modulpfad, Original */
	char *import_path;									/* voller Modulpfad, editable */
 	char *textseg_begin;
	char alert[128];
	char *dummy = NULL;
	long mod_magic;

	int pathlen, bh, bl, anzahl_extensions, t, tt;

	long ProcLen;
	long temp, lback;

	BASPAG *import_baspag;
	MOD_INFO *module_info;

	struct DIRENTRY *filelist, *actual;


	/*---- Pfade vorbereiten ----*/
	importpath = calloc(1, strlen(Sys_info.standard_path) + strlen("\\modules\\import\\") + 1);
	strcpy(importpath, Sys_info.standard_path);
	strcat(importpath, "\\modules\\import\\");

	Name_Max = get_maxnamelen(importpath);

	pathlen = (int)(strlen(importpath) + Name_Max);
	import_path = (char *)calloc(1, pathlen + 1);

	files_read = 0;
	filelist = build_up_filelist(importpath, "sim", pathlen);

	anzahl_importmods = 0;
	anzahl_extensions = 0;
	
	Dialog.busy.reset(0, "Importmodule...");

	bh = files_read / 8;			/* busy-height */
	bl = 0;							/* busy-length */

	/* bisherige Liste freigeben */
	free(Import_list.imp_mod_list[0]);

	files_read = 0;
	actual = filelist;
	while(actual != NULL)
	{
		if(!(files_read%bh))
		{
			Dialog.busy.draw(bl);
			bl += 16;
		}

		/*---- Modul laden und Basepage ermitteln */
		strcpy(import_path, importpath);
		strcat(import_path, actual->modname);

		temp = Pexec(3, import_path, NULL, NULL);
		if(temp < 0)
		{
			strcpy(alert, "[1][Fehler in File|");
			strcat(alert, actual->modname);
			strcat(alert, "|im Ordner|");
			strcat(alert, "\\modules\\import\\!]");
			strcat(alert, "[ OK ]");
			form_alert(1, alert);
		}
		else
		{	
			import_baspag = (BASPAG *)temp;

			mod_magic = get_modmagic(import_baspag);		/* Zeiger auf Magic (muž 'SIMD' sein!) */
			if(mod_magic != 'SIMD')
			{
				strcpy(alert, "[1][Datei ");
				strcat(alert, actual->modname);
				strcat(alert, "|im Ordner|");
				strcat(alert, "\\modules\\import\\|");
				strcat(alert, "ist kein importmodul!][ OK ]");
				form_alert(1, alert);
			}
			else
			{
				/*---- L„nge des gesamten Tochterprozesses ermitteln */
				ProcLen = get_proclen(import_baspag);
				Mshrink(0, import_baspag, ProcLen);			/* Speicherblock verkrzen */
				import_baspag->p_hitpa = (void *)((long)import_baspag + ProcLen);		

				lback = Pexec(4, 0L, (char *)import_baspag, 0L);
				if(lback < 0L)
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);

				textseg_begin = import_baspag->p_tbase;			/* Textsegment-Startadresse holen */

				module_info = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));
			
				/*---- Modul eintragen */
				Import_list.imp_mod_list[anzahl_importmods] = malloc(strlen(actual->modname) + 1);
				strcpy(Import_list.imp_mod_list[anzahl_importmods], actual->modname);
				
				/*---- Extensionen merken */
				save_extensions(module_info);

				for(t = 0; t < 10; t++)
				{
					if(strlen(Import_list.mod_exts[t][anzahl_importmods]) != 0)
						anzahl_extensions++;
				}
			
				anzahl_importmods++;
			}

/*			Pexec(102, dummy, import_baspag, 0L); */
			SMfree(import_baspag->p_env);
			SMfree(import_baspag);
		}

		actual = actual->next;
		files_read++;
	} /* while(actual != NULL) */

	if(filelist == NULL)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_IMODS_FOUND].TextCast, NULL, NULL, NULL, 1);

	save_import_list();

	destroy_filelist(filelist);

	free(import_path);
	free(importpath);

	/*
	 * reservierten Speicher wieder freigeben
	 */
	for(t = 0; t < anzahl_importmods; t++)
	{
		free(Import_list.imp_mod_list[t]);

		for(tt = 0; tt < 10; tt++)
			free(Import_list.mod_exts[tt][t]);
	}

	load_import_list();

	return;
} /* f_scan_import */



/* ----------------------------------------------------------------	*/
/*		Extensionen eines Importers in Import_list speichern		*/
/* ----------------------------------------------------------------	*/
void save_extensions(MOD_INFO *module_info)
{
	int t;


	t = 0;
	do
	{
		Import_list.mod_exts[t][anzahl_importmods] = malloc(strlen(module_info->ext[t]) + 1);
		strcpy(Import_list.mod_exts[t][anzahl_importmods], module_info->ext[t]);
		strupr(Import_list.mod_exts[t][anzahl_importmods]);
	} while(++t < 10);

	return;
} /* save_extensions */



/******************************************************************	*/
/*					Importerliste speichern							*/
/******************************************************************	*/
void save_import_list(void)
{
	char *string, listpath[256],
		 len;

	int filehandle, t, extnum, werror = 0, bh, bl;

	long dummy;


	strcpy(listpath, Sys_info.home_path);
	strcat(listpath, "\\ilist.dat");

	if((dummy = Fcreate(listpath, 0)) < 0)
	{
		Dialog.busy.reset(128, "Error");
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[CNF_SAVEERR].TextCast, NULL, NULL, NULL, 1);
	}
	else
	{
		filehandle = (int)dummy;

		Dialog.busy.reset(0, "Liste speichern...");
	
		if(Fwrite(filehandle, 2, &anzahl_importmods) != 2)		/* Anzahl speichern */
			werror = 1;
			
		if(!werror)
		{
			bh = anzahl_importmods / 6;			/* busy-height */
			bl = 0;								/* busy-length */

			for(t = 0; t < anzahl_importmods; t++)
			{
				if(!(t%bh))
				{
					Dialog.busy.draw(bl);
					bl += 10;
				}
	
				/* Modulname speichern */
				string = Import_list.imp_mod_list[t];
				len = (char)strlen(string) + 1;
				if(Fwrite(filehandle, len, string) != len)
				{
					werror = 1;
					break;
				}
			}
		}
		
		if(!werror)
		{
			for(t = 0; t < anzahl_importmods; t++)
			{
				if(!(t%bh))
				{
					Dialog.busy.draw(bl);
					bl += 11;
				}		
		
				for(extnum = 0; extnum < 10; extnum++)
				{		
					string = Import_list.mod_exts[extnum][t];
					len = (char)strlen(string) + 1;
					if(Fwrite(filehandle, len, string) != len)		/* EXT speichern */
					{
						werror = 1;
						break;
					}
				}
			}
		}
		
		Fclose(filehandle);
	}

	if(werror)
		Dialog.winAlert.openAlert("Fehler beim Schreiben der Importerliste!", NULL, NULL, NULL, 1);
	else
		Dialog.busy.ok();

	return;
} /* save_import_list */


/******************************************************************	*/
/*						Importerliste laden							*/
/******************************************************************	*/
int load_import_list(void)
{
	char *buf, listpath[256];

	int filehandle, t, extnum;

	long dummy, f_len;


	strcpy(listpath, Sys_info.home_path);
	strcat(listpath, "\\ilist.dat");

	if((dummy = Fopen(listpath, FO_READ)) < 0)
		return(-1);
	else
	{
		filehandle = (int)dummy;

		Dialog.busy.reset(0, "ILIST.DAT laden...");

		f_len = Fseek(0, filehandle, 2) - 2;
		Fseek(0, filehandle, 0);

		Fread(filehandle, 2, &anzahl_importmods);	/* Anzahl Importer auslesen */

		buf = (char *)malloc(f_len);
		Fread(filehandle, f_len, buf);

	/*---------------------------------------------- Importer-Pfade laden ------------*/
		for(t = 0; t < anzahl_importmods; t++)
		{
			Import_list.imp_mod_list[t] = buf;
			buf += strlen(buf) + 1;
		}


	/*---------------------------------------------- Importer-Extensions laden ------------*/
		for(t = 0; t < anzahl_importmods; t++)
		{
			for(extnum = 0; extnum < 10; extnum++)
			{
				Import_list.mod_exts[extnum][t] = buf;
				buf += strlen(buf) + 1;
			}
		}

		Fclose(filehandle);
	}

	Dialog.busy.ok();

	return(0);
} /* load_import_list */



/******************************************************************	*/
/*					Importerliste durchsuchen						*/
/*	Sucht die Importerlisten nach einer Extension ab, startet das	*/
/*	dazugeh”rige Importmodul und liefert dessen Rckgabe zurck.	*/
/*	Wird '*' als Extension bergeben, werden alle Importer 			*/
/*	durchlaufen.													*/
/******************************************************************	*/
int seek_module(SMURF_PIC *picture, char *extension)
{
	char modpath[257];
	char new_ext[5];

	int t = 0, seek;
	int mod_ret = M_INVALID;


	if(anzahl_importmods <= 0)
		return(M_INVALID);

	strncpy(new_ext, extension, 5);
	strupr(new_ext);

	if(*extension == '*')
	{
		t = 0;
		do
		{
			Dialog.busy.reset(0, Import_list.imp_mod_list[t]);
			strncpy(modpath, Sys_info.standard_path, 257);
			strcat(modpath, "\\modules\\import\\");
			strcat(modpath, Import_list.imp_mod_list[t]);
			if((mod_ret = module.comm.startImport(modpath, picture)) == M_STARTERR)
				Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);
		} while(++t < anzahl_importmods && mod_ret != M_PICDONE && mod_ret != M_DONEEXIT);
	}
	else
	{
		t = 0;
		do
		{
			for(seek = 0; seek < 10; seek++)
			{
				if(!Import_list.mod_exts[seek][t])
					break;
				if(strncmp(new_ext, Import_list.mod_exts[seek][t], 4) == 0)
				{
					Dialog.busy.reset(0, Import_list.imp_mod_list[t]);
					strncpy(modpath, Sys_info.standard_path, 257);
					strcat(modpath, "\\modules\\import\\");
					strcat(modpath, Import_list.imp_mod_list[t]);

					if((mod_ret = module.comm.startImport(modpath, picture)) == M_STARTERR)
						Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);
					break;
				}
			}
		} while(++t < anzahl_importmods && mod_ret != M_PICDONE && mod_ret != M_DONEEXIT);
	}

	return(mod_ret);
} /* seek_module */



/* ----------------------------------------------------------------	*/
/* -------------- Export-Module suchen und eintragen ---------------*/
/* ----------------------------------------------------------------	*/
void f_scan_export(void)
{
	char *expath;										/* voller Modulpfad, Original */
	char *ex_path;										/* voller Modulpfad, editable */
	char *swapstr, alert[128];
	char *textseg_begin;
	char *dummy = NULL;
	char edstring[64], strn[4];
	extern void set_startupdial(char *string);
	long mod_magic;

	int t, tt, biggest, pathlen;

	long ProcLen, entrlen;
	long temp, lback;

	BASPAG *export_baspag;
	MOD_INFO *module_info;

	struct DIRENTRY *filelist, *actual;


	/*---- Pfade vorbereiten ----*/
	expath = calloc(1, strlen(Sys_info.standard_path) + strlen("\\modules\\export\\") + 1);
	strcpy(expath, Sys_info.standard_path);
	strcat(expath, "\\modules\\export\\");

	Name_Max = get_maxnamelen(expath);

	pathlen = (int)(strlen(expath) + Name_Max);
	ex_path = (char *)calloc(1, pathlen + 1);
	swapstr = (char *)calloc(1, pathlen + 1);

	filelist = build_up_filelist(expath, "sxm", pathlen);

	Dialog.expmodList.anzahl = 0;

	actual = filelist;
	while(actual != NULL)
	{
		/*---- Modul als Overlay laden und Basepage ermitteln */
		strcpy(ex_path, expath);
		strcat(ex_path, actual->modname);

		temp = Pexec(3, ex_path, NULL, NULL);
		if(temp < 0)
		{
			strcpy(alert, "[1][Fehler in File|");
			strcat(alert, actual->modname);
			strcat(alert, "|im Ordner|");
			strcat(alert, "\\modules\\export\\!]");
			strcat(alert, "[ OK ]");
			form_alert(1, alert);
		}
		else
		{	
			export_baspag = (BASPAG *)temp;

			mod_magic = get_modmagic(export_baspag);			/* Zeiger auf Magic (muž 'SXMD' sein!) */
			if(mod_magic != 'SXMD')
			{
				strcpy(alert, "[1][Datei ");
				strcat(alert, actual->modname);
				strcat(alert, "|im Ordner|");
				strcat(alert, "\\modules\\export\\|");
				strcat(alert, "ist kein Exportmodul!][ OK ]");
				form_alert(1, alert);
			}
			else
			{
				/*---- L„nge des gesamten Tochterprozesses ermitteln */
				ProcLen = get_proclen(export_baspag);
				Mshrink(0, export_baspag, ProcLen);				/* Speicherblock verkrzen */
				export_baspag->p_hitpa = (void *)((long)export_baspag + ProcLen);

				lback = Pexec(4, 0L, (char *)export_baspag, 0L);
				if(lback < 0L)
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);

				textseg_begin = export_baspag->p_tbase;			/* Textsegment-Startadresse holen */

				module_info = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));

				/*---- Modul eintragen */
				export_modules[Dialog.expmodList.anzahl] = malloc(pathlen + 1);		/* keine variable L„nge wegen Sortierung! */
				strcpy(export_modules[Dialog.expmodList.anzahl], ex_path);
			
				Dialog.expmodList.modNames[Dialog.expmodList.anzahl] = (char *)calloc(1, 29);
				strncpy(Dialog.expmodList.modNames[Dialog.expmodList.anzahl], module_info->mod_name, 28);
				entrlen = strlen(Dialog.expmodList.modNames[Dialog.expmodList.anzahl]);
				if(entrlen < 28)
					strncat(Dialog.expmodList.modNames[Dialog.expmodList.anzahl], "                           ", 28 - entrlen);
		
				Dialog.expmodList.anzahl++;

				/*---- gescante Module im Startupdialog hochz„hlen */
				if(!(Dialog.expmodList.anzahl&3))
				{
					strcpy(edstring, itoa(Dialog.expmodList.anzahl, strn, 10));
					strcat(edstring, " Exporter");
					set_startupdial(edstring);
				}
			}

/*			Pexec(102, dummy, export_baspag, 0L); */
			SMfree(export_baspag->p_env);
			SMfree(export_baspag);
		}

		actual = actual->next;
	}

	strcpy(edstring, itoa(Dialog.expmodList.anzahl, strn, 10));
	strcat(edstring, " Exporter");
	set_startupdial(edstring);

	/*---------------- Liste alphabetisch sortieren -------------*/
	for(t = 0; t < Dialog.expmodList.anzahl; t++)
	{
		biggest = t;
		for(tt = t + 1; tt < Dialog.expmodList.anzahl; tt++)
			if(stricmp(Dialog.expmodList.modNames[tt], Dialog.expmodList.modNames[biggest]) < 0)
				biggest = tt;

		strncpy(swapstr, Dialog.expmodList.modNames[t], 29);
		strncpy(Dialog.expmodList.modNames[t], Dialog.expmodList.modNames[biggest], 29);
		strncpy(Dialog.expmodList.modNames[biggest], swapstr, 29);
		strcpy(swapstr, export_modules[t]);
		strcpy(export_modules[t], export_modules[biggest]);
		strcpy(export_modules[biggest], swapstr);
	}

	destroy_filelist(filelist);

	free(swapstr);
	free(ex_path);
	free(expath);

	return;
} /* f_scan_export */



/* ----------------------------------------------------------------	*/
/* -------------- Dither-Module suchen und eintragen ---------------*/
/*	Dithermodule stellen eine Sonderklasse der Module dar, da sie	*/
/*	grunds„tzlich resident gehalten werden und spezielle Info-		*/
/*	strukturen haben. 												*/
/* ----------------------------------------------------------------	*/
void f_scan_dither(void)
{
	char *ditpath;
	char *dit_path;
	char alert[128], string[20] = "";
	char *textseg_begin;
	extern void set_startupdial(char *string);
	long mod_magic;

	int pathlen;

	long ProcLen;
	long temp, lback;

	BASPAG *dit_baspag;

	struct DIRENTRY *filelist, *actual;


	/*---- Pfade vorbereiten ----*/
	ditpath = calloc(1, strlen(Sys_info.standard_path) + strlen("\\modules\\dither\\") + 1);
	strcpy(ditpath, Sys_info.standard_path);
	strcat(ditpath, "\\modules\\dither\\");

	Name_Max = get_maxnamelen(ditpath);	
	pathlen = (int)(strlen(ditpath) + Name_Max);
	dit_path = (char *)calloc(1, pathlen + 1);

	filelist = build_up_filelist(ditpath, "sdm", pathlen);

	anzahl_dithermods = 0;

	actual = filelist;
	while(actual != NULL)
	{
		/*---- Modul als Overlay laden und Basepage ermitteln */
		strcpy(dit_path, ditpath);
		strcat(dit_path, actual->modname);

		temp = Pexec(3, dit_path, NULL, NULL);
		if(temp < 0)
		{
			strcpy(alert, "[1][Fehler in File|");
			strcat(alert, actual->modname);
			strcat(alert, "|im Ordner|");
			strcat(alert, "\\modules\\dither\\!]");
			strcat(alert, "[ OK ]");
			form_alert(1, alert);
		}
		else
		{
			dit_baspag = (BASPAG *)temp;
		
			mod_magic = get_modmagic(dit_baspag);			/* Zeiger auf Magic (muž 'SDMD' sein!) */
			if(mod_magic != 'SDMD')
			{
				strcpy(alert, "[1][Datei ");
				strcat(alert, actual->modname);
				strcat(alert, "|im Ordner|");
				strcat(alert, "\\modules\\dither\\|");
				strcat(alert, "ist kein Dithermodul!][ OK ]");
				form_alert(1, alert);
			}
			else
			{
				/*---- L„nge des gesamten Tochterprozesses ermitteln */
				ProcLen = get_proclen(dit_baspag);
				Mshrink(0, dit_baspag, ProcLen);			/* Speicherblock verkrzen */
				dit_baspag->p_hitpa = (void *)((long)dit_baspag + ProcLen);

				lback = Pexec(4, 0L, (char *)dit_baspag, 0L);
				if(lback < 0L)
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_LOAD_ERR].TextCast, NULL, NULL, NULL, 1);

				textseg_begin = dit_baspag->p_tbase;			/* Textsegment-Startadresse holen */

				ditmod_info[anzahl_dithermods] = (DITHER_MOD_INFO *)*((DITHER_MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));
				Dithermod_Basepage[anzahl_dithermods] = dit_baspag;
				strncpy(string, ditmod_info[anzahl_dithermods]->algo_name, 15);
				ditmod_info[anzahl_dithermods]->algo_name[15] = '\0';
				set_startupdial(string);

				/* Algorithmus ins Popup eintragen und Button einschalten */	
				col_pop[anzahl_dithermods + 1].TextCast = ditmod_info[anzahl_dithermods]->algo_name;
				col_pop[anzahl_dithermods + 1].ob_state &= ~DISABLED;

				anzahl_dithermods++;
			}
		}

		actual = actual->next;
	}

	destroy_filelist(filelist);

	free(dit_path);
	free(ditpath);

	return;
} /* f_scan_dither */


/* build_up_filelist baut eine doppelt verkettete Liste der im Verzeichnis vorhandenen */
/* Dateien auf. */
/* Bevorzugt werden hierfr die MiNT-Funktionen Dopendir() und Dxreaddir() verwendet. */
/* Nur wenn diese nicht vorhanden sind, wird auf Fsfirst()/Fsnext() ausgewichen. */
/* Die Extensions werden jetzt auch bei Fsfirst()/Fsnext() manuell getestet! */
struct DIRENTRY *build_up_filelist(char *path, char *ext, int pathlen)
{
	char *mod_path;
	char *buf, *_buf, *temp;

	int buflen;

	long back, dirhandle;

	DTA *old_dta, *new_dta;

	struct DIRENTRY *begin, *actual;
	struct DIRENTRY Element;


	if((back = Dopendir(path, 0)) != EINVFN)		/* Verzeichnis im Normalmodus ”ffnen */
	{												/* und Test ob Dopendir() existiert */
/*		printf("Dopendir()/Dreaddir()\n\n");
		getch(); */

		Element.next = NULL;						/* Initial auf "keine Dateien enthalten" setzen */

		if((back&0xff000000L) != 0xff000000L)		/* Directory gefunden? */
		{											/* negativer Backval ist _nicht_ automatisch Fehler! */
			dirhandle = back;						/* Dopendir-Rckgabe an die richtige Variable geben */

			buflen = 4 + pathlen + 1;
			buf = (char *)calloc(1, buflen);

			actual = &Element;
			do
			{
				if((back = Dreaddir(buflen, dirhandle, buf)) != ENMFIL)	/* Noch eine Datei vorhanden */
				{
					_buf = buf + 4;										/* Dateiindex bergehen */

					if((temp = strrchr(_buf, '.')) != NULL)
						if(stricmp(temp + 1, ext) == 0)						/* Dreaddir beherrscht n„mlich keine Wildcards! */
						{
							actual = insert_entry(actual, _buf);		/* neuen Eintrag einh„ngen */
							files_read++;
						}
				}
				else
					break;												/* und wech wenn keine Datei mehr vorhanden */
			} while(actual != NULL);

			Dclosedir(dirhandle);										/* Verzeichnis wieder zu */
		}

		begin = Element.next;											/* Zeiger auf den ersten Eintrag holen */
	}
	else					/* Fsfirst()/Fsnext() */
	{
/*		printf("Fsfirst()/Fsnext()\n\n");
		getch(); */

		old_dta = Fgetdta();			/* DTA holen */
		new_dta = malloc(sizeof(DTA));
		Fsetdta(new_dta);				/* neue DTA setzen */

		/*---- Pfade vorbereiten ----*/
		mod_path = (char *)calloc(1, strlen(path) + 14);	/* path + Extender */
		strcpy(mod_path, path);
		strcat(mod_path, "*.*");

		Element.next = NULL;						/* Initial auf "keine Dateien enthalten" setzen */

		/*---- erstes File ermitteln ----*/
		if(Fsfirst(mod_path, 0) == 0)
		{
			actual = &Element;
			do
			{
				if((temp = strrchr(new_dta->d_fname, '.')) != NULL)
					if(stricmp(temp + 1, ext) == 0)
					{
						actual = insert_entry(actual, new_dta->d_fname);	/* neuen Eintrag einh„ngen */
						files_read++;
					}

				/*---- n„chsten Modulpfad ermitteln */
				strcpy(mod_path, path);
				strcat(mod_path, new_dta->d_fname);
			} while(Fsnext() == 0);					/* und das fr alle Module */
		}

		begin = Element.next;

		Fsetdta(old_dta);							/* alte DTA zurcksetzen */
		free(new_dta);
	}

	return(begin);
} /* build_up_filelist */


/* insert_entry fgt einen Eintrag (eine Datei) in die verkettete Liste ein. */
/* Argumente sind ein Zeiger auf den einzufgenden Eintrag sowie das aktuelle */
/* Ende der Liste. */
struct DIRENTRY *insert_entry(struct DIRENTRY *ende, char *string)
{
	struct DIRENTRY *new;				/* zeigt auf den neuen Speicherblock */


	/* Speicher fr Struktur und String anfordern */
	if((new = (struct DIRENTRY *)malloc(sizeof(struct DIRENTRY))) == 0 ||
	   (new->modname = (char *)malloc(strlen(string) + 1)) == 0)
	{
		free(new);
		return(NULL);
	}

	strcpy(new->modname, string);										/* String reinkopieren */
	new->next = NULL;			/* neue Ende-Markierung */

	ende->next = new;			/* Vorg„nger (also bisher letzter) zeigt auf neues Element */

	return(new);
} /* insert_entry */


/* L”st die verkettete Liste wieder auf. */
/* Oder muž ich von hinten kommen (d.h. doppelt verketten), */
/* um Speicherzersplitterung zu vermeiden? */
void destroy_filelist(struct DIRENTRY *actual)
{
	struct DIRENTRY *next;


	while(actual != NULL)
	{
		next = actual->next;	/* wegsichern, da nach Freigabe von */
		free(actual->modname);	/* actual ja nicht mehr darauf */
		free(actual);			/* zugegriffen werden darf */
		actual = next;
	}

	return;
} /* destroy_filelist */