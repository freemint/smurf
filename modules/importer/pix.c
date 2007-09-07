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
/*				Degas (Elite)-Image Decoder					*/
/* Version 0.1  --  12.01.96								*/
/*	  Degas Importer von Dale umgeschrieben, Ausgabe in		*/
/*	  Planes, geringerer Speicherverbrauch					*/
/* Version 0.2  --  11.02.96								*/
/*	  nur PCx geht											*/
/* Version 0.3  --  26.02.96								*/
/*	  PIx geht nun auch, vielleicht gibt es bei manchen		*/
/*	  Bildern aber noch Farbprobleme						*/
/* Version 0.4  --  10.03.97								*/
/*	  Von Arrays auf Zeiger umgestellt.						*/
/*	  Erkennung war totale Scheiûe. Die Palette wurde auch	*/
/*	  zu wenig geshiftet, jetzt lÑuft's mit allen meinen	*/
/*	  Bildern. 												*/
/* Version 0.5  --  xx.xx.96								*/
/*	  PI4 - PI6 und PC4 bis PC6 eingefÅhrt					*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

char *fileext(char *filename);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Degas-Importer",
						0x0040,
						"Christian Eyrich, Dale Russell",
						"PI1","PI2","PI3","PC1","PC2",
						"PC3","","","","",
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
/*			Degas-(Elite) Dekomprimierer			*/
/*		1, 2, 4 Bit, keine und RLE					*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel, *pal, *extender,
		 p, comp, Planes, BitsPerPixel;
	char dummy[3], impmessag[21];

	unsigned int *ppal, i, n, x, y, w, v1, v2,
				 width, height, DatenOffset;

	unsigned long pla, plh, plo;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	extender = fileext(smurf_struct->smurf_pic->filename);

	if(((*buffer != 0x00 && *buffer != 0x80) || *(buffer + 1) > 2) ||
		(strnicmp(extender, "PI", 2) != 0 && strnicmp(extender, "PC", 2) != 0) ||
		!(*(extender + 2) >= '1' && *(extender + 2) <= '3'))
		return(M_INVALID);
	else	
	{
		comp = *buffer;

		switch(*(buffer + 1))
		{
			case 0:	width = 320;
					height = 200;
					Planes = 4;
					break;
			case 1:	width = 640;
					height = 200;
					Planes = 2;
					break;
			case 2:	width = 640;
					height = 400;
					Planes = 1;
					break;
/*			case 3:	width = 320;
					height = 480;
					Planes = 8;
					break;
			case 4:	width = 640;
					height = 480;
					Planes = 4;
					break;
			case 5:	width = 1280;
					height = 960;
					Planes = 1;
					break; */
			default:break;
		}
		BitsPerPixel = Planes;

		DatenOffset = 34;
		
		strncpy(smurf_struct->smurf_pic->format_name, "Degas .PIx, PCx", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "Degas ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		w = width / 8;

		if((ziel = SMalloc(((long)width * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			oziel = ziel;

			buffer += DatenOffset;

			plh = (height * w);		/* Hîhe einer Plane in Byte */

			if(!comp)				/* unkomprimiert */
			{
				y = 0;
				do /* height */
				{
					plo = (y * w);	/* Offset vom Planeanfang in Bytes */
					x = 0;
					do /* width */
					{
						p = 0;
						do /* Planes */
						{
							pla = (plh * p);	/* Abstand dieser Plane vom Bildanfang */
							ziel = oziel + pla + plo + x; /* Zieladresse der dekodierten Scanline */
							*ziel++ = *buffer++;
							*ziel = *buffer++;
						} while(++p < Planes);
						x += 2;
					} while(x < w);
				} while(++y < height);
			} /* comp? */
			else					/* komprimiert */
			{
				y = 0;
				do /* height */
				{
					plo = (y * w);	/* Offset vom Planeanfang in Bytes */
					p = 0;
					do /* Planes */
					{
						pla = (plh * p);	/* Abstand dieser Plane vom Bildanfang */
						ziel = oziel + pla + plo; /* Zieladresse der dekodierten Scanline */
						x = 0;
						do /* width */
						{
							v1 = *buffer++;
							if((v1 & 0x80) == 0x80)
							{
								n = (0x101 - v1);
								v2 = *buffer++;
								for(i = 0; i < n; i++)
									*ziel++ = v2;
								x += n;
							}
							else
							{
								for (i = 0; i < v1 + 1; i++)
									*ziel++ = *buffer++;
								x += (v1 + 1);
							}
						} while(x < w);
					} while(++p < Planes);
				} while(++y < height);
			} /* comp? */

			buffer = obuffer;
			ziel = oziel;

			smurf_struct->smurf_pic->pic_data = ziel;

			smurf_struct->smurf_pic->format_type = FORM_STANDARD;
			
			pal = smurf_struct->smurf_pic->palette;
			ppal = (unsigned int *)(buffer + 2);
		
			if(BitsPerPixel == 1)
			{
				pal[0] = 255;
				pal[1] = 255;
				pal[2] = 255;
				pal[3] = 0;
				pal[4] = 0;
				pal[5] = 0;
			}
			else
				for(i = 16; i > 0; i--)
				{
					v1 = *ppal++;
					*pal++ = (char)((v1 & 0x700) >> 3);
					*pal++ = (char)((v1 & 0x070) << 1);
					*pal++ = (char)((v1 & 0x007) << 5);
				}


			smurf_struct->smurf_pic->col_format = RGB;
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);
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