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

#include "tos.h"
#include "multiaes.h"
#include "vdi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "smurf.h"
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "popdefin.h"
#include "smurf_st.h"
#include "smurf_f.h"


extern SYSTEM_INFO Sys_info;
extern DISPLAY_MODES Display_Opt;


void make_thumbnail(SMURF_PIC *original_pic, SMURF_PIC *thumbnail, int dither)
{
	int zoom;

	DISPLAY_MODES thisDisplay;

	extern int compute_zoom(SMURF_PIC *picture, int twid, int thgt);


	zoom = compute_zoom(original_pic, thumbnail->pic_width, thumbnail->pic_height);

	/*
	 * altes Thumbnail freigeben
	 */
	if(thumbnail->screen_pic != NULL)
	{
		if(thumbnail->screen_pic->fd_addr != 0) 
			SMfree(thumbnail->screen_pic->fd_addr);
		
		free(thumbnail->screen_pic);
	}


	/*
	 * Thumbnail-Bildstruktur fertig machen
	 */
	thumbnail->pic_data = original_pic->pic_data;
	thumbnail->pic_width = original_pic->pic_width;
	thumbnail->pic_height = original_pic->pic_height;
	thumbnail->depth = original_pic->depth;
	thumbnail->palette = original_pic->palette;
	thumbnail->format_type = original_pic->format_type;
	thumbnail->col_format = original_pic->col_format;

	thumbnail->zoom = zoom;

	/*
	 * Display-Options tempor„r umbauen
	 */
	thisDisplay.dither_24 = dither;
	thisDisplay.dither_8 = dither;
	thisDisplay.dither_4 = dither;
	thisDisplay.syspal_24 = CR_SYSPAL;
	thisDisplay.syspal_8 = CR_SYSPAL;
	thisDisplay.syspal_4 = CR_SYSPAL;

	/*
	 * dithern
	 */
	f_dither(thumbnail, &Sys_info, 0, NULL, &thisDisplay);

	return;
} /* make_thumbnail */