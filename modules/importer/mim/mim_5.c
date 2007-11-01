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
		Smurf MIM Loader
*/

#ifdef GERMAN
#define alertstr "[2][Die Datei enth„lt keine|Farbtabelle! Welche Farbpalette|soll generiert werden?|(Ebenenbilder sollten als|Graustufen geladen werden.)][Standard|Graustufen]"
#else
#ifdef ENGLISH
#define alertstr "[2][The file contains no|colortable! Which colortable should|be generated?|(...images should be loaded|with greyscales.)][Standard|Greyscale]"
#else
#ifdef FRENCH
#define alertstr "[2][The file contains no|colortable! Which colortable should|be generated?|(...images should be loaded|with greyscales.)][Standard|Greyscale]"
#else
#error "keine Sprache!"
#endif
#endif
#endif

#include <tos.h>
#include <ext.h>
#include <stdio.h>
#include <string.h>
#include <portab.h>
#include <aes.h>
#include <vdi.h>

#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

char orgft[3*16]={
255,255,255,
255,0,0,
0,255,0,
255,255,0,
0,0,255,
255,0,255,
0,255,255,
200,200,200,
128,128,128,
180,0,0,
0,180,0,
180,180,0,
0,0,180,
180,0,180,
0,180,180,
0,0,0};

/* Infostruktur fr Hauptmodul */
MOD_INFO	module_info={
"MIM Format",0x0120,"Bj”rn Spruck",
"MIM","","","","",
"","","","","",
/* Objekttitel */
"","","","","","",
"","","","","","",
/* Objektgrenzwerte */
0,128,0,128,0,128,0,128,
0,10,0,10,0,10,0,10,
/* Slider-Defaultwerte */
0,0,0,0,0,0,0,0,0,0,0,0,
};

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

uchar suf[256];
int pre[256], tabl;


long unpack_upd(uchar *s,uchar *d,long k)
{
	long l=0;
	uchar vorz, ina;
	int w, ll=0, i, invers[256];

	tabl=0;

	vorz=*s++;
	while( k>0){
		w=*s++;
		if( vorz&128 ) w+=256;
		if( ll==7 ){
			ll=0;
			vorz=*s++;
			k--;
						if( k==0 ) break;
		}else{
			vorz<<=1;
			ll++;
		}
		ina=0;
		pre[tabl]=w;
		while( w>255 ){
			w-=256;
			/* inverse Ausgabe von suf[w]	*/
			invers[ina]=w;
			w=pre[w];
			ina++;
		}
		*(d++)=w;
		l++;
		if( tabl>0 ) suf[tabl-1]=w;
		tabl++;
		if( tabl==256 ) tabl=0;
		for( i=0; i<ina; i++){
			*(d++)=suf[invers[ina-i-1]];
			l++;
		}

		k--;
	}

	return( l);
}

void convertiere_bild(unsigned int *sc1,unsigned int *sc2,unsigned int x,unsigned int y,unsigned int e)
{
	long i;
	switch(e){
		case 1 :{
			for( i=0; i<(x+15)/16*y; i++ ){
				*sc2++=*sc1++;
			}
		} break;
		case 2 :{
			unsigned int *s1, *s2;
			s1=sc2;
			s2=s1+(x+15)/16*y;
			for( i=0; i<(x+15)/16*y; i++ ){
				*s1++=*sc1++;
				*s2++=*sc1++;
			}
		} break;
		case 4 :{
			unsigned int *s1, *s2, *s3, *s4;
			s1=sc2;
			s2=s1+(x+15)/16*y;
			s3=s2+(x+15)/16*y;
			s4=s3+(x+15)/16*y;
			for( i=0; i<(x+15)/16*y; i++ ){
				*s1++=*sc1++;
				*s2++=*sc1++;
				*s3++=*sc1++;
				*s4++=*sc1++;
			}
		} break;
		case 8 :{
			unsigned int *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8;
			s1=sc2;
			s2=s1+(x+15)/16*y;
			s3=s2+(x+15)/16*y;
			s4=s3+(x+15)/16*y;
			s5=s4+(x+15)/16*y;
			s6=s5+(x+15)/16*y;
			s7=s6+(x+15)/16*y;
			s8=s7+(x+15)/16*y;
			for( i=0; i<(x+15)/16*y; i++ ){
				*s1++=*sc1++;
				*s2++=*sc1++;
				*s3++=*sc1++;
				*s4++=*sc1++;
				*s5++=*sc1++;
				*s6++=*sc1++;
				*s7++=*sc1++;
				*s8++=*sc1++;
			}
		} break;
		case 16 :{
			for( i=0; i<(x+15)/16*y*32; i++ ){
				*sc2++=*sc1++;
			}
		} break;
	}
}

