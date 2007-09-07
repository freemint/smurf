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
/*						Microsoft Paint						*/
/* Version 0.1  --  irgendwann Anfang 1996					*/
/*	  1 Bit, Version 1 und 2								*/
/* Version 0.2  --  31.01.97								*/
/*	  AbstÅrze bei manchen Bildern behoben und fÅr Block-	*/
/*	  routinen vorbereitet.									*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

/* Dies bastelt direct ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Microsoft Paint",
						0x0020,
						"Dale Russell, Christian Eyrich",
						"MSP", "", "", "", "",
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

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Microsoft Paint					*/
/*		1 Bit, unkomprimiert, RLE					*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel, *pal,
		 version, i, vpos = 0, v1, v2, n;

	unsigned int *Scanmap, *Scanpoint,
				 width, height, x, y, valid = 0;

	unsigned long w;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	if(strncmp(buffer, "DanM", 4) == 0)
			version = 1;
	else
		if(strncmp(buffer, "LinS", 4) == 0)
			version = 2;
		else
			return(M_INVALID);

	for(i = 0; i < 13; i++)
	{
		valid = valid ^ *(unsigned int *)(buffer + vpos);
		vpos += 2;
	}	

	if(valid != 0)
		return(M_PICERR);

	width = swap_word(*(unsigned int *)(buffer + 0x04));
	height = swap_word(*(unsigned int *)(buffer + 0x06));

	strncpy(smurf_struct->smurf_pic->format_name, "Microsoft Paint", 21);
	smurf_struct->smurf_pic->pic_width = width;
	smurf_struct->smurf_pic->pic_height = height;
	smurf_struct->smurf_pic->depth = 1;

	smurf_struct->services->reset_busybox(128, "Microsoft Paint 1 Bit");

	w = (unsigned long)(width + 7) / 8;
	if((ziel = SMalloc(w * (long)height)) == 0)
		return(M_MEMORY);
	else
	{
		memset(ziel, 0, w * (long)height);

		if((Scanmap = (unsigned int *)Malloc((long)height * 2L)) == 0)
		{
			SMfree(ziel);
			return(M_MEMORY);
		}
		memset(Scanmap, 0, (long)height * 2L);

		Scanpoint = (unsigned int *)(buffer + 0x20);
		for(x = 0; x < height; x++)
			Scanmap[x] = swap_word(*Scanpoint++);		/* LÑngewerte Åbertragen */

		buffer += 0x20;
		if(version == 2)
			buffer += (long)height * 2L;
		oziel = ziel;

		y = 0;
		do
		{
			if(version == 1)
			{
				memcpy(ziel, buffer, w);
				ziel += w;
				buffer += w;
			}
			else
			{
				if(Scanmap[y])
				{
					x = 0;
					do
					{
						v1 = *buffer++;
						if(v1)
						{
							n = v1;

							x += n;
							while(n--)
								*ziel++ = *buffer++;
						}
						else
						{
							n = *buffer++;
							v2 = *buffer++;

							x += n;
							while(n--)
								*ziel++ = v2;
						}		
					} while(x < Scanmap[y]);
				}
				else 
				{
					memset(ziel, 0xff, w);	
					ziel += w;
				}
			} /* version */
		} while(++y < height); /* y */

		buffer = obuffer;
		ziel = oziel;

		smurf_struct->smurf_pic->pic_data = ziel;

		smurf_struct->smurf_pic->format_type = FORM_STANDARD;

		pal = smurf_struct->smurf_pic->palette;

		pal[0] = 255;
		pal[1] = 255;
		pal[2] = 255;
		pal[3] = 0;
		pal[4] = 0;
		pal[5] = 0;

		smurf_struct->smurf_pic->col_format = RGB;

		Mfree(Scanmap);

		SMfree(buffer);
	} /* Malloc */

	return(M_PICDONE);
}