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

#include <tos.h>
#include <multiaes.h>
#include <vdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <screen.h>
#include <string.h>
#include <ext.h>
/*#include <time.h>*/
#include "smurf.h"
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "popdefin.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "plugin\plugin.h"

#include "xrsrc.h"
#include "globdefs.h"

#include "smurfobs.h"
#include "ext_obs.h"

void realtime_dither(GRECT *picbox, WINDOW *window, int *pxy, int *vdiclip, int stripheight);
extern void f_display_bwh(WINDOW *pic_window);

extern	MFORM	*dummy_ptr;				/* Dummymouse fÅr Maus-Form */

extern	int picwindthere, dialwindthere, picthere;
extern	OBJECT *alert_form;				/* WindAlert-Formular	*/
extern	int *messagebuf;				/* Zeiger fÅr Messageevents */

extern int add_flags[40];

extern	SERVICE_FUNCTIONS global_services;

extern	WINDOW wind_s[25];
extern	WINDOW picture_windows[MAX_PIC];

extern	int openmode;					/* Dialog neu geîffnet (0) oder buttonevent? (!=0) */
extern int active_pic;

extern	OBJECT *pic_form;				/* Pic-Window-Formular	*/
extern	OBJECT *u_tree;					/* Zeiger auf Radiobutton/Checkbox-Formular	*/

extern	GRECT	screen;					/* globales Screen-GRECT */

extern	int resource_global[100];

extern	SYSTEM_INFO Sys_info;			/* Systemkonfiguration */
extern	DISPLAY_MODES Display_Opt;

extern char Startup;					/* hochfahren des ganzen Monster-Systems */

extern	CROSSHAIR position_markers[20];
extern	signed char	module_pics[21][7];

void display_windowlist(WINDOW *window);
void draw_iconified(WINDOW *window, int *vdiclip);
void insert_picwinzoom(WINDOW *window);

void draw_picmanboxes(void);

void scrollWindowRT(WINDOW *window, int xamount, int yamount);

char alerttok(char *text, char maxlen);


/* ----------------------------------------------------------------	*/
/* 						Fenster îffnen								*/
/* ôffnet oder toppt Fenster, behandelt Bildfenster und Strukturen	*/
/* selbstÑndig, clippt Fenster auf Bildschirmkoordinaten und fÅhrt	*/
/* eine Erstinitialisierung eines eventuellen Editobjekts durch.	*/
/* ----------------------------------------------------------------	*/
int f_open_window(WINDOW *window)
{
	int m_whandle;
	int m_wind_x, m_wind_y, m_wind_w, m_wind_h;
	int flags, dummy;
	int abx, aby, abw, abh;


	m_wind_x = window->wx;
	m_wind_y = window->wy;
	m_wind_w = window->ww;
	m_wind_h = window->wh;
	m_whandle = window->whandlem;

	/* Doppelbedingung notwendig da der if sonst auch auf Bildfenster Nr. 6 anschlÑgt */
	if(!window->pflag && window->wnum == WIND_ALERT)
		flags = NAME|MOVER;
	else
		flags = CLOSER|NAME|MOVER|SMALLER;

	if(window->pflag && window->picture != NULL) 
		flags |= FULLER|SIZER|UPARROW|DNARROW|LFARROW|RTARROW|VSLIDE|HSLIDE;
	else
		if(!window->pflag)
			flags |= add_flags[window->wnum];


	/*---- kein Bildfenster? */
	if(!window->pflag)
	{
		/*------ Systempalette einstellen */
		if(!Startup)
			f_set_syspal();

		/*---- Fenster neu, Dialoge zentrieren an? */
		if(m_whandle <= 0 && Sys_info.center_dialog&SELECTED)
		{
			if((window->wnum != WIND_BUSY) && (Sys_info.dialog_opened[window->wnum] == 0))
				form_center(window->resource_form, &m_wind_x,&m_wind_y,&m_wind_w,&m_wind_h);
		}

		/*---- Dialoge zentrieren aus oder Startup? */
		if((!(Sys_info.center_dialog&SELECTED) || Startup == 1) && window->wnum != WIND_ALERT)
		{
			m_wind_x = Sys_info.dialog_xposition[window->wnum];
			m_wind_y = Sys_info.dialog_yposition[window->wnum];
		}
	}

	if(window->wnum == WIND_MODFORM || window->wnum == WIND_PICORDER ||
	   window->wnum == WIND_ALERT || window->wnum == WIND_PICINFO)
		Sys_info.dialog_opened[window->wnum] = 0;
	else
		if(!window->pflag && !window->module)
			Sys_info.dialog_opened[window->wnum] = 1;

	/*---------------- Fensterkoordinaten prÅfen und anpassen */
	Window.windGet(0, WF_WORKXYWH, &abx, &aby, &abw, &abh);
	abh += aby;

	/* aus Nettokoordinaten Bruttokoordinaten machen */
	wind_calc(WC_BORDER, flags, m_wind_x,m_wind_y,m_wind_w,m_wind_h, &m_wind_x,&m_wind_y,&m_wind_w,&m_wind_h);

	if(!window->pflag)
	{
		if(m_wind_x + m_wind_w > abw)
			m_wind_x = abw - m_wind_w;

		if(m_wind_y + m_wind_h > abh)
			m_wind_y = abh - m_wind_h;
	}
	else
	{
		if(m_wind_x + m_wind_w > abw)
			m_wind_w = abw - m_wind_x;

		if(m_wind_y + m_wind_h > abh)
			m_wind_h = abh - m_wind_y;
	}

	/* erst hier, weil obige Berechnungen negative Werte ergeben kînnen */
	if(m_wind_x < abx)
		m_wind_x = abx;

	if(m_wind_y < aby)
		m_wind_y = aby;

	/* Nettokoordinaten in resource_form speichern */
	wind_calc(WC_WORK, flags, m_wind_x,m_wind_y,m_wind_w,m_wind_h, &window->resource_form->ob_x,&window->resource_form->ob_y, &dummy, &dummy);

	/* ist das gewÅnschte Fenster noch zu? */
	if(m_whandle <= 0)
	{
		window->whandlem = wind_create(flags, m_wind_x, m_wind_y, m_wind_w, m_wind_h);
		
		if(window->whandlem < 0) 
		{
			form_alert(1, "[0][Keine Fenster mehr frei!][ Oh ]"); 
			return(-1);
		}

		if(Sys_info.OSFeatures&BEVT)	
			Window.windSet(window->whandlem,WF_BEVENT,1,0,0,0);

		Window.windSet(window->whandlem, WF_NAME, LONG2_2INT((long)window->wtitle), 0,0);

		if(add_flags[window->wnum]&INFO) 
		{
			if(window->wnum == WIND_MODULES) 
				Window.windSet(window->whandlem, WF_INFO, LONG2_2INT((long)Dialog.emodList.modList.autolocator), 0,0);
			else
				if(window->wnum == WIND_EXPORT) 
					Window.windSet(window->whandlem, WF_INFO, LONG2_2INT((long)Dialog.expmodList.modList.autolocator), 0,0);
		}

		wind_open(window->whandlem, m_wind_x, m_wind_y, m_wind_w, m_wind_h);

		if(window->resource_form != alert_form)
			Window.windowToList(window);
		
		window->fullx = -1;
		window->shaded = 0;

		if(window->pflag && Display_Opt.palette_mode == PAL_TOPWIN)
			f_set_picpal(window->picture);

	}
	else
		Window.top(window->whandlem);

	if(window->pflag == 0)
		Dialog.topDialog = window->whandlem;
	else
		imageWindow.topPicwin = window->whandlem;

	/* wieder Nettokoordinaten des Fensters holen */
	Window.windGet(window->whandlem, WF_WORKXYWH, &m_wind_x, &m_wind_y, &m_wind_w, &m_wind_h);

	/* Werte in Struktur aktualisieren */
	window->wx = m_wind_x;
	window->wy = m_wind_y;
	window->ww = m_wind_w;
	window->wh = m_wind_h;

	window->resource_form->ob_x = m_wind_x;
	window->resource_form->ob_y = m_wind_y;

	Sys_info.dialog_xposition[window->wnum] = m_wind_x;
	Sys_info.dialog_yposition[window->wnum] = m_wind_y;

	actualize_menu();

	return(0);
} /* f_open_window */


