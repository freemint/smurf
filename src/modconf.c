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

/* modconf.c		31.8. - 22.9.98, Olaf
	Dieses File enthÑlt die Routinen zum Schreiben und Auslesen der MODCONF.CNF,
	in der sich die Konfigurationen aller Editmodule befinden, und auûerdem
	zwei Funktionen zum Merken und ZurÅckholen der letzten Einstellungen der
	Module.
	Das ganze besteht aus einer Menge Freads und Fseeks und ist ein wenig knifflig,
	da ich nicht immer die komplette Datei einlesen will. Man kann nie wissen, wie lang
	die bei manchen Usern und bestimmten Modulen werden kann.
	Die beiden Routinen mconfSave() und mconfLoad() werden von den Modulen (bzw. von
	Smurf aus mpref_change() heraus) aufgerufen. Beide îffnen ggfs. ein 'Pseudopopup',
	in dem die gespeicherten Konfigurationen ausgewÑhlt werden kînnen.
	save_to_modconf und load_from_modconf speichern bzw. laden eine Modulkonfiguration
	in/aus der MODCONF.CNF. save_to legt die Datei, wenn nicht vorhanden, neu an.
	seek_modconf sucht in der Datei nach der zum jeweiligen Modul passenden	MCA (Module
	Configuration Area), die mit dem Magic MCAB anfÑngt und mit MCAE endet. 
	Dateiaufbau:
	
	<lÑnge in bytes>MCAB<modulname>\0<anzahl>
	MCNF<cnf-name>\0<lÑnge in bytes><cnf-block>
	MCNF<cnf-name>\0<...
	MCAE
	<lÑnge in bytes>MCAB<modulname2>\0...
	...
	
	<lÑnge in bytes> enthÑlt die LÑnge des gesamten MCAB inklusive Anfangs- und Endekennung,
	<anzahl> ist die Anzahl an MCNF-Blîcken fÅr jedes Modul in der Datei.
	<cnf-name> ist immer 33 Bytes lang, wenn ein kÅrzerer Name eingegeben wurde, wird mit	
	Nullbytes aufgefÅllt. <cnf-block> ist der beim Speichern vom jeweiligen Modul 
	Åbermittelte Konfigurationsblock der LÑnge <lÑnge in bytes>.

	Noch zu machen:
	 - lîschen von Konfigurationen (CTRL+Klick im Popup?)

	Die Magics MCAB/MCAE etc. sind noch ein wenig gefÑhrlich, da so keine Modulnamen und
	-konfigurationen diese Buchstabenkombinationen enthalten dÅrfen! Da mÅssen noch ein paar
	Kennbytes rein, die da garantiert nicht vorkommen kînnen...
	*/


#include <tos.h>
#include <multiaes.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "smurf.h"
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "globdefs.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "smurfine.h"

#include "smurfobs.h"
#include "ext_obs.h"

#define SEEKBUF_SIZE 2048


long Idxtab(unsigned char *s1, unsigned char *s2, long len);
void save_to_modconf(MOD_INFO *modinfo, void *confblock, long len, char *name, long type);
int open_modconf_popup(MOD_INFO *modinfo);
long seek_modconf(int filehandle, MOD_INFO *modinfo);
long seek_MCNF(char *cnfname, int filehandle);
void expandFile(int handle, long len);
int overwriteMCNF(MOD_INFO *modinfo, char *confblock, long newlen, char *name, int num, long type);
long seekInFile(int filehandle, char *SeekString);
int nametest(MOD_INFO *modinfo, char *name);

OBJECT *confsave_dialog;
OBJECT *modconf_popup;


/*
 * die Default-Konfigurationen fÅr die Editmodule
 */
void *edit_cnfblock[100];
int edit_cnflen[100];

extern char *export_cnfblock[50];
extern int export_cnflen[50];


/* mconfLoad --------------------------------------------
	ôffnet das Pseudopopup und lÑdt eine Konfiguration. Diese wird
	an das Modul zurÅckgegeben.
	------------------------------------------------------*/
void *mconfLoad(MOD_INFO *modinfo, int mod_id, char *name)
{
	char cnfname[33];

	int back;

	void *block = NULL;

	
	modconf_popup[NEW_CONF].ob_state |= DISABLED;
	back = open_modconf_popup(modinfo);				/* erstmal das Popup auf */
	modconf_popup[NEW_CONF].ob_state &= ~DISABLED;
	if(back <= 0)
		return(NULL);
	
	if(back == NEW_CONF)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_NOCONF].TextCast, NULL, NULL, NULL, 1);
	else
	{
		strcpy(cnfname, modconf_popup[back].TextCast);
		block = load_from_modconf(modinfo, cnfname, &back, 'SEMD');
	}

	return(block);
}


