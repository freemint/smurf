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
/*						XGA-Format Decoder					*/
/* Version 0.1  --  26.03.96								*/
/*	  16 Bit												*/
/*	  Der 30 Sekunden Decoder								*/
/*	  Ob andere Farbtiefen oder eine Komprimierung mîglich	*/
/*	  sind ist unbekannt.									*/
/* Version 0.2  --  04.06.96								*/
/*	  Es werden jetzt auch Bilder von Escape-Paint gelesen.	*/
/*	  Die haben keine Grîûenangabe im Namen, haben aber		*/
/*	  immer typische Screengrîûen wie 320*240 usw.			*/
/* Version 0.3  --  04.05.98								*/
/*	  Das Ersetzen der Extendeerkennung durch die Standard-	*/
/*	  routine fileext() brachte das Auslesen der Bildbreite	*/
/*	  aus dem Filenamen durcheinander. Das ist nun gefixt.	*/
/*	  Weiterhin bricht das Modul ab, wenn das letzte Zeichen*/
/*	  keine Ziffer ist.										*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

char *fileext(char *filename);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"XGA-Format",
						0x0030,
						"Christian Eyrich",
						"XGA", "", "", "", "",
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
						0,128,
						0,128,
						0,128,
						0,128,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					XGA-Format (XGA)				*/
/*		16 Bit, unkomprimiert						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	signed char i;
	char *fname;

	unsigned int width, height;


	fname = smurf_struct->smurf_pic->filename;

/* Die komplette PrÅfung besteht darin im Filenamen nach dem letzten */
/* Buchstaben zu suchen und die dahinterstehende Zahl als Breite zu */
/* interpretieren. Die FilelÑnge (die HÑlfte zumindest, das Format ist */
/* ja 16bitig) muû dann auch noch glatt durch die Breite teilbar sein. */
	if(stricmp(fileext(fname), "XGA") != 0)
		return(M_INVALID);
	else
	{
		switch(smurf_struct->smurf_pic->file_len)
		{
			case 128000L: width = 320;
						  height = 200;
						  break;
			case 153600L: width = 320;
						  height = 240;
						  break;
			case 196608L: width = 384;
						  height = 256;
						  break;
			default:	i = strlen(fname) - 1 - 4;	/* - 4 um den Extender zu Åbergehen, hier */
													/* kînnen wir ja sicher sein, daû es ".XGA" ist */

						if(fname[i] < '0' || fname[i] > '9')
							return(M_PICERR);

						while(fname[i] >= '0' && fname[i] <= '9' && i >= 0)
							i--;
						i++;

						width = atoi(fname + i);
						height = (unsigned int)((smurf_struct->smurf_pic->file_len >> 1) / width);

						break;
		}

		strncpy(smurf_struct->smurf_pic->format_name, "XGA Image .XGA", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = 16;

		smurf_struct->services->reset_busybox(128, "XGiga 16 Bit");

		smurf_struct->smurf_pic->col_format = RGB;
	} /* Erkennung */

	return(M_PICDONE);
}


char *fileext(char *filename)
{
	char *extstart;


	if((extstart = strrchr(filename, '.')) != NULL)
		extstart++;
	else
		extstart = strrchr(filename, '\0');
	
	return(extstart);
} /* fileext */