/*------------------------ Fenster neu zeichnen -------------------	*/
/* DurchlÑuft die Rechteckliste und fÅhrt einen kompletten			*/
/* Fensterredraw anhand dieser durch.								*/
/* Wenn das Fenster ein Bild enthÑlt, in dem keine 					*/
/* Bildschirmdarstellung enthalten ist, wird eine den Ausschnitten	*/
/* entsrpechende Darstellung online erzeugt und redrawed.			*/
/* Der Ditherpuffer (MFDB), der die Abmessungen des Ausschnittes	*/
/* hat, wird danach wieder freigegeben.								*/
/*																	*/
/* Der Grund, das alles hier einzubauen ist, daû es dann sofort mit	*/
/* allen Fenstern funktionieren mÅûte. Nur das vorherige Dithern	*/
/* des kompletten Bildes muû dann noch an den entsprechenden Stellen*/
/* ausgeschaltet werden, wenn der Online-Dither eingeschaltet ist.	*/
/*-----------------------------------------------------------------	*/
void f_redraw_window(WINDOW *window, GRECT *mwind, int startob, int flags)
{
	int	clip[10];								/* Clipping PXYARRAY... */
	int	vdiclip[10];							/* Clipping PXYARRAY... */
	int	pxy[10];								/* Clipping PXYARRAY... */
	int Colindex[3] = {1,0,0};
	int endwid, endhgt;
	int tophandle;
	int online = 0, dummy;
	int wind_x, wind_y, wind_w, wind_h;

	MFDB	m_screen;
	MFDB	*disp_pic;
	OBJECT	*tree;
	GRECT	picbox, box, work;


	if(window == NULL || window->whandlem <= 0)
		return;

	wind_x = window->wx;
	wind_y = window->wy;
	wind_w = window->ww;
	wind_h = window->wh;

	if(window->pflag)
	{
		window->resource_form->ob_x = wind_x;
		window->resource_form->ob_y = wind_y;
		window->resource_form->ob_width = wind_w;
	}

	tree = window->resource_form;
	if(mwind == NULL)
		mwind = &screen;	

	work.g_x = mwind->g_x;
	work.g_y = mwind->g_y;
	work.g_w = mwind->g_w;
	work.g_h = mwind->g_h;
	Window.windGet(window->whandlem, WF_FIRSTXYWH, &box.g_x, &box.g_y, &box.g_w, &box.g_h);

	if(!(flags&DRAWNOPICTURE))
		graf_mouse(M_OFF, dummy_ptr);
	if(wind_update(BEG_UPDATE) == 0)
		return;

	Window.cursorOff(window);

	while(box.g_w > 0 && box.g_h > 0)
	{
		if(Window.rcIntersect(&work, &box, &box))
		{
			clip[0] = box.g_x;
			clip[1] = box.g_y;
			clip[2] = box.g_w;
			clip[3] = box.g_h;

			vdiclip[0] = clip[0];
			vdiclip[1] = clip[1];
			vdiclip[2] = clip[2] + vdiclip[0] - 1;
			vdiclip[3] = clip[3] + vdiclip[1] - 1;

			if(window->shaded&ICONIFIED)
				Window.drawIconified(window, vdiclip);
			else 
			{
				if(tree != NULL && !(flags&DRAWNOTREE))			/* OBJECT-Baum */
				{
					if(window->pflag)							/* Bildfenster? */
					{
						imageWindow.insertZoom(window);
						if(window->wnum != active_pic)
							tree[0].ob_state |= SELECTED;		/* nicht aktiv ... */
						else
							tree[0].ob_state &= ~SELECTED;		/* ... oder aktiv? */

						if(window->picture->blockwidth || window->picture->blockheight)
						{
							itoa(window->picture->blockwidth, tree[PW_WID].TextCast, 10);
							itoa(window->picture->blockheight, tree[PW_HGT].TextCast, 10);
						}
					}

					vs_clip(Sys_info.vdi_handle, 1, vdiclip);
					objc_draw(tree, startob, MAX_DEPTH, clip[0],clip[1],clip[2],clip[3]);
					vs_clip(Sys_info.vdi_handle, 0, vdiclip);
				}

				/*
				 * Bild redrawen
				 */
				if(window->picture != NULL && !(flags&DRAWNOPICTURE))					/* SMURF_PIC-Bild */
				{
					endwid = window->picture->pic_width / (window->picture->zoom + 1);
					endhgt = window->picture->pic_height / (window->picture->zoom + 1);

					/*
					 * liegt das Bild Åberhaupt im angeforderten Rechteck? 
					 */
					picbox.g_x = wind_x + window->pic_xpos;
					picbox.g_y = wind_y + window->pic_ypos;
					picbox.g_w = window->clipwid;
					picbox.g_h = window->cliphgt;

					/* 
					 * Nach der rcIntersect-Operation liegt in <picbox> das Rechteck, das
					 * wirklich redrawed werden muû, in absoluten Bildschirmkoordinaten.
					 */
					if(Window.rcIntersect(&box, &picbox, &picbox))		/* fÅr Echtzeitdither auch in Bildfenstern! */
					{
						m_screen.fd_addr = NULL;
						disp_pic = window->picture->screen_pic;

						if(window->picture->changed == 0x80)
							pxy[0] = pxy[1] = 0;
						else
						{
							pxy[0] = window->xoffset;
							pxy[1] = window->yoffset;
						}

						pxy[2] = endwid - 1;
						pxy[3] = endhgt - 1;
						pxy[4] = wind_x + window->pic_xpos;
						pxy[5] = wind_y + window->pic_ypos;
						pxy[6] = pxy[4] + wind_w - 1;
						pxy[7] = pxy[5] + wind_h - 1;

						/*
						 * evtl. Clipping anpassen 
						 */
						if(!window->pflag)
						{
							vdiclip[0] = picbox.g_x;
							vdiclip[1] = picbox.g_y;
							vdiclip[2] = vdiclip[0] + picbox.g_w - 1;
							vdiclip[3] = vdiclip[1] + picbox.g_h - 1;
						}
						else
						{
							if(vdiclip[1] < wind_y + TOOLBAR_HEIGHT - 1)
								vdiclip[1] = wind_y + TOOLBAR_HEIGHT - 1;
						}
						
						/*
						 * Wenn es keine Bildschirmdarstellung gibt, muû ich mir hier
						 * fÅr das aktuelle Clippingrechteck eine erzeugen. 
						 */
						if(!(flags&BLOCK_ONLY))
						{
							if(disp_pic == NULL || disp_pic->fd_addr == NULL)
							{
								online = 1;
								realtime_dither(&picbox, window, pxy, vdiclip, 64);
/*								disp_pic = window->picture->screen_pic;		/* neu geditherten Ausschnitt einhÑngen */ */
							}
							
							/*
							 * Bild reinkopieren
							 */
							if(!online)
							{
								vs_clip(Sys_info.vdi_handle, 1, vdiclip);
								if(disp_pic->fd_nplanes==1) 
									vrt_cpyfm(Sys_info.vdi_handle, MD_REPLACE, pxy, disp_pic, &m_screen, Colindex);	
								else
									vro_cpyfm(Sys_info.vdi_handle, S_ONLY, pxy, disp_pic, &m_screen);
								vs_clip(Sys_info.vdi_handle, 0, vdiclip);
							}
						}

						vs_clip(Sys_info.vdi_handle, 1, vdiclip);

						if(window->pflag && window->picture->block!=NULL && !(flags&DRAWNOBLOCK))
							imageWindow.drawBlock(window, &picbox);

						vs_clip(Sys_info.vdi_handle, 1, vdiclip);			/* Clipping neu einschalten (wird in draw_block geÑndert!) */
						
						/*
						 * Ditherpuffer fÅr Online-Dithering wieder freigeben
						 */
						if(online)
							window->picture->screen_pic = NULL;
					
						/* Bildmanager? -> Box fÅr Bildausschnitt */
						if(window->wnum == WIND_PICMAN)
							Dialog.picMan.drawPicmanBoxes();
					}
					
					/*
					 * sonstige Bildfensterelemente zeichnen (Blockbox, Fadenkreuz, FensterflÑche...)
					 * das muû auûerhalb des box-picbox intersects, damit die Boxen nicht
					 * auf die BildflÑche beschnitten werden.
					 */	
					if(window->pflag)
					{
						vs_clip(Sys_info.vdi_handle, 1, vdiclip);
						vsf_color(Sys_info.vdi_handle, Sys_info.outcol);
						vsl_color(Sys_info.vdi_handle, 1);

						/* Box rechts vom Bild */
						pxy[0] = wind_x + endwid - window->xoffset;
						pxy[1] = wind_y + TOOLBAR_HEIGHT;
						pxy[2] = wind_x + wind_w;
						pxy[3] = wind_y + wind_h;
						vr_recfl(Sys_info.vdi_handle, pxy);
/*
						if(vdiclip[1] < pxy[1])
							vdiclip[1] = pxy[1];
*/
						/* Box unterm Bild */
						pxy[0] = wind_x;
						pxy[1] = wind_y + TOOLBAR_HEIGHT + endhgt - window->yoffset;
						pxy[2] = wind_x + wind_w;
						pxy[3] = wind_y + wind_h;
						vr_recfl(Sys_info.vdi_handle, pxy);

						if(!(flags&NOBLOCKBOX))
							imageWindow.drawBlockbox(window);

						imageWindow.drawCrosshair(window);
					}

					vs_clip(Sys_info.vdi_handle, 0, vdiclip);
				}
			}
		}

		Window.windGet( window->whandlem, WF_NEXTXYWH, &box.g_x, &box.g_y, &box.g_w, &box.g_h );
	}
	
	Window.windGet(window->whandlem, WF_TOP, &tophandle, &dummy,&dummy,&dummy);

	Window.cursorOn(window);						/* Editobjekt initialisieren */
	wind_update(END_UPDATE);

	if(!(flags&DRAWNOPICTURE))
		graf_mouse(M_ON, dummy_ptr);

	return;
} /* f_redraw_window */


/* realtime_dither ----------------------------------------------
	Diese Funktion wird genutzt, um aus dem Bild im Fenster window den durch
	picbox definierten Ausschnitt nach den Angaben in Sys_info (Bilddarstellungs-
	optionen) zu dithern. Die Darstellung wird in window->picture->screen_pic abgelegt
	und der auf den Screen zu kopierende Ausschnitt in pxy abgelegt.
	Was noch fehlt:
		- die FensterflÑche auûerhalb des Bildes muû auch beim Onlinedither redrawed werden
		- mit gezoomten Bildern funktionierz noch nicht so recht
		-
	-----------------------------------------------------------------*/
void realtime_dither(GRECT *picbox, WINDOW *window, int *pxy, int *vdiclip, int stripheight)
{
	int	partx;
	int line_blocks, line, block_count;
	int Colindex[3]={1,0,0};
	int lineblockHeight;
	int pxy2[4];

	SMURF_PIC *picture;
	DISPLAY_MODES old;
	MFDB m_screen, *disp_pic;
	GRECT picpart;

	extern void fulldisable_busybox(void);

	
	/*
	 * Bei Bildfenstern wird in Streifen zu <stripheight> Pixel Hîhe gedithert.
	 * andere (Previews&Tumbnails) werden auf einmal gedithert.
	 */
	if(window->pflag)
		lineblockHeight = stripheight;
	else
		lineblockHeight = picbox->g_h + 1;
	
	picture = window->picture;
	m_screen.fd_addr = 0;

	line_blocks = (picbox->g_h + (lineblockHeight - 1)) / lineblockHeight;
	block_count = picbox->g_h;

	Dialog.busy.fullDisable();
	if(!window->pflag)
		make_singular_display(&old, Sys_info.PreviewMoveDither, CR_SYSPAL);

	vs_clip(Sys_info.vdi_handle, 1, vdiclip);

	if(window->pflag && picbox->g_w > 128 && picbox->g_h > 128)
	{
		pxy2[0] = picbox->g_x;
		pxy2[1] = picbox->g_y;
		pxy2[2] = picbox->g_w + picbox->g_x;
		pxy2[3] = picbox->g_h + picbox->g_y;
		vsf_color(Sys_info.vdi_handle, 8);
		vsf_style(Sys_info.vdi_handle, 23);
		vsf_interior(Sys_info.vdi_handle, FIS_PATTERN);
		vswr_mode(Sys_info.vdi_handle, MD_TRANS);
		v_bar(Sys_info.vdi_handle, &pxy2[0]);
		vswr_mode(Sys_info.vdi_handle, MD_REPLACE);
		vsf_interior(Sys_info.vdi_handle, FIS_SOLID);
	}
	
	for(line = 0; line < line_blocks; line++)
	{
		picpart.g_x = ((picbox->g_x - window->wx - window->pic_xpos) * (picture->zoom + 1)) + window->xoffset * (picture->zoom + 1);
		picpart.g_y = ((long)(picbox->g_y - window->pic_ypos - window->wy) + (long)line * lineblockHeight) * (picture->zoom + 1) + window->yoffset * (picture->zoom + 1);
		picpart.g_w = picbox->g_w * (picture->zoom + 1);
		picpart.g_h = picbox->g_h * (picture->zoom + 1);

		if(block_count>lineblockHeight)
			picpart.g_h = lineblockHeight * (picture->zoom + 1);
		else 
			picpart.g_h = block_count * (picture->zoom + 1);
		
		block_count-=lineblockHeight;
		
		partx = picpart.g_x;
	
		/*
		 * Standardformatbild - also den Ausschnitt alignen
		 */
		if(picture->format_type == FORM_STANDARD)
		{
			picpart.g_x = (int)(picpart.g_x / 8) * 8;			/* XPos nach links alignen */
			picpart.g_w = (int)((picpart.g_w + 8 + 7) / 8) * 8;	/* und die Breite nach rechts. */
		}	
		
		/*
		 * und die Quellkoords auf die entsprechenden Werte setzen. Das evtl. Aligning bei
		 * Standardformatbildern wird durch die Subtraktion picpart.g_n - partn gehandlet.
		 * Wenn die Ditherroutinen in Zielblîcke mit beliebigen Abmessungen dithern kînnen, wird
		 * das noch ein wenig mehr, dann ist nÑmlich der Zielausschnitt gleich dem Quellausschnitt.
		 */
		pxy[0] = partx - picpart.g_x;
		pxy[1] = 0;
		pxy[2] = picbox->g_w + (partx-picpart.g_x) - 1;
		pxy[3] = (picpart.g_h/(picture->zoom + 1)) - 1;
		pxy[4] = picbox->g_x;
		pxy[5] = picbox->g_y + line * lineblockHeight;
		pxy[6] = pxy[4] + picbox->g_w - 1;
		pxy[7] = pxy[5] + (picpart.g_h/(picture->zoom+1)) - 1;

		f_dither(picture, &Sys_info, 0, &picpart, &Display_Opt);
		disp_pic = window->picture->screen_pic;		/* neu geditherten Ausschnitt einhÑngen */

		if(disp_pic->fd_nplanes==1)
			vrt_cpyfm(Sys_info.vdi_handle, MD_REPLACE, pxy, disp_pic, &m_screen, Colindex);
		else
			vro_cpyfm(Sys_info.vdi_handle, S_ONLY, pxy, disp_pic, &m_screen);

		SMfree(window->picture->screen_pic->fd_addr);
		free(window->picture->screen_pic);
	}

	Dialog.busy.enable();
	if(!window->pflag) 
		restore_display(&old);

	return;
} /* realtime_dither */


