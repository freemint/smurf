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
/*						Spiegeln-Modul						*/
/* Version 0.1  --  25.03.96								*/
/*	  1-8 (pixelpacked und Standardformat), 16 und 24 Bit	*/
/* Version 0.2  --  03.04.96								*/
/*	  im Standardformat wurden zwar die Zeilen byteweise	*/
/*	  aber nicht die Bytes bitweise gespiegelt.				*/
/* Version 0.3  --  25.05.96								*/
/*	  Mit eigenener Resource - seufz						*/
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

#include "mirror.rsh"
#include "mirror.rh"

int (*popup)(POP_UP *popup_struct, int mouseflag, int button, OBJECT *poptree);

MOD_INFO module_info = {"Spiegeln",
						0x0030,
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


MOD_ABILITY  module_ability = {
						1, 2, 4, 7, 8,
						16, 24, 0,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_STANDARD,
						FORM_BOTH,
						FORM_PIXELPAK,
						FORM_PIXELPAK,
						FORM_BOTH,
						0,
						};

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*			Spiegeln (horizontal und vertikal)		*/
/*		1-8, 16 und 24 Bit 							*/
/*	ein Preview sehe ich bei der Geschwindigkeit	*/
/*	und Umkehrbarkeit der Funktion als sinnlos an.	*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	char *data, *data2, *buffer, *cbuffer,
		 BitsPerPixel, byte, p, Planes, t;
	static char hor = 0, vert = 0, translate[256];

 	int item;
	unsigned int *data16, *cbuffer16, width, height, y, i;

	unsigned long x, realwidth, length;

	OBJECT *pop_form;
	POP_UP pop_up;


/* Wenn das Modul zum ersten Mal gestartet wurde */
	if(smurf_struct->module_mode == MSTART)
	{
		i = 0;
		do
		{
			translate[i] = (i & 0x01) << 7;
			translate[i] |= (i & 0x02) << 5;
			translate[i] |= (i & 0x04) << 3;
			translate[i] |= (i & 0x08) << 1;
			translate[i] |= (i & 0x10) >> 1;
			translate[i] |= (i & 0x20) >> 3;
			translate[i] |= (i & 0x40) >> 5;
			translate[i] |= (i & 0x80) >> 7;
		} while(++i < 256);

		for(t = 0; t < NUM_OBS; t++)
			rsrc_obfix(&rs_object[t], 0);

		pop_form = rs_trindex[MIRROR_POP];

		popup = smurf_struct->services->popup;

		pop_up.popup_tree = MIRROR_POP;
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
			case HORIZONT:	hor = 1;
							break;
			case VERTIKAL:	vert = 1;
							break;						
			case BOTH:	hor = 1;
						vert = 1;
						break;
			default: break;
		}

		smurf_struct->module_mode = M_STARTED;
		return;
	}
	else
		if(smurf_struct->module_mode == MEXEC)
		{
			BitsPerPixel = smurf_struct->smurf_pic->depth;
	
			data = smurf_struct->smurf_pic->pic_data;
			width = smurf_struct->smurf_pic->pic_width;
			height = smurf_struct->smurf_pic->pic_height;
	
	/* Spiegelung an der horizontalen Achse */
	
			if(vert)
			{
				if(smurf_struct->smurf_pic->format_type == FORM_PIXELPAK)
				{
					byte = BitsPerPixel >> 3;
		
					realwidth = width * byte;
					if((buffer = Malloc(realwidth)) == 0)
					{
						smurf_struct->module_mode = M_MEMORY;
						return;
					}
	
					data2 = data + (height - 1) * realwidth;
	
					y = height >> 1;
					do
					{
						memcpy(buffer, data, realwidth);
						memcpy(data, data2, realwidth);
						memcpy(data2, buffer, realwidth);

						data += realwidth;
						data2 -= realwidth;
					} while(--y);
	
					Mfree(buffer);
				}	/* pixelpacked oder Standardformat? */
				else
					if(smurf_struct->smurf_pic->format_type == FORM_STANDARD)
					{
						Planes = BitsPerPixel;
	
						realwidth = (width + 7) / 8;
						if((buffer = Malloc(realwidth)) == 0)
						{
							smurf_struct->module_mode = M_MEMORY;
							return;
						}
	
						length = ((height >> 1) + height%2)* realwidth;
						p = Planes;
						do
						{
							data2 = data + (height - 1) * realwidth;
	
							y = height >> 1;
							do
							{
								memcpy(buffer, data, realwidth);
								memcpy(data, data2, realwidth);
								memcpy(data2, buffer, realwidth);
								data += realwidth;
								data2 -= realwidth;
							} while(--y);
							if(--p != 0)
								data += length;
						} while(p);	/* Planes */
						
						Mfree(buffer);
					}	/* pixelpacked oder Standardformat? */
			}	/* horizontal? */
	
	
	/* Spiegelung an der vertikalen Achse */
	
			data = smurf_struct->smurf_pic->pic_data;
	
			if(hor)
			{
				if(smurf_struct->smurf_pic->format_type == FORM_PIXELPAK)
				{
					byte = BitsPerPixel >> 3;
		
					realwidth = width * byte;
					if((buffer = Malloc(realwidth)) == 0)
					{
						smurf_struct->module_mode = M_MEMORY;
						return;
					}
	
					switch(BitsPerPixel)
					{
						case 8:	y = height;
								do
								{
									memcpy(buffer, data, realwidth);
									cbuffer = buffer + realwidth - 1;
									x = width;
									do
									{
										*data++ = *cbuffer--;
									} while(--x);
								} while(--y);
								break;
						case 16: y = height;
								 data16 = (unsigned int *)data;
								 do
								 {
									 memcpy(buffer, data, realwidth);
									 cbuffer16 = (unsigned int *)(buffer + realwidth - 1);
									 x = width;
									 do
									 {
										 *data16++ = *cbuffer16--;
									 }
									 while(--x);
								 } while(--y);
								 break;
						case 24: y = height;
								 do
								 {
									 memcpy(buffer, data, realwidth);
									 cbuffer = buffer + realwidth - 1;
									 x = width;
									 do
									 {
										 *(data + 2) = *cbuffer--;
										 *(data + 1) = *cbuffer--;
										 *data = *cbuffer--;
									 	 data += 3;
									 } while(--x);
								 } while(--y);
								 break;
						default: break;
					} /* BitsPerPixel */
	
					Mfree(buffer);
				}	/* pixelpacked oder Standardformat? */
				else
					if(smurf_struct->smurf_pic->format_type == FORM_STANDARD)
					{
						Planes = BitsPerPixel;
	
						realwidth = (width + 7) / 8;
						if((buffer = Malloc(realwidth)) == 0)
						{
							smurf_struct->module_mode = M_MEMORY;
							return;
						}
	
						p = Planes;
						do
						{
							y = height * 3;
							do
							{
								memcpy(buffer, data, realwidth);
								cbuffer = buffer + realwidth - 1;
								x = realwidth;
								do
								{
									*data++ = translate[*cbuffer--];
								} while(--x);
							} while(y -= 3);
	
						} while(--p);	/* Planes */
						
						Mfree(buffer);
					}	/* pixelpacked oder Standardformat? */
			}	/* vertikal? */
	
	/* wie schnell waren wir? */
	/*	printf("\n%lu", get_timer());
		getch(); */
				
			smurf_struct->module_mode = M_DONEEXIT;
			return;
		}
}