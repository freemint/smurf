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

#include <tos.h>
#include <errno.h>
#include <aes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vaproto.h"
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurfine.h"
#include "smurf_f.h"
#include "popdefin.h"
#include "plugin\plugin.h"

#include "xrsrc.h"
#include "globdefs.h"

#include "smurfobs.h"
#include "ext_obs.h"
#include "bindings.h"

extern	WINDOW	wind_s[25];
extern	WINDOW	picture_windows[MAX_PIC];
extern	SYSTEM_INFO	Sys_info;

extern	char	*edit_modules[100];			/* Pfade fr bis zu 100 Edit-Module */

#define BUBBLEGEM_SHOW	0xBABB
#define	BUBBLEGEM_ACK	0xBABC

extern	int	obj;					/* Objekt beim loslassen des Buttons */


void bubble_gem(int windownum, int xpos, int ypos, int modulemode)
{
	int klickobj;
	char helppath[256];
	char helpname[64];
	char *helpstring;
	char *help_file, *search_pos, *end_of_string, *modpath;
	int found = 0, end = 0;
	char num_byte[5];
	int bubble_id, general_info=0;
	WINDOW *window;

	char mod_name_string[30];
	char *cmp_string;
	char *textseg_begin;
	MOD_INFO *mod_info;
	int mod_index, t;
	long mod_magic;

	extern	BASPAG *plugin_bp[11];
	extern	char *plugin_paths[11];

	obj = 0;									/* globale Objektvariable l”schen */


	/* 
	 * Fensterstruktur ermitteln
	 */
	if(modulemode == 0)					
	{
		if(windownum > 0)
			window = &wind_s[windownum];
		else
			if(windownum < 0)
				window = &picture_windows[-windownum];
	}
	else
		window = Window.myModuleWindow(windownum);

	if(window->shaded)
		return;
	if((klickobj = objc_find(window->resource_form, 0, MAX_DEPTH, xpos, ypos)) == -1)
		return;

	if(windownum == WIND_MODULES && !(Dialog.emodList.tree[INFO_MODULE].ob_state&SELECTED))
		if(klickobj >= MODULE1 && klickobj <= MODULE9)
			general_info = 1;
			
	if(windownum==WIND_MODFORM)
		modulemode=1;

	/*
	 * allgemeine Editmodulinfo?
	 */
	if(general_info)
	{
		mod_index = klickobj - MODULE1 + Dialog.emodList.modList.scroll_offset;
		strcpy(helpname, strrchr(edit_modules[mod_index], '\\'));
		cmp_string = strrchr(helpname, '.');
		*cmp_string = 0;
		strcat(helpname, ".hlp");
	}
	else
	{
		/*
		 * fest definierte Dialognummern
		 */
		if(!modulemode && windownum != WIND_MODFORM)
		{
			if(windownum < 0)											/* Bildfenster */
				windownum = 97;
			else
				if(windownum == WIND_MODULES)
				{
					if(Dialog.emodList.tree[INFO_MODULE].ob_state&SELECTED)	/* Editmodul-Info */
						windownum = 99;
				}
				else
					if(windownum == WIND_EXPORT)						/* Exportmodul-Info */
					{
						if(Dialog.expmodList.tree[EXMOD_INFO].ob_state&SELECTED) 
							windownum = 98;
					}

			strcpy(helpname, "\\bubble.hlp");
		}
		/*
		 * Modulfenster
		 */
		else
		{
			mod_index = (window->module) & 0xff;
			strcpy(helpname, "\\");

			if(window->module&0x200)
				mod_magic = get_modmagic(plugin_bp[mod_index]);
			else
				mod_magic = get_modmagic(module.bp[mod_index]);

			if(mod_magic=='SEMD' || mod_magic=='SXMD')
			{
				textseg_begin = module.bp[mod_index]->p_tbase;
				mod_info = (MOD_INFO *)*((MOD_INFO **)(textseg_begin + MOD_INFO_OFFSET));

				/*---- Modulname kopieren und mit Leerzeichen auffllen */
				memset(mod_name_string, 32, 30);
				strncpy(mod_name_string, mod_info->mod_name, 28);
				if(strlen(mod_name_string) < 28)
						strncat(mod_name_string, "                           ", 28 - strlen(mod_name_string));

				for(t = 0; t < 100; t++)
				{
					cmp_string = Dialog.emodList.modNames[t];
					if(strcmp(cmp_string, mod_name_string) == 0)
						break;
				}

				if(t >= 99)
					return;
				modpath = edit_modules[t];
			}
			else
				if(mod_magic == 'SPLG')
				{
					textseg_begin = plugin_bp[mod_index]->p_tbase;

					modpath = plugin_paths[mod_index];
				}

			/*---- Modulpfad kopieren, Name isolieren und .hlp dranh„ngen. */
			strcat(helpname, strrchr(modpath, '\\') + 1);
			cmp_string = strrchr(helpname, '.');
			*cmp_string = 0;
			strcat(helpname, ".hlp");
		}
	}

	strcpy(helppath, Sys_info.standard_path);
	strcat(helppath, "\\bubble");
	strcat(helppath, helpname);
	memset(num_byte, 0, 5);

	help_file = fload(helppath, 0);
	if(help_file == NULL)
		return;

	search_pos = help_file;


	/*-------- Block wenn m”glich als global anfordern? */
	if(Ssystem(FEATURES, 0L, 0L) != EINVFN || Sys_info.OS&MINT || Sys_info.OS&MATSCHIG)
		helpstring = (char *)Mxalloc(257, 0x20);
	else
		helpstring = (char *)SMalloc(257);

	if(general_info)
	{
		search_pos = strstr(help_file, "#GINF");
		if(search_pos == NULL)
		{
			SMfree(help_file);
			SMfree(helpstring);
			return;
		}

		search_pos += 5 + 2;								/* #GINF und 1 CRLF weiter */
		end_of_string = strchr(search_pos, '@');
		memset(helpstring, 0x0, 256);
		strncpy(helpstring, search_pos, end_of_string - search_pos);
	}
	else
	{
		/*------ "Directory-Eintrag" suchen */
		if(!modulemode)
		{
			do
			{
				search_pos = strchr(search_pos, '#');
				memcpy(num_byte, search_pos + 1, 3);
				if(atoi(num_byte) == windownum)
					found = 1;
				else
					if(atoi(num_byte) == 0)
						end = 1;
				search_pos++;
			} while(!found && !end);
		
			if(end)
			{
				SMfree(helpstring);
				SMfree(help_file);
				return;
			}
		}
	
		/*------ "Subdirectory-Eintrag" suchen */
		end = found = 0;
		do
		{
			search_pos = strchr(search_pos, '-');
			if(search_pos != NULL)
			{
				memset(num_byte, 0x0, 5);
				memcpy(num_byte, search_pos + 1, 3);
				if(atoi(num_byte) == klickobj)
					found = 1;
				else
					if(atoi(num_byte) == -1)
						end = 1;
					else 
						search_pos = strchr(search_pos, '@');
					
				search_pos++;
			} 
			else
				end = 1;
		} while(!found && !end);
	
		if(end)
		{
			SMfree(helpstring);
			SMfree(help_file);
			return;
		}
	
		search_pos += 5;						/* 3 Ziffern und 1 CRLF weiter */
		end_of_string = strchr(search_pos, '@');
	
		memset(helpstring, 0x0, 256);
		strncpy(helpstring, search_pos, end_of_string - search_pos);
	}

	/*------ Message an BubbleGEM schicken -----------*/
	if((bubble_id = appl_find("BUBBLE  ")) > 0)
		Comm.sendAESMsg(bubble_id, BUBBLEGEM_SHOW, xpos, ypos, LONG2_2INT(helpstring), -1);

	/* auf keinen Fall hier schon helpstring freigeben! */
	/* das wird nach Antwort von Bubble-GEM durch BUBBLEGEM_ACK gemacht */
	SMfree(help_file);

	return;	
} /* bubble_gem */



