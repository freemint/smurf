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
/*                  	HSI File-Format                 	*/
/* Version 0.1  --  12.01.96                                */
/*	  xxx													*/
/*==========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"HSI File-Format",
						0x0010,
						"Christian Eyrich",
						"RAW", "", "", "", "",
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
/*				HSI File-Format (RAW)				*/
/*		8, 24 Bit, keine Kodierung					*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel, *pal, *ppal,
		 BitsPerPixel, DatenOffset, cols;
	char dummy[3], impmessag[21];

	unsigned int i, width, height;

	long len;

	buffer = smurf_struct->smurf_pic->pic_data;

	if(strncmp(buffer, "mhwanh", 6) != 0)
		return(M_INVALID);
	else
	{
		if(*(unsigned int *)(buffer + 0x0c) > 256 ||
		   *(unsigned int *)(buffer + 0x0c) == 0)
			BitsPerPixel = 24;
		else
			BitsPerPixel = 8;

		width = *(unsigned int *)(buffer + 0x08); 
		height = *(unsigned int *)(buffer + 0x0a);

		if(BitsPerPixel == 8)
			cols = 255;
		else
			cols = 0;

		DatenOffset = 0x20 + cols * 3;

		strncpy(smurf_struct->smurf_pic->format_name, "HSI File-Format .RAW", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "HSI Raw ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if(BitsPerPixel != 24)
		{
			pal = smurf_struct->smurf_pic->palette;
			ppal = buffer + 0x20;               
			for(i = 0; i < cols; i++)
			{
				*pal++ = *ppal++;
				*pal++ = *ppal++;
				*pal++ = *ppal++;
			}
		} /* Palette */

		ziel = buffer;

		len = (((long)width * (long)height) * BitsPerPixel) >> 3;

		memcpy(ziel, buffer + DatenOffset, len);

		Mshrink(0, ziel, len);

		smurf_struct->smurf_pic->pic_data = ziel;
	} /* Erkennung */

	return(M_PICDONE);
}