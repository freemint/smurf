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

/*					destruct.c 							*/
/*	Destruktoren fÅr verschiedene Strukturen.			*/


#include <tos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vdi.h>
#include <ext.h>
#include <math.h>
#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurfine.h"
#include "smurf_f.h"
#include "popdefin.h"

#include "globdefs.h"
#include "vaproto.h"

#include "smurfobs.h"
#include "ext_obs.h"


/* destroy_smurfpic ----------------------------------------
	Destruktor fÅr SMURF_PIC-Struktur. Gibt allen von einer
	SMURF_PIC-Struktur belegten Speicher wieder frei, incl.
	des Blockes. u.U. findet hierfÅr ein rekursiver Aufruf statt! */
void destroy_smurfpic(SMURF_PIC *pic)
{
	int back = 0;

	SMURF_PIC *next = NULL;
	SMURF_PIC *prev = NULL;


	if(pic == NULL)
		return;

	if(pic->next_picture != NULL)
		next = pic->next_picture;
	if(pic->prev_picture != NULL)
		prev = pic->prev_picture;

	if(pic->pic_data != NULL)
		back = SMfree(pic->pic_data);
	if(pic->screen_pic != NULL && pic->screen_pic->fd_addr != NULL)
		back = SMfree(pic->screen_pic->fd_addr);
	if(pic->screen_pic != NULL)
		free(pic->screen_pic);
	if(pic->local_nct != NULL && pic->changed != 255)
		back = SMfree(pic->local_nct);
	if(pic->block != NULL)
	{
		destroy_smurfpic(pic->block);
		pic->block = NULL;
	}

	free(pic->palette);
	SMfree(pic);

	if(back != 0)
		Dialog.winAlert.openAlert("Fehler beim Freigeben einer Bildstruktur!", NULL, NULL, NULL, 1);
	
	if(next != NULL)
		next->prev_picture = prev;
	if(prev != NULL)
		prev->next_picture = next;

	return;
} /* destroy_smurfpic */



/* remove_block ----------------------------------------
	Entfernt einen Block aus picwindow, die Blockkordinaten
	werden resetted und das Fenster redrawed.	*/
void remove_block(WINDOW *picwindow)
{
	char block_there = 0;

	int zoom;

	GRECT redraw;


	if(picwindow->picture->block)
	{
		block_there = 1;

		zoom = picwindow->picture->zoom + 1;

		redraw.g_x = (picwindow->picture->blockx) / zoom - picwindow->xoffset + picwindow->pic_xpos + picwindow->wx - 1;
		redraw.g_y = (picwindow->picture->blocky) / zoom - picwindow->yoffset + picwindow->pic_ypos + picwindow->wy - 1;
		redraw.g_w = picwindow->picture->blockwidth / zoom + 2;
		redraw.g_h = picwindow->picture->blockheight / zoom + 2;
	}

	destroy_smurfpic(picwindow->picture->block);
	picwindow->picture->block = NULL;

	/* nur die Blockbox lîschen */
	Window.redraw(picwindow, NULL, 0, DRAWNOTREE|BLOCK_ONLY);

	picwindow->picture->blockx = 0;
	picwindow->picture->blocky = 0;
	picwindow->picture->blockwidth = 0;
	picwindow->picture->blockheight = 0;
	picwindow->picture->block = NULL;	

	if(block_there)
		Window.redraw(picwindow, &redraw, 0, DRAWNOTREE);

	return;
} /* remove_block */