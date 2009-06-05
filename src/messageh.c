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

/* ----------------------------------------------------------------	*/
/* ------------- Handler fÅr Messages des AES ---------------------	*/
/* 	Fragt ev_mgpbuff ab und leitet alles weitere in die Wege.		*/
/* ----------------------------------------------------------------	*/
#include <tos.h>
#include <string.h>
#include <stdlib.h>
#include <ext.h>
#include <stdio.h>
#include "smurf_st.h"
#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "smurf_f.h"
#include "smurf.h"
#include "vaproto.h"
#include "globdefs.h"
#include "destruct.h"
#include "plugin\plugin.h"
#include "olga.h"
#include "olgagems.h"
#include "screen.h"

#include "smurfobs.h"
#include "ext_obs.h"

#define	AP_ARGSTART		0x5001

#define	BUBBLEGEM_REQUEST	0xBABA
#define	BUBBLEGEM_ACK	0xBABC
#define DHST_ACK 0xdade

extern int picwindthere, dialwindthere, picthere;

extern WINDOW wind_s[25];
extern WINDOW picture_windows[MAX_PIC];
extern SMURF_PIC *smurf_picture[MAX_PIC];

extern int	obj;					/* Objekt beim loslassen des Buttons */

extern CROSSHAIR	position_markers[20];		/* Positionsmarker fÅr die Editmodule */
extern DISPLAY_MODES Display_Opt;

extern int *messagebuf;

extern char Smurf_locked;

extern SYSTEM_INFO Sys_info;

extern	OBJECT	*menu_tree;

#define	PRG_CLOSED	-2

extern char *send_smurfid;

extern char	module_pics[21][7];

extern OBJECT	*pic_form;					/* Pic-Window-Formular */

extern int num_of_pics, active_pic, pic_to_handle;
extern int openmode;				/* Fenster neu geîffnet (0) oder buttonevent? (!=0) */
extern int edit_mod_num;		/* Modul-ID des Moduls, das das Einstellformular benutzt */
extern int menu_id, ap_id;

extern	char	commpath[257];
extern	char	DraufschmeissBild;

extern int add_flags[40];

extern char *export_cnfblock[50];
extern int export_cnflen[50];

extern	BASPAG	*plugin_bp[11];
extern	PLUGIN_DATA *plg_data[11];
extern	PLUGIN_INFO *plg_info[11];
extern	int anzahl_plugins;

extern	EXPORT_CONFIG exp_conf;

typedef struct
{
  char *appname,
       *apppath,
       *docname,
       *docpath;
} DHSTINFO;


