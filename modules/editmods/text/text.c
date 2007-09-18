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
 * The Initial Developer of the Original Code is
 * Olaf Piesche
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* --------------------------------------------------------	*/
/*				FÅr SMURF Bildkonverter, 25.02.98 			*/
/*															*/
/* Version 1.02  --  04.05.98								*/
/*	  work_in wird vor v_opnbm() fÅr das Preview ausgenullt	*/
/* Version 1.03  --  17.05.98								*/
/*	  work_in wird vor v_opnbm() fÅr das Bild ausgenullt	*/
/* Version 1.04  --  14.06.98								*/
/*	  Ab jetzt wird auch der MagiC-Fontselektor unterstÅtzt	*/
/* Version 1.05  --  13.07.98								*/
/*	  Kleinen Fehler beim Aufruf des MagiC-Fontselektor		*/
/*	  behoben (Workstation wurde zu frÅh geschlossen).		*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdi.h>
#include <screen.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <math.h>
#include "fontsel.h"

#define ENGLISCH 0

#if ENGLISCH
	#include "text\en\text.rsh"
	#include "text\en\text.rh"
#else
	#include "text\de\text.rsh"
	#include "text\de\text.rh"
#endif

#define	TextCast	ob_spec.tedinfo->te_ptext


/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Text",
						0x0105,
						"Olaf Piesche",
						"","","","","","","","","","",
						"Slider 1",
						"Slider 2",
						"Slider 3",
						"Slider 4",
						"Checkbox 1",
						"Checkbox 2",
						"Checkbox 3",
						"Checkbox 4",
						"Edit 1",
						"Edit 2",
						"Edit 3",
						"Edit 4",
						0,128,
						0,128,
						0,128,
						0,128,
						0,10,
						0,10,
						0,10,
						0,10,
						0,0,0,0,
						0,0,0,0,
						0,0,0,0,
						1,			/* Bildanzahl */
						"","","","","",""
						};


SERVICE_FUNCTIONS *services;

MOD_ABILITY	module_ability=
{
	/* Farbtiefen, die vom Modul unterstÅtzt werden:			*/
	1,2,4,6,7,8,16,24,
	/*	Dazugehîrige Datenformate (FORM_PIXELPAK/FORM_STANDARD/FORM_BOTH) */
	FORM_STANDARD,FORM_STANDARD,FORM_STANDARD,FORM_STANDARD,FORM_STANDARD,
	FORM_BOTH,FORM_PIXELPAK,FORM_PIXELPAK,
	0	
};

extern	long vst_arbpt32(int handle, long height, int *char_width, int *char_height,
                           int *cell_width, int *cell_height);

extern	int get_cookie(unsigned long cookie, unsigned long *value);
extern	void v_opnbm(int *work_in, MFDB *bitmap, int *handle, int *work_out);
extern	void v_clsbm(int handle);
extern	void vqt_xfntinfo(int handle, int flags, int id, int index, XFNT_INFO *info);
extern	void vqt_real_extent( int handle, int x, int y, char *string,  int *extent );
int		call_fontsel(int handle, FONT_INFO *fontinfo);
int		handle_aesmsg(GARGAMEL *smurf_struct, FONT_INFO *font);

OBJECT *maintree, *alerts;
WINDOW *mwindow;
SMURF_PIC *picture;
SMURF_PIC preview;

MFDB offscreen;
int oshandle;
int work_in[25], work_out[57];
int char_height, cell_height, char_width, cell_width;
int black[]={0,0,0};


XFNT_INFO font_info;

void 	(*set_slider)(SLIDER *sliderstruct, long value);
void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);
void f_doit(GARGAMEL *smurfstruct, SMURF_PIC *picture, FONT_INFO *font);
void compute_preview(void);



/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
int (*get_window)(WINDOW *wind);		/* Funktion deklarieren */

int t, back;
int mod_id;
int wind_num=1;
int SmurfMessage;
char wt[]="Text V0.1";
int Button;
long dummy;