/* mconfSave --------------------------------------------
	Die Dienstfunktion, die vom Modul aufgerufen wird.
	ôffnet das Pseudopopup mit den Modulkonfigs und dann ggfs. den
	Dialog zur Eingabe eines neuen Namens, dann wird die Konfig
	in die MONDCONF.CNF gespeichert. ZurÅckgegeben wird ein Zeiger
	auf den vom User eingegebenen Namen (max. 32 Zeichen).
	------------------------------------------------------*/
void mconfSave(MOD_INFO *modinfo, int mod_id, void *confblock, long len, char *name)
{
	char cnfname[33];

	int back;
	int x, y, w, h;


	back = open_modconf_popup(modinfo);			/* Erstmal das Popup auf */
	if(back <= 0)
		return;
	
	/*
	 * Es soll eine neue Konfiguration gesichert werden?
	 * (-1 heiût, es gibt noch gar keine, also auf jeden Fall eine neue) 
	 */
	if(back == NEW_CONF)
	{
again:
		x = confsave_dialog->ob_x - 2;
		y = confsave_dialog->ob_y - 2;
		w = confsave_dialog->ob_width + 4;
		h = confsave_dialog->ob_height + 4;
	
		wind_update(BEG_UPDATE);
		wind_update(BEG_MCTRL);
		form_dial(FMD_START, x,y,w,h,x,y,w,h);
	
		/* damit das Feld leer ist und der Cursor vorne steht */
		strcpy(confsave_dialog[MODCONF_NAME].TextCast, "");

		do
		{
			objc_draw(confsave_dialog, 0, MAX_DEPTH, x, y, w, h);
			back = form_do(confsave_dialog, MODCONF_NAME);
			confsave_dialog[back].ob_state &= ~SELECTED;
		} while((back != MODCONF_SAVE || strlen(confsave_dialog[MODCONF_NAME].TextCast) == 0) &&
				back != MODCONF_CANCEL);

		form_dial(FMD_FINISH, x, y, w, h, x, y, w, h);
		wind_update(END_MCTRL);
		wind_update(END_UPDATE);


		if(back == MODCONF_SAVE)
		{
			strcpy(cnfname, confsave_dialog[MODCONF_NAME].TextCast);

			if(nametest(modinfo, cnfname))
			{
				back = Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_EXISTS].TextCast, "Nein", " Ja ", NULL, 1);
				if(back == 1)						/* nicht Åberschreiben */
					goto again;
				else								/* Åberschreiben */
					overwriteMCNF(modinfo, confblock, len, cnfname, 0, 'SEMD');
			}
			else
				save_to_modconf(modinfo, confblock, len, cnfname, 'SEMD');
		}
	}
	else
	{
		strcpy(cnfname, modconf_popup[back].TextCast);
		overwriteMCNF(modinfo, confblock, len, cnfname, back, 'SEMD');
	}

	return;	
}


/* open_modconf_popup ---------------------------------------------------
	lÑdt die zum Modul modinfo gehîrigen Konfigurationsnamen aus der MODCONF.CNF,
	îffnet das Pseudopopup mit den Modulkonfigurationen des Moduls mod_info,
	handled dessen Bedienung durch den User und gibt den darin angeklickten 
	Eintrag zurÅck.
	Scrolling im Popup muû noch gemacht werden!
	----------------------------------------------------------------------*/
