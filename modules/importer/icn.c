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
/*						GEM Icon Format						*/
/* Version 0.1  --  10.09.96								*/
/*	  monochrome Icons										*/
/* Version 0.2  --  17.11.96								*/
/*	  Nun auch farbige Icons								*/
/* Version 0.3  --  26.04.98								*/
/*	  Nun auch 256farbige Icons								*/
/* =========================================================*/
 
#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"
#include "icn.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

char *fileext(char *filename);

int strsrchl(char *s, char c);
int strsrchr(char *s, char c);
 
/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"C-Source",
						0x0030,
						"Christian Eyrich",
						"ICN", "", "", "", "",
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
/*				GEM Icon-Format (ICN)				*/
/*		1, 2, 4, 8 Bit, unkomprimiert				*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *obuffer, *helpstr, *help, *pal, *fname, name_and_type[256],
		 BitsPerPixel, Planes, hexTable[256],
		 end = '}', strvalue[7];
	char dummy[3], impmessag[21], dimension;

	unsigned int *ziel, *oziel, i, width = 0, height = 0,
				 w;

	unsigned long Datasize = 0, length;


	hexTable['0'] = 0;  hexTable['1'] = 1;
	hexTable['2'] = 2;  hexTable['3'] = 3;
	hexTable['4'] = 4;  hexTable['5'] = 5;
	hexTable['6'] = 6;  hexTable['7'] = 7;
	hexTable['8'] = 8;  hexTable['9'] = 9;
	hexTable['A'] = 10; hexTable['B'] = 11;
	hexTable['C'] = 12; hexTable['D'] = 13;
	hexTable['E'] = 14; hexTable['F'] = 15;
	hexTable['a'] = 10; hexTable['b'] = 11;
	hexTable['c'] = 12; hexTable['d'] = 13;
	hexTable['e'] = 14; hexTable['f'] = 15;


	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	buffer = smurf_struct->smurf_pic->pic_data;
	obuffer = buffer;
 
	fname = smurf_struct->smurf_pic->filename;
 
	/* Header Check */
	if(stricmp(fileext(fname), "ICN") != 0)
		return(M_INVALID);
	else
	{
		i = 0;
		do
		{
			if((helpstr = strstr(buffer, "#define")) == NULL)
				return(M_PICERR);
			if(sscanf(helpstr, "#define %s %s", name_and_type, strvalue) != 2)
				return(M_PICERR);

			help = strrchr(name_and_type, '_');
			if(help != NULL && *(help + 1) != '\0')
				dimension = *(help + 1);
			else
				if(i == 0)								/* erster define */
					dimension = 'w';
				else
					if(i == 1)							/* zweiter define */
						dimension = 'h';
					else
						dimension = ' ';

			buffer = helpstr + 7;						/* "#define" Åbergehen */

			if((dimension == 'W') || (dimension == 'w'))
			{
				help = strrchr(strvalue, 'x');
				if(help != NULL && *(help + 1) != '\0')
				{
					help++;
					width = (hexTable[*help++] << 12) + (hexTable[*help++] << 8) +
							(hexTable[*help++] << 4) + hexTable[*help];
				}
				else
					width = atoi(strvalue);
			}
			else
				if((dimension == 'H') || (dimension == 'h'))
				{
					help = strrchr(strvalue, 'x');
					if(help != NULL && *(help + 1) != '\0')
					{
						help++;
						height = (hexTable[*help++] << 12) + (hexTable[*help++] << 8) +
								 (hexTable[*help++] << 4) + hexTable[*help];
					}
					else
						width = atoi(strvalue);
				}
				else
					if(strnicmp(name_and_type, "DATASIZE", 8) == 0)
					{
						help = strrchr(strvalue, 'x');
						if(help != NULL && *(help + 1) != '\0')
						{
							help++;
							Datasize = (hexTable[*help++] << 12) + (hexTable[*help++] << 8) +
									   (hexTable[*help++] << 4) + hexTable[*help];
						}
						else
							width = atoi(strvalue);
					}
		} while(++i < 3 && (width == 0 || height == 0 || Datasize == 0));

		if(i == 3 && (width == 0 || height == 0 || Datasize == 0))
			return(M_PICERR);

		w = (width + 15) / 16;

		BitsPerPixel = (char)(Datasize / (w * (unsigned long)height));

		Planes = BitsPerPixel;
		if(Planes != 1 && Planes != 2 && Planes != 4 && Planes != 8)
			return(M_PICERR);

		strncpy(smurf_struct->smurf_pic->format_name, "GEM Icon .ICN", 21);
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		strcpy(impmessag, "GEM Icon ");
		strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
		strcat(impmessag, " Bit");
		smurf_struct->services->reset_busybox(128, impmessag);

		if((ziel = SMalloc(((w * 16) * (long)height * BitsPerPixel) >> 3)) == 0)
			return(M_MEMORY);
		else
		{
			oziel = ziel;

			helpstr = strchr(helpstr, '{');

			length = (unsigned long)w * (unsigned long)height * Planes;
			while(length--)
			{
				while(*helpstr != end && *helpstr != 'x' && *helpstr != 'X')
					helpstr++;
				if(*helpstr++ == end)
					break;
				
				*ziel++ = (hexTable[*helpstr++] << 12) + (hexTable[*helpstr++] << 8) +
						  (hexTable[*helpstr++] << 4) + hexTable[*helpstr++];
			}

			buffer = obuffer;
			ziel = oziel;

			pal = smurf_struct->smurf_pic->palette;
			switch(BitsPerPixel)
			{
				case 1:	for(i = 0; i < 6; i++)
							*pal++ = stdpal1[i];
						break;
				case 2:	for(i = 0; i < 12; i++)
							*pal++ = stdpal2[i];
						break;
				case 4:	for(i = 0; i < 48; i++)
							*pal++ = stdpal4[i];
						break;
				case 8:	for(i = 0; i < 768; i++)
							*pal++ = stdpal8[i];
						break;
			}

			smurf_struct->smurf_pic->pic_data = ziel;
			smurf_struct->smurf_pic->format_type = FORM_STANDARD;
		} /* Malloc */
	} /* Erkennung */

	SMfree(buffer);

	return(M_PICDONE);
}
 
 
/* --- STRSRCHR --- */
/* Diese Funktion sucht von hinten her nach dem ersten Vorkommen des */
/* Åbergebenen Zeichen und gibt die Position oder -1 zurÅck */

int strsrchr(char *s, char c)
{
	int i;


	for(i = (int)strlen(s) - 1; i >= 0 && s[i] != c; i--)
		;

	return(i);
} /* strsrchr */


/* --- STRSRCHL --- */
/* Diese Funktion sucht von vorne her nach dem ersten Vorkommen des */
/* Åbergebenen Zeichen und gibt die Position oder -1 zurÅck */

int strsrchl(char *s, char c)
{
	int i, len;


	len = (int)strlen(s);

	for(i = 0; i < len && s[i] != c; i++)
		;

	if(i == len)
		i = -1;
	return(i);
} /* strsrchl */


char *fileext(char *filename)
{
	char *extstart;


	if((extstart = strrchr(filename, '.')) != NULL)
		extstart++;
	else
		extstart = strrchr(filename, '\0');
	
	return(extstart);
} /* fileext */