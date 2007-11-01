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
 * The Initial Developer of the Original Code is
 * Bjoern Spruck
 *
 * This code is a module for the programm SMURF developed by
 * Olaf Piesche, Christian Eyrich, Dale Russell and Joerg Dittmer
 *         
 * Contributor(s):
 *         
 *
 * ***** END LICENSE BLOCK *****
 */

/*
		Smurf MagicPicture
*/

#ifndef GERMAN
#ifndef ENGLISH
#ifndef FRENCH
#error "keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portab.h>
#include <aes.h>
#include <vdi.h>

#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

#ifdef GERMAN
#include "ger\magic.rsh"
#include "ger\magic.rh"
#define fehler1 "Die Streifenbreite ist gr”žer als die Texturbreite. Bitte Textur vergr”žern oder Spaltenbreite kleiner w„hlen."
#define fehler2 "Die Streifenbreite zu klein."
#define zielbild_str "Zielbild"
#define textur_str "Textur"
#define hoehen_str "H”henbild"
#endif

#ifdef ENGLISH
#include "eng\magic.rsh"
#include "eng\magic.rh"
#define fehler1 "The ..width ist bigger than the texturewidth. Please increase texturewidth or use a smaller ...width."
#define fehler2 "The ...width is to small."
#define zielbild_str "destination"
#define textur_str "texture"
#define hoehen_str "height"
#endif

#ifdef FRENCH
#include "fra\magic.rsh"
#include "fra\magic.rh"
#define fehler1 "Die Streifenbreite ist gr”žer als die Texturbreite. Bitte Textur vergr”žern oder Spaltenbreite kleiner w„hlen."
#define fehler2 "Die Streifenbreite zu klein."
#define zielbild_str "Zielbild"
#define textur_str "Textur"
#define hoehen_str "H”henbild"
#endif

enum {DEST_PIC=0,TEXT_PIC,DEEP_PIC};

/* Infostruktur fr Hauptmodul */
MOD_INFO	module_info={
"Magic Picure",0x0060,"Bj”rn Spruck",
"","","","","",
"","","","","",
/* Objekttitel */
"","","","",
"","","","",
"Streifen","","","",
/* Min/Max Slider */
0,128,0,128,0,128,0,128,
/* Min/Max Editfelder */
10,1000,0,10,0,10,0,10,
/* Slider-Defaultwerte */
0,0,0,0,
0,0,0,0,
200,0,0,0,
/* Anzahl Bilder */
3,
/* Bildbeschreibung */
zielbild_str,textur_str,hoehen_str
"","",""
};

MOD_ABILITY  module_ability =
{
	24, 16, 8, 0, 0, 0, 0, 0,
	FORM_PIXELPAK, FORM_PIXELPAK,	
	FORM_PIXELPAK, FORM_PIXELPAK,
	FORM_PIXELPAK, FORM_PIXELPAK,
	FORM_PIXELPAK, FORM_PIXELPAK,
	0,
};

SERVICE_FUNCTIONS *service;
int (*busybox)(int lft);
void (*reset_busybox)(int lft, char *string);

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

SMURF_PIC *picture, *text_pic, *deep_pic;

extern int berechne_no_inter(int sposch,int ART,int SPALTE,
		SMURF_PIC *picture,SMURF_PIC *text_pic,SMURF_PIC *deep_pic);
/*
extern int berechne_inter(int sposch,int ART,int SPALTE,
		SMURF_PIC *picture,SMURF_PIC *text_pic,SMURF_PIC *deep_pic);
*/
extern unsigned char fuck_tab[256];

OBJECT *my_tree;
WINDOW my_window;

