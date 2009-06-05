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
#include <string.h>
#include <stdio.h>
#include <ext.h>

#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "globdefs.h"
#include "destruct.h"

#include "smurfobs.h"
#include "ext_obs.h"
#include "debug.h"


extern void	call_plugin(int menuentry);

extern OBJECT *menu_tree;
extern SMURF_PIC *smurf_picture[25];
extern WINDOW wind_s[25];
extern WINDOW picture_windows[25];
extern SYSTEM_INFO Sys_info;
extern int openmode;
extern void blockfunctions_off(void);
extern void blockfunctions_on(void);
extern void block_over_all(WINDOW *window);
extern void insert_blockcoords(SMURF_PIC *pic);
extern void color_choose(void);
extern int duplicate_pic(WINDOW *window);
extern void save_file(void);

extern int picwindthere, dialwindthere, picthere;
extern int appl_id;
extern int active_pic;
extern int *messagebuf;

extern int mouse_button, key_at_event, key_scancode;
extern char Smurf_locked;


void f_init_menu(void)
{
	menu_bar(menu_tree, 1);
}


void f_exit_menu(void)
{
	menu_bar(menu_tree, 0);
}


/*	-------------------------------------------	*/
/*					Menu Handler				*/
/*	-------------------------------------------	*/
int f_handle_menuevent(int *message)
{
	int menu_title, menu_entry;
	int quit = 0, dummy;
	int wh, wnum, nexthandle, t;

	WINDOW *nextwin;
	SMURF_PIC *picture;
	
	extern signed char menu2plugin[128];		/* feste MenÅeintrÑge, in die Plugins eingehÑngt sind */
	extern void reload_pic(WINDOW *picwindow);
	extern DISPLAY_MODES Display_Opt;


	menu_title = message[3];
	menu_entry = message[4];

	if(Smurf_locked)							/* GUI gesperrt? */
		return(0);

	picture = smurf_picture[active_pic];

/*	printf("\nMenÅ: %i", menu_title);
	printf("\nEintrag: %i", menu_entry);*/
/*
	if(Sys_info.OS&MAG_PC && (key_at_event == KEY_ALT || key_at_event == KEY_CTRL))
		menu_title -= 5;
*/

	openmode = 0;	/* Immer davon ausgehen, daû der Dialog 				*/
					/* (welcher auch immer) noch nicht offen ist,			*/
					/* um bei einem Menuevent einen folgenden Buttonevent	*/
					/* zu vermeiden / zu ignorieren. 						*/


	if(menu2plugin[menu_entry] != -1)			/* ein Plugin hat sich hier eingehÑngt! */
	{
		call_plugin(menu_entry);
		return(0);
	}

	switch(menu_title)
	{
		case MENU_DESK:		if(menu_entry == DESK_INFO)
								f_info();
							break;
	
		case MENU_FILE:		if(menu_entry == MFILE_LOAD)
								file_load("Bild laden", (char**)&"", 0);
							else
							if(menu_entry == FILE_RELOAD) 
								reload_pic(&picture_windows[active_pic]);
							else
							if(menu_entry == FILE_SAVEAS) 
								Dialog.expmodList.handleList();
							else
							if(menu_entry == FILE_SAVE) 
								save_file();
							else
							if(menu_entry == FILE_INFO)
								f_pic_info();
/*							else
							if(menu_entry == FILE_SCAN)
								gdps_main();
*/
							else if(menu_entry == FILE_QUIT)
							{
								quit = -1;
								if(picthere)
								{
									for(t = 1; t <= picthere; t++)
									{
										if(picture_windows[t].wtitle[11] == '*' && !(Sys_info.profi_mode&SELECTED))
											if(Dialog.winAlert.openAlert(Dialog.winAlert.alerts[QUIT_ALERT].TextCast, "Nein", " Ja ", NULL, 1) == 1)
												quit = 0;
									}
								}
							}
							else
							if(menu_entry == WIND_CLOSE)
							{
								Window.windGet(0, WF_TOP, &wh, &dummy, &dummy, &dummy);
								Window.close(wh);
							}
							else
							{
								if(menu_entry==FILE_NEW)
									f_newpic(0);
							}
							break;
	
		case MENU_DISP:		if(menu_entry == DISP_KONFIG)
								Dialog.dispOpt.displayOptions();
							/*---- Dialogfenster cyclen -------*/
							else
							if(menu_entry == DISP_NEXTWIN)
							{
								Window.windGet(0, WF_TOP, &wh, &dummy, &dummy, &dummy);
								wnum = Window.myWindow(wh);
								/*----- ist ein Dialog oben? ---*/
								if(wnum > 0)
								{
									nextwin = (WINDOW *)(wind_s[wnum].next_window);
									if(nextwin != NULL)
										nexthandle = nextwin->whandlem;
								}
								/*----- oder ein Bild? */
								else
								{
									if(Dialog.topDialog != 256)
										nexthandle = Dialog.topDialog;
									else
									{
										nexthandle = 256;
										for(t = 0; t < MAX_PIC; t++)
										{
											if(wind_s[t].whandlem != -1 && wind_s[t].whandlem < nexthandle)
												nexthandle = wind_s[t].whandlem;
										}
									}
								}
	
								if(nexthandle != 256)
								{
									Dialog.topDialog = nexthandle;
									Window.top(nexthandle);
								}
							}
							/*---- Bildfenster cyclen ---*/
							else
							if(menu_entry == DISP_NEXTPIC)
							{
								Window.windGet(0, WF_TOP, &wh, &dummy, &dummy, &dummy);
								wnum = Window.myWindow(wh);
								/*------ ist ein Bild oben? ---*/
								if(wnum < 0)
								{
									nextwin = (WINDOW *)(picture_windows[-wnum].next_window);
									if(nextwin != NULL)
										nexthandle = nextwin->whandlem;
								}
								/*--- oder ein Dialog? ---*/
								else
								{
									if(imageWindow.topPicwin != 256)
										nexthandle = imageWindow.topPicwin;
									else
									{
										nexthandle = 256;
										for(t = 0; t < MAX_PIC; t++)
										{
											if(picture_windows[t].whandlem != -1 && picture_windows[t].whandlem < nexthandle)
												nexthandle = picture_windows[t].whandlem;
										}
									}
								}
								
								if(nexthandle != 256) 
								{
									imageWindow.topPicwin = nexthandle;
/*									Window.top(nexthandle); */
									wnum = Window.myWindow(nexthandle);
									if(wnum < 0)
									{
										Window.topNow(&picture_windows[-wnum]);
										f_activate_pic(-wnum);
									}
									else
										Window.topNow(&picture_windows[-wnum]);
								}
							}
							break;
							
		case MENU_EDIT:		if(menu_entry == EDIT_EDIT)
								f_edit_pop();
							else
							if(menu_entry == EDIT_COLORS)
								color_choose();
							else
							if(menu_entry == EDIT_COLRED)
								transform_pic();
							else
							if(menu_entry == DUPLICATE_PIC)
								duplicate_pic(&picture_windows[active_pic]);
							else
							if(menu_entry == SELECT_ALL)
								block_over_all(&picture_windows[active_pic]);
							else
							if(menu_entry == EDIT_COPY)
								block2clip(picture, 0, NULL);
							else
							if(menu_entry == EDIT_INSERT)
							{
								clip2block(picture, NULL, -1, -1);
								Window.redraw(&picture_windows[active_pic], NULL, 0, 0);
							}
							else
							if(menu_entry == EDIT_CUT)
							{
								if(block2clip(picture, 1, NULL) != -1)
								{
									imageWindow.toggleAsterisk(&picture_windows[active_pic], 1);
									f_dither(picture, &Sys_info, 1, NULL, &Display_Opt);
									Window.redraw(&picture_windows[active_pic], NULL,0, 0);
								}
							}
							else
							if(menu_entry == EDIT_RELEASE)
							{
								imageWindow.removeBlock(&picture_windows[active_pic]);
								blockfunctions_off();
								Window.redraw(&picture_windows[active_pic], NULL, 0, 0);
								if(wind_s[WIND_BTYPEIN].whandlem!=-1)
									insert_blockcoords(picture);
							}
							else
							if(menu_entry == EDIT_CROP)
							{
								block_freistellen(&picture_windows[active_pic]);
								blockfunctions_off();
							}
							
							break;
	
		case MENU_OPTIONS:	if(menu_entry == OPT_OPTIONS)
								Dialog.smurfOpt.options();
							else
							if(menu_entry == OPT_STATUS)
								Window.open(&wind_s[WIND_BUSY]);
							else
							if(menu_entry == OPT_PICMAN)
								Window.open(&wind_s[WIND_PICMAN]);
							else
							if(menu_entry == OPT_SAVEOPT)
								save_config();
							break;

		case MENU_PLUGIN:	call_plugin(menu_entry);
							break;
	}
	
	return(quit);
}