/* draw_iconified ---------------------------------------------------
	öbernimmt den Redraw eines Ikonifizierten Fensters window. vdiclip
	gibt das Clippingrechteck an, das fÅr den objc_draw verwendet werden muû.
	-----------------------------------------------------------------*/
void draw_iconified(WINDOW *window, int *vdiclip)
{
	int pxy[5], icon;
	

	Window.windGet(window->whandlem, WF_WORKXYWH, &pxy[0], &pxy[1], &pxy[2], &pxy[3]);
	pxy[2] += pxy[0];
	pxy[3] += pxy[1];

	if(window->pflag)
		icon = IMAGE_ICON;
	else
		icon = SMURF_ICON;

	u_tree[icon].ob_x = pxy[0] + (pxy[2] - pxy[0] - 42) / 2;
	u_tree[icon].ob_y = pxy[1] + (pxy[3] - pxy[1] - 42) / 2;
	vsf_color(Sys_info.vdi_handle, Sys_info.AES_bgcolor);
	vs_clip(Sys_info.vdi_handle, 1, vdiclip);
	vr_recfl(Sys_info.vdi_handle, pxy);
	objc_draw(u_tree, icon, MAX_DEPTH, vdiclip[0],vdiclip[1],vdiclip[2]-vdiclip[0],vdiclip[3]-vdiclip[1]);
	vs_clip(Sys_info.vdi_handle, 0, vdiclip);

	return;
} /* draw_iconified */


/* draw_block ------------------------------------------------
	zeichnet die Blockbox im Bildfenster window neu.
	----------------------------------------------------------*/
void draw_block(WINDOW *window, GRECT *picbox)
{
	int Colindex[3] = {1, 0, 0};
	int endwid, endhgt;
	int pxy[10], clip[5];
	int wrmode, zoom;
	int pwx, pwy, pww, pwh;
	int strip, stripHeight, stripNum, stripCount;

	GRECT picpart, block, *ditherPart;
	SMURF_PIC *picture = window->picture, *previewedStrip;
	MFDB *disp_pic, m_screen;

	extern BLOCKMODE blockmode_conf;


	if(window->picture->block == NULL || (window->picture->blockwidth == 0 && window->picture->blockheight == 0))
		return;

	/*
	 * Screen-MFDB vorbereiten
	 */
	m_screen.fd_addr = NULL;
	disp_pic = window->picture->block->screen_pic;
	zoom = window->picture->zoom + 1;

	endwid = window->picture->block->pic_width / zoom;
	endhgt = window->picture->block->pic_height / zoom;

	Window.windGet(window->whandlem, WF_WORKXYWH, &pwx,&pwy,&pww,&pwh);
	
	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = endwid - 1;
	pxy[3] = endhgt - 1;
	pxy[4] = pwx + window->pic_xpos + window->picture->blockx / zoom - window->xoffset;
	pxy[5] = pwy + window->pic_ypos + window->picture->blocky / zoom - window->yoffset;
	pxy[6] = endwid + pxy[4] - 1;
	pxy[7] = endhgt + pxy[5] - 1;

	wrmode = MD_REPLACE;

	if(blockmode_conf.transparent == 2)
		wrmode = MD_TRANS;
	else
		if(blockmode_conf.transparent == 1)
			wrmode = MD_ERASE;

	/*
	 * Screendisplay vorhanden
	 */
	if(disp_pic)
	{
		if(disp_pic->fd_nplanes == 1)
			vrt_cpyfm(Sys_info.vdi_handle, wrmode, pxy, disp_pic, &m_screen, Colindex);
		else
			vro_cpyfm(Sys_info.vdi_handle, S_ONLY, pxy, disp_pic, &m_screen);
	}

	/*
	 * oder Echtzeitdither
	 */
	if(!disp_pic)
	{	
		/*
		 * Schwarzweiûschottenrock zeichnen
		 */
		if(window->pflag && (pxy[6] - pxy[4]) > 128 && (pxy[7] - pxy[5]) > 128)
		{
			vsf_color(Sys_info.vdi_handle, 8);
			vsf_style(Sys_info.vdi_handle, 23);
			vsf_interior(Sys_info.vdi_handle, FIS_PATTERN);
			vswr_mode(Sys_info.vdi_handle, MD_TRANS);
			v_bar(Sys_info.vdi_handle, &pxy[4]);
			vswr_mode(Sys_info.vdi_handle, MD_REPLACE);
			vsf_interior(Sys_info.vdi_handle, FIS_SOLID);
		}

		picpart.g_x = (picbox->g_x - window->pic_xpos - window->wx + window->xoffset) * zoom;
		picpart.g_y = (picbox->g_y - window->pic_ypos - window->wy + window->yoffset) * zoom;
		picpart.g_w = picbox->g_w * zoom;
		picpart.g_h = picbox->g_h * zoom;

		/*
		 * öberschneidung von zu redrawendem Rechteck und Blockrechteck?
		 */
		block.g_x = picture->blockx;
		block.g_y = picture->blocky;
		block.g_w = picture->block->pic_width;
		block.g_h = picture->block->pic_height;
		Window.rcIntersect(&block, &picpart, &picpart);

		if(picpart.g_w <= 0 || picpart.g_h <= 0)
			return;
		
		picpart.g_x -= picture->blockx;
		picpart.g_y -= picture->blocky;

		pxy[2] = picpart.g_w / zoom - 1;
		pxy[3] = picpart.g_h / zoom - 1;
		pxy[4] += picpart.g_x / zoom;
		pxy[5] += picpart.g_y / zoom;

		/*
		 * der Block wird, wie auch die Bilder, in Streifen zu je max. 64 geditherte Pixel
		 * Hîhe gedithert, damit man auch was sieht ;)
		 */
		stripHeight = 64 * zoom;
		stripNum = (picpart.g_h + (stripHeight - 1)) / stripHeight;
		stripCount = picpart.g_h;

		for(strip = 0; strip < stripNum; strip++)
		{
			if(stripCount>stripHeight)
				picpart.g_h = stripHeight;
			else 
				picpart.g_h = stripCount/* * zoom*/;

			pxy[3] = picpart.g_h/zoom - 1;

			Dialog.busy.fullDisable();

			/*
			 * ein nicht-ersetzender Modus ist eingestellt oder irgendeine Transparenz
			 * ->realtime-preview berechnen
			 */
			if(blockmode_conf.mode != BCONF_REPLACE || blockmode_conf.opacity != 100 || blockmode_conf.transparent != 0)
			{
				previewedStrip = previewBlock(picture, &picpart);
				ditherPart = NULL;
			}
			else
			{
				previewedStrip = picture->block;
				ditherPart = &picpart;
			}

			Dialog.busy.fullDisable();
			f_dither(previewedStrip, &Sys_info, 0, ditherPart, &Display_Opt);
			Dialog.busy.enable();

			if(previewedStrip->screen_pic->fd_nplanes == 1)
				vrt_cpyfm(Sys_info.vdi_handle, MD_REPLACE, pxy, previewedStrip->screen_pic, &m_screen, Colindex);
			else
				vro_cpyfm(Sys_info.vdi_handle, S_ONLY, pxy, previewedStrip->screen_pic, &m_screen);

			pxy[5] += picpart.g_h / zoom;
			picpart.g_y += stripHeight;
			stripCount -= stripHeight;

			SMfree(previewedStrip->screen_pic->fd_addr);
			free(previewedStrip->screen_pic);
			previewedStrip->screen_pic = NULL;

			/*
			 * haben wir oben in Echtzeit previewed?
			 * dann muû das Preview hier komplett freigegeben werden
			 */
			if(blockmode_conf.mode != BCONF_REPLACE || blockmode_conf.opacity!=100 || blockmode_conf.transparent != 0)
			{
				SMfree(previewedStrip->pic_data);
				SMfree(previewedStrip);
			}
		}
/*
		printf("\n%i  %i  %i  %i", picpart.g_x, picpart.g_y, picpart.g_w, picpart.g_h);
		printf("\n pxy1: %i  %i  %i  %i", pxy[0], pxy[1], pxy[2], pxy[3]);
		printf("\n pxy2: %i  %i  %i  %i", pxy[4], pxy[5], pxy[6], pxy[7]);
*/		

		picture->block->screen_pic = NULL;
	}

	vs_clip(Sys_info.vdi_handle, 0, clip);

	return;
} /* draw_block */


/* insert_picwinzoom ---------------------------------------------- 
	Aktuelle Zoomeinstellung in ein Bildfenster reinkopieren. Wird von
	f_redraw_window aufgerufen, um die Bildfenstertoolbar zu updaten.
	----------------------------------------------------------------*/