int open_modconf_popup(MOD_INFO *modinfo)
{
	char cnfpath[257];
	char *omca, *mca;

	int x, y, w, h, back, t, mx, my, mb, key;
	int mconf_index = 0;
	int fhandle, num_confs;

	long *confnames;
	long fback, mca_len, areaheader_pos, magic, len;

	extern int mouse_xpos, mouse_ypos;

	extern GRECT screen;
	extern SYSTEM_INFO Sys_info;


	/*
	 * Position des Dialogs festlegen
	 */
	x = mouse_xpos - modconf_popup->ob_width / 2;
	y = mouse_ypos - modconf_popup->ob_height / 2;
	w = modconf_popup->ob_width;
	h = modconf_popup->ob_height;

	if(x < 2)
	 x = 2;
	else
		if(x > screen.g_w - w - 4)
			x = screen.g_w - w - 4;
	if(y < 2)
		y = 2;
	else
		if(y > screen.g_h - h - 4)
			x = screen.g_h - h - 4;

	modconf_popup->ob_x = x;
	modconf_popup->ob_y = y;

	/*
	 * Jetzt werden die Konfigurationsnamen aus der Datei geladen
	 * und in confnames[n] eingetragen.
	 */
	strcpy(cnfpath, Sys_info.home_path);
	strcat(cnfpath, "\\modconf.cnf");
	fback = Fopen(cnfpath, FO_RW);
	if(fback < 0)
		return(NEW_CONF);

	fhandle = (int)fback;
	areaheader_pos = seek_modconf(fhandle, modinfo);
	if(areaheader_pos == -1) 
	{
		Fclose(fhandle);
		return(NEW_CONF);										/* fÅr dieses Modul keine CNF vorhanden */
	}

	Fseek(-4, fhandle, 1);
	Fread(fhandle, 4, &mca_len);
	omca = mca = SMalloc(mca_len);
	Fread(fhandle, mca_len, mca);
	Fclose(fhandle);

	mca += 4 + strlen(modinfo->mod_name) + 1;
	num_confs = *(int *)mca;									/* Konfigurationsanzahl auslesen */
	mca += 2;

	confnames = malloc(num_confs * 4);
	memset(confnames, 0x0, num_confs * 4);

	for(t = 0; t < num_confs; t++)
	{
		magic = *(long *)mca;
		mca += 4;

		if(magic == 'MCNF')
		{
			(char *)confnames[t] = malloc(33);
			strcpy((char *)confnames[t], mca);					/* Konfignamen lesen */
			mca += 33;
			len = *(long *)mca;									/* und seine LÑnge */
			mca += 4 + len;
		}
	}

	SMfree(omca);

	/*
	 * dann die Konfigurationen in den Dialog eintragen...
	 */
	for(t = 0; t < 8; t++)
	{
		if(t > num_confs - 1)
		{
			modconf_popup[t + MCONF1].ob_state |= DISABLED;			/* Rest disablen */
			modconf_popup[t + MCONF1].TextCast[0] = 0;
		}
		else
		{
			modconf_popup[t + MCONF1].ob_state &= ~DISABLED;		/* enablen */
			modconf_popup[t + MCONF1].TextCast = (char *)confnames[t];
		}
	}

	/*
	 * ggfs. Scrollbuttons disablen bzw. enablen
	 */
	modconf_popup[MCONF_UP].ob_state |= DISABLED;
	if(num_confs < 8)
		modconf_popup[MCONF_DN].ob_state |= DISABLED;
	else
		modconf_popup[MCONF_DN].ob_state &= ~DISABLED;


	/*
	 * jetzt den Dialog îffnen und hoffen, daû er dem User gefÑllt...
	 */
	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);
	form_dial(FMD_START, x,y,w,h,x,y,w,h);
	objc_draw(modconf_popup, 0,MAX_DEPTH, x,y,w,h);

	do
	{
		evnt_button(1,1,1, &mx,&my,&mb,&key);
		back = objc_find(modconf_popup, 0, MAX_DEPTH, mx,my);

		if(back == MCONF_UP || back == MCONF_DN)
		{
			if(num_confs >= 8)
			{
				if(back == MCONF_UP && mconf_index > 0)
					mconf_index--;
				if(back == MCONF_DN && mconf_index < num_confs - 8)
					mconf_index++;
				for(t = 0; t < 8; t++)
				{
						modconf_popup[t + MCONF1].ob_state &= ~DISABLED;		/* Enablen */
						modconf_popup[t + MCONF1].TextCast = (char *)confnames[t + mconf_index];
						objc_draw(modconf_popup, MCONF1+t, MAX_DEPTH, x,y,w,h);
				}
			}
		}

		if(mconf_index <= 0)
			modconf_popup[MCONF_UP].ob_state |= DISABLED;
		else
			modconf_popup[MCONF_UP].ob_state &= ~DISABLED;
		if(mconf_index >= num_confs - 8)
			modconf_popup[MCONF_DN].ob_state |= DISABLED;
		else
			modconf_popup[MCONF_DN].ob_state &= ~DISABLED;

		objc_draw(modconf_popup, MCONF_UP, MAX_DEPTH, x,y,w,h);
		objc_draw(modconf_popup, MCONF_DN, MAX_DEPTH, x,y,w,h);
	} while(back == MCONF_UP || back == MCONF_DN ||
			modconf_popup[back].ob_state&DISABLED || !IsSelectable(modconf_popup[back]) &&
			back != -1);

	form_dial(FMD_FINISH, x,y,w,h,x,y,w,h);
	wind_update(END_MCTRL);
	wind_update(END_UPDATE);

	for(t = 0; t < num_confs; t++)
		free((char *)confnames[t]);
	free(confnames);

	if(back == -1)
		return(-1);

	return(back + mconf_index);
}