/* call_stguide	--------------------------------------------------
	Ruft den ST-Guide mit dem zum Fensterhandle topwin_handle passenden
	Hypertext auf der passenden Seite auf, Module und Plugins inklusive.
	-------------------------------------------------------------*/ 
void call_stguide(int topwin_handle)
{
	int STG_id, wnum,t,mod_index;
	WINDOW *window;
	MOD_INFO *mod_info;
	char *wtitle, *modpath, *textseg_begin, *cmp_string;
	char hypname[128] = "*:\\smurf.hyp ";
	char mod_name_string[30];
	long mod_magic;

	/*
	 *	ST-Guide suchen
	 */
	STG_id = appl_find("ST-GUIDE");
	if(STG_id < 0)
	{
		Dialog.winAlert.openAlert("ST-Guide nicht gefunden!", NULL, NULL, NULL, 1);
		return;
	}

	/*
	 *	HELP in einem SMURF-Dialogfenster?
	 */
	wnum = Window.myWindow(topwin_handle);
	if(wnum > 0 && wnum != WIND_MODFORM)
	{
		wtitle = wind_s[wnum].wtitle;
		if(topwin_handle == wind_s[FORM_EXPORT].whandlem)
			strcat(hypname, "Exportdialog");
		else
			strcat(hypname, wtitle);
		Comm.sendAESMsg(STG_id, VA_START, (int)((long)hypname>>16), (int)hypname, -1);
	}
	else
	if(wnum < 0)		/* Bildfenster! */
	{
		strcat(hypname, "Bildfenster");
		Comm.sendAESMsg(STG_id, VA_START, (int)((long)hypname>>16), (int)hypname, -1);
	}

	/*
	 *	oder in einem Modulfenster?
	 */
	else
	if(wnum == 0)
	{
		window = Window.myModuleWindow(topwin_handle);
		if(window == NULL)
			return;
		
		strcpy(hypname, "*:\\");

		mod_index = (window->module)&0xFF;
		
		if(window->module&0x200)
			mod_magic = get_modmagic(plugin_bp[mod_index]);
		else 
			mod_magic = get_modmagic(module.bp[mod_index]);
		

		if(mod_magic=='SEMD' || mod_magic=='SXMD')
		{
			textseg_begin = module.bp[mod_index]->p_tbase;
			mod_info = (MOD_INFO *)*((MOD_INFO* *)(textseg_begin + MOD_INFO_OFFSET));
			
			/*---- Modulname kopieren und mit Leerzeichen auffllen */
			memset(mod_name_string, 32, 30);
			strncpy(mod_name_string, mod_info->mod_name, 28);
			if(strlen(mod_name_string) < 28)
				strncat(mod_name_string, "                           ", 28-strlen(mod_name_string));

			for(t = 0; t < 100; t++)
			{
				cmp_string = Dialog.emodList.modNames[t];
				if(strcmp(cmp_string, mod_name_string) == 0)
					break;
			}

			if(t >= 99)
				return;

			modpath = edit_modules[t];
		}
		else
		if(mod_magic == 'SPLG')
		{
			textseg_begin = plugin_bp[mod_index]->p_tbase;

			modpath = plugin_paths[mod_index];
		}

		/*---- Modulpfad kopieren, Name isolieren und .HYP dranh„ngen. */
		strcat(hypname, strrchr(modpath, '\\') + 1);
		cmp_string = strrchr(hypname, '.');
		*cmp_string = 0;
		strcat(hypname, ".hyp");

		Comm.sendAESMsg(STG_id, VA_START, (int)((long)hypname >> 16), (int)hypname, -1);
	}

	return;
}