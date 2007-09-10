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
/*						GEM Icon Format						*/
/* Version 0.1  --  02.07.98								*/
/*	  monochrome Icons										*/
/* Version 0.2  --  04.07.98								*/
/*	  2, 4 und 8 Bit										*/
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

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"C-Source",
						0x0020,
						"Christian Eyrich",
						"ICN", "", "", "", "",
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
						1, 2, 4, 8, 0,
						0, 0, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				GEM Icon-Format (ICN)				*/
/*		1, 2, 4, 8 Bit, unkomprimiert				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
	EXPORT_PIC *exp_pic;

	char *buffer, *obuffer, *ziel, *oziel, *fname, hexTable[16],
		 BitsPerPixel, two, p, Planes;

	unsigned int x, y, width, height;

	unsigned long w, headlen, f_len;


	hexTable[0] = '0';  hexTable[1] = '1';
	hexTable[2] = '2';  hexTable[3] = '3';
	hexTable[4] = '4';  hexTable[5] = '5';
	hexTable[6] = '6';  hexTable[7] = '7';
	hexTable[8] = '8';  hexTable[9] = '9';
	hexTable[10] = 'A'; hexTable[11] = 'B';
	hexTable[12] = 'C'; hexTable[13] = 'D';
	hexTable[14] = 'E'; hexTable[15] = 'F';

	switch(smurf_struct->module_mode)
	{
		case MSTART:
			smurf_struct->module_mode = M_WAITING;	/* Ich warte... */
			break;

	/* Farbsystem wird vom Smurf erfragt */
		case MCOLSYS:
			smurf_struct->event_par[0] = RGB;

			smurf_struct->module_mode = M_COLSYS;
			
			break;

		case MEXEC:
/* wie schnell sind wir? */
/*	init_timer(); */

			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			buffer = smurf_struct->smurf_pic->pic_data;
			obuffer = buffer;

			exp_pic = (EXPORT_PIC *)SMalloc(sizeof(EXPORT_PIC));

			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
			BitsPerPixel = smurf_struct->smurf_pic->depth;
			Planes = BitsPerPixel;

			/*
			/* GEM Icon Definition: */
			#define xxxxxxxx_W 0x0020
			#define xxxxxxxx_H 0x0020
			#define DATASIZE 0x0040
			UWORD xxxxxxxx[DATASIZE] =
			{
			};
			*/
			headlen = 144;

			w = (unsigned long)(width + 15) / 16;

			/* Breite (in 16tel Pixel) * H”he * Stringl„nge pro 16 Pixel * Planes + Planes mal CRLF */
			f_len = w * (unsigned long)height * strlen(" 0x0000,") * Planes + Planes * 2;

			if((ziel = (char *)SMalloc(headlen + f_len)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return(exp_pic);
			}
			else
			{
				memset(ziel, 0x0, headlen + f_len);
				oziel = ziel;

				fname = strrchr(smurf_struct->smurf_pic->filename, '\\');
				if(fname != NULL)
					fname++;
				else
					fname = smurf_struct->smurf_pic->filename;

				strtok(fname, ".");

				/* Header schreiben */
				sprintf(ziel, "/* GEM Icon Definition: */\r\n#define ICON_W 0x%04lx\r\n#define ICON_H 0x%04lx\r\n#define DATASIZE 0x%04lx\r\nUWORD %s[DATASIZE] =\r\n{",
							   (long)width, (long)height, w * (long)height * Planes, fname);

				ziel += strlen(ziel);

				w = (unsigned long)(width + 7) / 8;

				p = 0;
				do
				{
					*((unsigned int *)ziel)++ = '\r\n';

					y = 0;
					do
					{
						x = 0;
						do
						{
							*((unsigned int *)ziel)++ = '0x';

							two = 0;
							do
							{
								*ziel++ = hexTable[*buffer >> 4];
								*ziel++ = hexTable[(*buffer++)&0x0f];
								x++;
							} while(++two < 2 && x < w);

							if(two == 2)
								*((unsigned int *)ziel)++ = ', ';
						} while(x < w);
						if(two == 1)
							*((unsigned long *)ziel)++ = '00, ';
					} while(++y < height);
				} while(++p < Planes);

				ziel -= 2;
				*((unsigned int *)ziel)++ = '\r\n';
				*((unsigned int *)ziel)++ = '};';

				f_len = ziel - oziel;

				buffer = obuffer;
				ziel = oziel;

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