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
/*#include <multiaes.h>*/
#include <aes.h>
#include <vdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <screen.h>
#include <string.h>
#include <ext.h>
#include <time.h>
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

void *copyblock(SMURF_PIC *old_pic);
int insert_block(WINDOW *picwindow);

/*
 * fÅrs RT-Blockpreview:
 * kopiert den Bereich blockpart aus Bild und Block in Puffer und rechnet
 * den Blockbereich in den Bildbereich ein.
 * wird aus dem windowredraw (draw_block()) aufgerufen, wobei blockpart 
 * das Rechteck _innerhalb_ des Blockrechtecks ist, das redrawed 
 * werden muû.
 */

SMURF_PIC *previewBlock(SMURF_PIC *picture, GRECT *blockpart)
{
	SMURF_PIC *picArea, *blockArea;
	SMURF_PIC picCopy;
	WINDOW dummyPicwindow;
	
	picArea = SMalloc(sizeof(SMURF_PIC));
	blockArea = SMalloc(sizeof(SMURF_PIC));	
	
	/*
	 * den Blockbereich aus dem darunterliegenden Bild rauskopieren
	 * damit liegt in picArea ein Bild mit exakt blockpart ausmaûen
	 * und Koordinaten aus picture.
	 */
	memcpy(&picCopy, picture, sizeof(SMURF_PIC));
	memcpy(picArea, picture, sizeof(SMURF_PIC));
	picCopy.blockx = picture->blockx + blockpart->g_x;
	picCopy.blocky = picture->blocky + blockpart->g_y;
	picArea->pic_width = picCopy.blockwidth = blockpart->g_w;
	picArea->pic_height = picCopy.blockheight = blockpart->g_h;
	picArea->pic_data = copyblock(&picCopy);
	
	/*
	 * jetzt den Blockbereich aus dem Block genauso rauskopieren
	 */
	memcpy(&picCopy, picture->block, sizeof(SMURF_PIC));
	memcpy(blockArea, picture->block, sizeof(SMURF_PIC));
	picCopy.blockx = blockpart->g_x;
	picCopy.blocky = blockpart->g_y;
	blockArea->pic_width = picCopy.blockwidth = blockpart->g_w;
	blockArea->pic_height = picCopy.blockheight = blockpart->g_h;
	blockArea->pic_data = copyblock(&picCopy);

	/*
	 * picArea so vorbereiten, daû blockArea als Block bei 0/0
	 * drÅberliegt
	 */
	picArea->blockx = 0;
	picArea->blocky = 0;
	picArea->blockwidth = blockArea->pic_width;
	picArea->blockheight = blockArea->pic_height;
	picArea->block = blockArea;
	
	/* und jetzt blockArea in picArea einrechnen
	 */
	dummyPicwindow.picture = picArea;
	Dialog.busy.fullDisable();
	insert_block(&dummyPicwindow);
	SMfree(blockArea->pic_data);
	SMfree(blockArea);

	return(picArea);		/* und picArea an draw_block zurÅckgeben. */
}