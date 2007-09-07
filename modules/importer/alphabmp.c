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
/*				Alpha Microsystems BMP-Format				*/
/* Version 0.1  --  15.01.96								*/
/*	  Aufpassen bei 4 Bit, Pixel sind gepackt!				*/
/* Version 0.2  --  24.10.96								*/
/*	  RGB-BGR-Turn aus- und setzen einer Standardpalette bei*/
/*	  monochromen Bildern eingebaut.						*/
/* =========================================================*/

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
void nulltospace(char *string, char length);

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Alpha BMP-Format",
						0x0020,
						"Christian Eyrich",
						"BMP", "", "", "", "",
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
/*			Alpha Microsystems BMP-Format (BMP)		*/
/*		1, 4, 8 und 24 Bit, wahlweise RLE			*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel, *pal, *ppal, *fname,
		 n, v1, v2, comp, ifpal, BitsPerPixel,
		 DatenOffset;
	char dummy[3], impmessag[21];

	unsigned int x, y, i, j, cols, width, height;

	unsigned long w, src_pos = 0, dst_pos = 0;

	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;

	fname = smurf_struct->smurf_pic->filename;
	if(*(unsigned int *)buffer != 0xffff ||
	    stricmp(fileext(fname), "BMP") != 0)
		return(M_INVALID);
	else
	{
		BitsPerPixel = *(buffer + 0x0e);
		
		width = *(buffer + 0x0a) + (*(buffer + 0x0b) << 8); 
		height = *(buffer + 0x0c) + (*(buffer + 0x0d) << 8);

		ifpal = (char)*(unsigned int *)(buffer + 0x10);
		comp = (char)*(unsigned int *)(buffer + 0x12);

		cols = *(buffer + 0x50) + (*(buffer + 0x51) << 8);
		DatenOffset = 0x50 + cols * 3;

		nulltospace(buffer + 0x14, 112);
		strncpy(smurf_struct->smurf_pic->infotext, buffer + 0x14, 60);

		strncpy(smurf_struct->smurf_pic->format_name, "Alpha BMP-File .BMP", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "Alpha-BMP ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if((ziel = SMalloc(((long)width * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			if(BitsPerPixel == 1)
				w = (width + 7) / 8;
			else
				if(BitsPerPixel == 24)
					w = width * 3;
				else
					w = width;

			src_pos += DatenOffset;	
			y = 0;
			do /* height */
			{
				x = 0;
				do /* width */
				{
					if(comp == 0)
					{
						ziel[dst_pos++] = buffer[src_pos++];
						x++;
					}
					else
					{
						v1 = buffer[src_pos++];
						if(v1 > 0x80)
						{
							n = (0x101 - v1);
							v2 = buffer[src_pos++];
							for(i = 0; i < n; i++)
								ziel[dst_pos++] = v2;
							x += n;
						}
						else
						{
							for (i = 0; i < v1; i++)
							ziel[dst_pos++] = buffer[src_pos++];
							x += v1;             
						}
					}
				} while(x < w); /* x */
				y++;
			} while(y < height); /* y */

			smurf_struct->smurf_pic->pic_data = ziel;

			if(BitsPerPixel == 1)
				smurf_struct->smurf_pic->format_type = FORM_STANDARD;
			else
				smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

			if(ifpal)
			{
				pal = smurf_struct->smurf_pic->palette;
				ppal = buffer + 0x52;
				j = *(unsigned int *)(buffer + 0x50) * 3;
				while(j--)
					*pal++ = *ppal++;
			} /* Palette */
			else
				if(BitsPerPixel == 1)
				{
					pal[0] = 255;
					pal[1] = 255;
					pal[2] = 255;
					pal[3] = 0;
					pal[4] = 0;
					pal[5] = 0;
				}

			smurf_struct->smurf_pic->col_format = RGB;
		
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);

	return(M_PICDONE);
}


void nulltospace(char *string, char length)
{
	while(--length)
	{
		if(*string == '\0')
			*string = ' ';
		string++;
	}

	return;
} /* nulltospace */


char *fileext(char *filename)
{
	char *extstart;


	if((extstart = strrchr(filename, '.')) != NULL)
		extstart++;
	else
		extstart = strrchr(filename, '\0');
	
	return(extstart);
} /* fileext */