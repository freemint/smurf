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
		Berechnet das 3D Bild
*/


#include <stdio.h>
#include <portab.h>
#include <tos.h>
#include <aes.h>

#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

extern long get_point2(void *b ,unsigned int mx,int x,int y,void *xy);
extern long get_point2_fak(void *b ,unsigned int mx,int x,int y,void *xy);
extern long get_point4(void *b ,unsigned int mx,int x,int y,void *xy);
extern long get_point16(void *b ,unsigned int mx,int x,int y,void *xy);
extern long get_point256(void *b ,unsigned int mx,int x,int y,void *xy);
extern long get_point256PP(void *b ,unsigned int mx,int x,int y,void *xy);
extern long get_pointTC16(void *b ,unsigned int mx,int x,int y,void *xy);
extern long get_pointTC24(void *b ,unsigned int mx,int x,int y,void *xy);
extern long (*get_point)(void *b ,unsigned int mx,int x,int y,void *xy);

extern void fcopypoint2(void *a,void *b);
extern void fcopypoint4(void *a,void *b);
extern void fcopypoint16(void *a,void *b);
extern void fcopypoint256(void *a,void *b);
extern void fcopypoint256PP(void *a,void *b);
extern void fcopypointTC16(void *a,void *b);
extern void fcopypointTC24(void *a,void *b);
extern void	(*fcopypoint)(void *a,void *b);

extern void (*reset_busybox)(int lft, char *string);
extern int (*busybox)(int lft);

struct fcpypnt
{
	char *memaddr;
	unsigned int  x;
	long xy;
} fsource, fdestin;

void	*hintergrundspeicher, *bild3dspeicher, *ebenenspeicher;
unsigned int RX, RY, HX, HY, EX, EY;

/*int errno;
long _FilSysV;*/

long busystart, busyfak;
void update(void)
{
	busystart+=busyfak;
	busybox(busystart>>8);
}

