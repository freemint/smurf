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
#include <stdlib.h>
#include <stddef.h>
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "popdefin.h"
#include "smurf_st.h"
#include "smurf_f.h"

#include "smurfobs.h"
#include "ext_obs.h"


extern WINDOW wind_s[25];
extern int klicks;
extern int key_at_event, key_scancode, mouse_xpos, mouse_ypos, mouse_button;
extern int *messagebuf;


/* --------------------------------------------------------------------	*/
/*		RADIOBUTTONS (nicht hintergrundbedienbar wg. form_button!) 		*/
/*	Auûerdem Bildreihenfolge - Modulinfo - Button...					*/
/* --------------------------------------------------------------------	*/
void f_handle_radios(OBJECT *tree, int klickobj, int windnum)
{
	int newedit;


	if(tree[klickobj].ob_flags&RBUTTON && !(tree[klickobj].ob_state&DISABLED))
	{
		Window.topNow(&wind_s[windnum]);
		form_button(wind_s[windnum].resource_form, klickobj, klicks, &newedit);	
	}

	return;
} /* f_handle_radios */


/* ----------------------------------------------------------------	*/
/*						f_handle_editklicks							*/
/*	Handled das Setzen des Cursors auf ein neues Editobjekt bei		*/
/*	Klick darauf.													*/
/* ----------------------------------------------------------------	*/
void f_handle_editklicks(WINDOW *window, int object)
{
	OBJECT *resource;


	resource = window->resource_form;
	
	if((resource[object].ob_type == G_FTEXT || resource[object].ob_type == G_FBOXTEXT) &&
	   !IsDisabled(resource[object]))
	{
		Window.cursorOff(window);
		window->editob = object;
		Window.cursorOn(window);	
	}

	return;
} /* f_handle_editklicks */


/* ----------------------------------------------------------------	*/
/*							UDOs feststellen 						*/
/* ----------------------------------------------------------------	*/
int UDO_or_not(WINDOW *wind, int klickobj)
{
	static int mode;
	int objct, timo, evback, type;
	int ox,oy, dummy, button;

	extern int active_pic, picthere;
	extern void make_modpreview(WINDOW *wind);
	extern int cdecl f_do_checkbox	(PARMBLK *parm);
	extern int cdecl f_do_cycle(PARMBLK *parm);
	extern SMURF_PIC *smurf_picture[MAX_PIC];

	SMURF_PIC *prevpic;
	OBJECT *ob;

	
	ob = wind->resource_form;
	if(IsDisabled(ob[klickobj]))
		return(klickobj);

	type = ob[klickobj].ob_type >> 8;

	/* Use-Direct-Object? */
	if(klickobj > 0 && type != PREVIEW_ACTION)
	{
		if(type == UDO || (ob[klickobj].ob_flags&RBUTTON) ||
		   (ob[klickobj].ob_type&0x00ff) == G_USERDEF)
		{
			objct = klickobj;

			if(type == CHECKBOX)
				do
				{
					graf_mkstate(&dummy, &dummy, &button, &dummy);
				} while(button != 0);
			else
				if(type == CYCLEBUTTON)
				{
					evback = evnt_multi(MU_BUTTON|MU_TIMER, 1,3,0, 0,0,0,0,0,0,0,0,0,0,
					messagebuf, 300,0, &mouse_xpos, &mouse_ypos, &dummy, &key_at_event, &dummy, &klicks);

					if(evback&MU_TIMER)
						mode = 1;
					else
						if((evback&MU_BUTTON) && mode == 1)
						{
							change_object(wind, klickobj, UNSEL, 1);
							mode = 0;
							return(-1);
						}	
				}
		}
		else
		{
			objc_offset(ob, klickobj, &ox, &oy);

			wind_update(BEG_UPDATE);
	
			do
			{
				evback = evnt_multi(MU_TIMER, 1,3,0, 0,0,0,0,0,0,0,0,0,0,
				messagebuf, 50,0, &mouse_xpos, &mouse_ypos, &mouse_button, &key_at_event, &dummy, &klicks);

				timo = objc_find(ob, 0, MAX_DEPTH, mouse_xpos, mouse_ypos);
			
				if(timo != klickobj)
				{
					if((ob[klickobj].ob_state & SELECTED) && ob[klickobj].ob_flags&SELECTABLE)
						change_object(wind, klickobj, UNSEL, 1);
				}
				else
				{
					if(!(ob[klickobj].ob_state & SELECTED) && ob[klickobj].ob_flags&SELECTABLE) 
						change_object(wind, klickobj, SEL, 1);
				}
			} while(mouse_button != 0 && evback != MU_BUTTON);

			wind_update(END_UPDATE);
		
			objct = timo;
		}
	}


	if(type == PREVIEW_ACTION)
	{ 
		if(!picthere)
		{
			wind->picture = NULL;
			Window.redraw(wind, NULL, klickobj, 0);
			return(0);
		}
		
		if(smurf_picture[active_pic]->block == NULL)
			prevpic = smurf_picture[active_pic];
		else
			prevpic = smurf_picture[active_pic]->block;

		wind->pic_xpos = ob[klickobj].ob_x;
		wind->pic_ypos = ob[klickobj].ob_y;
		wind->clipwid = ob[klickobj].ob_width - 1;
		wind->cliphgt = ob[klickobj].ob_height - 1;
		if(prevpic->pic_width < wind->clipwid)
			wind->clipwid = prevpic->pic_width;
		if(prevpic->pic_height < wind->cliphgt)
			wind->cliphgt = prevpic->pic_height;

		/* alte Preview-Bildschirmdarstellung freigeben */
		if(wind->picture != NULL && wind->picture->changed == 0x80)
		{
			SMfree(wind->picture->screen_pic->fd_addr);
			free(wind->picture->screen_pic);
			free(wind->picture->palette);
			SMfree(wind->picture);
		}

		f_move_preview(wind, prevpic, klickobj);
		make_modpreview(wind);
		Window.redraw(wind, NULL, klickobj, 0);
	}

	return(objct);
} /* UDO_or_not */


/* ----------------------------------------------------------------	*/
/*	Kleine Funktion zum deselektieren von 2 Objekten (fÅr Popups)	*/
/* ----------------------------------------------------------------	*/
void f_deselect_popup(WINDOW *wind, int ob1, int ob2)
{
	change_object(wind, ob1, UNSEL, 1);
	change_object(wind, ob2, UNSEL, 1);

	return;
} /* f_deselect_popup */