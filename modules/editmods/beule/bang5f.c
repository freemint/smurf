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
		Smurf Ausbeulen (FPU)
*/

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portab.h>
#include <aes.h>
#include <math.h>

#include "..\..\import.h"#include "..\..\..\src\smurfine.h"
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

extern int (*busybox)(int lft);

int bang_inter_FPU(SMURF_PIC *bild,int mx,int my,int HOEHE,int RADIUS,int RANDS)
{
	unsigned char *bt, *bf, *bildto;
	int bb, hh;
	int i, j;
	long bb3;
	double rad, hrad, mrad;
	int sx=0, sy=0, ex=bild->pic_width, ey=bild->pic_height;
	double bfak;
	
	if( mx-RADIUS>0) sx=mx-RADIUS;
	if( my-RADIUS>0) sy=my-RADIUS;
	if( mx+RADIUS+1<ex) ex=mx+RADIUS+1;
	if( my+RADIUS+1<ey) ey=my+RADIUS+1;
	
	bb=bild->pic_width;
	hh=bild->pic_height;
	bildto=bt=Malloc((long)(ex-sx)*(ey-sy)*3);
	if( bildto==0) return(0);

	bf=bild->pic_data;
	
	bb3=bb*3;
	
	rad=(double)RADIUS;
	hrad=(double)HOEHE/RADIUS;
	mrad=(double)M_PI/RADIUS;
	
	bfak=(double)128/(ey-sy);

	if( RANDS){
	for(i=sy; i<ey; i++){
		double yd;
		int yd2;

		if((i&0xf)==0) busybox((int)(i-sy)*bfak);

		yd2=(my-i);
		yd=(double)yd2*yd2;
		for(j=sx; j<ex; j++){
			double mm2;
			int xd2;

			xd2=(mx-j);
			mm2=sqrt((double)xd2*xd2+yd);
			if(mm2>rad){
				unsigned char *x;
				x=bf+(long)j*3+(long)i*bb3;
				*bt++=*x++;
				*bt++=*x++;
				*bt++=*x;
			}else{
				double xx,yy;
				double bh;
				bh=sin(mm2*mrad)*hrad;
				xx=j+xd2*bh-0.5;
				yy=i+yd2*bh-0.5;
				if(xx<0 || yy<0 || xx>=bb-1 || yy>=hh-1){
					*bt++=0;
					*bt++=0;
					*bt++=0;
				}else{
					unsigned char *x;
					double rot, gruen, blau;
					double xx1, yy1, xx2, yy2, xy;

					xx1=fabs(xx-trunc(xx));
					xx2=fabs(1-xx1);
					yy1=fabs(yy-trunc(yy));
					yy2=fabs(1-yy1);

					x=bf+(long)trunc(xx)*3+(long)trunc(yy)*bb3;
					xy=xx2*yy2;
					rot=(*x++)*xy;
					gruen=(*x++)*xy;
					blau=(*x++)*xy;

					xy=xx1*yy2;
					rot+=(*x++)*xy;
					gruen+=(*x++)*xy;
					blau+=(*x)*xy;

					x+=bb3-5;
					xy=xx2*yy1;
					rot+=(*x++)*xy;
					gruen+=(*x++)*xy;
					blau+=(*x++)*xy;

					xy=xx1*yy1;
					rot+=(*x++)*xy;
					gruen+=(*x++)*xy;
					blau+=(*x)*xy;
					
					*bt++=(unsigned char) rot;
					*bt++=(unsigned char) gruen;
					*bt++=(unsigned char) blau;
				}
			}
		}
	}
	}else{/*RANDS*/
	for(i=sy; i<ey; i++){
		double yd;
		int yd2;

		if((i&0xf)==0) busybox((int)(i-sy)*bfak);

		yd2=(my-i);
		yd=(double)yd2*yd2;
		for(j=sx; j<ex; j++){
			double mm2;
			int xd2;

			xd2=(mx-j);
			mm2=sqrt((double)xd2*xd2+yd);
			if(mm2>rad){
				unsigned char *x;
				x=bf+(long)j*3+(long)i*bb3;
				*bt++=*x++;
				*bt++=*x++;
				*bt++=*x;
			}else{
				double xx,yy;
				double bh;
				bh=sin(mm2*mrad)*hrad;
				xx=j+xd2*bh-0.5;
				yy=i+yd2*bh-0.5;
				if(xx<0) xx=0;
				if(yy<0) yy=0;
				if(xx>=bb-1) xx=bb-1.001;
				if(yy>=hh-1) yy=hh-1.001;
				{
					unsigned char *x;
					double rot, gruen, blau;
					double xx1, yy1, xx2, yy2, xy;

					xx1=fabs(xx-trunc(xx));
					xx2=fabs(1-xx1);
					yy1=fabs(yy-trunc(yy));
					yy2=fabs(1-yy1);

					x=bf+(long)trunc(xx)*3+(long)trunc(yy)*bb3;
					xy=xx2*yy2;
					rot=(*x++)*xy;
					gruen=(*x++)*xy;
					blau=(*x++)*xy;

					xy=xx1*yy2;
					rot+=(*x++)*xy;
					gruen+=(*x++)*xy;
					blau+=(*x)*xy;

					x+=bb3-5;
					xy=xx2*yy1;
					rot+=(*x++)*xy;
					gruen+=(*x++)*xy;
					blau+=(*x++)*xy;

					xy=xx1*yy1;
					rot+=(*x++)*xy;
					gruen+=(*x++)*xy;
					blau+=(*x)*xy;
					
					*bt++=(unsigned char) rot;
					*bt++=(unsigned char) gruen;
					*bt++=(unsigned char) blau;
				}
			}
		}
	}
	}
	{
		bf=(unsigned char*)bild->pic_data+(long)sy*bb3+(long)sx*3;
		bt=bildto;
		for(i=sy; i<ey; i++){
			memcpy(bf, bt, (long)(ex-sx)*3);
			bt+=(long)(ex-sx)*3;
			bf+=(long)bb3;
		}
	}
	Mfree(bildto);
	return(-1);
}