/* save_to_modconf ------------------------------------------------------------
	Speichert den Konfigurationsbock *confblock des Moduls modinfo/mod_id mit
	der LÑnge len unter dem Namen name in der MODCONF.CNF. 
	Wird von mconfSave() aufgerufen!
	-------------------------------------------------------------------------*/
void save_to_modconf(MOD_INFO *modinfo, void *confblock, long len, char *name, long type)
{
	char cnfpath[257];

	int filehandle, num_confs;

	long oback, cback, mca_len = 0, pos, areaname_len, areaheader_pos;

	extern SYSTEM_INFO Sys_info;

	
	strcpy(cnfpath, Sys_info.home_path);
	strcat(cnfpath, "\\modconf.cnf");

	/*
	 * MODCONF.CNF îffnen oder erzeugen
	 */
	oback = Fopen(cnfpath, FO_RW);
	if(oback < 0)
	{
		cback = Fcreate(cnfpath, 0);
		if(cback < 0)
		{
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_CREATEERR].TextCast, NULL, NULL, NULL, 1);
			return;
		}

		filehandle = (int)cback;
	}
	else
		filehandle = (int)oback;


	/*
	 * Modulbereich in der cnf suchen
	 */
	if(oback >= 0)
		pos = seek_modconf(filehandle, modinfo);
	else
		pos = -1;

	/*
	 * in vorhandenen Filebereich des Moduls (sofern gefunden)
	 * einen neuen Eintrag einfÅgen
	 */
	if(pos != -1)
	{
		Fseek(-4, filehandle, 1);
		Fread(filehandle, 4, &mca_len);

		/*
		 * zuerst das Word fÅr die Konfigurationsanzahl erhîhen
		 */
		areaheader_pos = Fseek(0, filehandle, 1);
		areaname_len = strlen(modinfo->mod_name) + 1;
		Fseek(4 + areaname_len, filehandle, 1);
		Fread(filehandle, 2, &num_confs);
		num_confs++;
		Fseek(-2, filehandle, 1);
		Fwrite(filehandle, 2, &num_confs);
		Fseek(areaheader_pos + mca_len - 4, filehandle, 0);	/* ans Ende des MCA stellen */
		
		/*
		 * dann Platz in der Datei machen und die neue Config schreiben
		 */
		expandFile(filehandle, 4 + 33 + 4 + len);
		Fwrite(filehandle, 4, "MCNF");						/* "Module Configuration" */
		Fwrite(filehandle, 33, name);						/* Konfigurationsname schreiben */
		Fwrite(filehandle, 4, &len);						/* LÑnge schreiben */
		Fwrite(filehandle, len, confblock);					/* Konfigurationsblock schreiben */
		mca_len = Fseek(0, filehandle, 1) + 4 - areaheader_pos;
		Fseek(areaheader_pos - 4, filehandle, 0);
		Fwrite(filehandle, 4, &mca_len);					/* MCA-LÑnge schreiben */
	}
	/*
	 * oder einen neuen Bereich fÅr das Modul an die Datei anhÑngen?
	 */
	else 
	{
		Fseek(0, filehandle, 2);

		pos = Fwrite(filehandle, 4, &mca_len);				/* MCA-LÑnge initial schreiben */

		areaheader_pos = Fseek(0, filehandle, 1);
		Fwrite(filehandle, 4, "MCAB");						/* "Module Conf. Area Begin" oder so. */
		areaname_len = strlen(modinfo->mod_name) + 1;
		Fwrite(filehandle, areaname_len, modinfo->mod_name);/* Modulname inkl. Nullbyte schreiben */
		num_confs = 1;
		Fwrite(filehandle, 2, &num_confs);					/* Konf-Anzahl schreiben */

		Fwrite(filehandle, 4, "MCNF");						/* "Module Configuration" */
		if(type == 'SEMD')
			Fwrite(filehandle, 33, name);					/* Konfigurationsname inkl. Nullbyte schreiben */
		Fwrite(filehandle, 4, &len);						/* LÑnge schreiben */
		Fwrite(filehandle, len, confblock);					/* Konfigurationsblock schreiben */

		Fwrite(filehandle, 4, "MCAE");						/* "Module Conf. Area End" oder so. */

		mca_len = Fseek(0, filehandle, 1) - areaheader_pos;
		Fseek(areaheader_pos - 4, filehandle, 0);
		Fwrite(filehandle, 4, &mca_len);					/* MCA-LÑnge schreiben */
	}

	Fclose(filehandle);
}


