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

/*------------------------------------------------------------	*/
/* Sonderbehandlung fÅr PCD-Module								*/
/*																*/
/*		FÅr SMURF Grafikkonverter								*/
/*------------------------------------------------------------	*/

#include <tos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurf_f.h"

#include "popdefin.h"
#include "globdefs.h"

#include "smurfobs.h"
#include "ext_obs.h"


extern POP_UP popups[20];

extern int PCD;
extern int PCDwidth, PCDheight;
extern int mouse_xpos, mouse_ypos;
extern long f_len;

extern SYSTEM_INFO Sys_info;


/*	---------------------------------------	*/
/*	Extension stimmt - Is dat Ding en PCD?	*/
/*	---------------------------------------	*/
char *f_do_pcd(char *Path)
{
	char *buffer;
	int file;
	int imgtype;
	long DatenOffset, len, back, dummy;
	int idummy;
	int key;
	
	
	/* Die ersten 3600 Bytes laden ... */
	if((buffer = SMalloc(3700)) == NULL)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_MEM].TextCast, NULL, NULL, NULL, 1);
	else
	{
		if((dummy = Fopen(Path, FO_READ)) < 0)
		{
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[FILEOPEN_ERR].TextCast, NULL, NULL, NULL, 1);
			SMfree(buffer);
		}
		else
		{
			file = (int)dummy;
			back = Fread(file, 3600, buffer);
			Fclose(file);

			if(back != 3600)
			{
				Dialog.winAlert.openAlert("Fehler beim Lesen aus der Photo-CD-Datei!", NULL, NULL, NULL, 1);
				SMfree(buffer);
				return(FALSE);
			}

			/* isseseins? */
			if(strcmp(buffer + 2048, "PCD_IPI") != 0)
			{
				SMfree(buffer);
				return(FALSE);	/* Nî! */
			}
			else				/* Jupp! */
			{
				SMfree(buffer);
			
				PCD = 1;
				
				imgtype=Sys_info.PCD_Defsize;

				graf_mkstate(&idummy, &idummy, &idummy, &key);

				if(key&KEY_ALT)
					imgtype = f_pop(&popups[POPUP_KODAK], 1, PCD_DEF_PB, NULL);
				
				switch(imgtype)
				{
					case BASE_D16: DatenOffset = 0x2000;
								   PCDwidth = 192;
								   PCDheight = 128;
								   break;
					case BASE_D4: DatenOffset = 0xB800;
								  PCDwidth = 384;
								  PCDheight = 256;
								  break;
					case BASE: DatenOffset = 0x30000L;
							   PCDwidth = 768;
							   PCDheight = 512;
							   break;
					default: break;
				}

				/* Die Bilddaten laden... */
				len = (((long)PCDwidth*(long)PCDheight*3L)>>1L);
				f_len = len;
				if((buffer = SMalloc(len + 1024L)) == 0)
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_MEM].TextCast, NULL, NULL, NULL, 1);
				else
				{
					if((dummy = Fopen(Path, FO_READ)) < 0)
					{
						Dialog.winAlert.openAlert(Dialog.winAlert.alerts[FILEOPEN_ERR].TextCast, NULL, NULL, NULL, 1);
						SMfree(buffer);
					}
					else
					{
						file = (int)dummy;
						Fseek(DatenOffset, file, 0);		/* Ab zum DatenOffset */
						back = Fread(file, len, buffer);
						Fclose(file);
						if(back != len)
						{
							Dialog.winAlert.openAlert("Fehler beim Lesen aus der Photo-CD-Datei!", NULL, NULL, NULL, 1);
							SMfree(buffer);
						}
						else
							return(buffer);
					} /* Open fÅr Bilddaten */
				} /* Malloc fÅr Bilddaten */
			} /* Magic-Vergleich */
		} /* Open fÅr Erkennung */
	} /* Malloc fÅr Erkennung */

	return(FALSE);
} /* f_do_pcd */