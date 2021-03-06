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

/*---------------------------------------------------------	*/
/* -Save & Load Configuration 								*/
/* -Set Environment Config									*/
/*	f�r SMURF Bildkonverter, 								*/
/*	Copyright Therapy Seriouz Software						*/
/*---------------------------------------------------------	*/

#include <tos.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ext.h>
#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "smurf.h"
#include "globdefs.h"
#include "smurfine.h"

#include "smurfobs.h"
#include "ext_obs.h"
#include "debug.h"

#define PLOADINFO	0x500c

struct ploadinfo
{
	int fnamelen;
	char *cmdlin, *fname;
};

#define	DEBUGGER	1

void check_and_create(char *chpath);

unsigned int InqMagX(void);
void inquire_clipboard(void);
int getploadinfo(char *cmdlin, char *fname);

extern MFORM *dummy_ptr;				/* Dummymouse f�r Maus-Form */

extern SYSTEM_INFO Sys_info;
extern DISPLAY_MODES Display_Opt;
extern SERVICE_FUNCTIONS global_services;

extern char *stpath;

extern WINDOW wind_s[25];

int objc_sysvar(int ob_smode, int ob_swhich, int ob_sival1,
				int ob_sival2, int *ob_soval1, int *ob_soval2);


int load_config(void)
{
	char path[128];
	char *buf, help[256];

	DEBUG_MSG (( "Load smurf.cnf...\n" ));

	strcpy(path, Sys_info.home_path);
	strncat(path, "\\smurf.cnf", 11);
	
	buf = fload(path, 0);
	
	if(buf != NULL)
	{
		if(*(unsigned int *)buf != CNFVERSION)
		{
			f_alert("Falsche Version der Konfigurationsdatei gefunden! Aus Sicherheitsgr�nden wird diese nicht eingeladen.", NULL, NULL, NULL, 1);
			SMfree(buf);
			return(-1);
		}

		if(*(unsigned long  *)(buf + 2 + sizeof(SYSTEM_INFO) + sizeof(DISPLAY_MODES)) != 'CNF!')
		{
			f_alert("Keine Smurf-Konfigurationsdatei oder Datei defekt! Datei wird nicht geladen.", NULL, NULL, NULL, 1);
			SMfree(buf);
			DEBUG_MSG (( "Load smurf.cnf...Ende -1, 1\n" ));
			return(-1);
		}

		/* Zeiger retten, da er �berschrieben wird */
		strcpy(help, Sys_info.home_path);	

		memcpy(&Sys_info, buf + 2, sizeof(SYSTEM_INFO));
		memcpy(&Display_Opt, buf + 2 + sizeof(SYSTEM_INFO), sizeof(DISPLAY_MODES));
		strcpy(Sys_info.standard_path, stpath);	/* �berschriebenen String wieder restaurieren */

		/* Zeiger restaurieren */
		strcpy(Sys_info.home_path, help);	

		SMfree(buf);

		DEBUG_MSG (( "Load smurf.cnf...Ende\n" ));

		return(0);
	}
	
	SMfree(buf);
	DEBUG_MSG (( "Load smurf.cnf...Ende -1, 2\n" ));
	return(-1);
}


void save_config(void)
{
	char path[128];

	int filehandle, ver;

	long fcback, len;

	DEBUG_MSG (( "Save smurf.cnf...\n" ));

	Dialog.busy.reset(128, "speichere SMURF.CNF");

	graf_mouse(BUSYBEE, dummy_ptr);
	
	strcpy(path, Sys_info.home_path);
	strncat(path, "\\smurf.cnf", 11);

	if((fcback = Fcreate(path, 0)) >= 0)
	{
		filehandle=(int)fcback;
		ver = CNFVERSION;
		if(Fwrite(filehandle, 2, &ver) != 2)
			Dialog.winAlert.openAlert("Fehler beim Schreiben der Konfigurationsdatei! M�glicherweise ist das Laufwerk voll.", NULL, NULL, NULL, 1);
		len = sizeof(SYSTEM_INFO);
		if(Fwrite(filehandle, len, &Sys_info) != len)
			Dialog.winAlert.openAlert("Fehler beim Schreiben der Konfigurationsdatei! M�glicherweise ist das Laufwerk voll.", NULL, NULL, NULL, 1);
		len = sizeof(DISPLAY_MODES);
		if(Fwrite(filehandle, len, &Display_Opt) != len)
			Dialog.winAlert.openAlert("Fehler beim Schreiben der Konfigurationsdatei! M�glicherweise ist das Laufwerk voll.", NULL, NULL, NULL, 1);
		Fwrite(filehandle, 4, "CNF!");
		Fclose(filehandle);
	}
	else
	{
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[CNF_OPENERR].TextCast, NULL, NULL, NULL, 1);
		Dialog.busy.reset(128, "Fehler");
	}

	graf_mouse(ARROW, dummy_ptr);

	Dialog.busy.ok();

	return;
}


