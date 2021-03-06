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
/*							TRANSFM.C								*/
/*	Dialog "Bild wandeln" zum destruktiven expandieren oder redu-	*/
/*	zieren eines Bildes. Benutzt werden die CONVERT.C-Routinen		*/
/*	�ber eine MOD_ABILITY-Struktur.									*/
/*		05.06.1997 Olaf												*/
/*-----------------------------------------------------------------	*/

#include <stdio.h>
#include <tos.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "popdefin.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "smurf.h"

#include "smurfobs.h"
#include "ext_obs.h"


/*------------ externe Funktionen ------------------*/
	extern	int loadNCT(int loadplanes, SYSTEM_INFO *sysinfo);
	extern	int export_dither_dispatcher(SMURF_PIC *dest, SYSTEM_INFO *sys_info, int dest_colsys, DISPLAY_MODES *display, 
										 int *fixpal_red, int *fixpal_green, int *fixpal_blue);


/*------------ lokale Funktionen ------------------*/
	void actualize_convpopups(int dither_algo, int depth, int pal);
	void do_transform(int conv_depth, int conv_dither, int conv_pal);

	int dither_destruktiv(int dest_depth, int dest_dither, int dest_pal);
	int autoreduce_image(void);

/*------------ externer Kram ----------------------*/
	extern	DISPLAY_MODES Display_Opt;
	extern 	SYSTEM_INFO Sys_info;
	extern	WINDOW wind_s[25];
	extern	WINDOW picture_windows[MAX_PIC];
	extern	SMURF_PIC *smurf_picture[MAX_PIC];
	extern	DITHER_MOD_INFO *ditmod_info[10];
	extern	int klicks;
	extern	int key_at_event, key_scancode, mouse_xpos, mouse_ypos, mouse_button;
	extern	int active_pic;
	extern	POP_UP	popups[25];
	extern	int openmode;		/* Dialog neu ge�ffnet (0) / Buttonevent? (!=0) */
	extern	OBJECT	*colred_popup;
	extern	OBJECT 	*col_pop;					/* Zeiger auf Resource-DITHERPOPUP	*/

	extern	MOD_ABILITY export_mod_ability;
	extern	long f_len;

/*------------ globales f�r die Konvertierung ------*/
	int conv_depth = 24, conv_dither = DIT1, conv_pal = CR_SYSPAL;

	int tfm_fix_red[256], tfm_fix_blue[256], tfm_fix_green[256];