/* load_from_modconf -------------------------------------------------------------
	LÑdt die Konfiguration Nummer <num> fÅr das Modul modinfo aus
	der MODCONF.CNF. Der Name der Konfig wird in name kopiert.
	Ein Zeiger auf den geladenen Konfigurationsblock (SMalloc) wird zurÅckgegeben
	und in num kommt die LÑnge zurÅck.
	-------------------------------------------------------------------------------*/
void *load_from_modconf(MOD_INFO *modinfo, char *name, int *num, long type)
{
	char SeekString[37] = "MCNF";
	char cnfpath[257];
	char *mca, *block;

	int fhandle, num_confs;

	long back, mca_len, len, areaheader_pos;


	strcpy(cnfpath, Sys_info.home_path);
	strcat(cnfpath, "\\modconf.cnf");

	/*
	 * MODCONF.CNF îffnen
	 */
	back = Fopen(cnfpath, FO_RW);
	if(back < 0)
	{
		if(type == 'SEMD')
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_OPENERR].TextCast, NULL, NULL, NULL, 1);
		return(NULL);											/* fÅr dieses Modul keine CNF vorhanden */
	}
		
	fhandle = (int)back;
	areaheader_pos = seek_modconf(fhandle, modinfo);
	if(areaheader_pos == -1) 
	{
		Fclose(fhandle);
		if(type == 'SEMD')
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_READERR].TextCast, NULL, NULL, NULL, 1);
		return(NULL);											/* fÅr dieses Modul keine CNF vorhanden */
	}

	Fseek(-4, fhandle, 1);
	Fread(fhandle, 4, &mca_len);
	mca = SMalloc(mca_len);
	Fread(fhandle, mca_len, mca);
	Fclose(fhandle);

	num_confs = *(int *)(mca + 4 + strlen(modinfo->mod_name) + 1);		/* Konfigurationsanzahl auslesen */

	/*
	 * ist die Nummer Åberhaupt mîglich?
	 */
	if(*num > num_confs)
	{
		SMfree(mca);
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_NOTFOUND].TextCast, NULL, NULL, NULL, 1);
		return(NULL);
	}

	strcat(SeekString, name);
	back = Idxtab(mca, SeekString, mca_len);

	if(back == -1)
		return(NULL);

	strcpy(name, mca + back + 4);								/* Konfignamen lesen */
	if(type == 'SEMD')
		len = *(long *)(mca + back + 4 + 33);					/* und seine LÑnge */
	else
		len = *(long *)(mca + back + 4);						/* und seine LÑnge */
	*num = len;
	
	block = SMalloc(len);
	if(type == 'SEMD')
		memcpy(block, mca + back + 4 + 33 + 4, len);			/* und schlieûlich die Config */
	else
		memcpy(block, mca + back + 4 + 4, len);					/* und schlieûlich die Config */

	SMfree(mca);

	return(block);
}


/* overwriteMCNF ----------------------------------------------------
	öberschreibt eine Modulkonfiguration mit dem Index num in der Datei
	mit dem neuen Block confblock der LÑnge newlen.
	-------------------------------------------------------------------*/
