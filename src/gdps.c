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
 * The Initial Developer of the Original Code are
 * Olaf Piesche, Christian Eyrich, Dale Russell and J”rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* -------------------------------------------- */
/* Rudiment„res Interface zu einem GDPS-Treiber */
/* 21.3.99 Christian Eyrich                     */
/* -------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tos.h>
#include <ext.h>
#include "..\sym_gem.h"
#include "..\mod_devl\import.h"
#include "smurfine.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "globdefs.h"
#include "gdps.h"

#include <screen.h>

int send_command(COMSTRUCT *comstruct, int command);
int make_comstruct(COMSTRUCT *comstruct, SMURF_GDPS *smurf_gdps);
void save_pic(COMSTRUCT *comstruct);
int copy_gdpsmemory(SMURF_PIC *smurf_picture, COMSTRUCT *comstruct);

extern SMURF_PIC *smurf_picture[MAX_PIC];
extern SYSTEM_INFO Sys_info;
extern OBJECT *alerts;
extern WINDOW picture_windows[MAX_PIC];
extern DISPLAY_MODES Display_Opt;

extern int orig_red[256];
extern int orig_green[256];
extern int orig_blue[256];
extern char planetable[260];

extern int picthere;

GENERAL_GDPS *actual;
char firstblock;

#define DEBUG	1


void gdps_main(void)
{
	unsigned long *gdpsentry,
				  oldstack;

	SMURF_GDPS smurf_gdps;
	COMSTRUCT comstruct;


	Goto_pos(1,0);

	/* Laut Gerti nimmt der Anfang der GDPS-Treiber-Kette im vom */
	/* System ungenutzen Vektor $41C seinen Anfang. */
	/* Normalerweise soll da 0L stehen, ist ein GDPS-Treiber installiert */
	/* zeigt das erste Long an dieser Adresse auf den Treiber und das */
	/* zweite Long sollte das Magic 'GDPS' ($47445053) enthalten. */

	/* in Supervisormodus schalten */
	if(!Super((void *)1L))
		oldstack = Super(0L);

	/* Zeiger auf Einstiegspunkt der Treiberliste holen */
	gdpsentry = (unsigned long *)*((unsigned int **)0x41cL);

	/* Liste der GDPS-Treiberdurchlaufen und ersten Scannertreiber benutzen */
	actual = (GENERAL_GDPS *)gdpsentry;
	while(actual != 0L && actual->magic == 'GDPS')
	{
		if(actual->type >= 0x0 && actual->type <= 0x0ff)
		{
#if DEBUG
			printf("Magic: %lx\n", actual->magic);
			printf("Treiberversion: %d\n", actual->version);
			printf("Treibertyp: %x\n", actual->type);
			printf("Treiberinfo: %s\n", actual->info);
#endif
			smurf_gdps.version = actual->version;
			strcpy(smurf_gdps.info, actual->info);

			break;
		}
	}

	/* Supervisormodus verlassen */
	if(oldstack)
		Super((void *)oldstack);


	/* bestimmten Scanner benutzen */

	if(!actual)
	{
		f_alert("Kein GDPS-Treiber installiert.", NULL, NULL, NULL, 1);
		goto End;
	}

	/* warten bis der Scanner frei (d.h. free auf 0) ist */
	if(actual->free)
	{
		f_alert("GDPS-Treiber ist momentan nicht frei.", NULL, NULL, NULL, 1);
		goto End;
	}

	/* und dann reservieren */
	actual->free = 0xff00;

	if(!actual->desc)								/* Scanner noch nicht initialisiert? */
		send_command(&comstruct, 0x105);			/* dann tue es jetzt */

#if DEBUG
	printf("Scannerbeschreibung: %x\n", actual->desc);
	printf("Anzahl Farben: %x\n", actual->cols);
	printf("Bittiefen: %x\n", actual->depth);