void transform_pic(void)
{
	static char	tfm_filepal_name[32] = "feste Palette";
	char *name, str[4];
	char *pal_loadpath;

	int button, popbut, enablemode;
	int dbutton, back;

	OBJECT *resource;

	extern char *load_palfile(char *path, int *red, int *green, int *blue, int max_cols);
	extern OBJECT *form_pop;						/* Zeiger auf Resource	*/


	button = Dialog.init(WIND_TRANSFORM, CONV_START);

	resource = wind_s[WIND_TRANSFORM].resource_form;

	if(!openmode || openmode == 2)
	{
		conv_depth = smurf_picture[active_pic]->depth;
		switch(conv_depth)
		{
			case 1:	dbutton = DEPTH1; break;
			case 2:	dbutton = DEPTH2; break;
			case 3:	dbutton = DEPTH4; break;
			case 4:	dbutton = DEPTH4; break;
			case 5:	dbutton = DEPTH8; break;
			case 6:	dbutton = DEPTH8; break;
			case 7:	dbutton = DEPTH8; break;
			case 8:	dbutton = DEPTH8; break;
			case 9:	dbutton = DEPTH16; break;
			case 16:	dbutton = DEPTH16; break;
			case 24:	dbutton = DEPTH24; break;
		}

		popups[POPUP_CONVDEPTH].item = dbutton;
		strcpy(resource[CONV_DEPTHPB].TextCast, form_pop[dbutton].TextCast);
		change_object(&wind_s[WIND_TRANSFORM], CONV_DEPTHPB, UNSEL, 1);
		actualize_convpopups(conv_dither, conv_depth, conv_pal);

		if(dbutton == DEPTH1)
			conv_depth = 1;
		else
			if(dbutton == DEPTH2)
				conv_depth = 2;
			else
				if(dbutton == DEPTH4)
					conv_depth = 4;
				else
					if(dbutton == DEPTH8)
						conv_depth = 8;
					else
						if(dbutton == DEPTH16)
							conv_depth = 16;
						else
							if(dbutton == DEPTH24)
								conv_depth = 24;
	}

	switch(button)
	{
		case CONV_DEPTHPB:
		case CONV_DEPTHCB:	popbut = f_pop(&popups[POPUP_CONVDEPTH], 0, button, NULL);
							f_deselect_popup(&wind_s[WIND_TRANSFORM], CONV_DEPTHCB, CONV_DEPTHPB);

							switch(popbut)
							{
								case DEPTH1:	conv_depth = 1;
												break;
								case DEPTH2:	conv_depth = 2;
												break;								
								case DEPTH4:	conv_depth = 4;
												break;
								case DEPTH8:	conv_depth = 8;
												break;
								case DEPTH16:	conv_depth = 16;
												break;
								case DEPTH24:	conv_depth = 24;
												break;
								default:		return;
							}
									
							actualize_convpopups(conv_dither, conv_depth, conv_pal);

							if(conv_dither > 0 && conv_depth <= smurf_picture[active_pic]->depth && conv_depth != 16 && conv_depth != 24)
							{
								if(ditmod_info[conv_dither - 1]->pal_mode == FIXPAL)
								{
									conv_pal = CR_FIXPAL;
									popups[POPUP_CONVPAL].item = conv_pal;
									strncpy(resource[CONV_PALPB].TextCast, colred_popup[conv_pal].TextCast, 15);
									enablemode = DISABLED;
									change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, DISABLED, 1);
								}
								else 
								{
									if(conv_depth == 1)
									{
										strcpy(resource[CONV_PALPB].TextCast, "s/w");
										enablemode = DISABLED;
										change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, DISABLED, 1);
									}
									else
									{
										conv_pal = CR_SYSPAL;
										popups[POPUP_CONVPAL].item = conv_pal;
										strncpy(resource[CONV_PALPB].TextCast, colred_popup[conv_pal].TextCast, 15);
										enablemode = ENABLED;
										change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, DISABLED, 1);
									}
								}

								change_object(&wind_s[WIND_TRANSFORM], CONV_PALPB, enablemode, 1);
								change_object(&wind_s[WIND_TRANSFORM], CONV_PALCB, enablemode, 1);
							}

							break;

		case CONV_DITHPB:
		case CONV_DITHCB:	popbut = f_pop(&popups[POPUP_CONVDITHER], 0, button, NULL);
							f_deselect_popup(&wind_s[WIND_TRANSFORM], CONV_DITHCB, CONV_DITHPB);

							if(popbut > 0)
							{
								conv_dither = popbut;

								if(ditmod_info[conv_dither-1]->pal_mode == FIXPAL)
								{
									conv_pal = CR_FIXPAL;
									strncpy(resource[CONV_PALPB].TextCast, colred_popup[conv_pal].TextCast, 15);
									enablemode = DISABLED;
									change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, DISABLED, 1);
								}
								else 
								{
									conv_pal = CR_SYSPAL;
									popups[POPUP_CONVPAL].item = CR_SYSPAL;
									strncpy(resource[CONV_PALPB].TextCast, colred_popup[conv_pal].TextCast, 15);
									enablemode = ENABLED;
									change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, DISABLED, 1);
								}
								
								if(conv_depth == 1)
								{
									strcpy(resource[CONV_PALPB].TextCast, "s/w");
									enablemode = DISABLED;
									change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, DISABLED, 1);
								}
								
								change_object(&wind_s[WIND_TRANSFORM], CONV_PALPB, enablemode, 1);
								change_object(&wind_s[WIND_TRANSFORM], CONV_PALCB, enablemode, 1);
							}

							break;

		case CONV_PALPB:
		case CONV_PALCB:	strcpy(colred_popup[CR_FILEPAL].TextCast, tfm_filepal_name);
							popbut = f_pop(&popups[POPUP_CONVPAL], 0, button, NULL);
							if(popbut > 0)
								conv_pal = popbut;

							if(conv_pal == CR_FILEPAL)
								change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, ENABLED, 1);
							else
								change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, DISABLED, 1);

							f_deselect_popup(&wind_s[WIND_TRANSFORM], CONV_PALCB, CONV_PALPB);
							break;
		
		case CONV_START:	change_object(&wind_s[WIND_TRANSFORM], CONV_START, UNSEL, 1);
							Dialog.close(WIND_TRANSFORM);
							do_transform(conv_depth, conv_dither, conv_pal);
							f_pic_changed(&picture_windows[active_pic], 1);
							break;
								
		/*--------------------- feste Palette laden -----------------------*/
		case TFM_LOADPAL:	pal_loadpath = load_palfile(Sys_info.standard_path, tfm_fix_red, tfm_fix_green, tfm_fix_blue, (1<<conv_depth));

							if(pal_loadpath != NULL)
							{
								strcpy(tfm_filepal_name, strrchr(pal_loadpath, '\\')+1);
								strcpy(colred_popup[CR_FILEPAL].TextCast, tfm_filepal_name);
								strcpy(resource[CONV_PALPB].TextCast, tfm_filepal_name);
							}

							change_object(&wind_s[WIND_TRANSFORM], CONV_PALPB, UNSEL, 1);
							change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, UNSEL, 1);
							break;
	

		case TRANSFM_REDUCE:	change_object(&wind_s[WIND_TRANSFORM], TRANSFM_REDUCE, UNSEL, 1);
								Dialog.close(WIND_TRANSFORM);
								back = autoreduce_image();
								if(back == M_MEMORY)
									Dialog.winAlert.openAlert(Dialog.winAlert.alerts[DIT_NOMEM].TextCast, NULL, NULL, NULL, 1);
								break;

		default:			/*----------------- Name und Farbtiefe des Bildes einf�gen --------*/
							name = resource[CONV_PICNAME].TextCast;
							strncpy(name, picture_windows[active_pic].wtitle + 12, 24);
							name[23] = 0;
							itoa(smurf_picture[active_pic]->depth, str, 10);
							strcpy(resource[CONV_SRCDEPTH].TextCast, str);
							Window.redraw(&wind_s[WIND_TRANSFORM], NULL, CONV_OLDPIC, 0);
							break;
	}

	return;
}


