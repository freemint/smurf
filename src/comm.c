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

#include <stdio.h>
#include <stdlib.h>
#include <tos.h>
#include <string.h>
#include <errno.h>
#include <ext.h>
#include <screen.h>
#include "smurf_st.h"
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "smurf_f.h"
#include "globdefs.h"
#include "smurf.h"
#include "vaproto.h"
#include "smurfobs.h"
#include "ext_obs.h"
#include "bindings.h"

/*-------- lokale Funktionen -----------*/
int dd_getheader(DD_HEADER *dd_header, int pipe_handle);
int dd_sendheader(int pipe_handle, long ext, long size, char *data_name, char *file_name);

extern SYSTEM_INFO Sys_info;
extern MFORM	*dummy_ptr;				/* Dummymouse fr Maus-Form */
extern OBJECT *menu_tree;
extern char Smurf_locked;
extern char loadpath[257];
extern char commpath[257];
extern char DraufschmeissBild;
extern SMURF_PIC *smurf_picture[40];
extern int num_of_pics, active_pic, pic_to_handle, picthere;
extern long f_len;

extern	int	*messagebuf;

char *send_smurfid;

/*--------------------- Defines fr DHST-Protokoll */
#define DHST_ADD 0xdadd
#define DHST_ACK 0xdade

typedef struct
{
  char *appname,
       *apppath,
       *docname,
       *docpath;
} DHSTINFO;



