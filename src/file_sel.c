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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include <tos.h>
#include <ext.h>
#include <vdi.h>
/*#include <multiaes.h>*/
#include <aes.h>
#include "sym_gem.h"
#include "..\modules\import.h"
#include "select.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "globdefs.h"
#include "smurf.h"
#include "smurfine.h"

#include "smurfobs.h"
#include "ext_obs.h"


void set_multfiles(SLCT_STR *slct_str, char **selliste);
void get_multfiles(SLCT_STR *slct_str, char *pathname, char **selliste);
void f_build_filelist(char **param, int mode);
void f_free_filelist(void);

#define LOAD	1
#define SAVE	2

extern SYSTEM_INFO Sys_info;			/* Systemkonfiguration */

extern long f_len;

char *namelist[257];

extern int appl_id;

extern char loadpath[257];
extern char savepath[257];
extern char commpath[257];

extern SYSTEM_INFO Sys_info;
extern char DraufschmeissBild;

/* Ist nun die Hauptladefunktion und wird nicht mehr */
/* von f_loadpic() aufgerufen, sondern andersherum. */
void file_load(char *ltext, char **dateien, int mode)
{
	char *buf, *fullname, *ext;

	int t, fh;

	long dummy;

/* Drag&Drop, ARGV oder AV-BildÅbergabe ----------------------------*/
	if(DraufschmeissBild)
		f_build_filelist(dateien, mode);
	else
		if(f_fsbox(loadpath, ltext, LOAD) == FALSE)
			return;

	t = 0;
	while(t < 256 && (fullname = namelist[t]) != NULL)
	{
/*		printf("fullname: %s\n", fullname);
		getch(); */

		/* Sonderbehandlung fÅr Photo-CDs */
		ext = strrchr(fullname, '.');
		if(ext != NULL && strnicmp(ext, ".PCD", 4) == 0)
			buf = f_do_pcd(fullname);
		else
		{
			if((dummy = Fopen(fullname, FO_READ)) >= 0)		/* Datei îffnen */
			{
				fh = (int)dummy;

				f_len = Fseek(0, fh, 2);					/* FilelÑnge ermitteln */
				Fseek(0, fh, 0);							/* und Dateizeiger wieder auf Anfang */

				if((buf = SMalloc(f_len)) == NULL)
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_MEM].TextCast, NULL, NULL, NULL, 1);
				else
				{
					dummy = Fread(fh, f_len, buf);

					if(dummy < f_len)
					{
						Dialog.winAlert.openAlert(Dialog.winAlert.alerts[PICLOAD_READERR].TextCast, NULL, NULL, NULL, 1);
						SMfree(buf);
						buf = NULL;
					}
				}

				Fclose(fh);
			}
			else 										   /* Fehler beim ôffnen */
			{
				Dialog.winAlert.openAlert(Dialog.winAlert.alerts[FILEOPEN_ERR].TextCast, NULL, NULL, NULL, 1);
				buf = NULL;
			}
		}

		if(buf)												/* konnte File geladen werden, Bild handlen */
			if(f_loadpic(buf, fullname) == 0)
			{
				if(!DraufschmeissBild || DraufschmeissBild == START)
					strcpy(loadpath, fullname);
			}
			else
				break;

		t++;
	}

	DraufschmeissBild = 0;					/* Ja, jetzt erst */

	f_free_filelist();

	return;
} /* file_load */


