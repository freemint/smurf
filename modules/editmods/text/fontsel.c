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
 * Christian Eyrich
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

#include <stdio.h>
#include <vdi.h>
#include <aes.h>
#include <stddef.h>
#include "..\..\import.h"
#include "fontsel.h"
#include "text\de\text.h"

#define	TextCast	ob_spec.tedinfo->te_ptext

extern int work_in[25], work_out[50];
extern OBJECT *alerts;
extern SERVICE_FUNCTIONS *services;


extern	int get_cookie(unsigned long cookie, unsigned long *value);
int call_xfsl(int handle, FONT_INFO *fontinfo, xFSL *fontsel_struct);
int call_magic_fsl(int handle, FONT_INFO *fontinfo);

int call_fontsel(int handle, FONT_INFO *fontinfo)
{
	int back, idummy;

	xFSL *fontsel_struct;


	if(get_cookie('xFSL', (unsigned long *)&fontsel_struct))
		back = call_xfsl(handle, fontinfo, fontsel_struct);
	else
		if((_GemParBlk.global[0] >= 0x400 || appl_find("?AGI") >= 0) &&
		   appl_getinfo(7, &back, &idummy, &idummy, &idummy) && back&0x04)
			back = call_magic_fsl(handle, fontinfo);
		else
		{
			services->f_alert(alerts[NO_FSEL].TextCast, NULL, NULL, NULL, 1);
			return(-1);
		}

	return(back);
} /* call_fontsel */


int call_xfsl(int handle, FONT_INFO *fontinfo, xFSL *fontsel_struct)
{
	int new_id, new_size, back;
    int cdecl(*xfsl_input)(int vdihandle, unsigned int fontflags,
                    	const char *headline, int *id, int *size);


	new_id = fontinfo->ID;
	new_size = fontinfo->size;

	xfsl_input = fontsel_struct->xfsl_input;
	back = xfsl_input(0, 0x00FE, NULL, &new_id, &new_size);

	if(back>0)
	{
/*		printf("gew„hlt wurde der Font mit ID: %d in der Gr”že %d Punkt.\n", new_id, new_size); */
		fontinfo->size = new_size;
		fontinfo->ID = new_id;
	}
	else
		if(back<0)
			services->f_alert(alerts[ERROR_XFSL_FSEL].TextCast, NULL, NULL, NULL, 1);

	return(back);
} /* call_xfsl */


int call_magic_fsl(int handle, FONT_INFO *fontinfo)
{
	int back, dummy_handle;

	long new_id, new_size, ratio;

	FNT_DIALOG *fnt_dialog;


	new_id = fontinfo->ID;
	new_size = (long)fontinfo->size << 16;
	ratio = 1L << 16;

/*	printf("ID: %d, size: %d\n", fontinfo->ID, fontinfo->size); */

	/* Workaround weil das bescheuerte MagiC die Attribute */
	/* der bergebenen Workstation verstellt ... */
	v_opnvwk(work_in, &dummy_handle, work_out);
	fnt_dialog = fnts_create(dummy_handle, 0, 0xf, FNTS_3D, "The quick blue Smurf ...", 0L);

	if(fnt_dialog)
	{
		back = fnts_do(fnt_dialog, new_id, new_size, ratio, &new_id, &new_size, &ratio);

/*		printf("ID: %ld, size: %ld\n", new_id, new_size); */

		if(back == FNTS_OK || back == FNTS_SET)
		{
/*			printf("gew„hlt wurde der Font mit ID: %d in der Gr”že %d Punkt.\n", (int)new_id, (int)(new_size >> 16)); */
			fontinfo->ID = (int)new_id;
			fontinfo->size = (int)(new_size >> 16);
		}

		fnts_delete(fnt_dialog, 0);
	}
	else
/*		form_alert(1, "[1][Fehler beim Aufruf des|MagiC-Fontselectors!][ Hoppla ]"); */
/* 1.06 Style
		services->f_alert(alerts[ERROR_MAGIC_FSEL].TextCast, NULL, NULL, NULL, 1);
		services->f_alert(alerts[ERROR_MAGIC_FSEL].TextCast); */

	v_clsvwk(dummy_handle);

	return(back);
} /* call_magic_fsl */