/*-------------------------------------------------------------
	Aktualisiert die MenÅzeile je nachdem, ob ein Bild geladen 
	ist, oder nicht. 
	ZusÑtzlich werden die Start-Buttons in den Modullisten und im
	Standard-Editmoduldialog en-/disabled.
	----------------------------------------------------------*/
void actualize_menu(void)
{
	extern int printplug_found;
	extern EXPORT_CONFIG exp_conf;

	DEBUG_MSG (( "actualize_menu...\n" ));

	menu_tree[WIND_CLOSE].ob_state |= DISABLED;
	menu_tree[FILE_RELOAD].ob_state |= DISABLED;
	menu_tree[FILE_INFO].ob_state |= DISABLED;
	menu_tree[FILE_PRINT].ob_state |= DISABLED;
	menu_tree[FILE_SAVE].ob_state |= DISABLED;
	menu_tree[EDIT_COLRED].ob_state |= DISABLED;
	menu_tree[DUPLICATE_PIC].ob_state |= DISABLED;
	menu_tree[SELECT_ALL].ob_state |= DISABLED;
	menu_tree[EDIT_INSERT].ob_state |= DISABLED;
	menu_tree[DISP_NEXTPIC].ob_state |= DISABLED;
	menu_tree[DISP_NEXTWIN].ob_state |= DISABLED;

	change_object(&wind_s[WIND_MODULES], START_MODULE, DISABLED, 1);
	change_object(&wind_s[WIND_EXPORT], START_EMOD, DISABLED, 1);
	change_object(&wind_s[WIND_MODFORM], START_MOD, DISABLED, 1);

	if(Dialog.emodList.anzahl == 0)
		menu_tree[EDIT_EDIT].ob_state |= DISABLED;

	if(Dialog.expmodList.anzahl == 0)
		menu_tree[FILE_SAVEAS].ob_state |= DISABLED;

	if(picwindthere || dialwindthere)
	{
		menu_tree[WIND_CLOSE].ob_state &= ~DISABLED;

		if(picwindthere > 0)
		{
			menu_tree[FILE_RELOAD].ob_state &= ~DISABLED;
			menu_tree[FILE_INFO].ob_state &= ~DISABLED;

			if(printplug_found)
				menu_tree[FILE_PRINT].ob_state &= ~DISABLED;

			if(Sys_info.defaultExporter!=-1)
				menu_tree[FILE_SAVE].ob_state &= ~DISABLED;

			menu_tree[EDIT_COLRED].ob_state &= ~DISABLED;
			menu_tree[DUPLICATE_PIC].ob_state &= ~DISABLED;
			menu_tree[SELECT_ALL].ob_state &= ~DISABLED;
			menu_tree[EDIT_INSERT].ob_state &= ~DISABLED;
			menu_tree[DISP_NEXTPIC].ob_state &= ~DISABLED;

			/*
			 * mal was anderes, hat nichts mit der MenÅzeile
			 * zu tun, der Status wird aber genauso bestimmt
			 */
			change_object(&wind_s[WIND_MODULES], START_MODULE, ENABLED, 1);
			change_object(&wind_s[WIND_EXPORT], START_EMOD, ENABLED, 1);
			change_object(&wind_s[WIND_MODFORM], START_MOD, ENABLED, 1);
		}

		if(dialwindthere > 0) 
			menu_tree[DISP_NEXTWIN].ob_state &= ~DISABLED;
	}

	return;
}


