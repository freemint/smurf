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
/*					DA's Layout DIP-Preview					*/
/* Version 0.1  --  08.10.97								*/
/*	  24 Bit												*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

#undef WHITE				/* is used in AES.H	*/
#define WHITE	1
#define NWHITE	2
#define BANFANG	1
#define BMITTE	2
#define ZANFANG	1
#define ZMITTE	2

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"DA's Layout Preview",
						0x0010,
						"Christian Eyrich",
						"DIP", "", "", "", "",
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
/*				DA's Layout DIP-Preview				*/
/*		24 Bit, eigene Kompression					*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel,
		 BitsPerPixel, DatenOffset, align, status, pos, pix, run, pixel;
	char dummy[3], impmessag[21];

	unsigned int x, y, width, height, realwidth;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	/* kein DIP oder kein Preview enthalten */
	if(strncmp(buffer, "DALAYOUT", 8) != 0 ||
	   *(unsigned int *)(buffer + 0x0a) == 0x001e)
		return(M_INVALID);
	else
	{
		BitsPerPixel = 24;
		
		width = *(unsigned int *)(buffer + 0x1c); 
		realwidth = width * 3;
		height = *(unsigned int *)(buffer + 0x1e);

		DatenOffset = 0x24;
	
		strncpy(smurf_struct->smurf_pic->format_name, "DA's DIP-Preview", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "DIP-Preview ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if((ziel = SMalloc((long)width * (long)height * 3L)) == 0)
			return(M_MEMORY);
		else 
		{
			memset(ziel, 0x0, (long)width * (long)height * 3L);
			oziel = ziel;

			buffer += DatenOffset;

			y = 0;
			do
			{
				align = BANFANG;
				status = WHITE;
				pos = ZANFANG;

				x = 0;
				do
				{
					pix = *buffer++;

					/* restliche Pixel der Zeile weiû machen, also Åberspringen */
					if(pos == ZMITTE)
					{
						ziel += (width -x ) * 3;
						buffer--;						/* nÑchstes Statement nochmal */
						break;
					}

					/* ganze Zeile weiû, also Åberspringen */
					if(pix == width)
					{
						ziel += realwidth;

						x = width;
					}
					else
					{
						/* pix Pixel weiû, also Åberspringen */
						if(status == WHITE)
						{
							ziel += pix * 3;

							status = NWHITE;
						}
						else
						{
							run = pix;
							while(run--)
							{
								if(align == BANFANG)
								{
									pixel = *buffer++;
									*ziel++ = pixel&0xf0;
									*ziel++ = pixel << 4;
									pixel = *buffer++;
									*ziel++ = pixel&0xf0;
									align = BMITTE;
								}
								else
								{
									*ziel++ = pixel << 4;
									pixel = *buffer++;
									*ziel++ = pixel&0xf0;
									*ziel++ = pixel << 4;
									align = BANFANG;
								}
							}

							pos = ZMITTE;
						}

						x += pix;
					}
				} while(x < width);
			} while(++y < height);

			buffer = obuffer;
			ziel = oziel;
	
			smurf_struct->smurf_pic->pic_data = ziel;
			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

			smurf_struct->smurf_pic->col_format = CMY;

		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);
	return(M_PICDONE);
}