/**************************************************************	*/
/*			Aktualisieren der Popups im Wandeln-Dialog			*/
/**************************************************************	*/
void actualize_convpopups(int dither_algo, int depth, int pal)
{
	int mode_pal, mode_dit, mode_load;

	OBJECT *res;


	res=wind_s[WIND_TRANSFORM].resource_form;

	if(ditmod_info[dither_algo-1]->pal_mode == FIXPAL)
	{
		conv_pal = CR_FIXPAL;
		strncpy(res[CONV_PALPB].TextCast, colred_popup[CR_FIXPAL].TextCast, 15);
		mode_pal = DISABLED;
		mode_dit = ENABLED;
		mode_load = DISABLED;
	}

	if(depth>smurf_picture[active_pic]->depth || depth==16 || depth==24)
	{
		strcpy(res[CONV_PALPB].TextCast, "-");
		strcpy(res[CONV_DITHPB].TextCast, "-");
		mode_pal = DISABLED;
		mode_dit = DISABLED;
		mode_load = DISABLED;
	}
	else
		if(depth <= smurf_picture[active_pic]->depth)
		{
			strcpy(res[CONV_DITHPB].TextCast, col_pop[dither_algo].TextCast);
			mode_dit = ENABLED;

			if(ditmod_info[dither_algo-1]->pal_mode != FIXPAL)
			{
				strcpy(res[CONV_PALPB].TextCast, colred_popup[pal].TextCast);
				mode_pal = ENABLED;
			}

			if(pal == CR_FILEPAL)
				mode_load = ENABLED;
			else
				mode_load = DISABLED;
		}

	if(depth == 1)
	{
		strcpy(res[CONV_PALPB].TextCast, "s/w");
		mode_pal = DISABLED;
	}

	change_object(&wind_s[WIND_TRANSFORM], CONV_PALPB, mode_pal, 1);
	change_object(&wind_s[WIND_TRANSFORM], CONV_PALCB, mode_pal, 1);
	change_object(&wind_s[WIND_TRANSFORM], CONV_DITHPB, mode_dit, 1);
	change_object(&wind_s[WIND_TRANSFORM], CONV_DITHCB, mode_dit, 1);
	change_object(&wind_s[WIND_TRANSFORM], TFM_LOADPAL, mode_load, 1);

	return;
}