void insert_picwinzoom(WINDOW *window)
{
	int zoomindex;

	OBJECT *zptree, *tree;

	
	tree = window->resource_form;

	switch(window->picture->zoom)
	{
		case 1:		zoomindex = DURCH2;
					break;
		case 3:		zoomindex = DURCH3;
					break;
		case 9:		zoomindex = DURCH4;
					break;
		case 19:	zoomindex = DURCH5;
					break;
		case 29: 	zoomindex = DURCH6;
					break;
		case 0: 	zoomindex = ZOOM1;
					break;
	}

	xrsrc_gaddr(0, ZOOM_POP, &zptree, resource_global);
	strncpy(tree[ZOOM_FACTOR].TextCast, zptree[zoomindex].TextCast, 4);

	return;
} /* insert_picwinzoom */


/*---------------------------------------------------------------------	*/
/*						Positionierungskreuz zeichnen					*/
/*	Zeichnet nach den Angaben eines Moduls im Fenster window ein Posi-	*/
/*	tionierungskreuz (oder auch nicht).									*/
/*---------------------------------------------------------------------	*/
void f_draw_crosshair(WINDOW *window)
{
	int arrnum=-1, dummy;
	int xpos, ypos;
	int zoom, all_xoff, all_yoff;
	int pxy[11];
	int modwin_handle, top_handle;
	int minx, miny, maxx, maxy;

	
	zoom = window->picture->zoom + 1;

	arrnum = imageWindow.findCrosshair(window);

	if(arrnum == -1)
		return;

	if(module.smStruct[arrnum] == NULL)
		return;

	/*-------- Ist das dazugehîrige Modulfenster getoppt? -----*/
	modwin_handle = module.smStruct[arrnum]->wind_struct->whandlem;
	Window.windGet(0, WF_TOP, &top_handle, &dummy,&dummy,&dummy);
	if(modwin_handle != top_handle)
		return;

	if(position_markers[arrnum].mod_pic[0] == -1)
		return;
	
	Window.windGet(window->whandlem, WF_WORKXYWH, &minx, &miny, &maxx, &maxy);
	maxx += minx;
	maxy += miny;
	
	all_xoff = minx - window->xoffset;
	all_yoff = miny + TOOLBAR_HEIGHT - window->yoffset;

	xpos = position_markers[arrnum].xpos[0];
	ypos = position_markers[arrnum].ypos[0];
		
	vswr_mode(Sys_info.vdi_handle, MD_XOR);
	vsf_interior(Sys_info.vdi_handle, 0);
	
	pxy[0] = minx;
	pxy[1] = (ypos / zoom) + all_yoff;
	pxy[2] = maxx;
	pxy[3] = (ypos / zoom) + all_yoff;
	v_pline(Sys_info.vdi_handle, 2, pxy);	
	pxy[0] = (xpos / zoom) + all_xoff;
	pxy[1] = miny;
	pxy[2] = (xpos / zoom) + all_xoff;
	pxy[3] = maxy;
	v_pline(Sys_info.vdi_handle, 2, pxy);
	v_circle(Sys_info.vdi_handle, (xpos/zoom)+all_xoff, (ypos/zoom)+all_yoff, 3);

	vsf_interior(Sys_info.vdi_handle, 1);
	vswr_mode(Sys_info.vdi_handle, MD_REPLACE);

	return;
} /* f_draw_crosshair */


/*---------------------------------------------------------------------	*/
/*						Blockbox neu zeichnen							*/
/*---------------------------------------------------------------------	*/
void f_draw_blockbox(WINDOW *window)
{
	int pxy[11];
	int bx1, by1, bx2, by2;
	int zoom, all_xoff, all_yoff;


	if(window->picture->blockwidth!=0 && window->picture->blockheight!=0)
	{
		Window.windGet(window->whandlem, WF_WORKXYWH, &pxy[0], &pxy[1], &pxy[2], &pxy[3]);

		pxy[1] += TOOLBAR_HEIGHT;
		pxy[3] += TOOLBAR_HEIGHT;
/*		vs_clip(Sys_info.vdi_handle, 1, pxy);*/

		zoom = window->picture->zoom + 1;

		all_xoff = pxy[0] - window->xoffset;
		all_yoff = pxy[1] - window->yoffset;

		bx1 = window->picture->blockx;
		by1 = window->picture->blocky;

		bx2 = bx1 + window->picture->blockwidth;				
		/* - 1 weil die Endkoordinate und nicht Breite benîtigt wird */
		if(window->picture->blockwidth >= 0)
			bx2--;
		else
			bx2++;

		/* - 1 weil die Endkoordinate und nicht Breite benîtigt wird */
		by2 = by1 + window->picture->blockheight;				
		if(window->picture->blockheight >= 0)
			by2--;
		else
			by2++;

		vsl_type(Sys_info.vdi_handle, 1);
		vswr_mode(Sys_info.vdi_handle, MD_XOR);

		pxy[0] = all_xoff + (bx1/zoom) + 1;			/* x links oben */
		pxy[1] = all_yoff + (by1/zoom) + 1;			/* y links oben */
		pxy[2] = all_xoff + (bx2/zoom) - 1;			/* x rechts oben */
		pxy[3] = pxy[1];							/* y rechts oben */
		pxy[4] = pxy[2];							/* x rechts unten */
		pxy[5] = all_yoff + (by2/zoom) - 1;			/* y rechts unten */
		pxy[6] = pxy[0];							/* x links unten */
		pxy[7] = pxy[5];							/* y links unten */
		pxy[8] = pxy[0];							/* x links oben */
		pxy[9] = pxy[1];							/* y links oben */
		v_pline(Sys_info.vdi_handle, 5, pxy);

		pxy[0] = all_xoff + (bx1/zoom) - 1;			/* x links oben */
		pxy[1] = all_yoff + (by1/zoom) - 1;			/* y links oben */
		pxy[2] = all_xoff + (bx2/zoom) + 1;			/* x rechts oben */
		pxy[3] = pxy[1];							/* y rechts oben */
		pxy[4] = pxy[2];							/* x rechts unten */
		pxy[5] = all_yoff + (by2/zoom) + 1;			/* y rechts unten */
		pxy[6] = pxy[0];							/* x links unten */
		pxy[7] = pxy[5];							/* y links unten */
		pxy[8] = pxy[0];							/* x links oben */
		pxy[9] = pxy[1];							/* y links oben */
		v_pline(Sys_info.vdi_handle, 5, pxy);

		vswr_mode(Sys_info.vdi_handle, MD_REPLACE);
	}

	return;
} /* f_draw_blockbox */


/* draw_picmanboxes -----------------------------------------
	Wird von f_redraw_window aufgerufen und redrawed die Navigator-
	Box im Bildmanager.
	---------------------------------------------------------*/
void draw_picmanboxes(void)
{
	int zoom, piczoom;
	int absx, absy, xc1, yc1, xc2, yc2;
	int pxy[20], picnum;
	int picwid, pichgt;
	int dummy;
	OBJECT *pmtree;
	
	extern int picwindthere, dialwindthere, picthere;
	extern SMURF_PIC *smurf_picture[MAX_PIC];

	extern int compute_zoom(SMURF_PIC *picture, int twid, int thgt);


	if(picwindthere <= 0)
		return;

	pmtree = Dialog.picMan.tree;
	objc_offset(pmtree, PICMAN_PREVIEW, &absx, &absy);

	/*
	 * Bildnummer und Zooms ermitteln
	 */
	picnum = f_listfield((long *)Dialog.picMan.window, 0, 0, &Dialog.picMan.pictureList);
	picnum = Dialog.picMan.picmanList[picnum];
	zoom = compute_zoom(smurf_picture[picnum], pmtree[PM_PREVBOX].ob_width, pmtree[PM_PREVBOX].ob_height) + 1;
	piczoom = picture_windows[picnum].picture->zoom + 1;
	
	Window.windGet(picture_windows[picnum].whandlem, WF_WORKXYWH, &dummy, &dummy, &picwid, &pichgt);
	pichgt -= TOOLBAR_HEIGHT;

	/*
	 * OK. Jetzt rechnen wir uns aus den Fenster-Scrolloffsets, dem Zoom des Bildes
	 * und dem Zoom im Bildmanager die Koordinaten der Navigator-Box aus.
	 */
	xc1 = (picture_windows[picnum].xoffset / zoom * piczoom) + absx;
	yc1 = (picture_windows[picnum].yoffset / zoom * piczoom) + absy;
	xc2 = xc1 + (picwid / zoom * piczoom) - 1;			/* eins weg weil nicht Breite sondern Endkoordinate benîtigt */
	yc2 = yc1 + (pichgt / zoom * piczoom) - 1;			/* eins weg weil nicht Breite sondern Endkoordinate benîtigt */

	pxy[0] = xc1; pxy[1] = yc1;
	pxy[2] = xc2; pxy[3] = yc1;
	pxy[4] = xc2; pxy[5] = yc2;
	pxy[6] = xc1; pxy[7] = yc2;
	pxy[8] = xc1; pxy[9] = yc1;
	vsl_type(Sys_info.vdi_handle, 2);
	vswr_mode(Sys_info.vdi_handle, MD_XOR);
	v_pline(Sys_info.vdi_handle, 5, pxy);
	vswr_mode(Sys_info.vdi_handle, MD_REPLACE);

	return;
} /* draw_picmanboxes */


/* clip_picw2screen ---------------------------------------------
	Paût die Grîûe eines Fensters an die Bildschirmgrîûe an, falls
	das Fenster irgendwo Åber den Bildschirmrand heraussteht.
	-------------------------------------------------------------*/