/*------------------------- Drag&Drop - Empfangshandling --------------*/
int get_dragdrop(WINDOW *window_to_handle, int *messagebuf)
{
	char pipe_name[] = "U:\\PIPE\\DRAGDROP.AA", *namename;
	char *data, *data_tag;

	char dd_ok = DD_OK, dd_nak = DD_NAK;

	int pipe_handle, my_wnum;
	int new_pic, module_ret;

	long dummy;
	long dd_list[8] = {'.IMG', 'ARGS', '    ', '    ', '    ', '    ', '    ', '    '};

	DD_HEADER dd_header;

		
/*
	printf("\n ***Drag&Drop auf %s", window_to_handle->wtitle);	
	printf("\n    Mouse X/Y:  %i/%i", messagebuf[4], messagebuf[5]);	
	printf("\n    Kbshift  %i", messagebuf[6]);
*/
	
	/* Pipenamen zusammenbasteln */
	pipe_name[17] = (messagebuf[7] & 0xff00) >> 8;
	pipe_name[18] = messagebuf[7] & 0x00ff;
	
/*	printf("\n    Pipe-Name: %s", pipe_name); */
	
	dummy = Fopen(pipe_name, FO_RW);
/*	printf("\n    Fopen: %li", fopback); */
	if(dummy >= 0)
	{
		pipe_handle = (int)dummy;
/*		printf("\n    Pipe-Handle: %i", pipe_handle); */

		if(Smurf_locked)									/* GUI gesperrt? */
		{
			Fwrite(pipe_handle, 1, &dd_nak);				/* NAK senden */
			Fclose(pipe_handle);
			return(0);
		}
		else
			dummy = Fwrite(pipe_handle, 1, &dd_ok);			/* DD_OK! */
/*		printf("\n    DD_OK sent (1 Byte): %li", fopback); */
		dummy = Fwrite(pipe_handle, 32, dd_list);			/* Wunschliste schicken */
/*		printf("\n    Data list sent (32 Bytes): %li", fopback);*/
		
		if(!Comm.ddGetheader(&dd_header, pipe_handle))
		{
			Fwrite(pipe_handle, 1, &dd_nak);				/* NAK senden */
			Fclose(pipe_handle);							/* Rohr zumachen */
			return(0);
		}

		/* + 1 ist n”tig um Akzente, „hm, hinter den Namen sicher */
		/* ein Nullzeichen zu haben - ber die Pipe kommt n„mlich keines ... */
		if((data = SMalloc(dd_header.data_length + 1)) == NULL)
		{
			Fwrite(pipe_handle, 1, &dd_nak);				/* NAK senden */
			Fclose(pipe_handle);							/* Rohr zumachen */
			return(M_MEMORY);								/* und tschss */
		}
		memset(data, 0x0, dd_header.data_length + 1);

		Fwrite(pipe_handle, 1, &dd_ok);						/* OK senden */
		
		Fread(pipe_handle, dd_header.data_length, data);	/* Daten auslesen */

		Fclose(pipe_handle);								/* Pipe wieder schliežen */

		/*
		 * weil bei einem switch ja nur ints verglichen werde, muž man hier etwas aufpassen!
		 */
		switch(dd_header.data_type)
		{
			case 'ARGS':	/*----------------- Kommandozeile wurde geschickt ----*/
							data_tag = data;
							/* Eigenart von MagiXDesk und Jeenie bergehen, als */
							/* erstes Zeichen ein Nullzeichen zu schicken. */
							if(*data_tag == '\0')
								strcpy(data_tag, data_tag + 1);
							/* Extra Puffer anlegen, da sonst f_loadpic() die Daten kaputtmacht :-/ */
/*							printf("\nkomplette ARGS-Zeile: %s\n\n", data_tag);*/
							DraufschmeissBild = ARGS,
							file_load("", &data_tag, ARGS);
							SMfree(data);
							break;

			case '.IMG':	/*----------------- XIMG wurde geschickt ----*/
							graf_mouse(BUSYBEE, dummy_ptr);

							f_len = dd_header.data_length;
							my_wnum = Window.myWindow(window_to_handle->whandlem);
							if(my_wnum < 0)						/* Ziel ist ein Smurf-Bildfenster */
							{
								clip2block(window_to_handle->picture, data, messagebuf[4], messagebuf[5]);	/* IMG in Bildblock einlesen */
								Window.redraw(window_to_handle, NULL, 0, 0);
							}
							else
							{
								new_pic = 1;
								while(smurf_picture[new_pic] != NULL)
									new_pic++;
								if(new_pic > MAX_PIC)
								{
									Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_PIC_FREE].TextCast, NULL, NULL, NULL, 1);
									SMfree(data);
									return(-1);
								}

								smurf_picture[new_pic] = (SMURF_PIC *)SMalloc(sizeof(SMURF_PIC));
								smurf_picture[new_pic]->pic_data = data;

								/* Originalloadpath wegsichern, damit er zurckgesetzt */
								/* werden kann, wenn das Laden schiefl„uft. */

								/* Wenn kein Backslash vorhanden ist, montiere C:\ vorne dran. */
								/* Ist in file_name kein Filename mitgeliefert, nenne das Ding "Newpic.IMG" */
								if(strlen(dd_header.file_name) == 0)
									strcpy(dd_header.file_name, "C:\\Newpic.IMG");
								else
									if((namename = strrchr(dd_header.file_name, '.')) != NULL)
										strcpy(namename, ".IMG");

								if((namename = strrchr(dd_header.file_name, '\\')) == NULL)		/* Dateinamen abtrennen */
								{
									/* Dateinamen nach hinten schieben - memcpy() weil strcpy mit */
									/* berlappenden Bereichen nicht zurechtkommt */
									memcpy(dd_header.file_name + 3, dd_header.file_name, strlen(dd_header.file_name) + 1);
									strncpy(dd_header.file_name, "C:\\", 3);					/* Laufwerk vornedran */
									namename = dd_header.file_name + 3;
								}
								else
									namename++;					/* den Backslash noch abschneiden */

								strcpy(smurf_picture[new_pic]->filename, dd_header.file_name);

								module_ret = f_import_pic(smurf_picture[new_pic], "IMG");
								f_formhandle(new_pic, module_ret, namename);
								picthere++;
								active_pic = new_pic;
							}

							Dialog.busy.dispRAM();	/* Wieviel Ram? */
							actualize_menu();		/* Meneintr„ge ENABLEn / DISABLEn */

							graf_mouse(ARROW, dummy_ptr);

							break;
		}
	}

	return(0);
} /* get_dragdrop */


