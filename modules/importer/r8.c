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
/*					Cubicomb Picturemaker					*/
/* Version 0.1  --  11.01.96								*/
/*    xxx													*/
/* =========================================================*/

#ifdef GERMAN
#define ERROR "[1][Die Farbpalette konnte nicht|geladen werden!][ Stop ]"
#else
#ifdef ENGLISH
#define ERROR "[1][Die Farbpalette konnte nicht|geladen werden!][ Stop ]"
#else
#ifdef FRENCH
#define ERROR "[1][Die Farbpalette konnte nicht|geladen werden!][ Stop ]"
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
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *fload(char *Path, int header);
int strsrcr(char *s, char c);

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Cubicomb Import-Modul",
						0x0010,
						"Christian Eyrich",
						"R8", "G8", "B8", "A8", "",
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
/*		Cubicomp Picturemaker  .R8+G8+B8+A8			*/
/*		24 Bit, unkomprimiert						*/
/*	Ein genialer Einfall, ein Bild ber vier Dateien*/
/*	zu verteilen. Klasse komprimiert und gerade		*/
/*	richtig fr fr User mit dicker Platte.			*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, name[128];

	unsigned int help, width, height;

	unsigned long i;

	struct rgb
			{
				char b;
				char g;
				char r;
			} *ziel;


	buffer = smurf_struct->smurf_pic->pic_data;
	
	/* Header Check */
	if(!(*(buffer + 1) == 0x0c && *(buffer + 2) == 0xff && *(buffer + 3) == 0x02))
		return(M_INVALID);
	else
	{
		width = *(buffer + 0x0a) + (*(buffer + 0x0b) << 8);
		height = *(buffer + 0x0c) + (*(buffer + 0x0d) << 8);

		strncpy(smurf_struct->smurf_pic->format_name, "Cubicomb Picturemaker", 21);
		smurf_struct->smurf_pic->pic_width = width; 
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = 24;

		smurf_struct->services->reset_busybox(128, "Cubicomb 24 Bit");

		if((ziel = Malloc((long)width * (long)height * 3)) == 0)
			return(M_MEMORY);
		else
		{
	/* Extender holen */
			help = strsrcr(smurf_struct->smurf_pic->filename, '.');
			strncpy(name, smurf_struct->smurf_pic->filename, help + 1);

	/* Rotanteil folgt */
		
			if(strcmp(smurf_struct->smurf_pic->filename + help + 1, "R8") == 0)
			{
				buffer = smurf_struct->smurf_pic->pic_data;
				buffer += 0x80;
			}
			else
			{
				name[help + 1] = '\0';
				strcat(name, "R8");
				buffer = fload(name, 0x80);
			}

			for(i = 0; i < (width * height); i++)
				ziel++->r = *buffer++;
		
			if(strcmp(smurf_struct->smurf_pic->filename + help + 1, "R8") != 0)
				Mfree(buffer);

	/* Grnanteil folgt */

			if(strcmp(smurf_struct->smurf_pic->filename + help + 1, "G8") != 0)
			{
				buffer = smurf_struct->smurf_pic->pic_data;
				buffer += 0x80;
			}
			else
			{
				name[help + 1] = '\0';
				strcat(name, "G8");
				buffer = fload(name, 0x80);
			}

			for(i = 0; i < (width * height); i++)
				ziel++->g = *buffer++;
	
			if(strcmp(smurf_struct->smurf_pic->filename + help + 1, "G8") != 0)
				Mfree(buffer);
	
	/* Blauanteil folgt */

			if(strcmp(smurf_struct->smurf_pic->filename + help + 1, "B8") != 0)
			{
				buffer = smurf_struct->smurf_pic->pic_data;
				buffer += 0x80;
			}
			else
			{
				name[help + 1] = '\0';
				strcat(name, "B8");
				buffer = fload(name, 0x80);
			}

			for(i = 0; i < (width * height); i++)
				ziel++->b = *buffer++;

			if(strcmp(smurf_struct->smurf_pic->filename + help + 1, "B8") != 0)
				Mfree(buffer);
		
			Mfree(smurf_struct->smurf_pic->pic_data);
			smurf_struct->smurf_pic->pic_data = ziel;
			smurf_struct->smurf_pic->format_type = 0;

		} /* Malloc */

	} /* Erkennung */

	return(M_PICDONE);
}


/* --- FLOAD --- */
void *fload(char *Path, int header)
{
	char *fil;
	long f_len;
	int file;
	
	file = (int) Fopen(Path, FO_READ);        /* Datei ”ffnen */
	if(file >= 0)
	{
		f_len = Fseek(0, file, 2);      /* L„nge ermitteln */
		
		if((fil = Malloc((long)f_len - header)) == 0)
			fil = 0;
		else
		{
			Fseek(header, file, 0);         /* Anfang suchen */
		
			if(fil != 0) 
				Fread(file, (long)f_len - header, fil);   /* Bild lesen */
		}
		Fclose(file);               /* Kanal schliežen */
	}
	else 
	{
		form_alert(0, ERROR );
		fil = 0;
	}                   

	return (fil);
}


/* --- STRSRCR --- */

int strsrcr(char *s, char c)
{
    int i;

    for(i = (int) strlen(s) - 1; i >= 0 && s[i] != c; i--)
        ;

    return i;
} /* strsrcr */