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
/*					Q0 Japan Image File						*/
/* Version 0.1  --  13.01.96								*/
/*	 Ob die Bildgr”ženerkennung wohl geht?					*/
/*==========================================================*/

#ifdef GERMAN
#define ERROR "[1][Bildteil konnte nicht|geladen werden!][ Stop ]"
#else
#ifdef ENGLISH
#define ERROR "[1][Image portion could|not be loaded!][ Stop ]"
#else
#ifdef FRENCH
#define ERROR "[1][Image portion could|not be loaded!][ Stop ]"
#else
#error "Keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

char *fileext(char *filename);
void *fload(char *Path, int header);

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Q0 Japan Image",
						0x0010,
						"Christian Eyrich",
						"FAL", "Q0", "", "", "",
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
/*			Q0 Japan Image File-Format (FAL)		*/
/*	24 Bit, unkomprimiert							*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *ziel, *oziel, *buffer, *obuffer, *fname,
		 name[257], bf[12], c;
	
	unsigned int x, y, width, height;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;

	fname = smurf_struct->smurf_pic->filename;
	if(stricmp(fileext(fname), "FAL") != 0 &&
	   stricmp(fileext(fname), "Q0") != 0 ||
	   strncmp(buffer, "F_ALL(V1.00)", 12) != 0)
		return(M_INVALID);
	else
	{
		if(strcmp(fileext(fname), "Q0") == 0)
		{
			strcpy(name, fname);
			strcpy(fileext(name), "FAL");
		
			smurf_struct->services->reset_busybox(128, "Japan Image 24 Bit");

		/* Headerfile laden */        
			if((buffer = fload(name, 0)) == 0)
				return(M_MODERR);
			if(strcmp(buffer, "F_ALL(V1.00)") != 0)
			{
				Mfree(buffer);
				return(M_INVALID);
			}
		}

		c = 14;
		while(c < 12 && *(buffer + c) != ' ') /* 14 = Erkennungsstring + crlf */
			c++;			
		strncpy(bf, buffer + c , 12);
		width = atoi(strtok(bf, " "));
		height = atoi(strtok(0L, " "));

		SMfree(buffer);

		if(strcmp(fileext(fname), "Q0") == 0)
			buffer = smurf_struct->smurf_pic->pic_data;
		else
		{
			strcpy(name, fname);
			strcpy(fileext(name), "Q0");
		
		/* Bildfile laden */        
			if((buffer = fload(name, 0)) == 0)
				return(M_MODERR);
		}

		strncpy(smurf_struct->smurf_pic->format_name, "Q0 Japan Image .FAL + .Q0", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = 24;

		if((ziel = Malloc((long)width * (long)height * 3)) == 0)
			return(M_MEMORY);
		else
		{
			oziel = ziel;
			obuffer = buffer;

			y = 0;
			do
			{
				x = 0;
				do
				{
					*ziel++ = *buffer++;
					*ziel++ = *buffer++;
					*ziel++ = *buffer++;
				} while(++x < width);
			} while(++y < height);

			ziel  = oziel;
			buffer = obuffer;

			smurf_struct->smurf_pic->pic_data = ziel;

			smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);

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
			{
				Fread(file, (long)f_len - header, fil);   /* Bild lesen */
			}
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


char *fileext(char *filename)
{
	char *extstart;


	if((extstart = strrchr(filename, '.')) != NULL)
		extstart++;
	else
		extstart = strrchr(filename, '\0');
	
	return(extstart);
} /* fileext */