int overwriteMCNF(MOD_INFO *modinfo, char *confblock, long newlen, char *name, int num, long type)
{
	char SeekString[37] = "MCNF";
	char cnfpath[257];
	char *mca;

	int filehandle, num_confs;

	long back, mca_len, len, areaheader_pos;


	strcpy(cnfpath, Sys_info.home_path);
	strcat(cnfpath, "\\modconf.cnf");

	/*
	 * erstmal die Datei auf
	 */
	back = Fopen(cnfpath, FO_RW);
	if(back < 0)
	{
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_OPENERR].TextCast, NULL, NULL, NULL, 1);
		return(-1);
	}

	filehandle = (int)back;
	areaheader_pos = seek_modconf(filehandle, modinfo);
	if(areaheader_pos == -1) 
	{
		Fclose(filehandle);
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_READERR].TextCast, NULL, NULL, NULL, 1);
		return(-1);
	}

	Fseek(-4, filehandle, 1);
	Fread(filehandle, 4, &mca_len);
	mca = SMalloc(mca_len);
	Fread(filehandle, mca_len, mca);

	num_confs = *(int *)(mca + 4 + strlen(modinfo->mod_name) + 1);		/* Konfigurationsanzahl auslesen */

	/*
	 * ist die Nummer Åberhaupt mîglich?
	 */
	if(num > num_confs)
	{
		SMfree(mca);
		Fclose(filehandle);
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MCONF_NOTFOUND].TextCast, NULL, NULL, NULL, 1);
		return(-1);
	}

	strcat(SeekString, name);
	back = Idxtab(mca, SeekString, mca_len);
	SMfree(mca);

	if(back == -1)												/* eigentlich unmîglich */
	{
		Fclose(filehandle);
		return(-1);
	}

	Fseek(areaheader_pos + back + 4, filehandle, 0);			/* bis zum MCNF und noch drÅber */

	if(type == 'SEMD')
		Fread(filehandle, 33, name);							/* Konfignamen lesen */
	Fread(filehandle, 4, &len);									/* und seine LÑnge */

	/*
	 * und jetzt die neue Konfiguration schreiben. Wenn newlen grîûer ist
	 * als die in der Datei fÅr die zu Åberschreibende Konfiguration angegebene
	 * LÑnge, wird Platz in der Datei gemacht.
	 */
	Fseek(-4, filehandle, 1);
	if(newlen > len)
		expandFile(filehandle, newlen - len);
	Fwrite(filehandle, 4, &newlen);								/* LÑnge schreiben */
	Fwrite(filehandle, newlen, confblock);						/* Konfigurationsblock schreiben */

	Fclose(filehandle);

	return(0);
}


/* seek_modconf -----------------------------------------------
	Sucht eine MCA (Module Config Area, gekennzeichnet durch 'MCAB')
	zum Modul modinfo in der Datei filehandle (MODCONF.CNF) und positioniert 
	den Dateipointer auf das 'MCAB'.
	ZurÅckgegeben wird die neue Position relativ zur alten vor dem Aufruf.
	-------------------------------------------------------------*/
long seek_modconf(int filehandle, MOD_INFO *modinfo)
{
	char SeekString[128] = "MCAB";

	long filepos;

	
	/*
	 * Suchstring zusammenbasteln 
	 */
	strcat(SeekString, modinfo->mod_name);

	/* lossuchen, RÅckgabe ist -1 wenn nicht gefunden */	
	filepos = seekInFile(filehandle, SeekString);
		Fseek(filepos, filehandle, 1);

	return(filepos);
}


/* Schnelle Suchroutine nach Boyer-Moore */
/* implementiert 24.5.95 - 10.6.95 von Christian Eyrich */
/* sucht m in n */
long Idxtab(unsigned char *n, unsigned char *m, long len)
{
	char skiptab[256], t, lenofm, lenofm2;

	int j;

	long i, lenofn = len;


	lenofm = strlen(m);
	lenofm2 = lenofm - 1;

	for(i = 0; i < 256; i++)			/* initialisieren der Tabelle */
		skiptab[i] = lenofm;			/* mit der LÑnge des Suchstrings */

	for(i = 0; i < lenofm; i++)
		skiptab[m[i]] = lenofm - i - 1;

	/* die eigentliche Suchroutine */
	for(i = j = lenofm2; j > 0; i--, j--)
	{
		while(n[i] != m[j] && i < lenofn)
		{
			t = skiptab[n[i]];
			i += (lenofm - j > t) ? lenofm - j : t; 
			j = lenofm2;
		}

		if(i > lenofn)					/* wurde nichts gefunden */
			return(-1);                	/* wird -1 zurÅckgegeben */
	}

	return(i);							/* wurde die Routine fÅndig, */
}										/* wird die Fundstelle zurÅckgegeben */


/* expandFile -----------------------------------------
	Macht in der Datei handle an der aktuellen Fileposition len bytes 
	Platz. Der Dateipointer wird nicht verÑndert. Dateipuffer ist 2048
	Bytes, sollte vielleicht noch grîûer gewÑhlt werden?
	----------------------------------------------------*/
void expandFile(int handle, long len)
{
	char *buffer;

	long bytes, oldpos;


	if(len == 0)
		return;

	oldpos = Fseek(0, handle, 1);

	bytes = Fseek(0, handle, 2) - oldpos;	/* wieviel Speicher wird gebraucht? */
	buffer = SMalloc(bytes + len);			/* soviel + len Bytes anfordern */
	Fseek(oldpos, handle, 0);				/* und wieder zurÅck */

	Fread(handle, bytes, buffer);			/* jetzt Rest puffern */
	Fwrite(handle, len, buffer);			/* len Bytes Platz machen */

	Fseek(-bytes, handle, 1);				/* Rest Bytes zurÅck */
	Fwrite(handle, bytes, buffer);			/* und Puffer wegschreiben */

	Fseek(oldpos, handle, 0);				/* und wieder zurÅck */

	SMfree(buffer);

	return;
}