int f_handle_message(void)
{
	char module_name[30];
	char *textseg, *ext_com_ptr;

	int wind_num = 0, wn = 0;
	register int t;
	int back;
	int wxp, wyp, wwp, whp;
	int ismodule = 0, module_num, dummy, owner;
	int last_loaded_pic = 0, biggest_handle = 0; 
	int picnum, tophandle;
	int flags, innerwid, innerhgt;

	long argback;

	MOD_INFO *modinfo;
	GRECT aes_red;
	WINDOW *window_to_handle;
	WINDOW *picwindow;
	int topwin;
	OBJECT *ob;
	
	extern int mouse_xpos, mouse_ypos;

	extern	void blockfunctions_off(void);
	extern	void terminate_plugin(int index);
	void scrollWindowRT(WINDOW *window, int xamount, int yamount);
	
	aes_red.g_x = 0;
	aes_red.g_y = 0;
	aes_red.g_w = Sys_info.screen_width;
	aes_red.g_h = Sys_info.screen_height;



	/*
	 * Zur Message gehîrige Fensterstruktur ermitteln	
	 */
	if((messagebuf[0] >= 20 && messagebuf[0] <= 36) ||
		messagebuf[0] == WM_SHADED || messagebuf[0] == WM_UNSHADED)
	{
		wind_num = Window.myWindow(messagebuf[3]);

		if(wind_num > 0)
			window_to_handle = &wind_s[wind_num];
		else
			if(wind_num < 0)
				window_to_handle = &picture_windows[-wind_num];
			else
				if(wind_num == 0)
				{
					window_to_handle = Window.myModuleWindow(messagebuf[3]);
					ismodule = 1;
				}

		if(window_to_handle == NULL)
			return(0);

		if(wind_num < 0)
		{
			wn = -wind_num;
			aes_red.g_x = (picture_windows[wn].resource_form->ob_x) - 1;
			aes_red.g_y = (picture_windows[wn].resource_form->ob_y) - 1;
			aes_red.g_w = (picture_windows[wn].resource_form->ob_width) + 2;
			aes_red.g_h = (picture_windows[wn].resource_form->ob_height) + 2;
		}

		flags = CLOSER|NAME|MOVER|SMALLER;
		if(window_to_handle->picture != NULL && window_to_handle->pflag!=0) 
			flags |=FULLER|SIZER|UPARROW|DNARROW|LFARROW|RTARROW|VSLIDE|HSLIDE;
		else
			if(!window_to_handle->pflag)
				flags |= add_flags[window_to_handle->wnum];
	}


	switch(messagebuf[0])
	{
	/*-------------------- Application Messages ------------------------ */
	case AP_TERM:
	case AP_RESCHG:
        wind_close(wind_s[WIND_BUSY].whandlem);
        wind_delete(wind_s[WIND_BUSY].whandlem);
        if(_app)
        	return(PRG_CLOSED);
		break;

	case AP_DRAGDROP:
		Comm.getDragdrop(window_to_handle, messagebuf);
		break;

	case AP_ARGSTART:
		DraufschmeissBild = START;
		file_load("", (char **)*(long *)(messagebuf + 4), START);
		break;

	/*------------------------- MenÅ Messages -------------------------- */
	case MN_SELECTED:	
		if(!Dialog.winAlert.isTop)
		{
			menu_tnormal(menu_tree, messagebuf[3], 1);
			back = f_handle_menuevent(messagebuf);
			if(back == -1)
				return(PRG_CLOSED);
 		}
		break;

   	/*------------------------- Fenster Messages -------------------------*/
	/*
	 * Fenster wurde verschoben 
	 */
    case WM_MOVED:
		Window.windSet(messagebuf[3], WF_CURRXYWH, messagebuf[4], messagebuf[5], messagebuf[6], messagebuf[7]);
		Window.windGet(messagebuf[3], WF_WORKXYWH, &window_to_handle->wx,&window_to_handle->wy, &dummy, &dummy);

		if(wind_num > 0)
		{
			Sys_info.dialog_xposition[wind_num] = window_to_handle->wx;
			Sys_info.dialog_yposition[wind_num] = window_to_handle->wy;
		}

		ob = window_to_handle->resource_form;
		if(ob != NULL || window_to_handle->picture != NULL)
		{
			ob->ob_x = window_to_handle->wx;
			ob->ob_y = window_to_handle->wy;

			if(wind_num < 0)
			{
				pic_form->ob_x = window_to_handle->wx;
				pic_form->ob_y = window_to_handle->wy;
			}
		}
		break;


	/*
	 * Fenster wurde manuell nach oben geholt
	 */
	case WM_TOPPED:
		Window.windGet(0, WF_TOP, &tophandle, &dummy, &dummy, &dummy);

		/* Wenn das Fenster schon topped ist, nicht nochmal toppen! */
		if(tophandle == messagebuf[3]) 
			break;		
		
		topwin = Window.myWindow(tophandle);
		if(topwin > 0)
			Window.cursorOff(&wind_s[topwin]);
		
		if(Dialog.winAlert.isTop)
			Window.windSet(Dialog.winAlert.winHandle, WF_TOP, 0, 0, 0, 0);
		else
		{
			Window.windSet(messagebuf[3], WF_TOP, 0, 0, 0, 0);

			if(wind_num >= 0)						/* Modul- oder Formularfenster */
			{
				f_set_syspal();

				Window.cursorOn(window_to_handle);
				Dialog.picMan.tree[PM_BOX].ob_spec.obspec.framesize = 0;

				if(wind_num == WIND_MODFORM || (wind_num == 0 && ismodule == 1))		/* Modulfenster mit Fadenkreuz? ->Bildfenster neu zeichnen! */
				{
					picwindow = &picture_windows[position_markers[window_to_handle->module].smurfpic[0]];
						Window.redraw(picwindow, NULL, 0, DRAWNOTREE);
				}

				Dialog.topDialog = window_to_handle->whandlem;
				imageWindow.topPicwin = 256;
			}
			else
				if(wind_num < 0)						/* Bildfenster */
				{
					f_activate_pic(-wind_num);
					openmode = 1;
					if(Display_Opt.palette_mode == PAL_TOPWIN)
						f_set_picpal(window_to_handle->picture);
					imageWindow.topPicwin = window_to_handle->whandlem;
					Dialog.topDialog = 256;
				}
		}

		if(window_to_handle == 0)
			f_set_syspal();

		break;


	/*
	 * Window wurde durch Schlieûen eines anderen zum Top Window.
	 */
	case WM_NEWTOP:
	case WM_ONTOP:
		if(wind_num < 0)				/* Bildfenster */
		{
			imageWindow.topPicwin = window_to_handle->whandlem;
			Dialog.topDialog = 256;
			if(Display_Opt.palette_mode == PAL_TOPWIN)
				f_set_picpal(window_to_handle->picture);
		}
		else
			if(wind_num >= 0)			/* Dialogfenster */
			{
				f_set_syspal();
				Window.cursorOn(window_to_handle);
				Dialog.picMan.tree[PM_BOX].ob_spec.obspec.framesize = 0;

				if(wind_num == WIND_MODFORM || (wind_num == 0 && ismodule == 1))		/* Modulfenster/Fadenkreuz -> Bild redrawen */
				{
					picwindow = &picture_windows[position_markers[window_to_handle->module].smurfpic[0]];
					Window.redraw(picwindow, NULL, 0, DRAWNOTREE);
				}

				Dialog.topDialog = window_to_handle->whandlem;
				imageWindow.topPicwin = 256;
			}
		break;

	/*
	 * Fenster wurde manuell nach unten gelegt
	 */
	case WM_BOTTOMED:
		if(!Dialog.winAlert.isTop)
		{
			Window.cursorOff(window_to_handle);
			Window.windSet(messagebuf[3], WF_BOTTOM, 0, 0, 0, 0); 
			
			if(wind_num == WIND_MODFORM || (wind_num == 0 && ismodule == 1))		/* Modulfenster/FK -> Bilfenster redraw */
			{
				picwindow = &picture_windows[position_markers[window_to_handle->module].smurfpic[0]];
				Window.redraw(picwindow, NULL, 0, DRAWNOTREE);
			}
		}
		break;

	/*
	 * das Fenster wird gerade inaktiv
	 */
	case WM_UNTOPPED:
		if(wind_num>=0)
		{
			Window.cursorOff(window_to_handle);
		
			if(wind_num == WIND_MODFORM || (wind_num == 0 && ismodule == 1))		/* Modulfenster */
			{
				picwindow = &picture_windows[position_markers[window_to_handle->module].smurfpic[0]];
				Window.redraw(picwindow, NULL, 0, DRAWNOTREE);
			}
		}

		Window.windGet(dummy, WF_TOP, &tophandle, &owner, &dummy, &dummy);
		if(owner == Sys_info.app_id)
		{
			if(Display_Opt.palette_mode == PAL_TOPWIN)
			{
				topwin = Window.myWindow(tophandle);
				if(topwin >= 0)								/* Smurf-Dialog oder Modulfenster */
					f_set_syspal();
				else										/* Bildfenster */
					f_set_picpal(picture_windows[-topwin].picture);
			}
		}
		else
			f_set_syspal();

		break;

	/*
	 * Fuller wurde geklickt 
	 */
	case WM_FULLED:
		if(wind_num)
		{
			if(window_to_handle->fullx == -1)		/* Fullen */
			{
				/* momentane Bruttokoordinaten ausrechnen */
				Window.windGet(messagebuf[3], WF_CURRXYWH, &wxp, &wyp, &wwp, &whp);
				window_to_handle->fullx = wxp;
				window_to_handle->fully = wyp;
				window_to_handle->fullw = wwp;
				window_to_handle->fullh = whp;

				wxp = window_to_handle->xoffset;
				wyp = window_to_handle->yoffset;
				window_to_handle->xoffset = 0;
				window_to_handle->yoffset = 0;
				/* image.Window.clipPicwin() dazu bringen, daû sie das Fenster */
				/* ganz links oben hinsetzt */
				window_to_handle->wx = -1;
				window_to_handle->wy = -1;
				imageWindow.clipPicwin(window_to_handle);

				if(Sys_info.realtime_dither)
					imageWindow.scrollRT(window_to_handle, -wxp, -wyp);
			}
			else			/* ZurÅckstellen */
			{
				wxp = window_to_handle->fullx;
				wyp = window_to_handle->fully;
				wwp = window_to_handle->fullw;
				whp = window_to_handle->fullh;
				/* auf Normalgrîûe bringen */
				Window.windSet(messagebuf[3], WF_CURRXYWH, wxp, wyp, wwp, whp);
				window_to_handle->fullx = -1;
				window_to_handle->fully = -1;
				window_to_handle->fullw = -1;
				window_to_handle->fullh = -1;
				/* momentane Nettokoordinaten ausrechnen */
				Window.windGet(messagebuf[3], WF_WORKXYWH, &wxp, &wyp, &wwp, &whp);
				window_to_handle->wx = wxp;
				window_to_handle->wy = wyp;
				window_to_handle->ww = wwp;
				window_to_handle->wh = whp;
				window_to_handle->xoffset = 0;
				window_to_handle->yoffset = 0;
			}

			imageWindow.setSliders(window_to_handle);
		}
		break;

	/*
	 * Fenster scrolled 
	 */
	case WM_ARROWED:
		if(wind_num)
			imageWindow.arrowWindow(messagebuf[4], window_to_handle, 8);
		break;

	/*
	 * horizontaler Slider geklickt 
	 */
	case WM_VSLID:
		if(wind_num)
			imageWindow.slideWindow(messagebuf[4], window_to_handle, 0);
		break;

	/*
	 * vertikaler Slider geklickt
	 */
	case WM_HSLID:
		if(wind_num)
			imageWindow.slideWindow(messagebuf[4], window_to_handle, 1);
		break;
	
	/*
	 * Fenstergrîûe wurde verÑndert
	 */
	case WM_SIZED:
		if(wind_num)
		{
			/* neue Grîûe setzen */
			Window.windSet(messagebuf[3], WF_CURRXYWH, messagebuf[4], messagebuf[5], messagebuf[6], messagebuf[7]);
			window_to_handle->fullx = -1;

			/* und deren Nettokoordinaten ermitteln */
			Window.windGet(messagebuf[3], WF_WORKXYWH, &window_to_handle->wx,&window_to_handle->wy,&innerwid,&innerhgt);

			
			if(window_to_handle->pflag)
			{
				wxp = window_to_handle->xoffset;
				wyp = window_to_handle->yoffset;
				
				if(innerwid > window_to_handle->picture->pic_width - window_to_handle->xoffset)
					window_to_handle->xoffset = window_to_handle->picture->pic_width - innerwid;
				if(window_to_handle->xoffset < 0)
					window_to_handle->xoffset = 0;
				
				if(innerhgt - TOOLBAR_HEIGHT > window_to_handle->picture->pic_height - window_to_handle->yoffset)
					window_to_handle->yoffset = window_to_handle->picture->pic_height - (innerhgt - TOOLBAR_HEIGHT);
				if(window_to_handle->yoffset < 0)
					window_to_handle->yoffset = 0;

				wxp = window_to_handle->xoffset - wxp;
				wyp = window_to_handle->yoffset - wyp;
					
				if(!Sys_info.realtime_dither)
					Window.redraw(window_to_handle, NULL, 0, DRAWNOTREE);
				else
					imageWindow.scrollRT(window_to_handle, wxp, wyp);
				
				window_to_handle->ww = innerwid;
				window_to_handle->wh = innerhgt;
			}

			imageWindow.setSliders(window_to_handle);
		}
		break;
	
	/*
	 * Fenster muû redrawed werden
	 */
	case WM_REDRAW:
		if(wind_num || ismodule == 1)
		{
			if(window_to_handle != NULL && window_to_handle->whandlem != -1)
			{
				aes_red.g_x = messagebuf[4];
				aes_red.g_y = messagebuf[5];
				aes_red.g_w = messagebuf[6];
				aes_red.g_h = messagebuf[7];
				
				/*
				 * Wenn es ein Bildfenster ist, dÅrfen auch nur in dem die Anzeigen in
				 * der Toolbar dargestellt werden, Åber dem sich die Maus befindet.
				 */
				if(wind_num < 0)
				{
					tophandle = wind_find(mouse_xpos, mouse_ypos);

					if(tophandle == window_to_handle->whandlem)
						imageWindow.displayCoords(window_to_handle, mouse_xpos, mouse_ypos, 0);
					else
						imageWindow.nullCoords(window_to_handle);
				}

				Window.redraw(window_to_handle, &aes_red, 0, 0);
			}
		}
		break;

	/*
	 * Fenster wurde geschlossen
	 */
	case WM_CLOSED:
		if(wind_num || ismodule == 1)
		{
			if(wind_num < 0)				/* bearbeitetes Bild? -> RÅckfrage! */
			{
				if(window_to_handle->wtitle[11] == '*' && !(Sys_info.profi_mode&SELECTED))
				{
					if(Dialog.winAlert.openAlert(Dialog.winAlert.alerts[WCLOSE_ALERT].TextCast, "Nein", " Ja ", NULL, 1) == 1)
							return(0);
				}
			}
			
			if(Dialog.winAlert.isTop && window_to_handle->whandlem == Dialog.winAlert.winHandle)
				Dialog.winAlert.closeAlert();
			else
			{
				Window.cursorOff(window_to_handle);
				wind_close(window_to_handle->whandlem);
				wind_delete(window_to_handle->whandlem);
				Window.removeWindow(window_to_handle);		/* aus der Liste entfernen */
			}

			window_to_handle->shaded = 0;
			
			if(wind_num > 0)
				Sys_info.dialog_opened[wind_num] = 0;

			/* muû hierhin, weil f_redraw() unter N.AES sonst Mist baut! */
			window_to_handle->whandlem = -1;

			/*
			 * Bild? -> "Gib mir mein RAM zurÅck! (Du brauchst meinen Speicher nicht!)"
			 *								(frei nach H. Grînemeyer)
			 */
			if(wind_num < 0)
			{
				/*
				 * zuletzt geladenes Bild aktivieren 
				 */
				last_loaded_pic=-1;
				for(t = 0; t < MAX_PIC; t++)   
				{
					if(picture_windows[t].whandlem > biggest_handle)
					{
						biggest_handle = picture_windows[t].whandlem;
						last_loaded_pic = picture_windows[t].wnum;
					}
				}

				picthere--;
				picwindthere--;
				if(!picthere)
				{
					Dialog.close(WIND_BTYPEIN);
					Dialog.close(WIND_TRANSFORM);
					blockfunctions_off();
				}

				f_activate_pic(last_loaded_pic);

				destroy_smurfpic(window_to_handle->picture);
				window_to_handle->picture = NULL;
				window_to_handle->whandlem = -1;
				
				back = window_to_handle->wnum;
				smurf_picture[back] = NULL;

				f_set_syspal();
				Dialog.busy.dispRAM();

				/*
				 * aus dem Bildmanager lîschen 
				 */
				openmode = 1;
				obj = PMSL_PAR;
				Dialog.picMan.pictureList.number_entries--;
				if(Dialog.picMan.pictureList.scroll_offset > 0)
					Dialog.picMan.pictureList.scroll_offset--;

				Dialog.picMan.cleanupList();
				Dialog.picMan.handlePicman();
			}
			else 
			{
				/*
				 * Einstellformular? -> Konfiguration merken, Modul terminieren! 
				 */
				if(wind_num == WIND_MODFORM)
				{
					edit_mod_num = window_to_handle->module;
					
					/*
					 * Moduleinstellungen merken ...
					 */
/*
					ob = wind_s[WIND_MODFORM].resource_form;
					cnfblock = malloc(50);
					cnfblock[0] = sy1;
					cnfblock[1] = sy2;
					cnfblock[2] = sy3;
					cnfblock[3] = sy4;
					cnfblock[4] = ob[CHECK1].ob_state;
					cnfblock[5] = ob[CHECK2].ob_state;
					cnfblock[6] = ob[CHECK3].ob_state;
					cnfblock[7] = ob[CHECK4].ob_state;
					cnfblock[8] = atol(ob[ED1].TextCast);
					cnfblock[9] = atol(ob[ED2].TextCast);
					cnfblock[10] = atol(ob[ED3].TextCast);
					cnfblock[11] = atol(ob[ED4].TextCast);
					w1 = (long)cnfblock>>16;
					w2 = (int)cnfblock;
					module.smStruct[edit_mod_num]->event_par[0] = w1;
					module.smStruct[edit_mod_num]->event_par[1] = w2;
					module.smStruct[edit_mod_num]->event_par[2] = 12 * 4;
					memorize_emodConfig(module.bp[edit_mod_num], module.smStruct[edit_mod_num]);
*/
					
					/*
					 * und weg mit dem Modul.
					 */
					window_to_handle->module=0;
					module.comm.startEdit("", module.bp[edit_mod_num], MTERM, edit_mod_num, module.smStruct[edit_mod_num]);
					if(module.smStruct[edit_mod_num]->module_mode != M_EXIT)
						Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_TERM_ERR].TextCast, NULL, NULL, NULL, 1);
					check_and_terminate(MTERM, edit_mod_num);

					for(t = 0; t < 7; t++)
						module_pics[edit_mod_num][t] = 0;
					
					back = position_markers[edit_mod_num].smurfpic[0];
					position_markers[edit_mod_num].smurfpic[0] = -1;
					Window.redraw(&picture_windows[back], NULL, 0,0);
					edit_mod_num=-1;
					Dialog.busy.dispRAM();
					Dialog.close(WIND_PICORDER);
				}
	
				/* 
				 * irgendeine Art von Modul? -> Message senden 
				 */
				else
				if(wind_num == 0 && ismodule == 1)
				{
					module_num = window_to_handle->module;

					/*---- Plugin? -----*/
					if(module_num&0x200)
					{
						start_plugin(plugin_bp[module_num&0xFF], MWINDCLOSED, module_num&0xFF, plg_data[module_num&0xFF]);

						if(plg_info[module_num&0xFF]->resident == 0)	/* nichtresident? ->terminieren! */
							terminate_plugin(module_num&0xFF);
					}
					
					/* Exporter? 
					 * Dieser Block gilt nur fÅr exportereigene Dialoge, nicht fÅr das
					 * Smurf-Exportformular! Dieses wird in check_windclose() mit einer
					 * entsprechenden Modulterminierung behandelt.
					 * Hier wird das Modul nur dann terminiert, wenn es auûer fÅr dieses
					 * (sein eigenes) _nicht_ noch fÅr das Exportformular eingetragen ist.
					 * Ist das nÑmlich nicht der Fall, wurde der exportereigene Dialog 
					 * direkt aus dem Listfeld heraus aufgerufen und beim Schlieûen des Dialogs
					 * wird der Exporter nicht lÑnger benîtigt.
					 */
					else
					if(module_num&0x100)
					{
						/*
						 * Exporternummer suchen
						 */
						textseg = module.bp[module_num&0xFF] -> p_tbase;
						modinfo = (MOD_INFO *)*((MOD_INFO **)(textseg + MOD_INFO_OFFSET));	/* Zeiger auf Modulinfostruktur */
						strncpy(module_name, modinfo->mod_name, 30);
						for(t = 0; t < Dialog.expmodList.anzahl; t++)
						{
							if(strncmp(module_name, Dialog.expmodList.modNames[t], strlen(module_name)) == 0)
								break;
						}
	
						*((long *)module.smStruct[module_num&0xFF]->event_par) = (long)export_cnfblock[t];
						module.smStruct[module_num&0xFF]->event_par[2] = export_cnflen[t];
						module.comm.startExport("", MMORECANC, smurf_picture[active_pic], module.bp[module_num&0xFF], module.smStruct[module_num&0xFF], module_num);
						window_to_handle->module=0;
						
						/*
						 * jetzt das Modul ggfs. terminieren.
						 */
						if(exp_conf.export_mod_num == 0 || exp_conf.export_mod_num != module_num)
							check_and_terminate(MTERM, module_num&0xFF);

						Dialog.busy.dispRAM();
					}
					
					/*
					 * also ist es ein Editmodul
					 * Diese werden beim Fensterschlieûen immer terminiert. Daraus
					 * folgt leider, daû momentan nur das Verwalten _eines_ Fensters pro
					 * Editmodul sinnvoll mîglich ist.
					 */
					else
					{
						window_to_handle->module=0;
						module.comm.startEdit("", module.bp[module_num], MTERM, module_num, module.smStruct[module_num]);
						if(module.smStruct[module_num]->module_mode != M_EXIT)
							Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MOD_TERM_ERR].TextCast, NULL, NULL, NULL, 1);
						
						
						check_and_terminate(MTERM, module_num);
						back = position_markers[module_num].smurfpic[0];
						position_markers[module_num].smurfpic[0] = -1;
						Window.redraw(&picture_windows[back], NULL, 0,0);

						for(t = 0; t < 7; t++)
							module_pics[module_num][t] = 0;
					}
	
					Dialog.busy.dispRAM();
				}
				else
					check_windclose(wind_num);
				
				dialwindthere--;
			}

			actualize_menu();		/* MenÅeintrÑge ENABLEn / DISABLEn */
		}
		break;


	/*------------------------- WinX - Messages... --------------------------*/
	case WM_SHADED:
		window_to_handle->shaded |= SHADED;
		break;

	case WM_UNSHADED:
		window_to_handle->shaded &= ~SHADED;
		break;

	/*------------------------ MultiTOS - Messages... -----------------------*/
	/*
	 * Fenster soll iconifiziert werden
	 */
	case WM_ICONIFY: 
		if(wind_num || ismodule == 1)
		{
			Window.windSet(messagebuf[3], WF_ICONIFY, messagebuf[4], messagebuf[5], messagebuf[6], messagebuf[7]);
			Window.windGet(messagebuf[3], WF_WORKXYWH, &window_to_handle->wx, &window_to_handle->wy, &window_to_handle->ww, &window_to_handle->wh);

			window_to_handle->shaded |= ICONIFIED;
		}		
		break;

	/*
	 * Fenster soll deiconifiziert werden
	 */
	case WM_UNICONIFY: 
		if(wind_num || ismodule == 1) 
		{
			Window.windSet(messagebuf[3], WF_UNICONIFY, messagebuf[4],messagebuf[5],messagebuf[6],messagebuf[7]); 

			Window.windGet(messagebuf[3], WF_WORKXYWH, &window_to_handle->wx, &window_to_handle->wy, &window_to_handle->ww, &window_to_handle->wh);

			window_to_handle->shaded &= ~ICONIFIED;
		}
		break;


    /*----------------------- AV-Protokoll ------------------------ */
	/*
	 * Antwort auf Anmeldung per AV-PROTOKOLL
	 */
    case VA_PROTOSTATUS:
        SMfree(send_smurfid);
        break;

	
	/*
	 * Dateien wurden per VA_START Åbergeben 
	 */
	case VA_START:
		argback = *(long *)(messagebuf + 3);

		if(!Smurf_locked && *(char **)(messagebuf + 3) != NULL)
		{
			DraufschmeissBild = VA;
			file_load("", (char **)(messagebuf + 3), VA);
		}

		/*
		 * wenn die Message nicht durch Reload ausgelîst wurde, beim AV-Server bestÑtigen
		 */
		if(Comm.avComm.type != (AV_IMAGE|0xf0))
			Comm.sendAESMsg(Sys_info.ENV_avserver, AV_STARTED, LONG2_2INT(argback), -1);
		else
			Dialog.busy.noEvents = 0;

		break;

	
	/* 
	 * Antwort auf Objektabfrage per VA_WHAT_IZIT 
	 */
	case VA_THAT_IZIT:
		picnum = Window.myWindow(Comm.avComm.windowhandle);

		if(messagebuf[4] == VA_OB_SHREDDER || messagebuf[4] == VA_OB_TRASHCAN)	
		{
			if(Comm.avComm.type == AV_IMAGE)		/* Bild wurde weggeworfen */
			{
				if(picnum <0)
					Window.close(Comm.avComm.windowhandle);		/* und weg damit. */
			}
			else
			if(Comm.avComm.type == AV_BLOCK)		/* Block wurde weggeworfen */
			{
				if(picnum < 0)
					imageWindow.removeBlock(&picture_windows[-picnum]);
			}
		}
		else
		if(messagebuf[4] == VA_OB_CLIPBOARD)
		{
			if(Comm.avComm.type == AV_BLOCK && smurf_picture[-picnum]->block != NULL)
				block2clip(smurf_picture[-picnum], 0, NULL);
		}
		else
		if(messagebuf[4] == VA_OB_FOLDER || messagebuf[4] == VA_OB_DRIVE)
		{
			if(Comm.avComm.type == AV_BLOCK && smurf_picture[-picnum]->block != NULL)
				block2clip(smurf_picture[-picnum], 0, (char *)*((long *)(&messagebuf[5])));
		}
		else
		{
/*
			Dialog.winAlert.openAlrt("Nur auf Desk-Ordner, Laufwerke, Papierkorb und andere Programme!", NULL, NULL, NULL, 1);
	/*
			printf("\nZielobjekt: %i", messagebuf[4]);
			if((char *)*((long *)(&messagebuf[5])) !=NULL)
				printf("\nName %s", (char*) *((long*)(&messagebuf[5])));
	*/
*/
		}

		/* av_comm lîschen, damit ich bei der nÑxten msg bescheid weiû, ob sie von mir ist */
		memset(&Comm.avComm, 0, sizeof(AV_COMM));	

		break;


	/*----------------------- kleinere Protokolleinbindungen -------------------*/
	/*
	 * BubbleGEM-Request -> Bubblehelp aufrufen
	 */
	case BUBBLEGEM_REQUEST:
		wind_num = Window.myWindow(messagebuf[3]);
		if(wind_num < 0)
			Comm.bubbleGem(-(picture_windows[-wind_num].wnum), messagebuf[4], messagebuf[5], 0);
		else
			if(wind_num > 0)
				Comm.bubbleGem(wind_num, messagebuf[4], messagebuf[5], 0);
			else
				if(wind_num == 0 && Window.myModuleWindow(messagebuf[3]))
					Comm.bubbleGem(messagebuf[3], messagebuf[4], messagebuf[5], 1);
		break;

	/*
	 * BubbleGEM-Acknowledge -> Bubblehelp-String freigeben
	 */
	case BUBBLEGEM_ACK:
		ext_com_ptr = *(char **)&messagebuf[5];
		if(messagebuf[5] != 0 && messagebuf[6] != 0)
			SMfree(ext_com_ptr);							/* entspricht helpstring aus bubble_gem() */
		break;

	/*
	 * Document History - Acknowledge
	 */
	case DHST_ACK:
		ext_com_ptr = *(char **)&messagebuf[3];
		if(messagebuf[3] != 0 && messagebuf[4] != 0)
		{
			SMfree(((DHSTINFO *)ext_com_ptr)->appname);		/* entspricht string aus update_dhst() */
			SMfree(ext_com_ptr);							/* entspricht history aus update_dhst() */
		}
		break;


	/*
	 * unbekannte Messages an Module weiterleiten
	 */
	default:	AESmsg_to_modules(messagebuf);
				break;

	/*----------------------- grîûere Protokolleinbindungen... -------------------*/
	/*
	 * OLGA_INIT
	 */
	case OLGA_INIT:	if(messagebuf[7]!=0)
					{
						if(Sys_info.olgaman_ID <0)
							Sys_info.olgaman_ID = -Sys_info.olgaman_ID;
					}
					break;

	/*
	 * OLE_EXIT: Der Olgamanager hat terminiert
	 */
	case OLE_EXIT:	Sys_info.olgaman_ID = -1;
					break;

	/*
	 * OLE_NEW: ein Olgamanager wurde nachgestartet - mal Hallo sagen.
	 */
	case OLE_NEW:	Sys_info.olgaman_ID = messagebuf[1];
					Comm.sendAESMsg(Sys_info.olgaman_ID, OLE_INIT, OL_SERVER, 0,0,0, 'RG', -1);
					break;

	/*
	 * OLGA_ACK: Update- oder Renamemessage ist angekommen - Dateinamen freigeben
	 */
	case OLGA_ACK:	ext_com_ptr = *(char **)&messagebuf[3];
					if(messagebuf[3] != 0 && messagebuf[4] != 0)
						SMfree(ext_com_ptr);				/* entspricht old_filename oder new_filename aus bubble_gem() */					
					ext_com_ptr = *(char **)&messagebuf[5];
					if(messagebuf[5] != 0 && messagebuf[6] != 0)
						SMfree(ext_com_ptr);				/* entspricht new_filename aus bubble_gem() */
					break;
	
	}

	return(0);
} /* f_handle_message */


/* Messagehandler fÅr Betrieb des FSEL im Fenster */
/* 
 * Die Einsprungadresse dieser Funktion wird als Callback an BoxKite
 * Åbergeben. 
 * Wie man sieht, kann sie auch andere Messagetypen als WM_REDRAW
 * behandeln. WM_MOVED ist sogar ratsam, da es immerhin Betriebssystem-
 * versionen gibt, die das Verschieben von hintenliegenden Fenstern
 * ermîglichen. Messages, die implizit neue Fenster îffnen oder vorhandene 
 * nach oben bringen, sollten dagegen ignoriert oder aufgehoben und erst 
 * nach der RÅckkehr aus dem Fileselector behandelt werden.
 * WM_TOPPED-Messages werden nicht an den Message-Callback durchgereicht.
 */
void cdecl message_handler(int *msg)
{
	switch(msg[0])
	{
		case WM_REDRAW: memcpy(messagebuf, msg, 16);
						f_handle_message();
						break;
		case WM_MOVED:	memcpy(messagebuf, msg, 16);
						f_handle_message();
						break;
		default:		break;
	}

	return;
} /* message_handler */