void clip_picw2screen(WINDOW *picw)
{
	char wid[6], hgt[6], str[12];

	int whlen;
	int picwid, pichgt;
	int disppicwid, disppichgt;
	int abx, aby, abw, abh,
		m_wind_x, m_wind_y, m_wind_w, m_wind_h;
	int flags;

	SMURF_PIC *picture_to_handle;

	if(picw->shaded & ICONIFIED)
		return;

	picture_to_handle = picw->picture;
	picwid = picture_to_handle->pic_width;
	pichgt = picture_to_handle->pic_height;
	disppicwid = picwid / (picture_to_handle->zoom + 1);
	disppichgt = pichgt / (picture_to_handle->zoom + 1);

	/*
	 * Bildhîhe/-breite neu eintragen und Asterisk anhÑngen
	 */
	itoa(picwid, wid, 10);
	itoa(pichgt, hgt, 10);
	strcpy(str, wid);
	strcat(str, "*");
	strcat(str, hgt);
	whlen = (int)strlen(str);
	strncat(str, "            ", (long)(12 - whlen));
	strncpy(picw->wtitle, str, strlen(str) - 1);

	Window.windSet(picw->whandlem, WF_NAME, LONG2_2INT((long)picw->wtitle), 0,0);

	/*
	 * Fensterkoordinaten und -ausmaûe clippen 
	 */
	Window.windGet(0, WF_WORKXYWH, &abx, &aby, &abw, &abh);
	abh += aby;

	flags = CLOSER|NAME|MOVER|SMALLER;
	flags |= FULLER|SIZER|UPARROW|DNARROW|LFARROW|RTARROW|VSLIDE|HSLIDE;
	wind_calc(WC_BORDER, flags, picw->wx,picw->wy,disppicwid,disppichgt + TOOLBAR_HEIGHT, &m_wind_x,&m_wind_y,&m_wind_w,&m_wind_h);

	if(m_wind_x < abx)
		m_wind_x = abx;

	if(m_wind_x + m_wind_w > abw)
		m_wind_w = abw - m_wind_x;

	if(m_wind_y < aby)
		m_wind_y = aby;

	if(m_wind_y + m_wind_h > abh)
		m_wind_h = abh - m_wind_y;

	wind_calc(WC_WORK, flags, m_wind_x,m_wind_y,m_wind_w,m_wind_h, &m_wind_x,&m_wind_y,&m_wind_w,&m_wind_h);

	if(m_wind_w < 64)
		m_wind_w = 64;
	if(m_wind_h < 64)
		m_wind_h = 64;

	picw->wx = m_wind_x;
	picw->wy = m_wind_y;
	picw->ww = m_wind_w;
	picw->wh = m_wind_h;

	if(m_wind_w > disppicwid - picw->xoffset)
		picw->xoffset = disppicwid - m_wind_w;
	if(picw->xoffset < 0)
		picw->xoffset = 0;

	if(m_wind_h - TOOLBAR_HEIGHT > disppichgt - picw->yoffset)
		picw->yoffset = disppichgt - (m_wind_h - TOOLBAR_HEIGHT);
	if(picw->yoffset < 0)
		picw->yoffset = 0;

	wind_calc(WC_BORDER, flags, m_wind_x,m_wind_y,m_wind_w,m_wind_h, &m_wind_x,&m_wind_y,&m_wind_w, &m_wind_h);
	Window.windSet(picw->whandlem, WF_CURRXYWH, m_wind_x, m_wind_y, m_wind_w, m_wind_h);

	imageWindow.setSliders(picw);
	imageWindow.nullCoords(picw);

	return;
} /* clip_picw2screen */


/* rc_intersect -------------------------------------------------
	Berechnet öberschneidungen der Rechtecke r1 und r2, schreibt
	die SchnittflÑche in r3 und gibt als Returnwert zurÅck, ob
	eine SchnittflÑche Åberhaupt existiert.
	-------------------------------------------------------------*/
int rc_intersect( GRECT *r1, GRECT *r2, GRECT *r3)
{
   register int x, y, w, h;


   x = max(r2->g_x, r1->g_x);
   y = max(r2->g_y, r1->g_y);
   w = min(r2->g_x + r2->g_w, r1->g_x + r1->g_w);
   h = min(r2->g_y + r2->g_h, r1->g_y + r1->g_h);

   r3->g_x = x;
   r3->g_y = y;
   r3->g_w = w - x;
   r3->g_h = h - y;

   return(((w > x) && (h > y)));
} /* rc_intersect */


/*--------------------- DurchlÑuft die Windowhandleliste ------------------*/
int my_window(int handle)
{
	int t;
	
	if(handle == 0)
		return(0);			/* hiermit wird das Desktopfenster von vornherein ausgeschlossen */
	
	for(t = 0; t < 25; t++)
	{
		if(handle == wind_s[t].whandlem) 					/* Dialogfenster */
			return(t);									
		if(handle == picture_windows[t].whandlem)			/* Bildfenster */
			return(-t);			
	}
	
	return(0);		/* Nicht mein Fenster - Nicht mein Problem! */
} /* my_window */


/*----------------- DurchlÑuft die Windowhandleliste fÅr Module ------------------*/
WINDOW *my_module_window(int handle)
{
	int t;
	int wind_handle;
	WINDOW *m_window;

	extern	PLUGIN_DATA *plg_data[11];
	extern	int anzahl_plugins;

	
	if(handle == 0)
		return(0);			/* Hiermit wird das Desktopfenster von vornherein ausgeschlossen */
	
	for(t = 0; t < 20; t++)
	{
		if(module.smStruct[t] != NULL)
		{
			m_window = module.smStruct[t]->wind_struct;
			if(m_window)
			{
				wind_handle = m_window->whandlem;
				if(handle == wind_handle)
					return(m_window);		/* Fenster gehîrt zu Smurf-Modul! */
			}
		}
	}

	for(t = 0; t < anzahl_plugins; t++)
	{
		if(plg_data[t] != NULL)
		{
			m_window = plg_data[t]->wind_struct;
			if(m_window)
			{
				wind_handle = m_window->whandlem;
				if(handle == wind_handle)
					return(m_window);		/* Fenster gehîrt zu Smurf-Modul! */
			}
		}
	}
	
	return(0);				/* nicht mein Fenster - nicht mein Problem! */
} /* my_module_window */


/* -------------------------------------------------------------- 	*/
/*	Window-Slider setzen 											*/
/* -------------------------------------------------------------- 	*/
void f_setsliders(WINDOW *wind)
{
	int picw, pich;
	int slider_w, slider_h, slider_ypos, slider_xpos;
	int ydivi, xdivi;
	int oldslider_h, oldslider_w, oldslider_ypos, oldslider_xpos;
	int dummy;


	picw = wind->picture->pic_width / (wind->picture->zoom + 1);
	pich = wind->picture->pic_height / (wind->picture->zoom + 1);

	if(picw == 0)
		slider_w = 0;
	else
		slider_w = (int)((long)wind->ww * 1000L / (long)picw);

	if(slider_w < 5)
		slider_w = 5;
	
	if(pich == 0)
		slider_h = 0;
	else
		slider_h = (int)((long)(wind->wh - TOOLBAR_HEIGHT) * 1000L / (long)pich);
	
	if(slider_h < 5)
		slider_h = 5;
	
	xdivi = picw - wind->ww;
	ydivi = pich - (wind->wh - TOOLBAR_HEIGHT);

	if(xdivi == 0)
		slider_xpos = 0;
	else
		slider_xpos = (int)(((long)wind->xoffset * 1000L)  / (long)xdivi);
	
	if(ydivi == 0)
		slider_ypos = 0;
	else
		slider_ypos = (int)(((long)wind->yoffset * 1000L) / (long)ydivi);


	Window.windGet(wind->whandlem, WF_HSLIDE, &oldslider_xpos, &dummy,&dummy,&dummy);
	if(slider_xpos != oldslider_xpos)
		Window.windSet(wind->whandlem, WF_HSLIDE, slider_xpos, 0,0,0);

	Window.windGet(wind->whandlem, WF_HSLSIZE, &oldslider_w, &dummy,&dummy,&dummy);
	if(slider_w != oldslider_w)
		Window.windSet(wind->whandlem, WF_HSLSIZE, slider_w, 0,0,0);

	Window.windGet(wind->whandlem, WF_VSLIDE, &oldslider_ypos, &dummy,&dummy,&dummy);
	if(slider_ypos != oldslider_ypos)
		Window.windSet(wind->whandlem, WF_VSLIDE, slider_ypos, 0,0,0);

	Window.windGet(wind->whandlem, WF_VSLSIZE, &oldslider_h, &dummy,&dummy,&dummy);
	if(slider_h != oldslider_h)
		Window.windSet(wind->whandlem, WF_VSLSIZE, slider_h, 0,0,0);

	return;
} /* f_setsliders */


/* -------------------------------------------------------------- */
/* ----------------------- Fenster scrollen ----------------------*/
/* -------------------------------------------------------------- */
void f_arrow_window(int mode, WINDOW *wind,int amount)
{
	int xscroll=0, yscroll=0;
	int max_xs, max_ys;
	int endw, endh;

	GRECT picwind;

	
	endw = wind->picture->pic_width / (wind->picture->zoom + 1);
	endh = wind->picture->pic_height / (wind->picture->zoom + 1);

	max_xs = endw - wind->ww;
	max_ys = endh - (wind->wh - TOOLBAR_HEIGHT);

	if( ((mode==WA_UPLINE || mode==WA_UPPAGE) && wind->yoffset<=0)
		|| ((mode==WA_DNLINE || mode==WA_DNPAGE) && wind->yoffset>=max_ys)
		|| ((mode==WA_LFLINE || mode==WA_LFPAGE) && wind->xoffset<=0)
		|| ((mode==WA_RTLINE || mode==WA_RTPAGE) && wind->xoffset>=max_xs) )
			return;
	
	if(max_ys < 0)
		max_ys = 0;
	if(max_xs < 0)
		max_xs = 0;

	switch(mode)
	{
		case WA_DNLINE:	yscroll = amount;
						break;
		case WA_UPLINE:	yscroll = -amount;
						break;
		case WA_RTLINE:	xscroll = amount;
						break;
		case WA_LFLINE:	xscroll = -amount;
						break;
		case WA_UPPAGE:	yscroll -= (wind->wh - TOOLBAR_HEIGHT);
						break;
		case WA_DNPAGE:	yscroll += (wind->wh - TOOLBAR_HEIGHT);
						break;
		case WA_LFPAGE:	xscroll -= wind->ww;
						break;
		case WA_RTPAGE:	xscroll += wind->ww;
						break;
	}

	xscroll *= (wind->picture->zoom + 1);
	yscroll *= (wind->picture->zoom + 1);
	
	wind->xoffset += xscroll;
	wind->yoffset += yscroll;
	
	if(wind->yoffset > max_ys)
	{
		yscroll -= wind->yoffset - max_ys;
		wind->yoffset = max_ys;
	}
	else
		if(wind->yoffset < 0)
		{
			yscroll -= wind->yoffset;
			wind->yoffset = 0;
		}		

	
	if(wind->xoffset > max_xs)
	{
		xscroll -= wind->xoffset - max_xs;
		wind->xoffset = max_xs;
	}
	else
		if(wind->xoffset < 0)
		{
			xscroll -= wind->xoffset;
			wind->xoffset = 0;
		}
	
	picwind.g_x = wind->wx;
	picwind.g_y = wind->wy + TOOLBAR_HEIGHT;
	picwind.g_w = wind->ww;
	picwind.g_h = wind->wh;

	imageWindow.setSliders(wind);
	
	if(!Sys_info.realtime_dither)
		Window.redraw(wind, &picwind, 0, 0);
	else
	{
		if(mode == WA_UPPAGE || mode == WA_DNPAGE || mode == WA_DNLINE || mode == WA_UPLINE)
			imageWindow.scrollRT(wind, 0, yscroll);
		else
			imageWindow.scrollRT(wind, xscroll, 0);
	}

	return;
} /* f_arrow_window */


