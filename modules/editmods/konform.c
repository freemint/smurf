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
/*					VDI-/hardwarekonform wandeln			*/
/* Version 0.1  --  11.06.97								*/
/*	  1-8 Bit												*/
/* Version 0.2  --  27.03.98								*/
/*	  Zeilenweise Assemblerleseroutine eingebaut.			*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include <..\..\demolib.h>

#define VDI		1
#define HARD	2

int (*busybox)(int pos);
void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);
int setpix_std_line(char *buf, char *std, int depth, long planelen, int howmany);

MOD_INFO module_info = {"konform wandeln",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
						"", "", "", "", "",
						"",
						"",
						"",
						"",
						"VDI-konform",
						"hardwarekonform",
						"",
						"",
						"",
						"",
						"",
						"",
						0,64,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						2, 3, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						2, 3, 4, 5, 6,
						7, 8, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*				VDI-/hardwarekonform wandeln		*/
/*		2-8 Bit 									*/
/* Das Modul kann nur Palettenbilder brauchen, wird */
/* aber eigentlich nur ben”tigt, wenn keine Palette */
/* im Bild enthalten ist.							*/
/* In 2, 3 und 4 Bit ist die Umsetzung normiert,	*/
/* was mache ich aber in anderen Farbtiefen?		*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char table2_to_vdi[] = {0, 2, 3, 1};
	char table3_to_vdi[] = {0, 2, 3, 6, 4, 7, 5, 1};
	char table4_to_vdi[] = {0, 15, 1, 2, 4, 6, 3, 5, 7, 8, 9, 10, 12, 14, 11, 13};
	char table2_to_hard[] = {0, 2, 3, 1};
	char table3_to_hard[] = {0, 2, 3, 6, 4, 7, 5, 1};
	char table4_to_hard[] = {0, 2, 3, 6, 4, 7, 5, 8, 9, 10, 11, 14, 12, 15, 13, 1};
	char *data, *pixbuf, *line, *plane_table,
		 BitsPerPixel, mode;
	
	int module_id;
	unsigned int x, y, width, height, bh, bl;

	unsigned long planelength, w;
	

/* wie schnell sind wir? */
/*	init_timer(); */

	module_id = smurf_struct->module_number;

/* Wenn das Modul zum ersten Mal gestartet wurde */
	if(smurf_struct->module_mode == MSTART)
	{
		smurf_struct->services->f_module_prefs(&module_info, module_id);
		smurf_struct->module_mode = M_WAITING;
		return;
	}
	else
		if(smurf_struct->module_mode == MEXEC)
		{
			if(smurf_struct->check1 > smurf_struct->check2)
				mode = VDI;
			else
				mode = HARD;

			BitsPerPixel = smurf_struct->smurf_pic->depth;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;

			data = smurf_struct->smurf_pic->pic_data;

			busybox = smurf_struct->services->busybox;

			if(mode == VDI)
				switch(BitsPerPixel)
				{
					case 2: plane_table = table2_to_vdi;
							break;
					case 3: plane_table = table3_to_vdi;
							break;
					case 4: plane_table = table4_to_vdi;
							break;
				}
			else
				switch(BitsPerPixel)
				{
					case 2: plane_table = table2_to_hard;
							break;
					case 3: plane_table = table3_to_hard;
							break;
					case 4: plane_table = table4_to_hard;
							break;
				}			

			pixbuf = (char *)Malloc(width + 7);

			if((bh = height / 10) == 0) 	/* busy-height */
				bh = height;
			bl = 0;							/* busy-length */
	
			if(smurf_struct->smurf_pic->format_type == FORM_PIXELPAK)
			{
				y = 0;
				do
				{
					if(!(y%bh))
					{
						busybox(bl);
						bl += 12;
					}

					x = 0;
					do
					{
						*data++ = plane_table[*data];
					} while(++x < width);
				} while(++y < height);
			}
			else
			{
				w = (unsigned long)((width + 7) / 8);
				planelength = (unsigned long)((width + 7) / 8) * (unsigned long)height;   /* L„nge einer Plane in Bytes */

				y = 0;
				do
				{
					if(!(y%bh))
					{
						busybox(bl);
						bl += 12;
					}

					memset(pixbuf, 0x0, width);
					getpix_std_line(data, pixbuf, BitsPerPixel, planelength, width);
					line = pixbuf;

					x = 0;
					do
					{
						*line++ = plane_table[*line];
					} while(++x < width);
					setpix_std_line(pixbuf, data, BitsPerPixel, planelength, width);
					data += w;
				} while(++y < height);
			}

			Mfree(pixbuf);
	
/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */
			
			smurf_struct->module_mode = M_PICDONE;
			return;
		}

		/* Mterm empfangen - Speicher freigeben und beenden */
		else 
			if(smurf_struct->module_mode == MTERM)
			{
				smurf_struct->module_mode = M_EXIT;
				return;
			}
}