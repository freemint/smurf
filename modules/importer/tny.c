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
/*							Tiny-Format						*/
/* Version 0.1  --  22.02.96								*/
/*	  nichtanimierte Tinies									*/
/*	  mîglicherweise sind die Daten auch im ilbm-Format		*/
/*	  gespeichert, ich brauche ein Testbild!				*/
/* =========================================================*/

#ifdef GERMAN
#define ERROR "[1][(Noch) nicht unterstÅtztes Tinyformat | Bild bitte an uns einschicken][ OK ]"
#else
#ifdef ENGLISH
#define ERROR "[1][(Noch) nicht unterstÅtztes Tinyformat | Bild bitte an uns einschicken][ OK ]"
#else
#ifdef FRENCH
#define ERROR "[1][(Noch) nicht unterstÅtztes Tinyformat | Bild bitte an uns einschicken][ OK ]"
#else
#error "Keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

char *fileext(char *filename);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Tiny-Format",
						0x0010,
						"Christian Eyrich",
						"TNY", "TN1", "TN2", "TN3", "",
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
/*          		Tiny-Format (TNY)				*/
/*		1, 2 und 4 Bit, RLE							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer1, *pal, *fname,
		 BitsPerPixel, DatenOffset, v1, Planes;
	char dummy[3], impmessag[21];

	unsigned int *buffer2, *ziel, *ppal, i, n, width, height,
				 controlbytes, datawords, length, v2;

	unsigned long src_pos2 = 0, dst_pos = 0;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer1 = smurf_struct->smurf_pic->pic_data;
	buffer2 = (unsigned int *)buffer1;

	fname = smurf_struct->smurf_pic->filename;
	if(strnicmp(fileext(fname), "TN", 2) != 0 ||
	   *(char *)buffer2 < 0x6)
		return(M_INVALID);
	else
	{
		switch(*(char *)buffer2)
		{
			case 0:
					width = 320;
					height = 200;
					Planes = 4;
					break;
			case 1:	width = 640;
					height = 200;
					Planes = 2;
					break;
			case 2:
					width = 640;
					height = 400;
					Planes = 1;
					break;
			default:	form_alert(0,  ERROR);
						return(M_PICERR);
		}
		BitsPerPixel = Planes;

		controlbytes = *((char *)buffer2 + 0x23);
		datawords = *((char *)buffer2 + 0x25);			/* Anzahl Words sind abgespeichert! */

		DatenOffset = 0x27;

		strncpy(smurf_struct->smurf_pic->format_name, "Tiny-File .TNY", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "Tiny-File ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if((ziel = SMalloc(((long)width * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			buffer1 += DatenOffset;
			length = datawords;
			src_pos2 += DatenOffset + controlbytes;

			while(length)
			{
				if((v1 = *buffer1++) > 0x80)
				{
					n = 0x100 - v1;
					for(i = 0; i < n; i++)
						ziel[dst_pos++] = buffer2[src_pos2++];
					length -= n;
				}
				else
					if(v1 == 0x00)
					{
						n = *(unsigned int *)buffer1++;
						v2 = buffer2[src_pos2++];
						for(i = 0; i < n; i++)
							ziel[dst_pos++] = v2;
						length -= n;
					}
					else
						if(v1 == 0x01)
						{
							n = *(unsigned int *)buffer1++;
							ziel[dst_pos++] = buffer2[src_pos2++];
							length -= n;
						}
						else
						{
							n = v1;
							v2 = buffer2[src_pos2++];
							for(i = 0; i < n; i++)
								ziel[dst_pos++] = v2;
							length -= n;						
						}
			}
		
			smurf_struct->smurf_pic->format_type = 1;
	
			pal = smurf_struct->smurf_pic->palette;
			ppal = (unsigned int *)(buffer1 + 0x01);
				
			for(i = 0; i < 16; i++)
			{
				v2 = *ppal++;

				*pal++ = (char)((v2 & 0x700) >> 4);
				*pal++ = (char)(v2 & 0x070);
				*pal++ = (char)((v2 & 0x007) << 4);
				
			} /* Palette */

			smurf_struct->smurf_pic->pic_data = ziel;

			smurf_struct->smurf_pic->col_format = RGB; 
		} /* Malloc */
	} /* Erkennung */

	SMfree((char *)buffer2);
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