char name[40];
char str[6];
static FONT_INFO font;


	SmurfMessage = smurf_struct->module_mode;

	/*
	 * Hier werden die Funktionen aus der GARGAMEL-Struktur geholt.
	 */
	get_window=smurf_struct->services->f_module_window;			/* Windowfunktion	*/
	set_slider = smurf_struct->services->set_slider;
	redraw_window = smurf_struct->services->redraw_window;

	picture=smurf_struct->smurf_pic;
	services = smurf_struct->services;

	/* 
	 * Wenn das Modul zum ersten Mal gestartet wurde 
	 */
	if(SmurfMessage==MSTART)
	{
		maintree = rs_trindex[TEXT_MAIN];		/* Resourcebaum holen */
		alerts = rs_trindex[ALERT_STRINGS];		/* Resourcebaum holen */

		if(get_cookie('EdDI', &(unsigned long)dummy)==0)
		{
			services->f_alert(alerts[NO_NVDI].TextCast, NULL, NULL, NULL, 1);
			smurf_struct->module_mode = M_MODERR;
			return; 
		}

	
		/* Resource umbauen */
		for(t = 0; t < NUM_OBS; t++)
			rsrc_obfix (&rs_object[t], 0);
	
		mod_id=smurf_struct->module_number;			/* Welche ID habe ich?	*/

		mwindow = (WINDOW*)smurf_struct->services->SMalloc(sizeof(WINDOW));
		memset(mwindow, 0, sizeof(WINDOW));
		mwindow->whandlem = 0;						/* evtl. Handle lîschen	*/
		mwindow->module = mod_id;					/* ID in die Fensterstruktur eintragen	*/
		strcpy(mwindow->wtitle, wt);				/* Titel reinkopieren 	*/
		mwindow->wnum = wind_num;					/* Fenster nummer 1... 	*/
		mwindow->wx = -1;							/* Fenster X-...	*/
		mwindow->wy = -1;							/* ...und Y-Pos		*/
		mwindow->ww = maintree[0].ob_width;			/* Fensterbreite	*/
		mwindow->wh = maintree[0].ob_height;		/* Fensterhîhe		*/
		mwindow->resource_form = maintree;			/* Resource			*/
		mwindow->picture = NULL;					/* Preview als Bild	*/
		mwindow->pic_xpos = maintree[PREVIEW].ob_x;
		mwindow->pic_ypos = maintree[PREVIEW].ob_y+1;
		mwindow->clipwid = 404;
		mwindow->cliphgt = 72;
		mwindow->editob = TEXT_OB;
		mwindow->nextedit = FONTSIZE;
		mwindow->editx = 0;
	
		smurf_struct->wind_struct = mwindow;		/* und die Fensterstruktur in die Gargamel */
	
		/*
		 * Preview-Smurfpic-Struktur
		 */
		preview.pic_width = 416;
		preview.pic_height = 72;
		preview.depth = 1;
		preview.pic_data = smurf_struct->services->SMalloc( 416L*72L/8L);
		preview.screen_pic = NULL;
		preview.changed = 0;
		preview.format_type = FORM_STANDARD;
		preview.col_format = RGB;
		preview.zoom = 0;
		preview.block = NULL;
		preview.local_nct = NULL;
		preview.prev_picture = NULL;
		preview.next_picture = NULL;
		memset(preview.pic_data, 0, 416L*72L/8L);

		/*
		 * MFDB vorbereiten und Offscreen-Bitmap anmelden 
		 */
		offscreen.fd_addr = preview.pic_data;
		offscreen.fd_w = 416;
		offscreen.fd_h = 72;
		offscreen.fd_wdwidth = (416 + 15) / 16;
		offscreen.fd_stand = 0;
		offscreen.fd_nplanes = 1;
		memset(work_in, 0x0, 40);
		work_in[11] = offscreen.fd_w - 1;
		work_in[12] = offscreen.fd_h - 1;
		v_opnbm(work_in, &offscreen, &oshandle, work_out);
	
		/*
		 * Font-Voreinstellungen
		 */
		Goto_pos(1, 0);
		font.size = 24;
		vst_load_fonts(oshandle, 0);

		vst_color(oshandle, 1);
		vst_rotation(oshandle, 0);								/* unrotiert... */
		vswr_mode(oshandle, MD_REPLACE);
		vst_effects(oshandle, 0);
		vst_skew(oshandle, 0);
		vst_kern(oshandle, 0,1, &(int)dummy, &(int)dummy);		/* Pair-Kerning ein */

		vst_arbpt32(oshandle, (long)font.size*65536L, &char_width, &char_height, &cell_width, &cell_height);

		vst_alignment(oshandle, 0,5, &(int)dummy, &(int)dummy);
		
		font.ID = vst_font(oshandle, 0);
/*		printf("font.ID: %d\n", font.ID); */
		
		font_info.size = sizeof(XFNT_INFO);
		vqt_xfntinfo(oshandle, 0xFF, font.ID, 0, &font_info);
/*		printf("font_info.index: %d, font_info.id: %d\n", font_info.index, font_info.id); */
		font.index = font_info.index;
/*		printf("font.index: %d\n", font.index); */
	
		vqt_name(oshandle, font.index, name);
		strncpy(maintree[FONTNAME].ob_spec.tedinfo->te_ptext, name, 50);
	
		font.size = atoi(maintree[FONTSIZE].ob_spec.tedinfo->te_ptext);
		vst_arbpt32(oshandle, (long)font.size*65536L, &char_width, &char_height, &cell_width, &cell_height);

		strcpy(maintree[TEXT_OB].ob_spec.tedinfo->te_ptext, "The quick blue Smurf jumps over the lazy pic");
		strcpy(maintree[FONTSIZE].ob_spec.tedinfo->te_ptext, "24");

		back = get_window(mwindow);						/* Gib mir 'n Fenster! */
		if(back==-1)									/* keins mehr da? */
		{
			smurf_struct->module_mode=M_EXIT;
			smurf_struct->services->SMfree(mwindow);
			v_clsbm(oshandle);
			return;
		}
		else											/* doch? Ich warte... */
			smurf_struct->module_mode=M_WAITING;

		compute_preview();
		mwindow->picture = &preview;
		smurf_struct->module_mode=M_MODPIC;		

		return;
	}
	
	/*
	 * Buttonevent im Modulfenster
	 */
	else if(SmurfMessage==MBEVT)
	{
		Button=smurf_struct->event_par[0];
		
		if(Button==FONTNAME)
		{
			if(call_fontsel(oshandle, &font) > 0)
			{
				font.ID = vst_font(oshandle, font.ID);
/*				printf("font.ID: %d\n", font.ID); */

				font_info.size = 890;
				vqt_xfntinfo(oshandle, 0xFF, font.ID, 0, &font_info);
/*				printf("font_info.index: %d, font_info.id: %d\n", font_info.index, font_info.id);
				printf("Name lt. vqt_xfntinfo: %s\n", font_info.font_name); */
				font.index = font_info.index;
/*				printf("font.index: %d\n", font.index); */
				
				vqt_name(oshandle, font.index, name);
/*				printf("Name lt. vqt_name: %s\n", name); */
				strncpy(maintree[FONTNAME].ob_spec.tedinfo->te_ptext, name, 50);
				
				itoa(font.size, str, 10);
				strcpy(maintree[FONTSIZE].ob_spec.tedinfo->te_ptext, str);
				vst_arbpt32(oshandle, (long)font.size*65536L, &char_width, &char_height, &cell_width, &cell_height);
			
				compute_preview();
				mwindow->picture = &preview;
				redraw_window(mwindow, NULL, FONTNAME, 0);
				redraw_window(mwindow, NULL, FONTSIZE, 0);
				smurf_struct->module_mode=M_MODPIC;
			}
		}
		else if(Button==TEXT_START)
		{
			font.size = atoi(maintree[FONTSIZE].ob_spec.tedinfo->te_ptext);
			smurf_struct->module_mode=M_STARTED;			/* Ich will loslegen!... */
		}
		else if(Button==PREVIEW || Button==TEXT_OB || Button==FONTSIZE)
		{
			font.size = atoi(maintree[FONTSIZE].ob_spec.tedinfo->te_ptext);
			vst_arbpt32(oshandle, (long)font.size*65536L, &char_width, &char_height, 
									&cell_width, &cell_height);

			compute_preview();
			mwindow->picture = &preview;
			smurf_struct->module_mode=M_MODPIC;		
		}
		else smurf_struct->module_mode=M_WAITING;			/* Ich warte... */
	
		return;
	}

	/*
	 * Keyboardevent
	 */
	else if(SmurfMessage==MKEVT)
	{
		if(smurf_struct->event_par[0]==TEXT_OB || smurf_struct->event_par[0]==FONTSIZE)
		{
			if(smurf_struct->event_par[0]==FONTSIZE)
			{
				font.size = atoi(maintree[FONTSIZE].ob_spec.tedinfo->te_ptext);
				vst_arbpt32(oshandle, (long)font.size*65536L, &char_width, &char_height, 
										&cell_width, &cell_height);
			}

			compute_preview();
			mwindow->picture = &preview;
			smurf_struct->module_mode=M_MODPIC;
		}
		else if(smurf_struct->event_par[0]==TEXT_START)
		{
			font.size = atoi(maintree[FONTSIZE].ob_spec.tedinfo->te_ptext);
			smurf_struct->module_mode = M_STARTED;			/* Ich will loslegen!... */
		}
	}

	/*
	 * Preview fertig?
	 */
	else if(SmurfMessage==MDITHER_READY)
	{
		redraw_window(mwindow, NULL, PREVIEW, 0);
		smurf_struct->module_mode = M_WAITING;
	}

	/*
	 * Los gehts
	 */
	else if(SmurfMessage==MEXEC)
	{
		f_doit(smurf_struct, picture, &font);
		smurf_struct->module_mode=M_PICDONE;			/* Fertig!... */
	}

	/* 
	 * Mterm empfangen - Speicher freigeben und beenden
	 */
	else if(SmurfMessage==MTERM)
	{
		smurf_struct->services->SMfree(mwindow);
		smurf_struct->services->SMfree(preview.pic_data);
		vst_unload_fonts(oshandle, 0);
		v_clsbm(oshandle);
		smurf_struct->module_mode = M_EXIT;
		return;
	}
	else if(SmurfMessage == AES_MESSAGE)
	{
		if(handle_aesmsg(smurf_struct, &font)>0)
		{
			compute_preview();
			mwindow->picture = &preview;
			smurf_struct->module_mode = M_MODPIC;
		}
		else smurf_struct->module_mode = M_WAITING;
	}
	

	return;	
}