/*
	sposch - schieben (0) oder spiegeln (!=0)?
	ART - Berechnungmodus 1-10
	SPALTE - Spaltenbreite
*/
int berechne_no_inter(int sposch,int ART,int SPALTE,
		SMURF_PIC *picture,SMURF_PIC *text_pic,SMURF_PIC *deep_pic)
{
	int x2, y3, yyy;
	int x, y, rx2;
	int *faks, *f2;
	unsigned long xfak, yfak;
	int x2h, x2r, x2e;
	unsigned long xyh, xyr, xye;

	/* FTAB kopieren */

	HX=text_pic->pic_width;
	HY=text_pic->pic_height;
	hintergrundspeicher=text_pic->pic_data;

	RX=picture->pic_width;
	RY=picture->pic_height;
	bild3dspeicher=picture->pic_data;
	/* evtl freigeben und neune anfordern */

	EX=deep_pic->pic_width;
	EY=deep_pic->pic_height;
	ebenenspeicher=deep_pic->pic_data;

/*	x2h=(HX+7)/8;
	if( rechenfarbebene==16 ) x2h=HX*2;
	if( rechenfarbebene==24 ) x2h=HX*3;*/
	x2h=HX*3;
	xyh=(unsigned long)x2h*HY;

/*	x2r=(RX+7)/8;
	if( rechenfarbebene==16 ) x2r=RX*2;
	if( rechenfarbebene==24 ) x2r=RX*3;*/
	x2r=RX*3;
	xyr=(unsigned long)x2r*RY;

	
/*	x2e=(EX+7)/8;
	if( ebenenfarbe==255 ) x2e=EX;
	if( ebenenfarbebene==16 ) x2e=EX*2;
	if( ebenenfarbebene==24 ) x2e=EX*3;*/
	x2e=EX;
	xye=(unsigned long)x2e*EY;

/*	switch(rechenfarbe){
		case 2:
			fcopypoint = fcopypoint2;
			break;	
		case 4:
			fcopypoint = fcopypoint4;
			break;	
		case 16:
			fcopypoint = fcopypoint16;
			break;	
		case 255:
			fcopypoint = fcopypoint256PP;
			break;	
		case 256:
			fcopypoint = fcopypoint256;
			break;	
		case 16000:
			fcopypoint = fcopypointTC16;
			break;	
		case 24000:
			fcopypoint = fcopypointTC24;
			break;	
	}
	switch(ebenenfarbe){
		case 2:
			get_point = get_point2_fak;
			break;
		case 4:
			get_point = get_point4;
			break;
		case 16:
			get_point = get_point16;
			break;
		case 255:
			get_point = get_point256PP;
			break;
		case 256:
			get_point = get_point256;
			break;
		case 16000:
			get_point = get_pointTC16;
			break;
		case 24000:
			get_point = get_pointTC24;
			break;
	}*/

			fcopypoint = fcopypointTC24;
			get_point = get_point256PP;

	fdestin.memaddr = bild3dspeicher;
	fdestin.xy = xyr;

	fsource.memaddr = hintergrundspeicher;
	fsource.xy = xyh;

	yfak = ((unsigned long) EY<<10)/RY;
	xfak = ((unsigned long) EX<<10)/(RX-SPALTE);

	faks = Malloc((long) RX*2 );
	if( faks==0 ) return(-1);
	f2=faks;
	rx2=RX-1;
	for( x=0; x<RX-SPALTE; x++){
		*f2++=(int) ((x*xfak)>>10);
	}

	if(ART==2 || ART==4 || ART==7 || ART==8){
		busyfak=(128<<8)/(RY>>3);/* doppelt so viel */
	}else{
		busyfak=(128<<8)/(RY>>4);
	}

	if( busyfak==0) busyfak=1;
	busystart=0;
	reset_busybox(busystart,"magisch aktiv...");

/*	{
		FILE *fh;
		fh=fopen("c:\\bug.bug","a");
		if(fh){
			fprintf(fh,"Sp %d Art %d SP %d\n",sposch,ART,SPALTE);
			fprintf(fh,"Tex %d %d %d %ld %ld\n",HX,HY,x2h,xyh,hintergrundspeicher);
			fprintf(fh,"Hoe %d %d %d %ld %ld\n",EX,EY,x2e,xye,ebenenspeicher);
			fprintf(fh,"b3d %d %d %d %ld %ld\n",RX,RY,x2r,xyr,bild3dspeicher);
		
			fclose(fh);
		}
	}*/

	yyy=1;
	switch( ART ){
		case 3 :{/*		Vorw„rts >>>>	*/
			for( y=0; y<RY; y++){
				for( x=0; x<SPALTE; x++){
					fdestin.x = x;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;

			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=faks;
				for( x=0; x<RX-SPALTE; x++){

					x2 = x+(int)(*get_point)( ebenenspeicher , x2e, *f2++, y3,(void *) xye);
					fdestin.x = x+SPALTE;

					fsource.x = x2;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 1 :{/*		Rckw„rts		<<<<	*/
			for( y=0; y<RY; y++){
				for( x=0; x<SPALTE; x++){
					fdestin.x = RX-SPALTE+x;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;

			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[rx2-SPALTE];
				for( x=rx2-SPALTE; x>=0; x--){

					x2 = SPALTE+x-(int)(*get_point)( ebenenspeicher, x2e, *f2--, y3,(void *) xye);

					fdestin.x = x;
					fsource.x = x2;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 2 :{/*		Zentral		<<|>	*/
			int mitx;
			mitx = RX/2;
			x=mitx;
		/*		Hintergrundstreifen	*/
			for( y=0; y<RY; y++){
				for( x=mitx; x<=rx2; x++){
					fdestin.x = x;
					fsource.x = (x-mitx)%SPALTE;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;
		/*		<<<<	*/
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[mitx-1];
				for( x=mitx-1; x>=0; x--){

					x2 = SPALTE+x-(int)(*get_point)( ebenenspeicher, x2e, *f2--, y3,(void *) xye);

					fdestin.x = x;
					fsource.x = x2;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}

		/*		>>>>	Achtung, anders als oben !!!!!!!!!!!	*/
			fsource.memaddr = bild3dspeicher;
			fdestin.memaddr = bild3dspeicher;
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[mitx];
				for( x=mitx; x<=rx2-SPALTE; x++){
					x2 = x+SPALTE-(int)(*get_point)( ebenenspeicher, x2e, *f2++, y3,(void *) xye);
					fdestin.x = x2;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 4 :{/*		Zentral		<|>>	*/
			int mitx=RX/2;
		/*		Hintergrundstreifen	*/
			for( y=0; y<RY; y++){
				for( x=0; x<=mitx; x++){
					fdestin.x = x;
					fsource.x = x%SPALTE;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;
		/*		>>>>	*/
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[mitx-SPALTE];
				for( x=mitx; x<=rx2; x++){

					x2 = x-SPALTE+(int)(*get_point)( ebenenspeicher, x2e, *f2++, y3,(void *) xye);

					fdestin.x = x;
					fsource.x = x2;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}

		/*		<<<<	Achtung, anders als oben !!!!!!!!!!!	*/
			fsource.memaddr = bild3dspeicher;
			fdestin.memaddr = bild3dspeicher;
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[mitx-1-SPALTE];
				for( x=mitx-1; x>=SPALTE; x--){
					x2 = x-SPALTE+(int)(*get_point)( ebenenspeicher, x2e, *f2--, y3,(void *) xye);
					fdestin.x = x2;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 5 :{/*		|>>>	*/
		/*		Hintergrundstreifen	*/
			for( y=0; y<RY; y++){
				for( x=0; x<=rx2; x++){
					fdestin.x = x;
					fsource.x = x%SPALTE;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;
			
		/*		<<<<	Achtung, anders als oben !!!!!!!!!!!	*/
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[0];
				for( x=0; x<=rx2-SPALTE; x++){
					x2 = x+SPALTE-(int)(*get_point)( ebenenspeicher, x2e, *f2++, y3,(void *) xye);
					fdestin.x = x2;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 6 :{/*		<<<|	*/
		/*		Hintergrundstreifen	*/
			for( y=0; y<RY; y++){
				for( x=0; x<=rx2; x++){
					fdestin.x = x;
					fsource.x = x%SPALTE;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;
		/*		<<<<	Achtung, anders als oben !!!!!!!!!!!	*/
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[rx2-SPALTE];
				for( x=rx2; x>=SPALTE; x--){
					x2 = x-SPALTE+(int)(*get_point)( ebenenspeicher, x2e, *f2--, y3,(void *) xye);
					fdestin.x = x2;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 7 :{/*		Zentral		<<O>	*/
			int mitx, x3, i;
			mitx = RX/2;
			x=mitx;
		/*		Hintergrundstreifen	*/
			for( y=0; y<RY; y++){
				x3=mitx;
				for( x=0; x<SPALTE; x++){
					fdestin.x = x3++;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;
		/*		<<<<	*/
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[mitx-1];
				for( x=mitx-1; x>=0; x--){

					x2 = SPALTE+x-(int)(*get_point)( ebenenspeicher, x2e, *f2--, y3,(void *) xye);

					fdestin.x = x;
					fsource.x = x2;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}

		/*		>>>>	Achtung, anders als oben !!!!!!!!!!!	*/
			fsource.memaddr = bild3dspeicher;
			fdestin.memaddr = bild3dspeicher;
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[mitx];
				x3=mitx+SPALTE;
				x2=x3;
				for( x=mitx; x<=rx2-SPALTE; x++){
					x2 = x+SPALTE-(int)(*get_point)( ebenenspeicher, x2e, *f2++, y3,(void *) xye);
					fsource.x = x;
					if( x2-1>x3 ){
						for( i=x3+1; i<=x2; i++){
							fdestin.x = i;
							(*fcopypoint)( &fsource, &fdestin );
						}
					}else{
						fdestin.x = x2;
						(*fcopypoint)( &fsource, &fdestin );
					}
					x3=x2;
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 8 :{/*		Zentral		<O>>	*/
			int mitx=RX/2, i, x3;
		/*		Hintergrundstreifen	*/
			for( y=0; y<RY; y++){
				x3=mitx-SPALTE;
				for( x=0; x<SPALTE; x++){
					fdestin.x = x3++;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;
		/*		>>>>	*/
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[mitx-SPALTE];
				for( x=mitx; x<=rx2; x++){

					x2 = x-SPALTE+(int)(*get_point)( ebenenspeicher, x2e, *f2++, y3,(void *) xye);

					fdestin.x = x;
					fsource.x = x2;
					(*fcopypoint)( &fsource, &fdestin );
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}

		/*		<<<<	Achtung, anders als oben !!!!!!!!!!!	*/
			fsource.memaddr = bild3dspeicher;
			fdestin.memaddr = bild3dspeicher;
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[mitx-1-SPALTE];
				x3=0;
				for( x=mitx-1; x>=SPALTE; x--){
					x2 = x-SPALTE+(int)(*get_point)( ebenenspeicher, x2e, *f2--, y3,(void *) xye);
					fsource.x = x;
					if( x2+1<x3 ){
						for( i=x3-1; i>=x2; i--){
							fdestin.x = i;
							(*fcopypoint)( &fsource, &fdestin );
						}
					}else{
						fdestin.x = x2;
						(*fcopypoint)( &fsource, &fdestin );
					}
					x3=x2;
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 9 :{/*		O>>>	*/
			int i, x3;
		/*		Hintergrundstreifen	*/
			for( y=0; y<RY; y++){
				for( x=0; x<SPALTE; x++){
					fdestin.x = x;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;
			
		/*		<<<<	Achtung, anders als oben !!!!!!!!!!!	*/
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[0];
				x3=SPALTE;
				for( x=0; x<RX-SPALTE; x++){
					x2 = x+SPALTE-(int)(*get_point)( ebenenspeicher, x2e, *f2++, y3,(void *) xye);
					fsource.x = x;
					if( x2-1>x3 ){
						for( i=x3+1; i<=x2; i++){
							fdestin.x = i;
							(*fcopypoint)( &fsource, &fdestin );
						}
					}else{
						fdestin.x = x2;
						(*fcopypoint)( &fsource, &fdestin );
					}
					x3=x2;
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
		case 10 :{/*		<<<O	*/
			int i, x3;
		/*		Hintergrundstreifen	*/
			for( y=0; y<RY; y++){
				x3=RX-SPALTE;
				for( x=0; x<SPALTE; x++){
					fdestin.x = x3++;
					fsource.x = x;
					(*fcopypoint)( &fsource, &fdestin );
				}
				if( yyy==HY ){
					if( sposch ){
						yyy=1;
						x2h=-x2h;
					}else{
						yyy=0;
						fsource.memaddr=(char *)hintergrundspeicher-x2h;
					}
				}
				yyy++;
				fsource.memaddr += x2h;
				fdestin.memaddr += x2r;
			}		
			fsource.memaddr = bild3dspeicher;
			fsource.xy = xyr;
			fdestin.memaddr = bild3dspeicher;
		/*		<<<<	Achtung, anders als oben !!!!!!!!!!!	*/
			for( y=0; y<RY; y++){
				y3=(int) ((y*yfak)>>10);
				f2=&faks[rx2-SPALTE];
				x3=RX-SPALTE;
				x2=x3;
				for( x=rx2; x>=SPALTE; x--){
					x2 = x-SPALTE+(int)(*get_point)( ebenenspeicher, x2e, *f2--, y3,(void *) xye);
					fsource.x = x;
					if( x2+1<x3 ){
						for( i=x3-1; i>=x2; i--){
							fdestin.x = i;
							(*fcopypoint)( &fsource, &fdestin );
						}
					}else{
						fdestin.x = x2;
						(*fcopypoint)( &fsource, &fdestin );
					}
					x3=x2;
				}
				fsource.memaddr += x2r;
				fdestin.memaddr += x2r;
				if((y&0xF)==0) update();
			}
			break;
		}
	}
	Mfree( faks);
	return(0);
}