/*-------------------------- System analysieren ----------------------*/
void f_analyze_system(void)
{
	char gi = 0;

	int out1, out2, out3, out4, ok;

	long value;
	unsigned long dummy;


	Sys_info.OS = 0;

	/*-------------- OS feststellen -----------------*/
	if(get_cookie('MagX', &dummy) || get_cookie('MgMc', &dummy) || get_cookie('MgPC', &dummy))
		Sys_info.OS |= MATSCHIG;

	if(get_cookie('MgPC', &dummy))	/* MagiC PC nochmal extra */
		Sys_info.OS |= MAG_PC;

	if(get_cookie('MiNT', &dummy))
		Sys_info.OS |= MINT;
	
	if(get_cookie('nAES', &dummy))
		Sys_info.OS |= NAES;

	if(get_cookie('MTOS', &dummy))
		Sys_info.OS |= MTOS;

	if(get_cookie('WINX', &dummy))
		Sys_info.OS |= WINX;

	Sys_info.OSFeatures = 0;

/* GETINFO */
 	if(_GemParBlk.global[0] >= 0x400 ||
 	   appl_find("?AGI") >= 0)
	{
		Sys_info.OSFeatures |= GETINFO;
		gi = 1;
	}

/* CICONBLK */
	if(gi)
	{	
		ok = appl_getinfo(2, &out1, &out2, &out3, &out4);
		if(ok && out3 == 1)
			Sys_info.OSFeatures |= COLICONS;
	}

	if(!gi || !ok)
	{
		if(Sys_info.AES_version >= 0x340)
			Sys_info.OSFeatures |= COLICONS;
	}

/* WINDOW - BEVENT */
	if(gi)
	{	
		ok = appl_getinfo(11, &out1, &out2, &out3, &out4);
		if(ok && out1&32)
			Sys_info.OSFeatures |= BEVT;
	}

	if(!gi || !ok)
	{
		if(Sys_info.AES_version >= 0x331)
			Sys_info.OSFeatures |= BEVT;
	}

/* MENU_POPUP */
	if(gi)
	{
		ok = appl_getinfo(9, &out1, &out2, &out3, &out4);
		if(ok && out2 == 1)
			Sys_info.OSFeatures |= MPOPUP;
	}

	if(!gi || !ok)
	{
		if(Sys_info.AES_version >= 0x331 && !(Sys_info.OS&MATSCHIG))
			Sys_info.OSFeatures |= MPOPUP;
	}

/* 3D-AES */
	if(gi)
	{
		ok = appl_getinfo(13, &out1, &out2, &out3, &out4);
		if(ok && out1 == 1)
			Sys_info.OSFeatures |= AES3D;
	}

	if(!gi || !ok)
	{
		if(Sys_info.AES_version >= 0x331 && !(Sys_info.OS&MATSCHIG))
			Sys_info.OSFeatures |= AES3D;
	}

/* PEXEC(10x) */
	if(Sys_info.OS&MINT || InqMagX() >= 0x0500 && _GemParBlk.global[1] != 1)
		Sys_info.OSFeatures |= PEXEC10x;

	if(gi)
	{
		ok = appl_getinfo(13, &out1, &out2, &out3, &out4);
		if(ok && out2 == 1)
		{
			Sys_info.OSFeatures |= OSYSVAR;
			objc_sysvar(0, 5, 0, 0, &out1, &out2);	/* AES-Hintergrund */
			Sys_info.AES_bgcolor = out1;
		}
	}

	if(!gi || !ok)
		Sys_info.AES_bgcolor = 8;

	if(Sys_info.bitplanes == 1 || !(Sys_info.OSFeatures&AES3D))
		Sys_info.AES_bgcolor = 0;


	/* Clipboardpfad ermitteln und ggf. setzen */
	inquire_clipboard();


	/* CPU-Typ ermitteln */
	get_cookie('_CPU', (unsigned long*)&value);
	if(value == 0 || value == 10)
		global_services.CPU_type = MC68000;
	else if(value == 20)
		global_services.CPU_type = MC68020;
	else if(value == 30)
		global_services.CPU_type = MC68030;
	else if(value == 40)
		global_services.CPU_type = MC68040;
	else if(value == 60)
		global_services.CPU_type = MC68060;
		
	get_cookie('_FPU', (unsigned long*)&value);
	if((value >> 16) != 0)
		global_services.CPU_type |= FPU;

	DEBUG_MSG (( "--> Systeminfos\n" ));
	DEBUG_MSG (( "    OS         : %#x\n", Sys_info.OS ));
	DEBUG_MSG (( "    Features   : %#x\n", Sys_info.OSFeatures ));
	DEBUG_MSG (( "    AES_bgcolor: %#x\n", Sys_info.AES_bgcolor ));
	DEBUG_MSG (( "    CPU        : %#x\n", global_services.CPU_type ));

	return;
}


