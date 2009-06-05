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

/*#include <multiaes.h>*/
#include <aes.h>
#include <stdio.h>
#include <screen.h>
#include <stdlib.h>
#include <ext.h>
#include "sym_gem.h"
#include "..\modules\import.h"
#include "globdefs.h"
#include "smurfine.h"
#include "smurf_st.h"

#include "smurfobs.h"
#include "ext_obs.h"

void cursor_off(WINDOW *window);
void cursor_on(WINDOW *window);

extern int key_at_event;
extern SYSTEM_INFO Sys_info;

int handle_keyboardevent(WINDOW *wind_s, int scancode, int *sel_object)
{
	int keybback;
	int nextchar;
	int np_edob, np_nextob, np_edx;
	int my_code, ret_val;
	static int old_edob;
	OBJECT *form;


	np_edob = wind_s->editob;
	np_edx = wind_s->editx;
	np_nextob = np_edob;
	form = wind_s->resource_form;

	my_code = scancode >> 8;

	if(np_edob < 0)
	{ 
		np_edob = -np_edob;
		np_nextob = -np_nextob;
	}

	/* EDIT-OBJEKTE: Profibuch S. 568 ff */
	wind_update(BEG_UPDATE);

	keybback = form_keybd(form, np_edob, 0, scancode, &np_nextob, &nextchar);

	/*
	 * sonst wird beim ersten Durchlauf old_edob,
	 * und damit auch *sel_object auf 0 gesetzt
	 */
	 /* verstehe ich nicht, passiert doch gar nicht, bzw. editob wird */
	 /* doch fr jedes Formular in der init_smurfrsc() initialisiert */
	 /* testweise raus da sonst nach Benutzung eines Dialogs */
	 /* mit Editfeldern die Autolocater der Listfelder grundlos anschlugen */
/*	if(old_edob == 0) */
		old_edob = np_edob;
		
	/* ist das Editobjekt disabled? */
	if(IsDisabled(wind_s->resource_form[np_nextob]))
	{
		np_nextob = np_edob;
		np_nextob = -1;
	}

	/* Haben wir ein neues Editobjekt? */
	if(np_nextob != 0)
	{
		*sel_object = np_nextob;
		if(form[np_nextob].ob_flags&EDITABLE) old_edob = np_nextob;
		ret_val = keybback;
	}
	else
	{ 
		*sel_object = old_edob;
		ret_val = 1;

		if(IsDisabled(wind_s->resource_form[np_edob]))
			np_nextob = -1;
	}
		

	if(np_nextob != -1 && !(key_at_event&KEY_SHIFT && (my_code == KEY_DOWN || my_code == KEY_UP)))
	{
		/* anderes edob oder Ende? (Alten Cursor abschalten?) */
		if(keybback == 0)
		{
			if(!IsDisabled(wind_s->resource_form[np_nextob]))
				objc_edit(wind_s->resource_form, np_edob, 0, &(np_edx), ED_END);
		}
	
		/* Hat sich das Editobjekt ge„ndert? (neuen Cursor einschalten?) */
		if(np_nextob != np_edob && np_nextob != 0 && keybback != 0) 
		{
			if(!IsDisabled(wind_s->resource_form[np_nextob]))
				objc_edit(wind_s->resource_form, np_edob, 0, &(np_edx), ED_END);

			np_edob = np_nextob;
			np_nextob = 0;

			if(!IsDisabled(wind_s->resource_form[np_edob]))
				objc_edit(wind_s->resource_form, np_edob, 0, &(np_edx), ED_INIT);
		}
		else
			if(nextchar)
				objc_edit(form, np_edob, nextchar, &np_edx, ED_CHAR);

		wind_s->editob = -np_edob;
		wind_s->nextedit = np_nextob;
		wind_s->editx = np_edx;
	}

	wind_update(END_UPDATE);

	return(ret_val);
}


void cursor_on(WINDOW *window)
{
	char tempchar;

	int tophandle, dummy;


	Window.windGet(window->whandlem, WF_TOP, &tophandle, &dummy, &dummy, &dummy);

	if(window != NULL && window->editob > 0 && window->whandlem == tophandle)
		if(!IsDisabled(window->resource_form[window->editob]))
		{
			/* Faking editx fr Cursorpositionierung */
			tempchar = window->resource_form[window->editob].ob_spec.tedinfo->te_ptext[window->editx];
			window->resource_form[window->editob].ob_spec.tedinfo->te_ptext[window->editx] = '\0';

			objc_edit(window->resource_form, window->editob, 0, &window->editx, ED_INIT);
/*			if(Sys_info.OS&MATSCHIG)
				objc_edit(window->resource_form, window->editob, 0, &window->editx, 100); */

			window->resource_form[window->editob].ob_spec.tedinfo->te_ptext[window->editx] = tempchar;

			window->editob = -window->editob;
		}

	return;
}


void cursor_off(WINDOW *window)
{
	if(window!=NULL && window->editob<0 && !IsDisabled(window->resource_form[-window->editob]))
	{
		window->editob = -window->editob;
		objc_edit(window->resource_form, window->editob, 0, &window->editx, ED_END);
	}
}
