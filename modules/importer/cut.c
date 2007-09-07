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
/*			Dr.Halo Import-Format - Hallo Doktor!			*/
/*  Importformat fÅr das Programm Dr.Halo, da das Programm- */
/*  eigene Format nicht zur Speicherung von Bildern geeignet*/
/*  ist ...													*/
/* Version 0.1  --  07.01.96								*/
/*    Mehr schlecht als recht, eher zum kotzen				*/
/* Version 0.2  --  12.01.96								*/
/*    Da es ja irgendwann immer klappt, geht's nun			*/
/* Version 0.3  --  27.01.96								*/
/*	  Nun, ich hatte mich wohl geirrt, da hat nix geklappt.	*/
/*	  Leider ist nicht mehr eindeutig zu reproduzieren,		*/
/*	  weshalb ich mich zu so einer riskanten Aussage hin-	*/
/*	  reiûen lies. Apropos lies, Du, Christian, solltest das*/
/*	  nÑchste Mal Deine Kommentare etwas lesbarer schreiben.*/
/* Version 0.4  --  06.02.96								*/
/*	  In meinem Testbild war zwar was zu erkennen, aber das */
/*	  gelbe vom Ei war es nicht. Entgegen meiner Meinung, es*/
/*	  ginge eben nicht anders, geht es nun doch problemlos.	*/
/*	  Die Lîsung: Klaus hatte wiedermal Scheiûe erzÑhlt ...	*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

char *fileext(char *filename);
void *fload(char *Path, int header);

/* Dies bastelt direct ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"Dr. Halo Import-Modul",
						0x0040,
						"Christian Eyrich",
						"CUT", "PAL", "", "", "",
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
/*		Dr.Halo (Pferdedoktor)  .CUT + .PAL			*/
/*		8 Bit, RLE									*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *ziel, *oziel, *pal, *ppal, *fname,
		 n, name[257];
	
	unsigned int x, y, i, v1, v2,
				 width, height, maxr, maxg, maxb, cols;
	
	unsigned long pal_pos = 0;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;	

	buffer = smurf_struct->smurf_pic->pic_data;

	/* Header Check */
	fname = smurf_struct->smurf_pic->filename;
	if(stricmp(fileext(fname), "CUT") != 0 ||
	   *(unsigned int *)(buffer + 0x04) != 0)
		return(M_INVALID);
	else
	{
		width = swap_word(*(unsigned int *)buffer);
		height = swap_word(*(unsigned int *)(buffer + 0x02));
		
		strncpy(smurf_struct->smurf_pic->format_name, "Dr. Halo Import .CUT", 21);
		smurf_struct->smurf_pic->pic_width = width; 
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = 8;

		smurf_struct->services->reset_busybox(128, "Dr. Halo 8 Bit");
	
	/* Dr.Halo entpacken */
		if((ziel = SMalloc((long)width * (long)height)) == 0)
			return(M_MEMORY);
		else 
		{
			oziel = ziel;

			obuffer = buffer;
			buffer += 6;

			y = 0;
			do
			{
				x = swap_word(*((unsigned int *)buffer)++);

				do
				{
					v1 = *buffer++;
					x--;

					if((n = (v1 & 0x7f)) != 0)
					{
						if(v1 & 0x80)
						{
							v2 = *buffer++;
							x--;
	
							for(i = 0; i < n; i++)
								*ziel++ = v2;
						}
						else
						{
							for(i = 0; i < n; i++)
							{
								*ziel++ = *buffer++;
								x--;
							}
						}
					}
				} while(x > 0);
			} while(++y < height);

			ziel = oziel;
			buffer = obuffer;
		
			smurf_struct->smurf_pic->pic_data = ziel;
			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

		/* Palettenname zusammenstÅckeln */
			strcpy(name, fname);
			strcpy(fileext(name), "PAL");

		/* Palette laden */
		/* Vorsicht, Palette liegt in Blîcken vor */
			pal = smurf_struct->smurf_pic->palette;

			if((ppal = fload(name, 0)) == 0)
			{
				form_alert(0, "[1][Die Farbpalette konnte nicht|gefunden werden!][ OK ]");

				for(i = 0; i < 256; i++)
				{
					pal[pal_pos++] = (char)i;
					pal[pal_pos++] = (char)i;
					pal[pal_pos++] = (char)i;
				}
			}
			else
			{
				cols = swap_word(*(unsigned int *)(ppal + 0x0c));
				maxr = swap_word(*(unsigned int *)(ppal + 0x0e));
				maxg = swap_word(*(unsigned int *)(ppal + 0x10));
				maxb = swap_word(*(unsigned int *)(ppal + 0x12));

				ppal += 40;
				for(i = 0; i < cols; i++)
				{
/*					if(512 - i * 6 < 6)
						ppal += 512 - i * 6; */
					*pal++ = ((*ppal++ + (*ppal++ << 8)) * 255) / maxr;
					*pal++ = ((*ppal++ + (*ppal++ << 8)) * 255) / maxg;
					*pal++ = ((*ppal++ + (*ppal++ << 8)) * 255) / maxb;
				}
			}

			smurf_struct->smurf_pic->col_format = RGB;
					
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);

	return(M_PICDONE);
}


/* --- FLOAD --- */

void *fload(char *Path, int header)
{
	char *fil;

	int file;

	long back, f_len;

	if((back = Fopen(Path, FO_READ)) >= 0)	/* Datei îffnen */
	{
		file = (int)back;

		f_len = Fseek(0, file, 2);      /* LÑnge ermitteln */
		
		if((fil = Malloc((long)f_len - header)) == 0)
		{
			form_alert(0, "[1][FÅr die Farbpalette steht nicht|genug Speicher zur VerfÅgung!]\[ OK ]");
			fil = 0;
		}
		else
		{
			Fseek(header, file, 0);         /* Anfang suchen */
			Fread(file, (long)f_len - header, fil);   /* Palette lesen */
		}
		Fclose(file);               /* Kanal schlieûen */
	}
	else
		fil = 0;

	return(fil);
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