void f_doit(GARGAMEL *smurfstruct, SMURF_PIC *picture, FONT_INFO *font)
{
	int pxy[10];
	int extent[10], doithandle;
	MFDB osbmp;
	long size;
	long byte_width;
	int dummy;
	SMURF_PIC *destination;


	/*
	 * MFDB vorbereiten und Offscreen-Bitmap anmelden 
	 */
	vqt_real_extent(oshandle, 0,0, maintree[TEXT_OB].ob_spec.tedinfo->te_ptext, extent);

	byte_width = ((extent[2]+15)/16) *2;
	size = byte_width * (long)extent[5];
	osbmp.fd_addr = smurfstruct->services->SMalloc(size);
	osbmp.fd_w = ((extent[2]+15)/16)*16;
	osbmp.fd_h = extent[5];
	osbmp.fd_wdwidth = (extent[2]+15)/16;
	osbmp.fd_stand = 0;
	osbmp.fd_nplanes = 1;

	memset(work_in, 0, 40);
	memset(osbmp.fd_addr, 0, size);
	work_in[11] = osbmp.fd_w - 1;
	work_in[12] = osbmp.fd_h - 1;
	v_opnbm(work_in, &osbmp, &doithandle, work_out);

	/*
	 * Fonteinstellungen fÅr die neue osbmp Åbernehmen 
	 */
	vst_load_fonts(doithandle, 0);
	vst_font(doithandle, font->ID);
	vst_color(doithandle, 1);
	vst_rotation(doithandle, 0);							/* unrotiert... */
	vswr_mode(doithandle, MD_REPLACE);
	vst_effects(doithandle, 0);
	vst_skew(doithandle, 0);
	vst_kern(doithandle, 0,1, &dummy,&dummy);				/* Pair-Kerning ein */
	vst_alignment(doithandle, 0,5, &dummy, &dummy);

	vst_arbpt32(doithandle, (long)font->size*65536L, &char_width, &char_height, &cell_width, &cell_height);

	/*
	 * Clipping ein und rein mit dem Text
	 */
	pxy[0]=0;
	pxy[1]=0;
	pxy[2]=extent[4]-1;
	pxy[3]=extent[5]-1;
	vs_clip(doithandle, 1, pxy);
	
	v_ftext(doithandle, 0,0, maintree[TEXT_OB].ob_spec.tedinfo->te_ptext);

	if(picture->changed==255)					/* Åbergebenes Bild ist schon ein Block */
	{
		Mfree(picture->pic_data);
		destination = picture;
	}
	else if(picture->block!=NULL)				/* kein Block aber einer im Åbergebenen Bild */
	{
		Mfree(picture->block->pic_data);
		destination = picture->block;
	}
	else if(picture->block==NULL)				/* Block muû neu erstellt werden */
	{
		picture->block = smurfstruct->services->SMalloc(sizeof(SMURF_PIC));
		destination = picture->block;
	}

	destination->changed = 255;
	destination->pic_data = osbmp.fd_addr;
	destination->pic_width = osbmp.fd_w;
	destination->pic_height = osbmp.fd_h;
	destination->depth = 1;
	destination->block = NULL;
	destination->prev_picture = NULL;
	destination->next_picture = NULL;
	destination->screen_pic = NULL;
	destination->palette = malloc(1025);
	destination->format_type = FORM_STANDARD;
	destination->col_format = RGB;
	destination->zoom = 0;

	destination->palette[0]=255;
	destination->palette[1]=255;
	destination->palette[2]=255;
	destination->palette[3]=0;
	destination->palette[4]=0;
	destination->palette[5]=0;

	vst_unload_fonts(doithandle, 0);
	v_clsbm(doithandle);
}




