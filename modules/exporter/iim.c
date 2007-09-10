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
/*					Inshape-Format Encoder					*/
/* Version 0.1  --  25.01.96								*/
/*	  Der 2 Minuten Encoder									*/
/* Version 0.2  --  14.05.96								*/
/*	  Graukonvertierung fÅr 8 Bit eingebaut					*/
/* Version 0.3  --  08.05.97								*/
/*	  Graukonvertierung wird nun vom Smurf erledigt.		*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include <..\..\demolib.h>

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Inshape Image",
						0x0030,
						"Christian Eyrich",
						"IIM", "", "", "", "",
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
						0,0,0,0,
						0,0,0,0,
						0,0,0,0,
						0
						};


MOD_ABILITY  module_ability = {
						1, 8, 24, 0, 0,
						0, 0, 0,
						FORM_STANDARD,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0
						};

typedef struct
{
	char magic[8];
	unsigned int unknown;
	unsigned int BitsPerPixel;
	unsigned int width;
	unsigned int height;
} HEAD;

void write_header(HEAD *iim_header, unsigned int width, unsigned int height, char BitsPerPixel);

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				Inshape-Format (IIM)				*/
/*		1, 8 und 24 Bit, unkomprimiert				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *obuffer, *ziel, *oziel,
		 BitsPerPixel;

	unsigned int y, width, height;

	unsigned long w, headsize, f_len;


	switch(smurf_struct->module_mode)
	{
		case MSTART:
			smurf_struct->module_mode = M_WAITING;	/* Ich warte... */
			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			if(smurf_struct->smurf_pic->depth == 8)
				smurf_struct->event_par[0] = GREY;
			else
				smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */

			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			headsize = sizeof(HEAD);

			buffer = smurf_struct->smurf_pic->pic_data;
			obuffer = buffer;

			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			BitsPerPixel = smurf_struct->smurf_pic->depth;

			if(BitsPerPixel == 1)
				w = (unsigned long)(width + 7) / 8;
			else
				if(BitsPerPixel == 8)
					w = (unsigned long)width;
				else
					w = (unsigned long)width * 3L;

			f_len = w * (unsigned long)height;

			if((ziel = (char *)SMalloc(headsize + f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				oziel = ziel;

				write_header((HEAD *)ziel, width, height, BitsPerPixel);

				ziel += headsize;

				y = 0;
				do
				{
					memcpy(ziel, buffer, w);
					ziel += w;
					buffer += w;
				} while(++y < height);

				buffer = obuffer;
				ziel = oziel;

				f_len += headsize;
				exp_pic->pic_data = ziel;
				exp_pic->f_len = f_len;
			} /* Malloc */

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */

			smurf_struct->module_mode = M_DONEEXIT;
			return(exp_pic);

/* Mterm empfangen - Speicher freigeben und beenden */
		case MTERM:
			SMfree(exp_pic->pic_data);
			SMfree((char *)exp_pic);
			smurf_struct->module_mode = M_EXIT;
			break;

		default:
			smurf_struct->module_mode = M_WAITING;
			break;
	} /* switch */

	return(NULL);
}


void write_header(HEAD *iim_header, unsigned int width, unsigned int height, char BitsPerPixel)
{
 	strncpy(iim_header->magic, "IS_IMAGE", 8);

	iim_header->BitsPerPixel = BitsPerPixel;

	switch(BitsPerPixel)
	{
		case 1:	 iim_header->unknown = 0;
				 break;
		case 8:  iim_header->unknown = 1;
				 break;
		case 24: iim_header->unknown = 4;
				 break;
	}

	iim_header->width = width;
	iim_header->height = height;

	return;
} /* write_header */



/*
				if(BitsPerPixel == 8)
				{
					length = f_len;
					oziel = ziel;
					ziel += headsize;
					pal = smurf_struct->smurf_pic->palette;
					do
					{
						pixval = (*buffer++ * 3);
						*ziel++ = (((long)pal[pixval] * 871L)
								 + ((long)pal[pixval + 1] * 2929L)
								 + ((long)pal[pixval + 2] * 295L)) >> 12;
					} while(--length);
					ziel = oziel;
				}
				else
					memcpy(ziel + headsize, buffer, f_len);
*/