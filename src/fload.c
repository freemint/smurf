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

#include <stdio.h>
#include <tos.h>
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurf_st.h"
#include "smurf_f.h"

extern long f_len;

/* -------------------------- File laden --------------------------------*/
char *fload(char *Path, int header)
{
	char *buf;

	int fh;

	long dummy;


	if((dummy = Fopen(Path, FO_READ)) >= 0)			/* Datei îffnen */
	{
		fh = (int)dummy;

		f_len = Fseek(0, fh, 2) - header;			/* FilelÑnge ermitteln */
		Fseek(header, fh, 0);						/* und Dateizeiger wieder auf Anfang */

		if((buf = SMalloc(f_len)) == NULL)
			f_alert("Nicht genÅgend Speicher zum Laden der Datei!", NULL, NULL, NULL, 1);
		else
		{
			dummy = Fread(fh, f_len, buf);

			if(dummy < f_len)
			{
				f_alert("Fehler beim Laden der Datei!", NULL, NULL, NULL, 1);
				Mfree(buf);
				buf = NULL;
			}
		}

		Fclose(fh);
	}
	else
		buf = NULL;

	return(buf);
} /* fload */