/* scrollWindowRT -----------------------------------------------
	öbernimmt das Verschieben eines Bildfensterinhaltes bei 
	angeschaltetem	Realtime-Dither.
	Die Blockbox und evtl. Fadenkreuz werden per Rechteckliste nur
	in den zu redrawenden Bereichen neugezeichnet.
	--------------------------------------------------------------*/
void scrollWindowRT(WINDOW *window, int xamount, int yamount)
{
	GRECT box, work, redraw;
	MFDB src, dest;
	int pxy[10], clip[4], zoom;
	int Colindex[3] = {1, 0, 0};


	Window.windGet(window->whandlem, WF_WORKXYWH, &work.g_x, &work.g_y, &work.g_w, &work.g_h);
	work.g_y += TOOLBAR_HEIGHT;
	work.g_h -= TOOLBAR_HEIGHT;
	Window.windGet(window->whandlem, WF_FIRSTXYWH, &box.g_x, &box.g_y, &box.g_w, &box.g_h);

	if(box.g_x + box.g_w > Sys_info.screen_width)
		box.g_w = Sys_info.screen_width - box.g_x;
	if(box.g_y + box.g_h > Sys_info.screen_height)
		box.g_h = Sys_info.screen_height - box.g_y;

	dest.fd_addr = 0;
	src.fd_addr = 0;
	zoom = window->picture->zoom + 1;

	/*
	 * sichtbare Ausschnitte anhand der Rechteckliste verschieben 
	 */
	graf_mouse(M_OFF, dummy_ptr);
	while(box.g_w > 0 && box.g_h > 0)
	{
		if(Window.rcIntersect(&work, &box, &box))
		{
			pxy[0] = box.g_x;
			pxy[1] = box.g_y;
			pxy[2] = box.g_w + box.g_x - 1;
			pxy[3] = box.g_h + box.g_y - 1;
			pxy[4] = box.g_x;
			pxy[5] = box.g_y;
			pxy[6] = box.g_w + box.g_x - 1;
			pxy[7] = box.g_h + box.g_y - 1;

			memcpy(clip, pxy, 4*2);
			vs_clip(Sys_info.vdi_handle, 1, clip);

			/*
			 * Wenn der Scrollbetrag grîûer ist als der zu redrawende Ausschnitt, ist
			 * das Kopieren unnîtig und fÅhrt u.U. zum Herauskopieren von MÅll aus den
			 * Fensterelementen.
			 */
			if(xamount < box.g_w && yamount < box.g_h)
			{
				if(pxy[0] < 0)
					pxy[4] = pxy[0] = 0;
				
				if(pxy[1] < 0)
					pxy[5] = pxy[1] = 0;
			
				/*
				 * hier wird die jeweils relevante Koordinate fÅr das Scrolling
				 * um amount erhîht/verringert.
				 */
				if(xamount > 0)
					pxy[0] += xamount;
				else
					if(xamount < 0)
						pxy[4] -= xamount;
				
				if(yamount > 0)
					pxy[1] += yamount;
				else
					if(yamount < 0)
						pxy[5] -= yamount;

				if(pxy[2] > Sys_info.screen_width)
					pxy[2] = Sys_info.screen_width;
				if(pxy[3] > Sys_info.screen_height)
					pxy[3] = Sys_info.screen_height;

				/*
				 * Sicherheitshalber nicht kopieren, wenn das Rechteck
				 * Nullgrîsse hat... kînnte u.U. Probleme geben.
				 */
				if(pxy[2] - pxy[0] && pxy[3] - pxy[1] && pxy[6] - pxy[4] && pxy[7] - pxy[5])
				{
					if(Sys_info.bitplanes == 1)
						vrt_cpyfm(Sys_info.vdi_handle, MD_REPLACE, pxy, &src, &dest, Colindex);
					else
						vro_cpyfm(Sys_info.vdi_handle, S_ONLY, pxy, &src, &dest);
				}
			}


			/*
			 * Und jetzt mÅssen noch die Bereiche, die neu ins Fenster
			 * gescrollt wurden, reingedithered werden. Der Redraw wird nicht
			 * Åber f_redraw_window() sondern direkt Åber realtime_dither() erledigt,
			 * erstens zerstîrt f_redraw_window() wahrscheinlich den momentanen Status
			 * des Rechtecklistenauslesens und zweitens geht es so schneller.
			 */				
			if(xamount)
			{
				if(xamount > 0)
					redraw.g_x = box.g_x + box.g_w - xamount;
				else
					redraw.g_x = box.g_x;
				redraw.g_y = box.g_y;
				redraw.g_w = abs(xamount);
				redraw.g_h = box.g_h;

				/*
				 * abschlieûend die Koordinaten nochmal clippen
				 */
				if(redraw.g_x < 0)
					redraw.g_x = 0;

				if(redraw.g_x < box.g_x)
				{
					redraw.g_w -= box.g_x - redraw.g_x;
					redraw.g_x = box.g_x;
				}

				/*
				 * fÅr die rechte Seite muss ich nach Koordinaten umrechnen
				 * um korrekte Werte rauszubekommen.
				 */
				if(redraw.g_w + redraw.g_x > box.g_w + box.g_x)
					redraw.g_w = box.g_w + box.g_x - redraw.g_x;

				clip[0] = redraw.g_x;
				clip[1] = redraw.g_y;
				clip[2] = redraw.g_x + redraw.g_w - 1;
				clip[3] = redraw.g_y + redraw.g_h - 1;
				vs_clip(Sys_info.vdi_handle, 1, clip);

				realtime_dither(&redraw, window, pxy, clip, box.g_h*zoom+1);
/*
				SMfree(window->picture->screen_pic->fd_addr);
				free(window->picture->screen_pic);
*/
				window->picture->screen_pic = NULL;

				imageWindow.drawBlock(window, &redraw);
				vs_clip(Sys_info.vdi_handle, 1, clip);
				imageWindow.drawBlockbox(window);
			}
			
			if(yamount)
			{
				if(yamount > 0)
					redraw.g_y = box.g_y + box.g_h - yamount;
				else
					redraw.g_y = box.g_y;
				redraw.g_x = box.g_x;
				redraw.g_w = box.g_w;
				redraw.g_h = abs(yamount);

				/*
				 * abschlieûend die Koordinaten nochmal clippen
				 */
				if(redraw.g_y < box.g_y)
				{
					redraw.g_h -= box.g_y - redraw.g_y;
					redraw.g_y = box.g_y;
				}

				/*
				 * fÅr die untere Seite muss ich nach Koordinaten umrechnen
				 * um korrekte Werte rauszubekommen.
				 */
				if(redraw.g_h + redraw.g_x > box.g_y + box.g_y)
					redraw.g_h = box.g_h + box.g_y - redraw.g_y;				
/*
				if(redraw.g_h > box.g_h)
					redraw.g_h = box.g_h;
*/				
				clip[0] = redraw.g_x;
				clip[1] = redraw.g_y;
				clip[2] = redraw.g_x + redraw.g_w - 1;
				clip[3] = redraw.g_y + redraw.g_h - 1;
				vs_clip(Sys_info.vdi_handle, 1, clip);

				realtime_dither(&redraw, window, pxy, clip, box.g_h*zoom+1);
/*
				SMfree(window->picture->screen_pic->fd_addr);
				free(window->picture->screen_pic);
*/
				window->picture->screen_pic=NULL;

				imageWindow.drawBlock(window, &redraw);
				vs_clip(Sys_info.vdi_handle, 1, clip);
				imageWindow.drawBlockbox(window);
			}
		}
	
		Window.windGet( window->whandlem, WF_NEXTXYWH, &box.g_x, &box.g_y, &box.g_w, &box.g_h );
	}

	graf_mouse(M_ON, dummy_ptr);

	return;
} /* scrollWindowRT */


/* -------------------------------------------------------------- */
/* --------------------------- Fenster Sliden ------------------- */
/* -------------------------------------------------------------- */
void f_slide_window(int pos, WINDOW *wind, int mode)
{
	int endw, endh;
	int oldx, oldy;

	GRECT picwind;


	endw = (wind->picture->pic_width) / (wind->picture->zoom + 1);
	endh = (wind->picture->pic_height) / (wind->picture->zoom + 1);
	
	oldx = wind->xoffset;
	oldy = wind->yoffset;
	
	picwind.g_x = wind->wx;
	picwind.g_y = wind->wy + TOOLBAR_HEIGHT;
	picwind.g_w = wind->ww;
	picwind.g_h = wind->wh - TOOLBAR_HEIGHT;
	
	switch(mode)
	{
		case 0:	wind->yoffset = (int)(((long)pos * (long)(endh - picwind.g_h)) / 1000L);
				if(wind->yoffset != oldy && picwind.g_h < endh)
					if(Sys_info.realtime_dither)
						imageWindow.scrollRT(wind, 0, wind->yoffset - oldy);
					else
						Window.redraw(wind, NULL, 0, DRAWNOTREE);

				imageWindow.setSliders(wind);
					
				break;
	
		case 1:	wind->xoffset = (int)(((long)pos * (long)(endw - picwind.g_w)) / 1000L);
				if(wind->xoffset != oldx && picwind.g_w < endw)
					if(Sys_info.realtime_dither)
						imageWindow.scrollRT(wind, wind->xoffset - oldx, 0);
					else
						Window.redraw(wind, NULL, 0, DRAWNOTREE);

				imageWindow.setSliders(wind);

				break;
	}

	return;
} /* f_slide_window */


void toggle_asterisk(WINDOW *picwin, int onoff)
{
	if(onoff == 0)
		picwin->wtitle[11]=' ';
	else
		picwin->wtitle[11]='*';

	Window.windSet(picwin->whandlem, WF_NAME, LONG2_2INT((long)picwin->wtitle), 0,0);

	return;
} /* toggle_asterisk */