void lock_Smurf(void)
{
	menu_ienable(menu_tree, DESK_INFO, 0);	/* MenÅeintrag ausschalten */
	menu_ienable(menu_tree, MENU_FILE, 0);	/* MenÅeintrag ausschalten */
	menu_ienable(menu_tree, MENU_DISP, 0);	/* MenÅeintrag ausschalten */
	menu_ienable(menu_tree, MENU_EDIT, 0);	/* MenÅeintrag ausschalten */
	menu_ienable(menu_tree, MENU_OPTIONS, 0);	/* MenÅeintrag ausschalten */
	menu_ienable(menu_tree, MENU_PLUGIN, 0);	/* MenÅeintrag ausschalten */

	menu_bar(menu_tree, 1);

	Smurf_locked = 1;

	return;
} /* lock_Smurf */


void unlock_Smurf(void)
{
	menu_ienable(menu_tree, DESK_INFO, 1);	/* MenÅeintrag einschalten */
	menu_ienable(menu_tree, MENU_FILE, 1);	/* MenÅeintrag einschalten */
	menu_ienable(menu_tree, MENU_DISP, 1);	/* MenÅeintrag einschalten */
	menu_ienable(menu_tree, MENU_EDIT, 1);	/* MenÅeintrag einschalten */
	menu_ienable(menu_tree, MENU_OPTIONS, 1);	/* MenÅeintrag einschalten */
	menu_ienable(menu_tree, MENU_PLUGIN, 1);	/* MenÅeintrag einschalten */

	menu_bar(menu_tree, 1);

	Smurf_locked = 0;

	return;
} /* unlock_Smurf */