/* memorize_emodConfig ----------------------------------------------------------
	"merkt" sich die vom Modul modbase geschickten Default-Konfigurationsblock
	cnfblock (wird bei M_EXIT mitgeschickt) mit der LÑnge length in den
	Feldern edit_cnfblock[] und edit_cnflen[].
	-------------------------------------------------------------------------*/
void memorize_emodConfig(BASPAG *modbase, GARGAMEL *smurf_struct)
{
	char cmp_modname[30];
	char *textseg, *cnfblock;

	int index;

	MOD_INFO *modinfo;


	/*
	 * Erstmal muû der passende Index gefunden werden.
	 * Dazu nehmen wir uns den Namen des Moduls und suchen
	 * ihn unter den eingetragenen Editmodulen
	 */
	textseg = (char *)(modbase->p_tbase);
	modinfo = (MOD_INFO *) *((MOD_INFO **)(textseg + MOD_INFO_OFFSET));

	strcpy(cmp_modname, modinfo->mod_name);
	if(strlen(cmp_modname) < 28)
		strncat(cmp_modname, "                           ", 28 - strlen(cmp_modname));

	for(index = 0; index < Dialog.emodList.anzahl; index++)
	{
		if(strcmp(Dialog.emodList.modNames[index], cmp_modname) == 0)
			break;
	}

	if(edit_cnfblock[index] != NULL)
		free(edit_cnfblock[index]);


	cnfblock = (char *)*(long *)&smurf_struct->event_par[0];
	edit_cnflen[index] = smurf_struct->event_par[2];

	edit_cnfblock[index] = malloc(edit_cnflen[index]);
	memcpy(edit_cnfblock[index], cnfblock, edit_cnflen[index]);

	SMfree(cnfblock);

	return;
}


/* memorize_expmodConfig ----------------------------------------------------------
	"merkt" sich die vom Modul modbase geschickten Default-Konfigurationsblock
	cnfblock (wird bei M_EXIT mitgeschickt) mit der LÑnge length in den
	Feldern export_cnfblock[] und export_cnflen[].
	-------------------------------------------------------------------------*/
void memorize_expmodConfig(BASPAG *modbase, GARGAMEL *smurf_struct, char save)
{
	char cmp_modname[30];
	char *textseg, *cnfblock;

	int index, length;

	MOD_INFO *modinfo;


	/*
	 * Erstmal muû der passende Index gefunden werden.
	 * Dazu nehmen wir uns den Namen des Moduls und suchen
	 * ihn unter den eingetragenen Exportmodulen
	 */
	textseg = (char *)(modbase->p_tbase);
	modinfo = (MOD_INFO *) *((MOD_INFO **)(textseg + MOD_INFO_OFFSET));

	strcpy(cmp_modname, modinfo->mod_name);
	if(strlen(cmp_modname) < 28)
		strncat(cmp_modname, "                           ", 28 - strlen(cmp_modname));

	for(index = 0; index < Dialog.expmodList.anzahl; index++)
	{
		if(strcmp(Dialog.expmodList.modNames[index], cmp_modname) == 0)
			break;
	}

	if(export_cnfblock[index] != NULL)
		free(export_cnfblock[index]);

	cnfblock = (char *)*(long *)&smurf_struct->event_par[0];
	length = smurf_struct->event_par[2];
	if(!save)
	{
		export_cnflen[index] = length;
		export_cnfblock[index] = malloc(export_cnflen[index]);
		memcpy(export_cnfblock[index], cnfblock, export_cnflen[index]);
	}
	else
		if(nametest(modinfo, ""))
			overwriteMCNF(modinfo, cnfblock, length, "", 0, 'SXMD');
		else
			save_to_modconf(modinfo, cnfblock, length, "", 'SXMD');

	return;
}


/* transmitConfig ---------------------------------------------
	schickt einen mit memorize_emodConfig() gemerkten Konfigurationsblock
	(wenn Åberhaupt einer gemerkt wurde) ans Modul modbase/smurf_struct
	zurÅck. Das Modul muû diesen dann in seine internen Strukturen kopieren
	und entsprechend reagieren (Dialogupdate). Antwort vom Modul soll entweder
	M_WAITING oder M_MODPIC sein (wobei letzteres momentan noch nicht funkt!).
	---------------------------------------------------------------------*/