int bang_FPU(SMURF_PIC *bild,int mx,int my,int HOEHE,int RADIUS,int RANDS)
{
	unsigned char *bt, *bf, *bildto;
	unsigned char null[3]={0,0,0};
	int bb, hh;
	int i, j;
	long bb3;
	double rad, hrad, mrad;
	int sx=0, sy=0, ex=bild->pic_width, ey=bild->pic_height;
	double bfak;
	
	if( mx-RADIUS>0) sx=mx-RADIUS;
	if( my-RADIUS>0) sy=my-RADIUS;
	if( mx+RADIUS+1<ex) ex=mx+RADIUS+1;
	if( my+RADIUS+1<ey) ey=my+RADIUS+1;
	
	bb=bild->pic_width;
	hh=bild->pic_height;
	bildto=bt=Malloc((long)(ex-sx)*(ey-sy)*3);
	if( bildto==0) return(0);

	bf=bild->pic_data;

	bb3=bb*3;
	
	rad=(double)RADIUS;
	hrad=(double)HOEHE/RADIUS;
	mrad=(double)M_PI/RADIUS;
	
	bf+=(long)sy*bb3;
	bfak=(double)128/(ey-sy);
	
	if( RANDS){
	for(i=sy; i<ey; i++){
		double yd;
		int yd2;

		if((i&0xf)==0) busybox((int)(i-sy)*bfak);

		bf+=(long)sx*3;

		yd2=(my-i);
		yd=(double)yd2*yd2;
		for(j=sx; j<ex; j++){
			unsigned char *x;
			double mm2;
			int xd2;

			xd2=(mx-j);
			mm2=sqrt((double)xd2*xd2+yd);
			if(mm2>rad){
				*bt++=*bf++;
				*bt++=*bf++;
				*bt++=*bf++;
			}else{
				int xx,yy;
				double bh;
				bh=sin(mm2*mrad)*hrad;
				xx=xd2*bh;
				yy=yd2*bh;
				if(xx+j<0 || xx+j>=bb || yy+i<0 || yy+i>=hh){
					x=null;
				}else{
					x=bf+(long)xx*3+(long)yy*bb3;
				}
				*bt++=*x++;
				*bt++=*x++;
				*bt++=*x;
				bf+=3;
			}
		}
		bf+=(long)(bb-ex)*3;
	}
	}else{/*RANDS*/
	for(i=sy; i<ey; i++){
		double yd;
		int yd2;

		if((i&0xf)==0) busybox((int)i*bfak);

		bf+=(long)sx*3;

		yd2=(my-i);
		yd=(double)yd2*yd2;
		for(j=sx; j<ex; j++){
			unsigned char *x;
			double mm2;
			int xd2;

			xd2=(mx-j);
			mm2=sqrt((double)xd2*xd2+yd);
			if(mm2>rad){
				*bt++=*bf++;
				*bt++=*bf++;
				*bt++=*bf++;
			}else{
				int xx,yy;
				double bh;
				bh=sin(mm2*mrad)*hrad;
				xx=xd2*bh;
				yy=yd2*bh;
				if(xx+j<0) xx=-j;
				if(xx+j>=bb) xx=bb-j-1;
				if(yy+i<0) yy=-i;
				if(yy+i>=hh) yy=hh-i-1;
				x=bf+(long)xx*3+(long)yy*bb3;
				*bt++=*x++;
				*bt++=*x++;
				*bt++=*x;
				bf+=3;
			}
		}
		bf+=(long)(bb-ex)*3;
	}
	}
	{
		bf=(unsigned char*)bild->pic_data+(long)sy*bb3+(long)sx*3;
		bt=bildto;
		for(i=sy; i<ey; i++){
			memcpy(bf, bt, (long)(ex-sx)*3);
			bt+=(long)(ex-sx)*3;
			bf+=(long)bb3;
		}
	}
	Mfree(bildto);
	return(-1);
}