/**************************************************************	*/
/*						Bild wandeln							*/
/**************************************************************	*/
void do_transform(int conv_depth, int conv_dither, int conv_pal)
{
	char picdepth;

	MOD_ABILITY abs;


	memset(&abs, 0x0, sizeof(MOD_ABILITY));				/* l�schen... */

	abs.depth1 = conv_depth;

	if(conv_depth < 8)
		abs.form1 = FORM_STANDARD;
	else
		abs.form1 = FORM_PIXELPAK;

	picdepth = smurf_picture[active_pic]->depth;

	/*
	 * 24 nach 16 Bit
	 */
	if(picdepth == 24 && conv_depth == 16)
		tfm_24_to_16(smurf_picture[active_pic], SAME);
	else
		/* raufw�rts ... */
		if(conv_depth > picdepth || (conv_depth==24 && picdepth==24))
			f_convert(smurf_picture[active_pic], &abs, RGB, SAME, 0);
		else
			/* ... und runterw�rts */
			dither_destruktiv(conv_depth, conv_dither, conv_pal);

	return;
} /* do_transform */


/**************************************************************	*/
/*		Ditherdispatcher f�r destruktives Dithern 				*/
/*	Der einzige Teil von Smurf, der nicht konstruktiv ist ...	*/
/**************************************************************	*/
int dither_destruktiv(int dest_depth, int dest_dither, int dest_pal)
{
	char *dest_palette,
		 dest_form;

	int t;

	SMURF_PIC *convpic;
	MOD_ABILITY old_export_modabs;
	DISPLAY_MODES new_display;
	SYSTEM_INFO	Cheat_sysinfo;
	
	extern EXPORT_CONFIG exp_conf;


	convpic = smurf_picture[active_pic];
	
	if(dest_depth < 8)
		dest_form = FORM_STANDARD;
	else
		dest_form = FORM_PIXELPAK;

	/*----------- neue Strukturen zum Dithern generieren bzw. alte retten -----------*/
	memcpy(&old_export_modabs, &export_mod_ability, sizeof(MOD_ABILITY));
	memcpy(&new_display, &Display_Opt, sizeof(DISPLAY_MODES) );
	memcpy(&Cheat_sysinfo, &Sys_info, sizeof(SYSTEM_INFO));

	/*----- sind wir in einer anderen Farbtiefe (mu� eine andere NCT geladen werden)? -----*/
	if(dest_pal == CR_SYSPAL && dest_depth != Sys_info.bitplanes)
	{
		if(loadNCT(dest_depth, &Cheat_sysinfo) != 0)
		{
			Dialog.winAlert.openAlert("Kein NCT-File f�r die gew�hlte Farbtiefe gefunden. Dithering nicht m�glich.", NULL, NULL, NULL, 1);
			return(-1);
		}
	}

	/*---------------- neue Strukturen zurechtbeschei�en -----*/
	Cheat_sysinfo.bitplanes = dest_depth;
	Cheat_sysinfo.Max_col = (1 << dest_depth) - 1;

	memset(&export_mod_ability, 0x0, sizeof(MOD_ABILITY));
	export_mod_ability.depth1 = dest_depth;
	export_mod_ability.form1 = dest_form;

	exp_conf.exp_depth = dest_depth;
	exp_conf.exp_form = dest_form;

	new_display.syspal_4 = dest_pal;
	new_display.syspal_8 = dest_pal;
	new_display.syspal_24 = dest_pal;
	new_display.dither_4 = dest_dither;
	new_display.dither_8 = dest_dither;
	new_display.dither_24 = dest_dither;

	/* destruktiv dithern */
	export_dither_dispatcher(convpic, &Cheat_sysinfo, RGB, &new_display, tfm_fix_red, tfm_fix_green, tfm_fix_blue);

	/*---------------- Bilddaten aus dem MFDB �bertragen */
	SMfree(convpic->pic_data);
	convpic->pic_data = convpic->screen_pic->fd_addr;
	convpic->depth = Cheat_sysinfo.bitplanes;
	convpic->format_type = dest_form;
	
	free(convpic->screen_pic);		/* MFDB freigeben, um das Dithering */
	convpic->screen_pic = NULL;		/* problemlos zu erm�glichen */

	/*---------------- Palette �bertragen */
	dest_palette = convpic->palette;
	memset(dest_palette, 0x0, 1024);
	for(t = 0; t < 256; t++)
	{
		*dest_palette++ = (char)convpic->red[t];
		*dest_palette++ = (char)convpic->grn[t];
		*dest_palette++ = (char)convpic->blu[t];
	}

	/*---------------- und jetzt noch konvertieren, dann sind wir fertig. */
	/* Wird wohl in Zukunft wegfallen k�nnen */
	/* f_convert(convpic, mod_abs, dest_colsys, SAME, 0); */

	/*---------- alles aus der Cheat_sysinfo wegwerfen, -----*/
	/*---------- was in der load_nct angefordert wurde ------*/
	if(dest_pal == CR_SYSPAL && dest_depth != Sys_info.bitplanes)
	{
		free(Cheat_sysinfo.red);
		free(Cheat_sysinfo.grn);
		free(Cheat_sysinfo.blu);
		free(Cheat_sysinfo.plane_table);
		SMfree(Cheat_sysinfo.nc_table);
	}

	/* und jetzt Screendisplay neu dithern. */
	if(!Sys_info.realtime_dither)
		f_dither(convpic, &Sys_info, 0, NULL, &Display_Opt);

	Window.redraw(&picture_windows[active_pic], NULL, 0, 0);

	Dialog.busy.ok();

	return(0);
}



