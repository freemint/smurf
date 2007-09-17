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
/*						90ø Drehen-Modul					*/
/* Version 0.1  --  26.06.96								*/
/*	  TC-Bilder only										*/
/* Version 0.2  --  30.06.96								*/
/*	  8 und 16 Bit pp										*/
/* =========================================================*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
#include <..\..\..\demolib.h>

#include "spin90.rsh"
#include "spin90.rh"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*popup)(POP_UP *popup_struct, int mouseflag, int button, OBJECT *poptree);

MOD_INFO module_info = {"Drehen 90ø",
						0x0020,
						"Christian Eyrich",
						"", "", "", "", "",
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
						0,64,
						0,64,
						0,64,
						0,64,
						0,10,
						0,10,
						0,10,
						0,10,
						0, 0, 0, 0,
						0, 0, 0, 0,
						0, 0, 0, 0,
						1
						};


MOD_ABILITY module_ability = {
						8, 16, 24, 0, 0,
						0, 0, 0,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*					Drehen um 90ø					*/
/*		8, 16 und 24 Bit pp							*/
/*	Dieses Modul ist jetzt schon zum Tode verur-	*/
/*	teilt da es im frei drehen-Modul als Sonderfall	*/
/*	aufgehen wird.									*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data1, *data2, BitsPerPixel,
		 bytes, t;
	static char left = 0, right = 0, two = 0;
	
	int item;
	unsigned int width, height, x, y, x2;
	
	unsigned long pos2, jmp, length;

	OBJECT *pop_form;
	POP_UP pop_up;	


/* Wenn das Modul zum ersten Mal gestartet wurde */
	if(smurf_struct->module_mode == MSTART)
	{
		for(t = 0; t < NUM_OBS; t++)
			rsrc_obfix(&rs_object[t], 0);

		pop_form = rs_trindex[SPIN90];

		popup = smurf_struct->services->popup;

		pop_up.popup_tree = SPIN90;
		pop_up.item = 0;
		pop_up.display_tree = 0;
		pop_up.display_obj = -1;
		pop_up.Cyclebutton = -1;
		item = popup(&pop_up, 1, 1, pop_form);

/* wie schnell sind wir? */
/*	init_timer(); */

		switch(item)
		{
			case -1: smurf_struct->module_mode = M_EXIT; 
					 return;
			case LEFT: left = 1;
						break;
			case RIGHT: right = 1;
						break;
			case TWO: two = 1;
					  break;
			default: break;
		}

		smurf_struct->module_mode = M_STARTED;
		return;
	}
	else
		if(smurf_struct->module_mode == MEXEC)
		{
			SMalloc = smurf_struct->services->SMalloc;
			SMfree = smurf_struct->services->SMfree;

			BitsPerPixel = smurf_struct->smurf_pic->depth;
	
			bytes = BitsPerPixel >> 3;
	
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
	
			data1 = smurf_struct->smurf_pic->pic_data;
			if((data2 = SMalloc((unsigned long)width * (unsigned long)height * (long)bytes)) == 0)
			{
				smurf_struct->module_mode = M_MEMORY;
				return;
			}
		
			if(left)                
			{
				jmp = (height + 1) * bytes;
				length = ((unsigned long)width - 1) * ((unsigned long)height * bytes);
				y = 0;
				do
				{
					pos2 = length;
					x = 0;
					do
					{
						data2[pos2++] = *data1++;
						if(bytes > 1)
						{
							data2[pos2++] = *data1++;
							if(bytes > 2)
								data2[pos2++] = *data1++;
						}
						pos2 -= jmp;
					} while(++x < width);
					length += bytes;
				} while(++y < height);
			}
			else
				if(right)
				{
					jmp = (height - 1) * bytes;
					x2 = (height - 1) * bytes;
					y = 0;
					do
					{
						pos2 = x2;
						x = 0;
						do
						{
							data2[pos2++] = *data1++;
							if(bytes > 1)
							{
								data2[pos2++] = *data1++;
								if(bytes > 2)
									data2[pos2++] = *data1++;
							}
							pos2 += jmp;
						} while(++x < width);
						x2 -= bytes;
					} while(++y < height);
				}
				else
					if(two)                
					{
						jmp = bytes * 2;
						length = (unsigned long)width * (unsigned long)height;
						pos2 = (unsigned long)width * (unsigned long)height * bytes - bytes;
						do
						{
							data2[pos2++] = *data1++;
							if(bytes > 1)
							{
								data2[pos2++] = *data1++;
								if(bytes > 2)
									data2[pos2++] = *data1++;
							}
							pos2 -= jmp;
						} while(--length);
					}
	
	/* wie schnell waren wir? */
	/*	printf("\n%lu", get_timer());
		getch(); */
			
			SMfree(smurf_struct->smurf_pic->pic_data);
			smurf_struct->smurf_pic->pic_data = data2;
			if(!two)
			{
				smurf_struct->smurf_pic->pic_width = height;
				smurf_struct->smurf_pic->pic_height = width;
			}
			smurf_struct->module_mode = M_DONEEXIT;
			return;
		}
}