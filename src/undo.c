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
#include <stdlib.h>
#include <stddef.h>
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "smurf_st.h"
#include "smurf_f.h"

#include "smurfobs.h"
#include "ext_obs.h"

/*
 * imageWindow.undo.saveBuffer
 * speichert einen undobuffer unter <bildnummer>.sub ins TMP-Verzeichnis
 */
void saveUndoBuffer(SMURF_PIC *picture, int picnum)
{
	char filePath[257], temppath[257], *bslPos;

	int fHandle;

	long picLen;


	get_tmp_dir(temppath);
	bslPos = strchr(temppath, '\\');
	if(bslPos != NULL)
		*bslPos = 0;

	Dialog.winAlert.openAlert(temppath, 0, 0, 0, 0);

	/*
	 * Undobuffer schreiben
	 */
	strcpy(filePath, temppath);
	strcat(filePath, "\\");
	itoa(picnum, filePath + strlen(filePath), 10);
	strcat(filePath, ".sub");
	
	/*
	 * einfach die SMURF_PIC reinschreiben...
	 */
	fHandle = (int)Fcreate(filePath, 0);
	Fwrite(fHandle, sizeof(SMURF_PIC), picture);

	/*
	 * und dann die Daten hinterher
	 */
	if(picture->depth >= 8)
		picLen = (long)picture->pic_width * (long)picture->pic_height * (long)(picture->depth / 8);
	else
		picLen = (long)((picture->pic_width + 7) / 8) * (long)picture->pic_height * (long)(picture->depth) / 8;

	Fwrite(fHandle, picLen, picture->pic_data);

	/*
	 * jetzt noch die Palette
	 */
	Fwrite(fHandle, 1024, picture->palette);

	Fclose(fHandle);

	return;
}

/*
 * imageWindow.undo.swapBuffer
 * Tauscht Undobuffer und aktuelles Bild
 * ein wenig komplizierter... eine Datei tmp.sub wird geîffnet,
 * das aktuelle Bild reingeschrieben, die eigentliche
 * Undobufferdatei <picNum>.sub geîffnet, die Daten daraus nach
 * toPicture ausgelesen, <picNum>.sub gelîscht und tmp.sub in
 * <picNum>.sub umbenannt.
 */
void swapUndoBuffer(SMURF_PIC *toPicture, int picNum)
{
	char filePath[257], temppath[257], *bslPos;
	char tmpName[257];	

	int fHandle;

	long picLen, undoPicLen;

	SMURF_PIC undoPic;


	get_tmp_dir(temppath);
	bslPos = strchr(temppath, '\\');
	if(bslPos != NULL)
		*bslPos = 0;

	Dialog.winAlert.openAlert(temppath, 0, 0, 0, 0);

	/*
	 * Name der Pufferdateien
	 */
	strcpy(filePath, temppath);
	strcat(filePath, "\\");
	itoa(picNum, filePath + strlen(filePath), 10);
	strcat(filePath, ".sub");

	strcpy(tmpName, temppath);
	strcat(tmpName, "\\tmp.sub");

	/*
	 * Lesen der Daten aus dem Undopuffer
	 */
	fHandle = (int)Fopen(filePath, FO_READ);
	Fread(fHandle, sizeof(SMURF_PIC), &undoPic);

	/*
	 * die Bilddaten/Palette kînnen direkt in die Original-SMURF_PIC
	 * eingelesen werden.
	 */
	if(toPicture->depth >= 8)
		picLen = (long)toPicture->pic_width * (long)toPicture->pic_height * (long)(toPicture->depth/8);
	else
		picLen = (long)((toPicture->pic_width+7)/8) * (long)toPicture->pic_height * (long)(toPicture->depth) / 8;

	if(undoPic.depth >= 8)
		undoPicLen = (long)undoPic.pic_width * (long)undoPic.pic_height * (long)(undoPic.depth/8);
	else
		undoPicLen = (long)((undoPic.pic_width + 7) / 8) * (long)undoPic.pic_height * (long)(undoPic.depth) / 8;

	SMfree(toPicture->pic_data);
	toPicture->pic_data = SMalloc(undoPicLen);
	Fread(fHandle, picLen, toPicture->pic_data);
	Fread(fHandle, 1024, toPicture->palette);
	Fclose(fHandle);

	/*
	 * und jetzt alle énderbaren Daten aus der Pufferstruktur
	 * in die Originalstruktur Åbertragen.
	 */
	toPicture->pic_width = undoPic.pic_width;
	toPicture->pic_height = undoPic.pic_height;
	toPicture->depth = undoPic.depth;
	toPicture->depth = undoPic.depth;
	toPicture->format_type = undoPic.format_type;
	toPicture->col_format = undoPic.col_format;

	/*
	 * das mÅûte alles sein (hoffe ich).
	 * Das Bild muû dann (vom Aufrufer) redithered/redrawed werden.
	 */

	return;
}