unsigned int InqMagX(void)
{
	MAGX_COOKIE *cv;


	if(get_cookie('MagX', (unsigned long*)&cv)!=0)
		if(cv->aesvars)
			return(cv->aesvars->version);
		else
			return(FALSE);
	else
		return(FALSE);
} /* InqMagX */



/* inquire_clipboard --------------------------------------------------------
	Clipboardpfad ermitteln und ggf. setzen. Es wird ggfs. ein Clipboardordner
	C:\\CLIPBRD erzeugt.
	------------------------------------------------------------------------*/
void inquire_clipboard(void)
{
	char *env_path, chpath[256];

	int back;

	Sys_info.scrp_path = calloc(1, 256);

	back = scrp_read(Sys_info.scrp_path);
/*
	printf("back: %d\n", back);
	printf("scrp_path: %s\n", Sys_info.scrp_path);
*/

	if(back != 0 && *Sys_info.scrp_path != '\0')
	{
/*		printf("scrp_read hat geklappt\n"); */

		check_and_create(chpath);
	}
	else
	{
/*		printf("scrp_read hat nicht geklappt\n"); */

		if((env_path = getenv("CLIPBRD")) != NULL)
			strcpy(Sys_info.scrp_path, env_path);
		else
			if((env_path = getenv("SCRAPDIR")) != NULL)
				strcpy(Sys_info.scrp_path, env_path);
/*
		printf("getenv()\n");

		if(env_path != NULL)
			printf("env_path: %s\n", env_path);
		else
			printf("env_path: NULL\n");
*/
		if(env_path == NULL)
			strcpy(Sys_info.scrp_path, "c:\\clipbrd");

		check_and_create(chpath);
	}

	if(Sys_info.scrp_path != NULL)
	{
		if(Sys_info.scrp_path[strlen(Sys_info.scrp_path) - 1] != '\\')
		   strcat(Sys_info.scrp_path, "\\");

		/*
		 * Clipboardexporter testen 
		 */
		strcpy(chpath, Sys_info.standard_path);
		strcat(chpath, "\\modules\\clipbrd.sxm");
		if(Fattrib(chpath, 0, 0) < 0)
			Sys_info.scrp_path = NULL;
	}
/*
	if(Sys_info.scrp_path != NULL)
		printf("scrp_path: %s\n", Sys_info.scrp_path);
	else
		printf("scrp_path: NULL\n");

	getch();
*/
	return;
}


/* pr�ft ob der ermittelte Clipboardpfad tats�chlich vorhanden ist */
/* und versucht einen anzulegen wenn er nicht vorhanden ist */
void check_and_create(char *chpath)
{
/*
	if(Sys_info.scrp_path != NULL)
		printf("scrp_path: %s\n", Sys_info.scrp_path);
	else
		printf("scrp_path: NULL\n");

	getch();
*/
	if(Sys_info.scrp_path[strlen(Sys_info.scrp_path) - 1] == '\\')
	   Sys_info.scrp_path[strlen(Sys_info.scrp_path) - 1] = '\0';

	strcpy(chpath, Sys_info.scrp_path);

	if(Fattrib(chpath, 0, 0) < 0)		/* Pfad nicht gefunden */
	{
		if(Dcreate(Sys_info.scrp_path) < 0)
		{
			free(Sys_info.scrp_path);
			Sys_info.scrp_path = NULL;
			return;
		}
		else
		{
			strcat(Sys_info.scrp_path, "\\");
			scrp_write(Sys_info.scrp_path);
		}
	}

	return;
} /* check_and_create */