void compute_preview(void)
{
	int pxy[10];

	pxy[0]=0;
	pxy[1]=0;
	pxy[2]=403;
	pxy[3]=71;

	/*
	 * Preview lîschen und Text reinschreiben
	 */
	memset(preview.pic_data, 0, 416L*72L/8L);
	vs_clip(oshandle, 1, pxy);
	v_ftext(oshandle, 0,0, maintree[TEXT_OB].ob_spec.tedinfo->te_ptext);
}



int handle_aesmsg(GARGAMEL *smurf_struct, FONT_INFO *font)
{
	int *msg;
	char name[40];
	char str[10];
	
	msg = smurf_struct->event_par;

	if(msg[0]==0x7A18)				/* FONT_CHANGED */
	{
		if(msg[4])
		{
			font->ID = msg[4];
			font->ID = vst_font(oshandle, font->ID);

			font_info.size = 890;
			vqt_xfntinfo(oshandle, 0xFF, font->ID, 0, &font_info);
			font->index = font_info.index;
		
			vqt_name(oshandle, font->index, name);
			strncpy(maintree[FONTNAME].ob_spec.tedinfo->te_ptext, name, 50);
		}

		if(msg[5])
		{
			font->size = msg[5];
			itoa(font->size, str, 10);
			strcpy(maintree[FONTSIZE].ob_spec.tedinfo->te_ptext, str);
			vst_arbpt32(oshandle, (long)font->size*65536L, &char_width, &char_height, &cell_width, &cell_height);
		}

		redraw_window(mwindow, NULL, FONTNAME, 0);
		redraw_window(mwindow, NULL, FONTSIZE, 0);
		return(1);
	}

	return(0);
}