/*------------ D&D-Header einlesen ----------*/
int dd_getheader(DD_HEADER *dd_header, int pipe_handle)
{
	char buf[33];
	char *read_dd_string;
	char dd_ext = DD_EXT;

	int header_read_bytes, overread;

	long fopback;

	memset(dd_header, 0x0, sizeof(DD_HEADER));

	do
	{
		/* einlesen der ersten drei Strukturfelder */
		fopback = Fread(pipe_handle, 10, dd_header);
/*		printf("\n    Header read (10 Bytes): %li", fopback); */
		if(fopback != 10)
			return(FALSE);

/*		printf("\n    Drag&Drop Header length: %i", dd_header->header_length); */
/*		/* Nur fr printf() */
		strncpy(datatype, (char *)&(dd_header->data_type), 4);
		datatype[4] = 0;
		printf("\n    Data type: %s", datatype);
		printf("\n    Data length: %li", dd_header->data_length); */

		/* anscheinend z„hlt die Headerl„ngenangabe nicht dazu! */
		header_read_bytes = 8;

/*		printf("\n    header_read_bytes: %i", header_read_bytes); */

		read_dd_string = dd_header->data_name;
		while(header_read_bytes < dd_header->header_length)
		{
			Fread(pipe_handle, 1, read_dd_string);
			header_read_bytes++;
			if(*read_dd_string++ == '\0')
				break;
		}
/*		printf("\n    Datenname: %s", dd_header->data_name); */

		read_dd_string = dd_header->file_name;
		while(header_read_bytes < dd_header->header_length)
		{
			Fread(pipe_handle, 1, read_dd_string);
			header_read_bytes++;
			if(*read_dd_string++ == '\0')
				break;
		}
/*		printf("\n    Filename: %s", dd_header->file_name); */

		/* m”glichen Rest auslesen */
		while(header_read_bytes < dd_header->header_length)
		{
			overread = dd_header->header_length - header_read_bytes;
			if(overread > sizeof(buf))
				overread = (int)sizeof(buf);

			Fread(pipe_handle, overread, buf);
			header_read_bytes += overread;
		}

		if(dd_header->data_type != 'ARGS' && dd_header->data_type != '.IMG')
			Fwrite(pipe_handle, 4, &dd_ext);			/* N”, das mag ich nicht */
	} while(dd_header->data_type != 'ARGS' && dd_header->data_type != '.IMG');

	return(TRUE);
} /* dd_getheader */