/* Holt sich das Smurflaufwerk */
/* noch einzubauen ist der Test mit Fcntl(... PLOADINFO) */
/* auf die eigene Proze�datei */
void GetSMPath(void)
{
	char *sh_rptail, *sh_rpcmd, *path;

	int drivenum, back;


	stpath = calloc(1, 257L);
	sh_rpcmd = calloc(1, 257L);
	sh_rptail = calloc(1, 257L);

	if((back = getploadinfo(sh_rptail, sh_rpcmd)) == 0)
		back = shel_read(sh_rpcmd, sh_rptail);
/*
	printf("back: %d\n", back);
	printf("sh_rpcmd: %s\n", sh_rpcmd);
	printf("sh_rptail: %s\n", sh_rptail);
*/
	if(back != 0 && sh_rpcmd[0] != 0 && sh_rpcmd[1] == ':')			/* hat das geklappt ... */
	{																/* Pfad enthalten */
		if((back = strsrchr(sh_rpcmd, '\\')) != -1)
		{
			sh_rpcmd[back] = '\0';
			strcpy(stpath, sh_rpcmd);
		}
	}
	else															/* oder nicht (z.B. TOS 1.0) */
	{
		memset(stpath, 0x0, 257);
		if((drivenum = Dgetdrv()) <= 25)							/* drivenum hier: A=0, B=1 ... */
			stpath[0] = drivenum + 'A';								/* alte Laufwerke A - Z, Laufwerksbuchstabe ermitteln */
		else	
			stpath[0] = drivenum + '0';								/* neue Laufwerke 1 - 6, Laufwerksbuchstabe ermitteln */

		drivenum++;													/* von A=0 auf A=1 */

		stpath[1] = ':';

		Dgetpath(sh_rpcmd, drivenum);								/* drivenum hier: A=1, B=2 ... */

		if(sh_rpcmd[0] != '\\')
			stpath[2] = '\\';

		strcat(stpath, sh_rpcmd);
	}

	/* ***************Nur f�r den Debugger: ********************/
#if DEBUGGER
		if(stricmp(stpath, "E:\\PURE-C") == 0)
			strcpy(stpath, "e:\\smurf");
#endif

/*	printf("stpath: %s\n", stpath); */

	strcpy(Sys_info.standard_path, stpath);

/*	printf("Sys_info.standard_path: %s\n", Sys_info.standard_path); */

	/*
	 *	Pfad des $HOME-Verzeichnis auslesen
	 */
	path = getenv("HOME");

	if(path == NULL)
		strcpy(Sys_info.home_path, Sys_info.standard_path);
	else
	{
		strcpy(Sys_info.home_path, path);
		
		if(Sys_info.home_path[strlen(Sys_info.home_path) - 1] == '\\')
		   Sys_info.home_path[strlen(Sys_info.home_path) - 1] = '\0';

		/* mit einem Trick testen, ob der Pfad vorhanden ist ist */
		/* Ergebnis < 0 wenn die Datei noch nicht vorhanden ist */
		strcpy(sh_rptail, Sys_info.home_path);
		if(Fattrib(sh_rptail, 0, 0) >= 0)					/* Pfad gefunden */
		{
			strcat(sh_rptail, "\\defaults");
			if(Fattrib(sh_rptail, 0, 0) >= 0)				/* Pfad gefunden */
				strcat(Sys_info.home_path, "\\defaults");
		}
		else
			strcpy(Sys_info.home_path, Sys_info.standard_path);
	}

	free(sh_rpcmd);
	free(sh_rptail);

	return;
} /* GetSMPath */


/* gibt im Fehlerfall 0 zur�ck um returnwertkompatibel */
/* zu shel_read() zu sein (s.o.) */
int getploadinfo(char *cmdlin, char *fname)
{
	long handle, ret;

	struct ploadinfo pl;


	if((handle = Fopen("u:\\proc\\x.-1", FO_READ)) >= 0)
	{
		pl.fnamelen = 257;
		pl.cmdlin = cmdlin;
		pl.fname = fname;

		ret = Fcntl((int)handle, (long)&pl, PLOADINFO);

		Fclose((int)handle);

		if(ret == 0)
			return(1);
	}
	else
		return(0);

	return(1);
} /* getploadinfo */