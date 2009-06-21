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
/*				PSD - Adobe Photoshop 2.5/3.0/4.0			*/
/* Version 0.1  --  15.02.96								*/
/*	  xxx													*/
/* Version 0.2  --  27.05.96								*/
/*	  PSDs mit > 3 Planes werden nun auch korrekt gelesen	*/
/*	  indem die Åbrigen Planes ignoriert werden.			*/
/* Version 0.3  --  20.03.97								*/
/*	  Nach Photoshop-Doku von Mario runderneuert			*/
/*==========================================================*/

#ifdef GERMAN
#define ERROR1 "[1][Versionsnummer ungleich 1! | Bild bitte an uns einschicken][ OK ]"
#else
#ifdef ENGLISH
#define ERROR1 "[1][Version number is not 1! | Please send the picture to us.][ OK ]"
#else
#ifdef FRENCH
#define ERROR1 "[1][Version number is not 1! | Please send the picture to us.][ OK ]"
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

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Photoshop 3.0",
						0x0030,
						"Christian Eyrich",
						"PSD", "", "", "", "",
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
/*      PSD - Photoshop 2.5/3.0/4.0-Format (PSD)	*/
/*		1, 8, 16 und 24 Bit (in 3 Ebenen),			*/
/*		unkomprimiert, RLE							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char v1, v2;
	char *buffer, *obuffer, *ziel, *oziel, *pal, *ppal, 
		 n, p, comp, BitsPerPixel, Channels, Planes, ColMode;
	char dummy[3], impmessag[21];
	
	unsigned int i, x, y, w, width, height, cols;

	unsigned long length;

	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;

	if(*(unsigned long *)buffer != '8BPS')
		return(M_INVALID);
	else
	{
		if(*(unsigned int *)(buffer + 4) != 1)
		{
			form_alert(0, ERROR1 );
			return(M_PICERR);
		}	

		Channels = Planes = (char)*(unsigned int *)(buffer + 0x0c);
		if(Planes > 3)
			Planes = 3;

	/* BitsPerPixel  = BitsPerChannel * Planes */
		BitsPerPixel = (char)(*(unsigned int *)(buffer + 0x016) * Planes);

		height = (unsigned int)*(unsigned long *)(buffer + 0x0e); 
		width = (unsigned int)*(unsigned long *)(buffer + 0x12);

		/* 0 = Bitmap, 1 = Grayscale, 2 = indexed, 3 = RGB */
		/* 4 = CMYK, 7 = Multichannel, 8 = Duotone, 9 = Lab */
		ColMode = (char)*(unsigned int *)(buffer + 0x18);

		buffer += 0x1a;
	/* Color Mode Data */
		length = *((unsigned long *)buffer)++;
		if(ColMode == 2)									/* Palettenbild */
			cols = (unsigned int) (length / 3);
		else
			if(ColMode == 1 || ColMode == 8)				/* Graustufen oder Duotone */
				cols = 1 << BitsPerPixel;
			else
				cols = 0;
		ppal = buffer + 4;
		buffer += length;									/* Block Åberspringen */

	/* Image Resources Block */
		length = *((unsigned long *)buffer)++;
		buffer += length;									/* Block Åberspringen */

	/* Layer and Mask Information Block */
		length = *((unsigned long *)buffer)++;
		buffer += length;									/* Block Åberspringen */

	/* Image Data Block */
		comp = (char)*((unsigned int *)buffer)++;
		if(comp)
			buffer += (long)Channels * (long)height * 2L;	/* LÑngentabelle Åberspringen */

		strncpy(smurf_struct->smurf_pic->format_name, "Photoshop 2.5/3.0 .PSD", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "Photoshop ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if((ziel = SMalloc(((long)width * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			oziel = ziel;

			if(BitsPerPixel == 1)
				w = (width + 7) / 8;
			else
				w = width;
			
			for(p = 0; p < Planes; p++) /* Planes */
			{
				ziel = oziel + p;

				y = 0;
				do
				{
					x = 0;
					do
					{
						if(!comp)
						{
							*ziel = *buffer++;
							ziel += Planes;
							x++;
						}
						else
						{
							v1 = *buffer++;
/*	/* -128 kommt lt. Doku nie vor */
							if(v1 != -128)
							{ */

								if(v1 > 0x7f)
								{
									n = 0x101 - v1;

									v2 = *buffer++;

									x += n;

									while(n--)
									{
										*ziel = v2;
										ziel += Planes;
									}
								}
								else
								{
									n = v1 + 1;

									x += n;

									while(n--)
									{
										*ziel = *buffer++;
										ziel += Planes;
									}
								}
/*							} /* -128? */ */
						} /* comp? */
					} while(x < w); /* x */
					y++;
				} while(y < height); /* y */
			} /* p */

			buffer = obuffer;
			ziel = oziel;

			smurf_struct->smurf_pic->pic_data = ziel;

			if(BitsPerPixel == 1)
				smurf_struct->smurf_pic->format_type = FORM_STANDARD;
			else
				smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

			pal = smurf_struct->smurf_pic->palette;
			switch(ColMode)
			{
				/* Bitmap */
				case 0:	pal[0] = 255;
						pal[1] = 255;
						pal[2] = 255;
						pal[3] = 0;
						pal[4] = 0;
						pal[5] = 0;
						break;
				/* Grayscale und Duotone */
				case 1:
				case 8:	for(i = 0; i < cols; i++)
						{	
							*pal++ = (char)i;
							*pal++ = (char)i;
							*pal++ = (char)i;
						}
						break;
				/* Indexed */
				case 2:	for(i = 0; i < cols; i++)
						{
							*pal++ = *ppal;
							*pal++ = *(ppal + cols);
							*pal++ = *(ppal + cols + cols);
							ppal++;
						}
						break;
				default: break;
			}

			smurf_struct->smurf_pic->col_format = RGB;
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);
	return(M_PICDONE);
}