void transmitConfig(BASPAG *modbase, GARGAMEL *smurf_struct)
{
	char *textseg;
	char cmp_modname[30];

	int index;

	MOD_INFO *modinfo;


	/*
	 * Erstmal muû der passende Index gefunden werden.
	 * Dazu nehmen wir uns den Namen des Moduls und suchen
	 * ihn unter den eingetragenen Editmodulen
	 */
	textseg = (char*)(modbase->p_tbase);
	modinfo = (MOD_INFO *)*((MOD_INFO **)(textseg + MOD_INFO_OFFSET));
	
	strcpy(cmp_modname, modinfo->mod_name);
	if(strlen(cmp_modname) < 28)
		strncat(cmp_modname, "                           ", 28 - strlen(cmp_modname));

	for(index = 0; index < Dialog.emodList.anzahl; index++)
	{
		if(strcmp(Dialog.emodList.modNames[index], cmp_modname) == 0)
			break;
	}

	if(edit_cnfblock[index])
	{
		smurf_struct->event_par[0] = (int)((unsigned long)edit_cnfblock[index]>>16);
		smurf_struct->event_par[1] = (int)((long)edit_cnfblock[index]&0xFFFF);
		smurf_struct->event_par[2] = edit_cnflen[index];

		module.comm.startEdit("", modbase, CONFIG_TRANSMIT, smurf_struct->module_number, smurf_struct);
		f_handle_modmessage(smurf_struct);
	}

	return;
}



/* seekInFile --------------------------------------------------------------
	Sucht den String SeekString in der Datei filehandle ab der aktuellen
	Position und gibt relativ zur Dateiposition beim Aufruf den Abstand des
	gefundenen Strings in der Datei zurÅck, bzw. -1, wenn nix gefunden wurde.
	-------------------------------------------------------------------------*/
long seekInFile(int filehandle, char *SeekString)
{
	char *seekbuffer;

	long pos, filepos = 0, read_bytes;
	long oldpos, seekLen;

	
	seekLen = strlen(SeekString);
	oldpos = Fseek(0, filehandle, 1);

	/*
	 * jetzt wird die Datei 2K-blockweise nach SeekString durchsucht
	 */
	seekbuffer = malloc(SEEKBUF_SIZE + 8);
	memset(seekbuffer, 0x0, SEEKBUF_SIZE + 8);
	filepos = 0;
	do
	{
		read_bytes = Fread(filehandle, SEEKBUF_SIZE, seekbuffer);
		
		if((pos = Idxtab(seekbuffer, SeekString, SEEKBUF_SIZE)) != -1)	/* versuchen, SeekString zu finden */
			break;
		
		Fseek(-seekLen, filehandle, 1);									/* SeekLen zurÅck */
		filepos += SEEKBUF_SIZE - seekLen;
	} while(read_bytes == SEEKBUF_SIZE);

	free(seekbuffer);

	Fseek(oldpos, filehandle, 0);

	if(pos != -1)
		filepos += pos;
	else
		filepos = -1;

	return(filepos);
}


/* Testet ob der Konfigname schon vergeben fÅr dieses Modul */
/* RÅckgabe 0: alles ist klar, 1: schon vergeben */
int nametest(MOD_INFO *modinfo, char *name)
{
	char cnfpath[257];
	char SeekString[37] = "MCNF";
	char *mca;

	int filehandle;

	long back, mca_len = 0;

	extern SYSTEM_INFO Sys_info;


	strcpy(cnfpath, Sys_info.home_path);
	strcat(cnfpath, "\\modconf.cnf");

	/*
	 * MODCONF.CNF îffnen
	 */
	back = Fopen(cnfpath, FO_READ);
	if(back < 0)
		return(0);
	else
		filehandle = (int)back;

	/*
	 * Modulbereich in der cnf suchen
	 */
	if(seek_modconf(filehandle, modinfo) == -1)
	{
		Fclose(filehandle);
		return(0);
	}
	else
	{
		Fseek(-4, filehandle, 1);
		Fread(filehandle, 4, &mca_len);
		mca = SMalloc(mca_len);
		Fread(filehandle, mca_len, mca);
		Fclose(filehandle);

		strcat(SeekString, name);
		back = Idxtab(mca, SeekString, mca_len);

		SMfree(mca);

		if(back == -1)
			return(0);
		else
			return(1);
	}
} /* nametest */