int get_highest(SMURF_PIC *d)
{
	unsigned char *c=d->pic_data, m=0, *cm;
	cm=c+(long)d->pic_width*d->pic_height;
	
	while(c<cm){
		if(*c>m) m=*c;
		c++;
	}
	return((int)m);
}

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*		MagicPicture (by Sage) 						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	int message = smurf_struct->module_mode;
	int my_id, t, objct;
	int interpol=0, sparen=0;

	service=smurf_struct->services;

	busybox = smurf_struct->services->busybox;
	reset_busybox = smurf_struct->services->reset_busybox;

	switch(message)
	{
		case MSTART:
			my_id = smurf_struct->module_number;

			my_tree = rs_trindex[SM_MAGIC];
			
			for(t=0; t<NUM_OBS; t++)
				rsrc_obfix(&rs_object[t], 0);
			
			/* WINDOW - Struktur vorbereiten
			 */
			my_window.picture = NULL;
			my_window.clipwid = my_window.cliphgt = 0;
			my_window.resource_form = my_tree;
			my_window.module = my_id;
			my_window.wnum = 1;
			strcpy(my_window.wtitle, "Magic Picture");
			my_window.pic_xpos = my_window.pic_ypos = 0;
			my_window.editob = HOEHE_SW;
			my_window.pflag = 0;
			my_window.prev_window = NULL;
			my_window.next_window = NULL;
			my_window.wx = -1;
			my_window.wy = -1;
			my_window.ww = my_tree[0].ob_width;
			my_window.wh = my_tree[0].ob_height;
			smurf_struct->wind_struct = &my_window;
			
			if( service->f_module_window(&my_window) != -1)
				smurf_struct->module_mode = M_WAITING;
			else
				smurf_struct->module_mode = M_MODERR;
			break;

		case MPICS:
			if(my_tree[INTERPOL].ob_state&SELECTED) interpol=1;
			if(my_tree[SPEICHER].ob_state&SELECTED) sparen=1;
			/*
				 intepol 
				 	alles in TC
				 
				 !interpol && sparen
				 	alles (aužer hoehe) belassen wie es ist
				 
				 !interpol && !sparen
				 	wenn hinter<=256 in PP wandeln, sonst lassen
				 
			*/
			switch(smurf_struct->event_par[0]){
				case DEST_PIC:
					/* destpic braucht nicht gewandelt zu werden */
					smurf_struct->module_mode=M_PICTURE;
					break;
				case TEXT_PIC:
					smurf_struct->event_par[0]=24;
					smurf_struct->event_par[1]=FORM_PIXELPAK;
					smurf_struct->event_par[2]=RGB;

/*					if( interpol){
						smurf_struct->event_par[0]=24;
						smurf_struct->event_par[1]=FORM_PIXELPAK;
						smurf_struct->event_par[2]=RGB;
					}else{
/*						if( !sparen && <=8){
							smurf_struct->event_par[0]=8;
							smurf_struct->event_par[1]=FORM_PIXELPAK;
							smurf_struct->event_par[2]=RGB;
						}else{/* else keine aenderung ausser evtl 8Plane->PP*/
							if( ==8 && !=FORM_PIXELPAK){
								smurf_struct->event_par[0]=8;
								smurf_struct->event_par[1]=FORM_PIXELPAK;
								smurf_struct->event_par[2]=RGB;						
							}
						}*/
					}*/
					smurf_struct->module_mode=M_PICTURE;
					break;
				case DEEP_PIC:
					smurf_struct->event_par[0]=8;
					smurf_struct->event_par[1]=FORM_PIXELPAK;
					smurf_struct->event_par[2]=GREY;
					smurf_struct->module_mode=M_PICTURE;
					break;
				default:
					smurf_struct->module_mode=M_WAITING;
					break;
			}
			break;
			
		case MPICTURE:
			switch(smurf_struct->event_par[0]){
				case DEST_PIC:
					picture=smurf_struct->smurf_pic;
					break;
				case TEXT_PIC:
					text_pic=smurf_struct->smurf_pic;
					break;
				case DEEP_PIC:
					deep_pic=smurf_struct->smurf_pic;
					break;
				default:
					smurf_struct->module_mode = M_WAITING;
					break;
			}
			break;
			
		case MEXEC:{
			int ART=0, SPALTE=1, sposch=0;
			int i;

			if(my_tree[RI1].ob_state&SELECTED) ART=1;
			if(my_tree[RI2].ob_state&SELECTED) ART=2;
			if(my_tree[RI3].ob_state&SELECTED) ART=3;
			if(my_tree[RI4].ob_state&SELECTED) ART=4;
			if(my_tree[RI5].ob_state&SELECTED) ART=5;
			if(my_tree[RI6].ob_state&SELECTED) ART=6;
			if(my_tree[RI7].ob_state&SELECTED) ART=7;
			if(my_tree[RI8].ob_state&SELECTED) ART=8;
			if(my_tree[RI9].ob_state&SELECTED) ART=9;
			if(my_tree[RI10].ob_state&SELECTED) ART=10;

			if(my_tree[SPOSCH1].ob_state&SELECTED) sposch=1;
			if(my_tree[SPOSCH2].ob_state&SELECTED) sposch=0;

			if( my_tree[STRF1].ob_state&SELECTED){
				SPALTE=text_pic->pic_width;
			}else{
				SPALTE = atoi( my_tree[ANZSPALT].ob_spec.tedinfo->te_ptext );
				if( SPALTE>text_pic->pic_width){
					service->f_alert(fehler1,"O.K.",0,0,1);
					smurf_struct->module_mode = M_WAITING;
					return;
				}
			}
			if( SPALTE<=5){
				service->f_alert(fehler2,"O.K.",0,0,1);
				smurf_struct->module_mode = M_WAITING;
				return;
			}

			if(my_tree[SHORTEN].ob_state&SELECTED){
				int MAXPRO=30, high;
				long l;
				MAXPRO = atoi( my_tree[HOEHE_SW].ob_spec.tedinfo->te_ptext);
				high=get_highest(deep_pic);

				for(i=0; i<256; i++){
					l=((long)i*SPALTE*MAXPRO)/((long)high*1000);
					if( l>255) l=255;
					fuck_tab[i]=(unsigned char) l;
				}
			}else{
				for(i=0; i<256; i++){
					fuck_tab[i]=(unsigned char) i;
				}
			}

/*			if(interpol){
				if(berechne_inter(sposch,ART,SPALTE,picture,text_pic,deep_pic)){
					smurf_struct->module_mode = M_MEMORY;	
				}else{
					smurf_struct->module_mode = M_PICDONE;
				}
			}else{*/
				if(berechne_no_inter(sposch,ART,SPALTE,picture,text_pic,deep_pic)){
					smurf_struct->module_mode = M_MEMORY;	
				}else{
					smurf_struct->module_mode = M_PICDONE;
				}
/*			}*/
		} break;
			
		case MTERM:
			smurf_struct->module_mode = M_EXIT;
			break;

		case MBEVT:
			objct = smurf_struct->event_par[0];
			if(objct==GO)
				smurf_struct->module_mode = M_STARTED;
			break;
		case MKEVT:
			objct = smurf_struct->event_par[0];
			if(objct==GO)
				smurf_struct->module_mode = M_STARTED;
			break;

	}

	return;
}
