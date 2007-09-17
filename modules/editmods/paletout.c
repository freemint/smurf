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

/* =========================================================*/
/*				Palettenausgabe-Modul						*/
/* Version 0.1  --  06.09.97								*/
/*	  1-8 Bit												*/
/* Version 0.1  --  02.02.98								*/
/*	  Extender BIN wird nun vorgeblendet und als Standard	*/
/*	  verwendet, wird keiner angegeben.						*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ext.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

char *conv(char val);

MOD_INFO module_info = {"Palettenausgabe",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"Slider 1",
						"Slider 2",
						"Slider 3",
						"Slider 4",
						"Checkbox 1",
						"Checkbox 2",
						"Checkbox 3",
						"Checkbox 4",
						"Edit 1",
						"Edit 2",
						"Edit 3",
						"Edit 4",
						0,64,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						1, 2, 3, 4, 5,
						6, 7, 8,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						0,
						};

char wert[3], wert1[3], wert2[3], wert3[3];

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Palettenausgabe					*/
/*		1-8 Bit										*/
/*	Gibt die jeweilige Bildpalette als ASCII-Datei	*/
/*	(Extender .ASC, Hexwerte) oder als Bin„rdatei	*/
/*	(Extender .BIN) unter dem anzugebenden Namen aus*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data, buffer[25], path[257], file[65], palette[257], *part, *extender,
		 BitsPerPixel;

	int button, fp;
	unsigned int i;


	if(smurf_struct->module_mode == MSTART)
	{
		smurf_struct->module_mode = M_STARTED;
		return;
	}
	else
		if(smurf_struct->module_mode == MEXEC)
		{
			data = smurf_struct->smurf_pic->palette;			
			BitsPerPixel = smurf_struct->smurf_pic->depth;			
			i = (1 << BitsPerPixel);

			strcpy(path, "*.bin");						/* PAL als Extender vorblenden */
			fsel_input(path, file, &button);
			if(button && strlen(file))
			{
				extender = strrchr(file, '.');
				if(extender)							/* kein NULL-Zeiger, d.h. ein Punkt */
				{				
					if(stricmp(extender, ".") == 0)		/* kein Extender nach dem Punkt */
						strcat(file, "bin");
				}
				else									/* weder Punkt noch Extender vorhanden */
				{
					strcat(file, ".bin");
					extender = strrchr(file, '.');
				}

				extender++;								/* den Punkt bergehen */

				strcpy(palette, path);
				part = strrchr(palette, '\\') + 1;
				strcpy(part, file);
				fp = (int)Fcreate(palette, 0);

				if(stricmp(extender, "asc") == 0)
				{
					sprintf(buffer, "palette[] = {");
					Fwrite(fp, strlen(buffer), buffer);

					i--;							/* damit am Ende noch eine Farbe bleibt */
					while(i--)
					{
						strcpy(wert1, conv(*data++));
						strcpy(wert2, conv(*data++));
						strcpy(wert3, conv(*data++));
						sprintf(buffer, "\t\t\t 0x%s, 0x%s, 0x%s,\r\n", wert1, wert2, wert3);
						Fwrite(fp, strlen(buffer), buffer);
					}			

					sprintf(buffer, "\t\t\t 0x%s, 0x%s, 0x%s};", conv(*data++), conv(*data++), conv(*data++));
					Fwrite(fp, strlen(buffer), buffer);

				}
				else
				{
					while(i--)
					{
						*(unsigned int *)buffer = (unsigned int)(((unsigned long)*data++ * 1000L) / 255);
						*(unsigned int *)(buffer + 2) = (unsigned int)(((unsigned long)*data++ * 1000L) / 255);
						*(unsigned int *)(buffer + 4) = (unsigned int)(((unsigned long)*data++ * 1000L) / 255);
						Fwrite(fp, 6, (void *)buffer);
					}
				}

				Fclose(fp);
			}

			smurf_struct->module_mode = M_MODERR;
			
			return;
		}
		else 
			if(smurf_struct->module_mode == MTERM)
			{
				smurf_struct->module_mode = M_EXIT;
				return;
			}
}


char *conv(char val)
{
	sprintf(wert, "%x", (unsigned int)val);

	if(wert[1] == '\0')				/* wenn 1 schon Ende, dann ist nur eine Stelle vorhanden */
	{
		wert[2] = wert[1];
		wert[1] = wert[0];
		wert[0] = '0';
	}

	return(wert);
} /* conv */