#endif
	smurf_gdps.desc = actual->desc;
	smurf_gdps.cols = actual->cols;
	smurf_gdps.depth = actual->depth;

	firstblock = 1;
	comstruct.memory = NULL;

	do
	{
		if(make_comstruct(&comstruct, &smurf_gdps) < 0)	/* Fehler aufgetreten, wahrscheinlich nicht genug Speicher */
			break;										/* Schleife verlassen */

		/* Scannvorgang mit Aufruf Benutzerdialog beginnen */
		send_command(&comstruct, 0x100);
		if(comstruct.result < 0xfffd)					/* Fehler beim Scannen? */
		{
#if DEBUG
			printf("Fehler beim Scanvorgang!\n");
#endif
			break;										/* Schleife verlassen */
		}

		save_pic(&comstruct);
	} while(comstruct.result != 0xffff);


End:

	SMfree(comstruct.memory);

	/* Scanner freigeben */
	actual->free = 0;

	return;
}


/* sendet ein Kommando an ein GDPS-Device */
int send_command(COMSTRUCT *comstruct, int command)
{
	/* Adresse der eigenen Kommandostruktur eintragen */
	actual->comstruct = comstruct;

	/* und Kommando eintragen */
	actual->command = command;

	/* warten bis Kommando vom Treiber wieder auf 0 */
	while(actual->command)
		evnt_timer(100, 0);

	return(0);
} /* send_command */


/* fllt die Kommandostruktur aus */
/* voerst nur nach GDPS 1.00 */
int make_comstruct(COMSTRUCT *comstruct, SMURF_GDPS *smurf_gdps)
{
	comstruct->result = 0;						/* Rckgabe initialisieren */

	/* s/w, Dithering, Multivalue, Multivalue in Farbe, */
	/* blockweise Rckgabe erlaubt */
	comstruct->modes = 1 | 2 | 4 | 64 | 512;	/* von Smurf untersttzte Modi ... */
	comstruct->modes &= smurf_gdps->desc;		/* ... und mit den m”glichen vom Treiber gelieferten abgleichen */

	comstruct->depth = 1 | 4 | 16 | 256;		/* 1, 4, 16 und 256 Graustufen (Farben pro Kanal) ... */
	comstruct->depth &= smurf_gdps->depth;		/* ... und mit den m”glichen vom Treiber gelieferten abgleichen */

	if(comstruct->memory == NULL)
	{
		comstruct->memory = SMalloc(262144L);	/* 256 KB anfordern */
		if(comstruct->memory == 0)
			return(-1);
	}

	comstruct->memlen = 262144L;				/* und das auch explizit mitteilen */

	comstruct->width_byte = 0;					/* und den Rest alles auf 0 */
	comstruct->height = 0;
	comstruct->mmwidth = 0;
	comstruct->mmheight = 0;
	comstruct->xdpi = 0;
	comstruct->ydpi = 0;
	comstruct->modulo = 0;
	comstruct->start_x = 0;
	comstruct->start_y = 0;
	comstruct->ser_nr = 0;
	comstruct->add_bits = 0;

	return(0);	
} /* make_comstruct */