/*----------------------------------------------------------------------------------*/
/*									WINDOW_TO_LIST									*/
/*	HÑngt ein Fenster *window in die doppelt verkettete List aller Fenster ein		*/
/*	zwischen Bildfenster und Dialog wird hier selbstÑndig unterschieden! 			*/
/*----------------------------------------------------------------------------------*/
void window_to_list(WINDOW *window)
{
	int t, biggest_handle, first_handle;
	int first, last;
	int handle;


	if(window->whandlem == Dialog.winAlert.winHandle)
		return;

	/*--------- Eigenes Fenster mit kleinerem Handle suchen, da jedoch das grîûte */
	/*			also das zuletzt geîffnete mit anderen Worten.	*/
	biggest_handle = -1;
	for(t = 0; t < 20; t++)
	{
		if(window->pflag)
			handle = picture_windows[t].whandlem;
		else	
			handle = wind_s[t].whandlem;

		if(handle != window->whandlem && handle > biggest_handle && handle != -1)
			biggest_handle = handle;
	}

	/* Handle des ersten Fensters (kleinstes) Åberhaupt suchen */
	first_handle = 1024;
	for(t = 0; t < 20; t++)
	{
		if(window->pflag)
			handle = picture_windows[t].whandlem;
		else
			handle = wind_s[t].whandlem;
			
		if(handle != window->whandlem && handle < first_handle && handle != -1)
			first_handle = handle;
	}

	/*--------- Fenster in die Liste hÑngen */
	if(biggest_handle != -1)
	{
		last = Window.myWindow(biggest_handle);
		if(last > 0)
		{
			wind_s[last].next_window = window;
			window->prev_window = &wind_s[last];
		}
		else
			if(last < 0)
			{
				picture_windows[-last].next_window = window;
				window->prev_window = &picture_windows[-last];
			}
	}

	if(first_handle != 1024)
	{
		first = Window.myWindow(first_handle);
		if(first > 0)
		{
			window->next_window = &wind_s[first];
			wind_s[first].prev_window = window;
			dialwindthere++;
		}
		else
			if(first < 0)
			{ 
				window->next_window = &picture_windows[-first];
				picture_windows[-first].prev_window=window;
				picwindthere++;
		}
	}
	else
	{
		if(window->pflag)
			picwindthere++;
		else
			dialwindthere++;

		window->next_window = window;
		window->prev_window = window;
	}

/*	display_windowlist(window); */

	return;
} /* window_to_list */


/*----------------------------------------------------------------------------------*/
/*									REMOVE_WINDOW									*/
/*	Enfernt ein Fenster aus der Liste, die LÅcke wird ÅberbrÅckt.					*/
/*----------------------------------------------------------------------------------*/
void remove_window(WINDOW *window)
{
	WINDOW *previous;
	WINDOW *next;


	if(window->whandlem == Dialog.winAlert.winHandle)
		return;

	previous = window->prev_window;
	next = window->next_window;

	if(previous != NULL)
		previous->next_window = next;
	if(next != NULL)
		next->prev_window = previous;

	return;
} /* remove_window */



/*----------------------------------------------------------------------------------*/
/*				CROSSHAIR-Struktur zu einem Bildfenster finden 						*/
/*----------------------------------------------------------------------------------*/
int find_crosshair(WINDOW *window)
{
	int picnum = -1, t, arrnum = -1;

	
	/*----------------- Bildnummer suchen */
	for(t = 0; t < MAX_PIC; t++)
	{
		if(&picture_windows[t] == window)
		{
			picnum = t;
			break;
		}
	}
	
	if(picnum == -1)
		return(-1);
	
	/*------------- dazugehîriges Modul suchen */
	for(t = 0; t < 20; t++)
	{
		if(position_markers[t].smurfpic[0] == picnum)
		{
			if(position_markers[t].mod_pic[0] >= 0)
			{		
				if(module_pics[t][position_markers[t].mod_pic[0]] == picnum)
				{
					arrnum = t;
					break;
				}
			}
			else
				if(picnum == active_pic && position_markers[t].mod_pic[0] == -2)
				{
					arrnum = t;
					break;
				}
		}
	}

	return(arrnum);
} /* find_crosshair */


/*
void display_windowlist(WINDOW *window)
{
	int first, last, t, biggest_handle, first_handle, handle;
	WINDOW *picwin, *firstwin;


	/*--------- Eigenes Fenster mit kleinerem Handle suchen, da jedoch das grîûte */
	/*			also das zuletzt geîffnete mit anderen Worten.	*/
	biggest_handle = -1;
	for(t = 0; t < 20; t++)
	{
		if(window->pflag)
			handle = &picture_windows[t]->whandlem;
		else
			handle = wind_s[t].whandlem;

		if(handle != window->whandlem && handle > biggest_handle && handle != -1)
			biggest_handle = handle;
	}

	/* Handle des ersten Fensters (kleinstes) Åberhaupt suchen */
		first_handle = 1024;
		for(t = 0; t < 20; t++)
		{
			if(window->pflag)
				handle = &picture_windows[t]->whandlem;
			else
				handle = wind_s[t].whandlem;
			
			if(handle < first_handle && handle != -1)
				first_handle = handle;
		}


		if(biggest_handle != -1)
			last = Window.myWindow(biggest_handle);
		if(first_handle != 1024)
			first = Window.myWindow(first_handle);


		/*------------ Fensterliste ausgeben -----*/
/*		Goto_pos(1,0);*/
		handle = first_handle;
		first = my_window(handle);

		if(first > 0)
			picwin = &wind_s[first];
		else
			if(first < 0)
				picwin = &picture_windows[first];
		firstwin = picwin;

		printf("VorwÑrts:          \n");		
		if(first_handle != 1024)
		{
			do
			{
				printf("   %s  - %i                  \n", picwin->wtitle, picwin->whandlem);
				picwin = picwin->next_window;
			} while(picwin != firstwin);

			printf("   %s  - %i                  \n", picwin->wtitle, picwin->whandlem);
		}

		printf("RÅckwÑrts:         \n");		

		if(last > 0)
			picwin = &wind_s[last];
		else
			if(last < 0)
				picwin = &picture_windows[last];
		firstwin = picwin;

		if(last != 0)
		{
			do
			{
				printf("  %s  - %i                   \n", picwin->wtitle, picwin->whandlem);
				picwin=picwin->prev_window;
			} while(picwin!=firstwin);

			printf("  %s  - %i                    \n", picwin->wtitle, picwin->whandlem);
		}

		printf("--------------------------\n");		
}
*/



/*-----------------------------------------------------------------	*/
/*							top_window								*/
/* schickt eine WM_TOPPED - Message fÅr das Fenster mit dem Handle	*/
/* handle an Smurf selbst. Immer anstatt wind_set verwenden, da		*/
/* hiermit der Cursor richtig an- und ausgeschaltet wird!			*/
/*-----------------------------------------------------------------	*/
void top_window(int handle)
{
	int toph, dummy;

	
	if(handle)
	{
		/* nur schicken, wenn das Fenster nicht schon getopped ist, */
		/* sonst kommt die Message u.U. zu spÑt und das Fenster wird */
		/* wann anders getopped. */
		Window.windGet(0, WF_TOP, &toph,&dummy,&dummy,&dummy);
		if(toph != handle)
		{
			messagebuf[0] = WM_TOPPED;
			messagebuf[1] = Sys_info.app_id;
			messagebuf[2] = 0;		
			messagebuf[3] = handle;
			appl_write(Sys_info.app_id, 16, messagebuf);			/* Message an uns selbst */
		}
 	}

	return;
} /* top_window */


/*-----------------------------------------------------------------	*/
/* Fenster sofort toppen (handlet Cursorein- und -ausschalten 		*/
/* selber - manchmal ist es notwendig, daû ein Fenster aufgrund 	*/
/* irgendeiner Aktion _sofort_ nach vorne geholt werden muû.		*/
/*-----------------------------------------------------------------	*/
void top_window_now(WINDOW *window)
{
	int toph, dummy;


	Window.windGet(0, WF_TOP, &toph, &dummy, &dummy, &dummy);
	if(toph != window->whandlem)									/* nicht schon top? */
	{
		Window.cursorOff(window);
		Window.windSet(window->whandlem, WF_TOP, 0, 0, 0, 0);
		Window.cursorOn(window);

		if(window->pflag)											/* Bildfenster */
			f_set_picpal(window->picture);
		else														/* alles andere */
			f_set_syspal();
	}

	return;
} /* top_window_now */


void top_windowhandle(int handle)
{
	int toph, dummy, wnum;

	WINDOW *window = NULL;


	if(handle <= 0)
		return;

	Window.windGet(0, WF_TOP, &toph, &dummy, &dummy, &dummy);
	if(toph != handle)												/* nicht schon top? */
	{
		wnum = Window.myWindow(handle);
		if(wnum == 0)
			window = Window.myModuleWindow(handle);
		else
			if(wnum < 0)
				window = &picture_windows[wnum];
			else
				if(wnum > 0)
					window = &wind_s[wnum];

		if(window == NULL)
			return;
		
		Window.cursorOff(window);
		Window.windSet(handle, WF_TOP, 0, 0, 0, 0);
		Window.cursorOn(window);

		if(window->pflag)											/* Bildfenster */
			f_set_picpal(window->picture);
		else														/* alles andere */
			f_set_syspal();
	}

	return;
} /* top_windowhandle */


/*-----------------------------------------------------------------	*/
/*						close_window								*/
/* schickt eine WM_CLOSED - Message fÅr das Fenster mit dem Handle	*/
/* handle an Smurf selbst. Immer anstatt wind_close verwenden, da	*/
/* hiermit der Cursor richtig an- und ausgeschaltet wird und alle	*/
/* OptionsÅbernahmen etc. richtig laufen!							*/
/*-----------------------------------------------------------------	*/
void close_window(int handle)
{
	if(handle > 0)
		Comm.sendAESMsg(Sys_info.app_id, WM_CLOSED, handle, -1);

	return;
} /* close_window */


/* f_alert--------------------------------------------------------------
	ôffnet eine Alertbox im Fenster mit dem Text *alertstring und den maximal
	drei Buttons *b1, *b2, *b3. Defaultbutton ist defbt. Die öbergabe eines Nullzeigers
	bei einem der Buttons fÅhrt zu dessen Ausblenden, die Buttons mÅssen in der Reihenfolge
	b1,b2,b3 benutzt werden!
	Die Funktion trennt den String automatisch in mehrere Zeilen auf und baut, wenn nicht
	mehr genÅgend Fenster frei oder die Windowalerts ausgeschaltet sind, selbstÑndig eine
	AES-Alertbox draus. 
	WÑhrend der Windowalert offen ist, wird in einer kleinen Eventschleife in dieser Funktion
	verweilt, die WM_MOVES und WM_REDRAW (WM_TOPPED fÅhrt zu einem Toppen der Alertbox)
	bearbeitet. 
	Diese Funktion darf nur bedingt aus dem Messagehandler aufgerufen werden, da dieser nicht
	Reentrant ist und der auf das Fensterîffnen folgende WM_REDRAW den originalen Messagebuffer
	bereits zerstîrt!
	-----------------------------------------------------------------------*/
