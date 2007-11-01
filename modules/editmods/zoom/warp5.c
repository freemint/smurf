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
		Smurf Warps mir
*/

#ifndef GERMAN
#ifndef ENGLISH
#ifndef FRENCH
#error "keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portab.h>
#include <aes.h>
#include <math.h>

#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

/* Infostruktur fr Hauptmodul */
MOD_INFO	module_info={
"Zoom",0x0080,	"Bj”rn Spruck",
"","","","","",
"","","","","",
/* Objekttitel */

#ifdef GERMAN
"St„rke","","","",
"radial","tangential","negativ","",
#else
#ifdef ENGLISH
"strength","","","",
"radial","tangential","negativ","",
#else
#ifdef FRENCH
"strength","","","",
"radial","tangential","negativ","",
#else
#error "keine Sprache!"
#endif
#endif
#endif

"","","","",
/* Min/Max Slider */
1,100,0,128,0,128,0,128,
/* Min/Max Editfelder */
1,100,0,10,0,10,0,10,
/* Slider-Defaultwerte */
10,0,0,0,
2,3,3,0,
0,0,0,0,
/* Anzahl Bilder */
1,
/* Bildbeschreibung */
"","","",
"","",""
};

MOD_ABILITY  module_ability =
{
	24, 0, 0, 0, 0, 0, 0, 0,
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

int warp_rad_int(SMURF_PIC *bild,int MAXBR,int mx,int my)
{
	unsigned char *bt, *bildto, *bildfr;
	int bb, hh;
	int i, j;
	long bb3;
	long distfak;
	
	bb=bild->pic_width;
	hh=bild->pic_height;
	bildto=bt=Malloc((long)bb*hh*3);
	if( bildto==0) return(0);

	bildfr=bild->pic_data;
	bb3=bb*3;

	{
		int bb1=bb>1, hh1=hh>>1;
		if( mx>=bb1 && my>=hh1) distfak=(long)(128*MAXBR/sqrt((double)mx*mx+(double)my*my));
		if( mx<bb1 && my>=hh1) distfak=(long)(128*MAXBR/sqrt((double)(bb-mx)*(bb-mx)+(double)my*my));
		if( mx>=bb1 && my<hh1) distfak=(long)(128*MAXBR/sqrt((double)mx*mx+(double)(hh-my)*(hh-my)));
		if( mx<bb1 && my<hh1) distfak=(long)(128*MAXBR/sqrt((double)(bb-mx)*(bb-mx)+(double)(hh-my)*(hh-my)));
	}

	for(i=0; i<hh; i++){
		int dy;
		long ddy;
		if((i&0xf)==0) busybox((int)((long)128*i/hh));

		dy=i-my;
		ddy=distfak*dy/MAXBR;

		for(j=0; j<bb; j++){
			int dx, c, k;
			long ddx;
			long nx, ny;
			unsigned int rot, gruen, blau;

			dx=j-mx;
			ddx=distfak*dx/MAXBR;

			rot=0;
			gruen=0;
			blau=0;
			c=0;

			nx=((long)j<<8)-(long)ddx*MAXBR+128;
			ny=((long)i<<8)-(long)ddy*MAXBR+128;
			
			for( k=0; k<MAXBR; k++){
				int nnx, nny;
				nnx=(int)(nx>>8);
				nny=(int)(ny>>8);
				if( nnx>=0 && nnx<bb && nny>=0 && nny<hh){
					unsigned char *f;
					f=bildfr+(long)nnx*3+(long)nny*bb3;
					rot+=*f++;
					gruen+=*f++;
					blau+=*f;
					c++;
				}else break;
				nx+=ddx;
				ny+=ddy;
			}

			rot/=c;
			if(rot>255) rot=255;
			*bt++=(unsigned char)rot;
			gruen/=c;
			if(gruen>255) gruen=255;
			*bt++=(unsigned char)gruen;
			blau/=c;
			if(blau>255) blau=255;
			*bt++=(unsigned char)blau;
		}
	}
	memcpy(bild->pic_data, bildto, (long)bb*hh*3);
	Mfree(bildto);
	return(-1);
}

int warp_tan_int(SMURF_PIC *bild,int MAXBR,int mx,int my)
{
	unsigned char *bt, *bildto, *bildfr;
	int bb, hh;
	int i, j;
	long bb3;
	long distfak;
	
	bb=bild->pic_width;
	hh=bild->pic_height;
	bildto=bt=Malloc((long)bb*hh*3);
	if( bildto==0) return(0);

	bildfr=bild->pic_data;
	bb3=bb*3;

	{
		int bb1=bb>1, hh1=hh>>1;
		if( mx>=bb1 && my>=hh1) distfak=(long)(128*MAXBR/sqrt((double)mx*mx+(double)my*my));
		if( mx<bb1 && my>=hh1) distfak=(long)(128*MAXBR/sqrt((double)(bb-mx)*(bb-mx)+(double)my*my));
		if( mx>=bb1 && my<hh1) distfak=(long)(128*MAXBR/sqrt((double)mx*mx+(double)(hh-my)*(hh-my)));
		if( mx<bb1 && my<hh1) distfak=(long)(128*MAXBR/sqrt((double)(bb-mx)*(bb-mx)+(double)(hh-my)*(hh-my)));
	}

	for(i=0; i<hh; i++){
		int dy;
		long ddy;
		if((i&0xf)==0) busybox((int)((long)128*i/hh));

		dy=i-my;
		ddy=distfak*dy/MAXBR;

		for(j=0; j<bb; j++){
			int dx, c, k;
			long ddx;
			long nx, ny;
			unsigned int rot, gruen, blau;

			dx=j-mx;
			ddx=distfak*dx/MAXBR;

			rot=0;
			gruen=0;
			blau=0;
			c=0;

			nx=((long)j<<8)+(long)ddy*MAXBR+128;
			ny=((long)i<<8)-(long)ddx*MAXBR+128;
			
			for( k=0; k<MAXBR; k++){
				int nnx, nny;
				nnx=(int)(nx>>8);
				nny=(int)(ny>>8);
				if( nnx>=0 && nnx<bb && nny>=0 && nny<hh){
					unsigned char *f;
					f=bildfr+(long)nnx*3+(long)nny*bb3;
					rot+=*f++;
					gruen+=*f++;
					blau+=*f;
					c++;
				}
				nx-=ddy;
				ny+=ddx;
			}

			rot/=c;
			if(rot>255) rot=255;
			*bt++=(unsigned char)rot;
			gruen/=c;
			if(gruen>255) gruen=255;
			*bt++=(unsigned char)gruen;
			blau/=c;
			if(blau>255) blau=255;
			*bt++=(unsigned char)blau;
		}
	}
	memcpy(bild->pic_data, bildto, (long)bb*hh*3);
	Mfree(bildto);
	return(-1);
}

int warp_neg_int(SMURF_PIC *bild,int MAXBR,int mx,int my)
{
	unsigned char *bt, *bildto, *bildfr;
	int bb, hh;
	int i, j;
	long bb3;
	long distfak;
	
	bb=bild->pic_width;
	hh=bild->pic_height;
	bildto=bt=Malloc((long)bb*hh*3);
	if( bildto==0) return(0);

	bildfr=bild->pic_data;
	bb3=bb*3;

	{
		int bb1=bb>1, hh1=hh>>1;
		if( mx>=bb1 && my>=hh1) distfak=(long)(128*MAXBR/sqrt((double)mx*mx+(double)my*my));
		if( mx<bb1 && my>=hh1) distfak=(long)(128*MAXBR/sqrt((double)(bb-mx)*(bb-mx)+(double)my*my));
		if( mx>=bb1 && my<hh1) distfak=(long)(128*MAXBR/sqrt((double)mx*mx+(double)(hh-my)*(hh-my)));
		if( mx<bb1 && my<hh1) distfak=(long)(128*MAXBR/sqrt((double)(bb-mx)*(bb-mx)+(double)(hh-my)*(hh-my)));
	}

	for(i=0; i<hh; i++){
		int dy;
		long ddy;
		if((i&0xf)==0) busybox((int)((long)128*i/hh));

		dy=i-my;
		ddy=distfak*dy/MAXBR;

		for(j=0; j<bb; j++){
			int dx, c, k;
			long ddx;
			long nx, ny;
			unsigned int rot, gruen, blau;

			dx=j-mx;
			ddx=distfak*dx/MAXBR;

			rot=0;
			gruen=0;
			blau=0;
			c=0;

			nx=((long)j<<8)-(long)ddy*MAXBR+128;
			ny=((long)i<<8)-(long)ddx*MAXBR+128;
			
			for( k=0; k<MAXBR; k++){
				int nnx, nny;
				nnx=(int)(nx>>8);
				nny=(int)(ny>>8);
				if( nnx>=0 && nnx<bb && nny>=0 && nny<hh){
					unsigned char *f;
					f=bildfr+(long)nnx*3+(long)nny*bb3;
					rot+=*f++;
					gruen+=*f++;
					blau+=*f;
					c++;
				}
				nx+=ddy;
				ny+=ddx;
			}

			rot/=c;
			if(rot>255) rot=255;
			*bt++=(unsigned char)rot;
			gruen/=c;
			if(gruen>255) gruen=255;
			*bt++=(unsigned char)gruen;
			blau/=c;
			if(blau>255) blau=255;
			*bt++=(unsigned char)blau;
		}
	}
	memcpy(bild->pic_data, bildto, (long)bb*hh*3);
	Mfree(bildto);
	return(-1);
}

OBJECT *my_tree;
WINDOW my_window;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*		Warps mir (by Sage) 						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
	int message = smurf_struct->module_mode;
	int my_id;
	static int crhx, crhy;

	service=smurf_struct->services;

	busybox = smurf_struct->services->busybox;
	reset_busybox = smurf_struct->services->reset_busybox;

	switch(message)
	{
		case MSTART:
			my_id = smurf_struct->module_number;
			smurf_struct->services->f_module_prefs(&module_info, my_id);
			smurf_struct->module_mode = M_WAITING;
			break;

		case MCROSSHAIR:
			smurf_struct->event_par[0]=1;
			smurf_struct->event_par[1]=0;
			smurf_struct->module_mode=M_CROSSHAIR;
			break;

		case MCH_DEFCOO:/* Smurf fragt: und die Default-Koordinaten?*/
			crhx = smurf_struct->smurf_pic->pic_width>>1;
			crhy = smurf_struct->smurf_pic->pic_height>>1;
			smurf_struct->event_par[0]=crhx;
			smurf_struct->event_par[1]=crhy;
			smurf_struct->module_mode = M_CHDEFCOO;
			break;

		case MCH_COORDS:/* Smurf sagt: hier sind die eingestellten Fadenkreuzkoordinaten */
			crhx = smurf_struct->event_par[0];
			crhy = smurf_struct->event_par[1];
			smurf_struct->module_mode = M_WAITING;
			break;

		case MEXEC:

			reset_busybox(0,"Zoo...ooM");
			if( smurf_struct->check1&1){
				if(warp_rad_int(smurf_struct->smurf_pic,(int)smurf_struct->slide1,crhx,crhy)==0){
					smurf_struct->module_mode = M_MEMORY;
				}else{
					smurf_struct->module_mode = M_PICDONE;
				}
			}
			if( smurf_struct->check2&1){
				if(warp_tan_int(smurf_struct->smurf_pic,(int)smurf_struct->slide1,crhx,crhy)==0){
					smurf_struct->module_mode = M_MEMORY;
				}else{
					smurf_struct->module_mode = M_PICDONE;
				}
			}
			if( smurf_struct->check3&1){
				if(warp_neg_int(smurf_struct->smurf_pic,(int)smurf_struct->slide1,crhx,crhy)==0){
					smurf_struct->module_mode = M_MEMORY;
				}else{
					smurf_struct->module_mode = M_PICDONE;
				}
			}
			break;

		case MTERM:
			smurf_struct->module_mode = M_EXIT;
			break;
	}

	return;
}
