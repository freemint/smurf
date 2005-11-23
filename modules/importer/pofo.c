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
 * The Original Code is from Gerhard Stoll.
 *
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* =================================================================*/
/*	PGF files Importer																							*/
/*																																	*/
/* Version 0.4  --  26.04.2003         															*/
/*                  Umstellung auf Smurf Malloc/Free								*/
/* Version 0.3  --  05.04.2003																			*/
/*                  Kann nun auch PGX-Dateien lesen. Es wird nur das*/
/*									erste Bild geladen.															*/ 
/* Version 0.2  --  01.04.2003																			*/
/* Version 0.1  --  29.03.2003																			*/
/*																																	*/
/* =================================================================*/

#include <tos.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*f_alert)(char *alertstring, char *b1, char *b2, char *b3, char defbt);

static char *fileext(char *filename);


/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"Portfolio PGF",
						0x0030,
						"Gerhard Stoll",
						"PGF", "PGC", "PGX", "", "",
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
/*		PGF files Importer														*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main ( GARGAMEL *smurf_struct)
{
	char *buffer, *pal, *fname, *ziel;
	char index, data, BitsPerPixel;

	int i, k, dest;
	unsigned int width, height;
	long file_len;

	f_alert = smurf_struct->services->f_alert;

	fname = smurf_struct->smurf_pic->filename;

	if ( stricmp(fileext(fname), "PGF") != 0 && 
	     stricmp(fileext(fname), "PGC") != 0 &&
	     stricmp(fileext(fname), "PGX") != 0 )
		return(M_INVALID);

	file_len = smurf_struct->smurf_pic->file_len;

	if(stricmp(fileext(fname), "PGF") == 0 && file_len != 1920L)
	{
		f_alert("Falsche Gr”že (!=1920) fr ein PGF)", "OK", NULL, NULL, 1);
		return(M_PICERR);
	}

	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;
	
	buffer = smurf_struct->smurf_pic->pic_data;

	width = 240;
	height = 64;

	BitsPerPixel = 1;

	strncpy(smurf_struct->smurf_pic->format_name, "Portfolio Graphics",18);
	smurf_struct->smurf_pic->pic_width = width;
	smurf_struct->smurf_pic->pic_height = height;
	smurf_struct->smurf_pic->depth = BitsPerPixel;

	pal = smurf_struct->smurf_pic->palette;

	pal[0] = 255;
	pal[1] = 255;
	pal[2] = 255;
	pal[3] = 0;
	pal[4] = 0;
	pal[5] = 0;

	if ( stricmp(fileext(fname), "PGC") == 0 )
	{
		if ( buffer[0] != 'P' || buffer[1] != 'G' || buffer[2] != 1)
		{
			f_alert("Dies ist keine Portfolio PGC-Datei", "OK", NULL, NULL, 1);
			return(M_PICERR);
		}
		strncpy(smurf_struct->smurf_pic->format_name, "Portfolio Graphics Compressed",29);

		ziel = (char *)SMalloc( 1920L );
		memset(ziel, 0x0, 1920L );
	
		i = 0;	
		dest = 0;
		do 
		{
			index = buffer[i+3];
			if ( (index & 0x80 ) != 0 )			/* High Bit ist gesetzt						*/
			{																/* Gleiche Bytefolge							*/
				index -= 0x80;								/* Anzahl der Bytes 							*/
				i++;
				data = buffer[i+3];
				for ( k = 0; k < index; k++ )
					ziel [dest++] = data;
				i++;
			}
			else
			{																/* Unterschiedliche Bytefolge			*/
				i++;
				for ( k = 0; k < index; k++, i++ )
					ziel [dest++] = buffer[i+3];
			} 
		}while (dest < 1920 );
		SMfree(buffer);

		smurf_struct->smurf_pic->pic_data = ziel;
	}

	if ( stricmp(fileext(fname), "PGX") == 0 )
	{
		if ( buffer[0] != 'P' || buffer[1] != 'G' || buffer[2] != 'X')
		{
			f_alert("Dies ist keine Portfolio PGX-Datei", "OK", NULL, NULL, 1);
			return(M_PICERR);
		}
		strncpy(smurf_struct->smurf_pic->format_name, "Portfolio Graphics, PGX",23);

		i = 8;														/* Start der Daten								*/
		do
		{
			if ( buffer [i] == 0xff )				/* Ende erreicht?									*/
			{
				f_alert("Diese Portfolio PGX-Datei enth„lt keine|PGC Grafik.", "OK", NULL, NULL, 1);
				return(M_PICERR);
			}
			if ( buffer [i] == 0x00 )				/* PGC-Datei											*/
			{
				ziel = (char *)SMalloc( 1920L );
				memset(ziel, 0x0, 1920L );
			
				i += 8;	
				dest = 0;
				do 
				{
					index = buffer[i];
					if ( (index & 0x80 ) != 0 )	/* High Bit ist gesetzt						*/
					{														/* Gleiche Bytefolge							*/
						index -= 0x80;						/* Anzahl der Bytes 							*/
						i++;
						data = buffer[i];
						for ( k = 0; k < index; k++ )
							ziel [dest++] = data;
						i++;
					}
					else
					{														/* Unterschiedliche Bytefolge			*/
						i++;
						for ( k = 0; k < index; k++, i++ )
							ziel [dest++] = buffer[i];
					} 
				}while (dest < 1920 );
				SMfree(buffer);
		
				smurf_struct->smurf_pic->pic_data = ziel;
				break;
			}
			i += 8 + buffer[i+1] + (buffer[i+2]  * 256);
		}while ( i < file_len );
	}

	smurf_struct->smurf_pic->format_type = FORM_STANDARD;

	return M_PICDONE;
}

/*---------------------------------------------------------------------*/

char *fileext(char *filename)
{
	char *extstart;


	if((extstart = strrchr(filename, '.')) != NULL)
		extstart++;
	else
		extstart = strrchr(filename, '\0');
	
	return(extstart);
} /* fileext */