int file_save(char *stext, char *buf, long length)
{
	char desk_name[9];

	int fh;
	int out1, out2, out3, out4;
	int mbuf[8], desk_id, drive;
	int attrib;
	long dummy, check;


start:
	if(f_fsbox(savepath, stext, SAVE) == FALSE)
		return(FALSE);

	/* mit einem Trick testen, ob die Datei da ist */
	/* Ergebnis < 0 wenn die Datei noch nicht vorhanden ist */
	if(Fattrib(savepath, 0, 0) >= 0)
		if(Dialog.winAlert.openAlert("Es existiert bereits eine Datei dieses Namens! Soll Sie ersetzt werden?", "Nein", " Ja ", NULL, 2) == 1)
			goto start;

	if((dummy = Fcreate(savepath, 0)) >= 0)
	{
		fh = (int)dummy;

		check = Fwrite(fh, length, buf);
		Fclose(fh);

		if(check != length) 
		{
			if(check < 0)
				Dialog.winAlert.openAlert("Es ist ein fataler Schreibfehler aufgetreten! Laufwerk voll, oder schlimmeres.", NULL, NULL, NULL, 1);
			else
				Dialog.winAlert.openAlert("Die Datei konnte nicht vollstÑndig geschrieben werden. Wahrscheinlich ist das Laufwerk voll.", NULL, NULL, NULL, 1);
		}

		/*------- "Laufwerksinhalt verÑndert" ans Desktop -------*/
		/* Erst mal appl_getinfo() fragen, weil das TOS sonst "falscher */
		/* AES-Funktionsaufruf" bringt - Scheiûe, kann ihm doch egal sein. */
		if(Sys_info.OSFeatures&GETINFO && appl_getinfo(4, &out1, &out2, &out3, &out4) && out3 == 1)
			if(appl_search(2, desk_name, &attrib, &desk_id) == 1)
			{
				mbuf[0] = SH_WDRAW;
				mbuf[1] = appl_id;
				mbuf[2] = 0;
				drive = *savepath;
				if(drive >= 'a')
					drive -= 'a';
				else 
					if(drive >= 'A')
						drive -= 'A';
					else
						drive -= '0';
				mbuf[3] = drive;
				appl_write(desk_id, 16, mbuf);
			}

	}
	else
	{
		Dialog.winAlert.openAlert("Es ist ein Fehler beim Anlegen der Datei aufgetreten!", NULL, NULL, NULL, 1);
		return(FALSE);
	}

	return(TRUE);
} /* file_save */


/* Datei mittels Fileselector îffnen. */
int f_fsbox(char *Path, char *fbtext, char selectart)
{
	char pathname[257], filename[65], *part, *sellist[30];

	int back = 0, t;

	SLCT_STR *slct_str = NULL;


	strncpy(pathname, Path, t = strsrchr(Path, '\\') + 1);	/* Pfadteil kopieren */
	pathname[t] = '\0';										/* und */
	strcat(pathname, "*.*");								/* Maske auf alles setzen. */
	part = strrchr(Path, '\\') + 1;							/* Zeiger auf Fileteil holen */
	if(strcmp(part, "*.*") != 0)							/* und wenn nicht gleich "*.*", */
		strcpy(filename, part);								/* Filenamen extrahieren. */
	else													/* Ansonsten */
		strcpy(filename, "");								/* leeren Filenamen eintragen */

/*	printf("vor Fsel - pathname: %s, filename: %s\n", pathname, filename);
	getch(); */

	get_cookie('FSEL', &(unsigned long)slct_str);

	if(selectart == LOAD)
		set_multfiles(slct_str, sellist);

	f_set_syspal();

	/* version enthÑlt die Version der Schnittstelle */
	/* 1.0 ist Original Selectric */
	/* 1.02 ist Boxkite ab nonmodal */
	/* 2.0 ist Freedom mit nonmodal */
	/* Bei NonmodalitÑt keine wind_update()-Klammerung! */
/*	if(!slct_str || slct_str->id != 'SLCT' || (slct_str->version <= 0x0110 && slct_str->version!=0x0102))*/
		wind_update(BEG_UPDATE);

	if(Sys_info.AES_version >= 0x0140 || (slct_str && slct_str->id == 'SLCT'))
		back = fsel_boxinput(pathname, filename, &t, fbtext, message_handler);
	else
		back = fsel_input(pathname, filename, &t);

/*	if(!slct_str || slct_str->id != 'SLCT' || (slct_str->version <= 0x0110 && slct_str->version!=0x0102))*/
		wind_update(END_UPDATE);

	if(back == FALSE)
	{
		Dialog.winAlert.openAlert("Fehler beim Aufruf der Dateiauswahl!", NULL, NULL, NULL, 1);
		return(FALSE);
	}

/*	printf("nach Fsel - pathname: %s, filename: %s\n", pathname, filename);
	getch(); */

	if(t == 1 && (strlen(filename)>0 || selectart==PATH))				/* wurde der Selektor erfolgreich verlassen */
	{
		/* Dieser Mechanismus beruht darauf, daû bei allen Selektoren mit */
		/* selectrickompatibler Mehrfachselektion auch bei nur einem selektierten */
		/* File dieser Name in der Mehrfachliste steht! */
		/* Sollte dies nicht der Fall sein, hammer verschissen. */
		if(selectart == LOAD && slct_str && slct_str->config.onoff == 1)	/* Selectric da und an? */
			get_multfiles(slct_str, pathname, sellist);
		else
		{
			strncpy(Path, pathname, t = strsrchr(pathname, '\\') + 1);
			Path[t] = '\0';

			if(selectart!=PATH)
			{
				strcat(Path, filename);
				f_build_filelist(&Path, FSBOX2);
			}
		}
		

		return(TRUE);
	}
	else
		return(FALSE);
} /* f_fsbox */