/* Klabustert die Daten im Zielspeicher auseinander */
/* und setzt sie zum Bild zusammen */
void save_pic(COMSTRUCT *comstruct)
{
	int width, height, depth;
	char *palette;

	int pic_to_make = 1;
	int tt, index;
	int aligned_width;

	long PicLen;

#if DEBUG
	printf("modes: %x\n", comstruct->modes);
	printf("depth: %x\n", comstruct->depth);
	printf("memlen: %ld\n", comstruct->memlen);
	printf("width_byte: %x\n", comstruct->width_byte);
	printf("height: %x\n", comstruct->height);
	printf("mmwidth: %x\n", comstruct->mmwidth);
	printf("mmheight: %x\n", comstruct->mmheight);
	printf("xdpi: %x\n", comstruct->xdpi);
	printf("ydpi: %x\n", comstruct->ydpi);
	printf("modulo: %x\n", comstruct->modulo);
	printf("start_x: %x\n", comstruct->start_x);
	printf("start_y: %x\n", comstruct->start_y);

	getch();
#endif

	if(comstruct->modes&0x1 || comstruct->modes&0x2)
		width = comstruct->width_byte * 8;				/* Bei Bi-Level und Dithered immer mit 8 Pixel pro Byte */
	else
		width = comstruct->width_byte;
	height = comstruct->height;
	depth = comstruct->depth;

	if(firstblock)
	{
		firstblock = 0;

		/**************/
		#if DEMOVERSION
			if(picthere >= 3)
			{
				f_alert(alerts[PREVIEW_ERR].TextCast, NULL, NULL, NULL, 1);
				return(-1);
			}
		#endif
		/**************/


		/* erstes freies Bild ermitteln */
		while(smurf_picture[pic_to_make] != NULL)
			pic_to_make++;
		if(pic_to_make > MAX_PIC)
		{
			f_alert(alerts[NO_PIC_FREE].TextCast, NULL, NULL, NULL, 1); 
			return;
		}

		smurf_picture[pic_to_make] = (SMURF_PIC *)SMalloc(sizeof(SMURF_PIC));

		if(depth >= 8)
			PicLen = (long)((long)width *(long)height) * (long)depth / 8L;
		else
		{	
			aligned_width = (width + 7) >> 3;
			PicLen = (long)((long)aligned_width * (long)height * (long)depth);
		}

		if((smurf_picture[pic_to_make]->pic_data = SMalloc(PicLen)) == NULL)
		{
			f_alert(alerts[NOMEM_NEWPIC].TextCast, NULL, NULL, NULL, 1);
			SMfree(smurf_picture[pic_to_make]);
			smurf_picture[pic_to_make] = NULL;
			return;
		}
	}

	copy_gdpsmemory(smurf_picture[pic_to_make], comstruct);

	if(comstruct->result == 0xffff)
	{
		smurf_picture[pic_to_make]->palette = malloc(1025);				/* Paletten-Puffer */
		memset(smurf_picture[pic_to_make]->palette, 0x0, 1025);

		make_smurf_pic(pic_to_make, width, height, depth, smurf_picture[pic_to_make]->pic_data);
		make_pic_window(pic_to_make, width, height, "GDPS");

		smurf_picture[pic_to_make]->col_format = RGB;
		strncpy(smurf_picture[pic_to_make]->format_name, "-", 2);
		memset(smurf_picture[pic_to_make]->filename, 0x0, 257);
		strcpy(smurf_picture[pic_to_make]->filename, "C:\\GDPS");
	
		/*
		 * pr„ventiv die Systempalette eintragen SMURF_PIC
		 */
		palette = smurf_picture[pic_to_make]->palette;
		if(depth == 1)
		{
			palette[0] = palette[1] = palette[2] = 255;
			palette[3] = palette[4] = palette[5] = 0;
		}
		else
			for(tt = 0; tt <= Sys_info.Max_col; tt++)
			{
				index = planetable[tt];
				*(palette + 0 + index*3) = (int)((long)(orig_red[tt] * 255L) / 1000L);
				*(palette + 1 + index*3) = (int)((long)(orig_green[tt] * 255L) / 1000L);
				*(palette + 2 + index*3) = (int)((long)(orig_blue[tt] * 255L) / 1000L);
			}

		if(!Sys_info.realtime_dither)
			f_dither(smurf_picture[pic_to_make], &Sys_info, 0, NULL, &Display_Opt);
		f_open_window(&picture_windows[pic_to_make]);
		clip_picw2screen(&picture_windows[pic_to_make]);
		insert_to_picman(pic_to_make);
		f_activate_pic(pic_to_make);

		picthere++;
	}
		
	actualize_ram();	/* wieviel RAM? */
	actualize_menu();	/* Meneintr„ge ENABLEn / DISABLEn */

	return;
} /* save_picture */


int copy_gdpsmemory(SMURF_PIC *smurf_picture, COMSTRUCT *comstruct)
{

	if(comstruct->modes&1 || comstruct->modes&2)
		memcpy(smurf_picture->pic_data, comstruct->memory, comstruct->memlen);
	else
		if(comstruct->depth&256)
			if(comstruct->result == 0xff)
				return(0);

	return(0);
} /* copy_gdpsmemory */