/*---------------------------- Drag&Drop - Sendehandling --------------*/
int send_dragdrop(SMURF_PIC *picture, int dest_whandle, int mx, int my)
{
	static char pipe_name[] = "U:\\PIPE\\DRAGDROP.AA";
	char *data = 0, *dd_data = 0, *file_name, dest_path[129] = "";
	char dd_akt, i;

	int pipe_handle, recv_id, wi_gw2, wi_gw3, wi_gw4;

	long dd_list[32];
	long my_dd_list[8] = {'.IMG', 'ARGS', '    ', '    ', '    ', '    ', '    ', '    '};
	long dummy, fd_mask, data_length = 0;

	EXPORT_PIC *pic_to_save;


	/* Pipe ”ffnen */
	/* Pipenamen zusammenbasteln */
	pipe_name[17] = 'P';
	pipe_name[18] = 'T';	
	if((dummy = Fcreate(pipe_name, FA_HIDDEN)) < 0)
		return(-1);

	pipe_handle = (int)dummy;

	messagebuf[0] = AP_DRAGDROP;
	messagebuf[1] = Sys_info.app_id;
	messagebuf[2] = 0;
	messagebuf[3] = dest_whandle;				/* Handle des Zielfensters */
	messagebuf[4] = mx;							/* X-Mausposition */
	messagebuf[5] = my;							/* Y-Mausposition */
	messagebuf[6] = 0;							/* Keyboard-Shift-Status */
	/* Pipeextensions zusammensuchen */
	messagebuf[7] = ('P' << 8) | 'T';			/* diese Kombination sollte noch nicht benutzt sein */

	/* Besitzer des Zielfensters ermitteln - hoffentlich ist WF_OWNER vorhanden ... */
	Window.windGet(dest_whandle, WF_OWNER, &recv_id, &wi_gw2, &wi_gw3, &wi_gw4);
	appl_write(recv_id, 16, messagebuf);

	/* Auf Antwort warten */
	fd_mask = (1L << pipe_handle);
	dummy = Fselect(3000, &fd_mask, 0L, 0L);
	if(!dummy || !fd_mask)						/* Timeout eingetreten */
	{											/* andere Applikation beherrscht wahrscheinlich kein D&D */
		Fclose(pipe_handle);
		return(-2);
	}

	dummy = Fread(pipe_handle, 1, &dd_akt);		/* Pipe auslesen */
/*	printf("dd_akt 1: %d\n", dd_akt); */
	if(dd_akt != DD_OK)							/* andere Applikation beherrscht kein D&D */
	{
		Fclose(pipe_handle);
		return(-2);
	}

	if(Fread(pipe_handle, 32L, dd_list) != 32L)
	{
		Fclose(pipe_handle);
		return(-1);
	}

	/* fake mich zum Block */
	picture->block = picture;
	if(encode_block(picture, &pic_to_save) != 0)
	{
		picture->block = NULL;
		SMfree(pic_to_save->pic_data);
		SMfree(pic_to_save);
		return(-1);
	}
	/* Fake rckg„ngig machen */
	picture->block = NULL;

	data = (char *)pic_to_save->pic_data;
	f_len = pic_to_save->f_len;

	if((file_name = strrchr(picture->filename, '\\')) == NULL)
		file_name = picture->filename;
	else
		file_name++;
	
/*	printf("file_name: %s\n", file_name); */

/*	Goto_pos(1, 0); */

	dd_akt = DD_EXT;
	i = 0;
	do
	{
/*		printf("dd_akt 2: %d\n", (int)dd_akt); */

		switch(dd_akt)
		{
			case DD_OK:		if(my_dd_list[i - 1] == '.IMG')
							{
								Fwrite(pipe_handle, data_length, dd_data);
								SMfree(pic_to_save->pic_data);
								SMfree(pic_to_save);
							}
							else
							if(my_dd_list[i - 1] == 'ARGS')
							{
								if(save_block(pic_to_save, dest_path) != 0)
								{
									picture->block = NULL;
									SMfree(pic_to_save->pic_data);
									SMfree(pic_to_save);
									return(-1);
								}
/*								printf("dd_data: %s\n", dd_data); */
								Fwrite(pipe_handle, data_length, dd_data);
								free(dd_data);
							}
/*							printf("Daten schreiben\n"); */
							break;
			case DD_NAK:	Fclose(pipe_handle);
							return(-1);			/* andere Applikation will nicht mehr */
							break;
			case DD_EXT:	/* Header ausw„hlen */
/*							printf("neuer Extender\n"); */
							if(my_dd_list[i] == '.IMG')
							{
/*								printf(".IMG\n"); */
								dd_data = data;
								data_length = f_len;
								Comm.ddSendheader(pipe_handle, '.IMG', data_length, "", file_name);
							}
							else
							if(my_dd_list[i] == 'ARGS')
							{
/*								printf("ARGS\n"); */
								get_tmp_dir(dest_path);
								strcat(dest_path, "ARGS.IMG");
/*								dd_data = quote_arg(dest_path); */
/*								printf("dest_path: %s\n", dest_path); */
								dd_data = dest_path;
								data_length = strlen(dd_data) + 1;
								Comm.ddSendheader(pipe_handle, 'ARGS', data_length, "", file_name);
							}
							else
							{
								Fclose(pipe_handle);
								return(-1);
							}
							break;
			case DD_LEN:	Fclose(pipe_handle);	/* vergiž es, weniger Daten habe ich nicht */
							return(-1);
							break;
			case DD_TRASH:	Comm.sendAESMsg(Sys_info.app_id, VA_THAT_IZIT, VA_OB_TRASHCAN, -1);		/* Bild wurde auf Mlleimer gezogen */
								break;
			case DD_PRINTER:	Comm.sendAESMsg(Sys_info.app_id, VA_THAT_IZIT, VA_OB_PRINTER, -1);	/* Bild wurde auf Drucker gezogen */
								break;
			case DD_CLIPBOARD:	Comm.sendAESMsg(Sys_info.app_id, VA_THAT_IZIT, VA_OB_CLIPBOARD, -1);	/* Bild wurde auf Klemmbrett gezogen */
								break;
			default:		break;
		}

		if(dd_akt != DD_EXT)
			break;

		dummy = Fread(pipe_handle, 1, &dd_akt);			/* Pipe auslesen */
/*		printf("read: %ld\n", dummy); */
	} while(my_dd_list[i++] != '    ');

/*	printf("Hier2\n"); */

	Fclose(pipe_handle);
/*	Fdelete(dest_path); */

	return(0);
} /* send_dragdrop */


/*----------- D&D-Header verschicken --------*/
int dd_sendheader(int pipe_handle, long ext, long size, char *data_name, char *file_name)
{
	int hdrlen;

	/* 4 Bytes fr Datentyp, 4 Bytes fr Datenl„nge, */
	/* 2 Bytes fr Stringendnullen */
	hdrlen = (int)(4 + 4 + strlen(data_name) + 1 + strlen(file_name) + 1);

	/* Header senden */
	if(Fwrite(pipe_handle, 2L, &hdrlen) != 2L)
		return(DD_NAK);

	Fwrite(pipe_handle, 4L, &ext);
	Fwrite(pipe_handle, 4L, &size);
	Fwrite(pipe_handle, strlen(data_name) + 1, data_name);
	Fwrite(pipe_handle, strlen(file_name) + 1, file_name);

	return(0);
} /* dd_sendheader */