/*
	Modul kann kein Fenster anfordern! Schlieûen Sie ein anderes Fenster.
	12345678901234567890123456789012345678901234567890123456789012345678901
	0         1         2         3         4        5          6         7
*/
int f_alert(char *alertstring, char *b1, char *b2, char *b3, char defbt)
{
	char t, *olds, lines, alwidth;
	char normal_al1[210] = "[1][";
	char normal_al2[35] = "][";
	char tmp[41] = "", *aend;

	int windback, back, mouse_xpos, mouse_ypos, key_scancode, klickobj, obj, msg, windhandle, dummy;

	long len;

	OBJECT *alert;

	extern int gl_hchar, gl_wchar, gl_hbox, gl_wbox;


	graf_mouse(ARROW, dummy_ptr);

	if(Sys_info.window_alert&SELECTED)
	{
		alwidth = 40;

		alert = wind_s[WIND_ALERT].resource_form;
		memset(alert[ALERTTEXT1].TextCast, 0x0, alwidth);
		memset(alert[ALERTTEXT2].TextCast, 0x0, alwidth);
		memset(alert[ALERTTEXT3].TextCast, 0x0, alwidth);
		memset(alert[ALERTTEXT4].TextCast, 0x0, alwidth);
		memset(alert[ALERTTEXT5].TextCast, 0x0, alwidth);
		
		/*-------------- String tokenisieren --------------------*/
		olds = alertstring;
		aend = alertstring + strlen(alertstring);
		for(t = ALERTTEXT1, lines = 0; t < ALERTTEXT5 && olds < aend; t++)
		{
			if(*olds == ' ')					/* Leerzeichen am Zeilenanfang vermeiden */
				olds++;

			strncpy(tmp, olds, alwidth);
			len = alerttok(tmp, alwidth);
			
			strncpy(alert[t].TextCast, olds, len);
			olds += len;

			lines++;
		}

		/*
		 * zunÑchst alle Buttons gut verstecken
		 */
		alert[ALBUTTON1].ob_flags |= HIDETREE;
		alert[ALBUTTON2].ob_flags |= HIDETREE;
		alert[ALBUTTON3].ob_flags |= HIDETREE;

		/*
		 * Button-Defaultstatus setzen
		 */
		alert[ALBUTTON1].ob_flags &= ~DEFAULT;
		alert[ALBUTTON2].ob_flags &= ~DEFAULT;
		alert[ALBUTTON3].ob_flags &= ~DEFAULT;
		alert[ALBUTTON1 + defbt - 1].ob_flags |= DEFAULT;

		/*
		 * die Texte in die Buttons und aufdecken
		 */
		if(b1 || (!b1 && !b2 && !b3))
		{
			if(!b1)
				strcpy(alert[ALBUTTON1].ob_spec.free_string, " OK ");
			else
				strcpy(alert[ALBUTTON1].ob_spec.free_string, b1);

			alert[ALBUTTON1].ob_flags &= ~HIDETREE;
		}
		
		if(b2)
		{
			strcpy(alert[ALBUTTON2].ob_spec.free_string, b2);
			alert[ALBUTTON2].ob_flags &= ~HIDETREE;
		}

		if(b3)
		{
			strcpy(alert[ALBUTTON3].ob_spec.free_string, b3);
			alert[ALBUTTON3].ob_flags &= ~HIDETREE;
		}

		/*
		 * Buttonbreiten berechnen
		 */
		alert[ALBUTTON1].ob_width = strlen(alert[ALBUTTON1].ob_spec.free_string) * gl_wchar + gl_wchar;
		alert[ALBUTTON2].ob_width = strlen(alert[ALBUTTON2].ob_spec.free_string) * gl_wchar + gl_wchar;
		alert[ALBUTTON3].ob_width = strlen(alert[ALBUTTON3].ob_spec.free_string) * gl_wchar + gl_wchar;


		/*
		 * Buttons positionieren
		 */
		alert[ALBUTTON1].ob_y = alert[ALBUTTON2].ob_y = alert[ALBUTTON3].ob_y = alert[ALERTTEXT1].ob_y + (lines * gl_hchar) + gl_hchar;
		alert[0].ob_height = alert[ALBUTTON1].ob_y + alert[ALBUTTON1].ob_height + gl_hchar;
		wind_s[WIND_ALERT].wh = alert[0].ob_height;

		if(!b2 && !b3)
			alert[ALBUTTON1].ob_x = (alert[0].ob_width - alert[ALBUTTON1].ob_width) / 2;
		else
			if(!b3)
			{
				alert[ALBUTTON1].ob_x = (alert[0].ob_width - (alert[ALBUTTON1].ob_width + 2 * gl_wchar + alert[ALBUTTON2].ob_width)) / 2;
				alert[ALBUTTON2].ob_x = alert[ALBUTTON1].ob_x + alert[ALBUTTON1].ob_width + 2 * gl_wchar;
			}
			else
			{
				alert[ALBUTTON1].ob_x = (alert[0].ob_width - (alert[ALBUTTON1].ob_width + 2 * gl_wchar + alert[ALBUTTON2].ob_width + 2 * gl_wchar + alert[ALBUTTON3].ob_width)) / 2;
				alert[ALBUTTON2].ob_x = alert[ALBUTTON1].ob_x + alert[ALBUTTON1].ob_width + 2 * gl_wchar;
				alert[ALBUTTON3].ob_x = alert[ALBUTTON2].ob_x + alert[ALBUTTON2].ob_width + 2 * gl_wchar;
			}
		
		if((windback = f_open_window(&wind_s[WIND_ALERT])) != -1)
		{
			lock_Smurf();
			f_redraw_window(&wind_s[WIND_ALERT], NULL, 0, 0);
			Dialog.winAlert.isTop = 1;
			Dialog.winAlert.winHandle = wind_s[WIND_ALERT].whandlem;

			do
			{
				back = evnt_multi(MU_BUTTON|MU_KEYBD|MU_MESAG,1,0x1,0x1, 0,0,0,0,0, 1,0,0,0,0,
								  messagebuf, 0,0, &mouse_xpos, &mouse_ypos, 
								  &dummy, &dummy, &key_scancode, &dummy);

				msg = messagebuf[0];

				if(back == MU_BUTTON)
				{
					windhandle = wind_find(mouse_xpos, mouse_ypos);
					klickobj = objc_find(wind_s[WIND_ALERT].resource_form, 0,MAX_DEPTH,mouse_xpos,mouse_ypos);

					obj = UDO_or_not(&wind_s[WIND_ALERT], klickobj);		/* UDOs behandeln */

					if(windhandle == Dialog.winAlert.winHandle && obj == klickobj &&
					   (obj == ALBUTTON1 || obj == ALBUTTON2 || obj == ALBUTTON3))
						break;
					else
						Window.windSet(Dialog.winAlert.winHandle, WF_TOP, 0, 0, 0, 0);
				}
				else 
					if(back == MU_MESAG && (msg == WM_MOVED || msg == WM_REDRAW || msg == WM_SIZED || msg == WM_TOPPED))
					{
						if(msg == WM_TOPPED)
							Window.windSet(Dialog.winAlert.winHandle, WF_TOP, 0, 0, 0, 0);
						else
							f_handle_message();
					}
					else
						if(back == MU_KEYBD)
							key_scancode >>= 8;
						else
							key_scancode = 0;
			} while(key_scancode != SCAN_RETURN && key_scancode != SCAN_ENTER);

			close_alert();

			/*
			 * RÅckgabewert ermitteln
			 */
			if(key_scancode == SCAN_RETURN || key_scancode == SCAN_ENTER)
				return(defbt);
			else
				return(obj - ALBUTTON1 + 1);
		}
	}

	if(!(Sys_info.window_alert&SELECTED) || windback == -1)
	{
		if(Sys_info.OS&MATSCHIG || Sys_info.OS&NAES)
			alwidth = 40;
		else
			alwidth = 30;

		olds = alertstring;
		aend = alertstring + strlen(alertstring);
		for(lines = 0; lines < 5 && olds < aend; lines++)
		{
			if(*olds == ' ')						/* Leerzeichen am Zeilenanfang vermeiden */
				olds++;

			strncpy(tmp, olds, alwidth);
			len = alerttok(tmp, alwidth);
			
			strncat(normal_al1, olds, len);
			strncat(normal_al1, "|", 1);
			olds += len;
		}

		normal_al1[strlen(normal_al1) - 1] = '\0';	/* letztes Pipe abschneiden */

		if(!b1 && !b2 && !b3)
			strcat(normal_al2, " OK ");
		else
		{
			if(b1)
				strcat(normal_al2, b1);

			if(b2)
			{
				strcat(normal_al2, "|");
				strcat(normal_al2, b2);
			}

			if(b3)
			{
				strcat(normal_al2, "|");
				strcat(normal_al2, b3);
			}
		}

		strcat(normal_al2, "]");

		strcat(normal_al1, normal_al2);
		return(form_alert(defbt, normal_al1));
	}

	return(0);
} /* f_alert */



/* close_alert() -----------------------------------------------
	Smurf-internen Window-Alert schlieûen, Button deselektieren, 
	Fenster lîschen, Smurf wieder freigeben
	-------------------------------------------------------------*/
void close_alert(void)
{
	if(wind_s[WIND_ALERT].whandlem == -1)				/* ist der Alert ist schon zu? */
		return;

	change_object(&wind_s[WIND_ALERT], ALBUTTON1, UNSEL, 1);
	change_object(&wind_s[WIND_ALERT], ALBUTTON2, UNSEL, 1);
	change_object(&wind_s[WIND_ALERT], ALBUTTON3, UNSEL, 1);
	wind_close(Dialog.winAlert.winHandle);
	wind_delete(Dialog.winAlert.winHandle);
	wind_s[WIND_ALERT].whandlem = -1;
	unlock_Smurf();
	Dialog.winAlert.winHandle = -1;
	Dialog.winAlert.isTop = 0;

	return;
} /* close_alert */


/* gibt den lÑngstmîglichen Teilstring aus s zurÅck, wobei */
/* versucht wird, den String nur nach einem der Zeichen in */
/* brk abzusÑgen. maxlen gibt die MaximallÑnge an. */
char alerttok(char *s, char maxlen)
{
	char *brk = " ,;.:-?!";		/* als Trenner fungierende Zeichen */
	char *jep;


	if(strlen(s) < maxlen)
		return(maxlen);

	strrev(s);					/* String umdrehen */

	jep = strpbrk(s, brk);		/* und dann ganz normale Suche */

	if(jep)
		return(strlen(jep));
	else
		return(maxlen);
} /* alerttok */


/* wird immer aufgerufen wenn sich ein Bild geÑndert hat und 
 * veranlaût Aktualisierung aller Bildfensterstatusanzeigen
 * wie BIldinfodialog, Picmanpreview sowie Bildmaû- und
 * tiefenanzeige im Picmanager 
 */
void f_pic_changed(WINDOW *window, int onoff)
{
	openmode = 1;				/* sonst geht der Pic-Info-Dialog immer auf */
								/* keine Ahnung fÅr was openmode eigentlich da ist */
	f_pic_info();
	Dialog.picMan.makeThumbnail(active_pic);
/*	Dialog.picMan.showWH(smurf_picture[active_pic]); */
	imageWindow.toggleAsterisk(window, onoff);

	return;
} /* f_pic_changed */