/* Anlegen und initialisieren der Filenamenliste, welche durch den */
/* Fileselektor bei einer Mehrfachselektion dann gefÅllt wird. */
void set_multfiles(SLCT_STR *slct_str, char **sellist)
{
	int i;

	if(!slct_str || slct_str->config.onoff == 0)	/* Selectric nicht da oder aus? */
		return;

	/* Mehrfachselektion ein, Array von Zeigern */
	slct_str->comm = 0x01;

	i = 0;
	do
	{
		if((sellist[i] = (char *)calloc(1, 65)) == NULL)
		{
			i--;				/* letzter war der Letzte */
			break;
		}
	} while(++i < 30);

	/* konnte zumindest ein Zeiger angefordert werden? */
	if(i >= 0)
	{
		/* maximale Anzahl Bilder */
		slct_str->out_count = i;
		slct_str->out_ptr = sellist;
	}

	return;
} /* set_multfiles */


/* Alle Files der Liste werden direkt Åber die f_loadpic() geladen */
/* und der Laderoutine dann ein Abbruch vorgegaukelt. */
void get_multfiles(SLCT_STR *slct_str, char *pathname, char **sellist)
{
	char *part;

	int i, sel;


	if(!slct_str)
		return;

	sel = slct_str->out_count;

	/* initial holen, Position bleibt in der Schleife gleich */
	strcpy(commpath, pathname);
	part = strrchr(commpath, '\\') + 1;

	/* Liste in die Standardliste Åbertragen */
	i = 0;
	while(i < 256 && i < sel)
	{
		strcpy(commpath, pathname);
		strcpy(part, sellist[i]);

		if((namelist[i] = (char *)calloc(1, strlen(commpath) + 1)) != NULL)
			strcpy(namelist[i++], commpath);
		else
			break;
	}

	namelist[i] = NULL;			/* Ende markieren */

	/* angeforderte Zeiger wieder freigeben */
	i = 0;
	while(i < 30 && sellist[i])
		free(sellist[i++]);

	/* Mehrfachselektion aus */
	slct_str->comm = 0x0;

	return;
} /* get_multfiles */


/* Packt eine Fileliste des jeweiligen Typs in eine Liste des */
/* Standardformates fÅr file_load() und gibt einen Zeiger darauf zurÅck */
/* FSBOX = Liste von Selectric, Array von Zeigern - wird eigentlich in get_multfiles() gemacht */
/* FSBOX2 = Ein einzelner Filename in einem String */
/* ARGS = Liste von D&D-Type ARGS, String mit gequoteten Namen */
/* START = Liste vom ARGV-Verfahren, Array von Zeigern */
/* VA = Liste von VA_START, String mit gequoteten Namen */
void f_build_filelist(char **param, int mode)
{
	char *arg_name;

	int i;


	switch(mode)
	{
		case START:
			i = 0;
			while(i < 256 && (arg_name = (param[i + 1])) != NULL)
			{	
				if((namelist[i] = (char *)calloc(1, strlen(arg_name) + 1)) != NULL)
					strcpy(namelist[i++], arg_name);
				else
					break;
			}			

			namelist[i] = NULL;		/* terminieren */
			
			break;
		case VA:
		case ARGS:
			i = 0;
			while(i < 256 && (arg_name = strargvtok(*param)) != NULL)
			{	
				if((namelist[i] = (char *)calloc(1, strlen(arg_name) + 1)) != NULL)
					strcpy(namelist[i++], arg_name);
				else
					break;
			}

			namelist[i] = NULL;		/* terminieren */
			break;
		case FSBOX2:
			namelist[0] = (char *)calloc(1, strlen(*param) + 1);
			strcpy(namelist[0], *param);
			namelist[1] = NULL;		/* terminieren */
			break;
	}

	return;
} /* f_build_filelist() */


void f_free_filelist(void)
{
	int i;


	/* angeforderte Zeiger wieder freigeben */
	i = 0;
	while(i < 256 && namelist[i])
		free(namelist[i++]);

	return;
} /* f_free_filelist() */