/*---------------------- Document History updaten -----------------	*/
void update_dhst(char *path)
{
	char *namestring, *string;

	int dhst_id;

	unsigned long value;

	DHSTINFO *history;


	/* DHST-Server finden */
	if( get_cookie('DHST', &value) == FALSE )
		return;
	dhst_id = (int)value;

	/* Block wenn m”glich als global anfordern? */
	if(Ssystem(FEATURES, 0L, 0L) != EINVFN || Sys_info.OS&MINT || Sys_info.OS&MATSCHIG)
	{
		history = (DHSTINFO *)Mxalloc(sizeof(DHSTINFO), 0x20);
		string = (char *)Mxalloc(646, 0x20);
	}
	else
	{
		history = (DHSTINFO *)SMalloc(sizeof(DHSTINFO));
		string = (char *)SMalloc(646);
	}

	history->appname = string;
	history->apppath = string + 6;
	history->docname = string + 262;
	history->docpath = string + 390;

	/* Struktur fllen */
	strcpy(history->appname, "Smurf");

	strcpy(history->apppath, Sys_info.standard_path);
	strcat(history->apppath, "\\smurf.prg");				
	
	namestring = strrchr(path, '\\');
	if(namestring == NULL)
	{
		SMfree(string);
		SMfree(history);
		return;
	}
	
	strcpy(history->docname, namestring + 1);

	strcpy(history->docpath, path);

	Comm.sendAESMsg(dhst_id, DHST_ADD, LONG2_2INT(history), -1);

	/* auf keinen Fall hier schon string oder history freigeben! */
	/* das wird nach Antwort vom History-Server durch DHST_ACK gemacht */

	return;
} /* update_dhst */


/*----------- Ben”tigte Environment-Variablen auslesen -------------*/
void get_avserv(void)
{	
	char *avs;
	char av_name[9];
	char av_string[21];

	if((avs = getenv("AVSERVER")) != NULL)
	{
		strcpy(av_name, avs);
		strncat(av_name, "          ", 8 - strlen(avs));
		av_name[8] = 0;

		strcpy(av_string, "AV-Server: ");
		strcpy(av_string, av_name);
		Dialog.busy.reset(128, av_string);

		Sys_info.ENV_avserver = appl_find(av_name);		/* Avserver ermitteln */
	}
	else
		Sys_info.ENV_avserver = -1;

	return;
} /* get_avserv */


/*------------- Initialisierung des AV-Protokolls -------------*/
void init_AVPROTO(void)
{
	int ap_buf[9];


	/*-------- Block wenn m”glich als global anfordern? */
	if(Ssystem(FEATURES, 0L, 0L) != EINVFN || Sys_info.OS&MINT || Sys_info.OS&MATSCHIG)
		send_smurfid = Mxalloc(9, 0x20);
	else
		send_smurfid = SMalloc(9);

	strcpy(send_smurfid, "SMURF   ");

	if(Sys_info.ENV_avserver != -1)
	{
		ap_buf[0] = AV_PROTOKOLL;
		ap_buf[1] = Sys_info.app_id;
		ap_buf[2] = 0;
		ap_buf[3] = 0x16;		/* ich kann VA_START, VA_STARTED und Quoting von Dateinamen */
		ap_buf[4] = 0;
		ap_buf[5] = 0;

		*((char * *)&ap_buf[6]) = send_smurfid;
		appl_write(Sys_info.ENV_avserver, 16, ap_buf);
	}

	return;
} /* init_AVPROTO */



void send_AESMessage(int dest_id, int msg, ...)
{
	int ap_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int var, t;

	va_list *parms;

	va_start(parms, msg);

	if(dest_id != -1)
	{
		ap_buf[0] = msg;
		ap_buf[1] = Sys_info.app_id;
		ap_buf[2] = 0;

		t = 3;
		do
		{
			var = va_arg(parms, int);
			if(var == -1)
				break;

			ap_buf[t] = var;
		} while(++t < 8);

/*		printf("ap_buf[7]: %d\n", ap_buf[7]);*/

		appl_write(dest_id, 16, ap_buf);
	}
    
	va_end(parms);

	return;
} /* send_AESMessage */