int autoreduce_image(void)
{
	char *rt, *gt, *bt, *data;
	char R, G, B;

	int idx, *intdata;
	int depth, found = 1, ddepth;

	long pixlen, t, colcount = 0, find = 0;

	SMURF_PIC *convpic;


	/* zu aller erst mal die Farben im Bild z�hlen */
	rt = SMalloc(32769L);
	if(rt == NULL)
		return(M_MEMORY);

	gt = SMalloc(32769L);
	if(gt == NULL)
	{
		SMfree(rt);
		return(M_MEMORY);
	}

	bt = SMalloc(32769L);
	if(bt == NULL)
	{
		SMfree(rt);
		SMfree(gt);
		return(M_MEMORY);
	}

	Dialog.busy.reset(0, "z�hle Farben...");

	convpic = smurf_picture[active_pic];
	pixlen = (long)convpic->pic_width*(long)convpic->pic_height;
	depth = convpic->depth;
	intdata = (int*)data = convpic->pic_data;

	for(t = 0; t < pixlen; t++)
	{
		if(!(t&2047))
			Dialog.busy.draw((int)((long)t * 128L / pixlen));
		if(convpic->format_type==FORM_PIXELPAK)
		{
			/*--------- RGB-Werte holen----- */
			switch(depth)
			{
				case 24:	R = *data++;
							G = *data++;
							B = *data++;
							break;

				case 16:	idx = *intdata++;
							R = idx >> 11;
							G = (idx >> 6)&31;
							B = idx&31;
							break;

				case 8:		idx = *data++;
							R = *(convpic->palette + idx + idx + idx);
							G = *(convpic->palette + idx + idx + idx+1);
							B = *(convpic->palette + idx + idx + idx+2);
							break;
			}

			/*----------- Farbe in der Tabelle suchen */
			found = 0;
			find = colcount;
			while(find--)
			{
				if(rt[find] == R && gt[find] == G && bt[find] == B)
				{
					found=1;
					break;
				}
			}

			if(found == 0)		/* NICHT gefunden? -> Eintragen und Farbz�hler erh�hen */
			{
				colcount++;
				rt[colcount] = R;
				gt[colcount] = G;
				bt[colcount] = B;
			}
		}

		if(colcount > 32768L)
			break;
	}

/*	printf("\n %li Farben", colcount);*/
	
	/* Such-Tabellen freigeben */
	SMfree(rt);
	SMfree(gt);
	SMfree(bt);

	/* Bild reduzieren */
	if(colcount > 32768L)
		return(0);
	else
		if(colcount <= 2)
			ddepth = 1;
		else
			if(colcount <= 4)
				ddepth = 2;
			else
				if(colcount <= 16)
					ddepth = 4;
				else
					if(colcount <= 256)
						ddepth = 8;
					else
						if(colcount <= 32768L)
						{
							if(smurf_picture[active_pic]->depth == 24)
								if(Dialog.winAlert.openAlert("Das Bild kann nach 16 Bit Farbtiefe gewandelt werden. Folge kann aber ein Qualit�tsverlust sein!", "Abbruch", " OK ", NULL, 1) == 2)
									tfm_24_to_16(smurf_picture[active_pic], NEW);
		return(0);
	}

	if(ddepth != depth)
		dither_destruktiv(ddepth, DIT1, CR_MEDIAN);
	
	return(0);
}