ulong unpack(uchar *s, uchar *d,ulong ls)
{
	ulong i, le=0;
	uchar w, len;
	
	for( i=0; i<ls; i++){
		if( *s>0 && *s<128 ){
			len=*s;
			s++;
			w=*s;
			s++;
			le+=len;
			i++;
			memset( d, w, len);
			d+=len;
		}
	}
	return( le);
}

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*		MIM Format (by Sage) 				.MIM	*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char *buffer, t[32];
unsigned int width, height, feb;
void *bs, *bso=0, *smbuffer, *locbuffer=0;
int not_conv=0;
long l;
int grey=0;

	smbuffer=smurf_struct->smurf_pic->pic_data;
	buffer=smurf_struct->smurf_pic->pic_data;
	memcpy( t, buffer, 32);
	
	if( *(long *)buffer!='MIMF'){
		return(M_INVALID);
	}
	
	strcpy(smurf_struct->smurf_pic->format_name, "MIM Format");

	if( t[4]>=3 ) not_conv=1;
	feb=t[5];
	if( feb>=16 ) not_conv=1;
	width=*((int *)&t[6]);
	height=*((int *)&t[8]);
	
	buffer+=32;

	{
		unsigned char *f=(unsigned char *)smurf_struct->smurf_pic->palette;
		int i;
		if( t[11]&1 ){/*	Farbpalette lesen	*/
			for( i=0; i<*((int *)&t[16]); i++){
				*f++=*buffer++;
				*f++=*buffer++;
				*f++=*buffer++;
			}
		}else{
			if( feb<=8){
				int i, flag=2;
				if( feb!=1) flag=form_alert(2,alertstr);
				if(flag==1){
					int i;
					for( i=0; i<16; i++){
						*f++=orgft[i*3];
						*f++=orgft[i*3+1];
						*f++=orgft[i*3+2];
					}
				}else{
					grey=1;
					switch(feb){
					case 1:
						*f++=0;
						*f++=0;
						*f++=0;
						*f++=255;
						*f++=255;
						*f=255;
						break;
					case 2:
						*f++=0;
						*f++=0;
						*f++=0;
						*f++=100;
						*f++=100;
						*f++=100;
						*f++=190;
						*f++=190;
						*f++=190;
						*f++=255;
						*f++=255;
						*f=255;
						break;
					case 4:
						for( i=0; i<16; i++){
							*f++=255*i/15;
							*f++=255*i/15;
							*f++=255*i/15;
						}
						break;
					case 8:
						for( i=0; i<256; i++){
							*f++=i;
							*f++=i;
							*f++=i;
						}
						break;
					}
				}
			}
		}
	}
	if( t[11]&4 ) not_conv=1;

	memmove( smbuffer, buffer, *((long *)&t[12]));
	bs=smbuffer;
	l=(long)(width+15)/16*2*feb* height;

	if( t[11]&2 && t[11]&4){
		locbuffer= Malloc( l);
		if( locbuffer==0){
			return(M_MEMORY);
		}
		/*l2 = */unpack( bs, locbuffer, *((long *)&t[12]));
		bs=locbuffer;
		bso=smbuffer;
	}else{
		if( t[11]&8 ){
			locbuffer= Malloc( l);
			if( locbuffer==0 ){
				return(M_MEMORY);
			}
			/*l2 = */unpack_upd( bs, locbuffer, *((long *)&t[12])-1);
			bs=locbuffer;
			bso=smbuffer;
		}else{
			memmove( smurf_struct->smurf_pic->pic_data, bs,(long)(width+15)/16*2* feb* height);
			bs=smurf_struct->smurf_pic->pic_data;
			Mshrink(0,bs,(long)(width+15)/16*2* feb* height);
		}
	}
	
	if( not_conv==0){
		if( locbuffer==0){
			locbuffer=Malloc(l);
			if( locbuffer){
				convertiere_bild( bs, locbuffer, width, height, feb);
				bso=bs;
				bs=locbuffer;
			}
		}
	}

	if( bso){
		Mfree( bso);
	}

	if( t[11]&4){
		if( width&0xF){/* Keine MOD 16 Pixelbreite!*/
			int j;
			char *bs1, *bs2;
			bs1=bs;
			bs2=bs;
			for( j=0; j<height; j++){
				memcpy( bs1, bs2, width);
				bs1+=width;
				bs2+=(width+15)&0xFFF0;
			}
			Mshrink(0,bs,(long)width*height);
		}
	}else{
		if( (width&0xF)<=8 && (width&0xF)!=0){
			int i, j;
			char *bs1, *bs2;
			bs1=bs;
			bs2=bs;
			for( i=0; i<feb; i++){
				for( j=0; j<height; j++){
					memcpy( bs1, bs2, (width+7)/8);
					bs1+=(long)(width+7)/8;
					bs2+=(long)(width+15)/16*2;
				}
			}
			Mshrink(0,bs,(long)(width+7)/8* feb* height);
		}
	}
	
	smurf_struct->smurf_pic->pic_width=width;
	smurf_struct->smurf_pic->pic_height=height;
	smurf_struct->smurf_pic->depth=feb;
	if(grey){
		smurf_struct->smurf_pic->col_format=GREY;
	}else{
		smurf_struct->smurf_pic->col_format=RGB;
	}
	if(feb<=8 && (t[11]&4)==0){
		smurf_struct->smurf_pic->format_type=FORM_STANDARD;
	}else{/* Pixelpacked auch fr Ebenen!!! */
		smurf_struct->smurf_pic->format_type=FORM_PIXELPAK;
	}
	smurf_struct->smurf_pic->pic_data=bs;

	return(M_PICDONE);
}

