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
		Smurf B&W 256 Loader
*/

#ifdef GERMAN
#define fehler "[1][Dieses Bild scheint defekt zu sein.|Trotzdem laden?][Ja|Nein]"
#else
#ifdef ENGLISH
#define fehler "[1][Picture seems to be corupted.|Load it anyway?][Yes|No]"
#else
#ifdef FRENCH
#define fehler "[1][Picture seems to be corupted.|Load it anyway?][Yes|No]"
#else
#error "Keine Sprache!"
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


/* Infostruktur fr Hauptmodul */
MOD_INFO	module_info={
"BW Format",0x0130,"Bj”rn Spruck",
"B_W","","","","","","","","","",
"","","","","","","","","","","","",
0,128,0,128,0,128,0,128,
0,10,0,10,0,10,0,10,
0,0,0,0,0,0,0,0,0,0,0,0,
};

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*		BW Format (by Sage) 				.B_W	*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *smbuffer;
	unsigned int x, y, defekt=0;

	if( smurf_struct->smurf_pic->file_len<=10) return(M_INVALID);
	smbuffer=smurf_struct->smurf_pic->pic_data;
	if( strncmp( smbuffer, "B&W256", 6)) return(M_INVALID);
	
	x=*((unsigned int *)&smbuffer[6]);
	y=*((unsigned int *)&smbuffer[8]);
	if( x==0 || y==0) return(M_INVALID);
	if( (long)x*y+10!=smurf_struct->smurf_pic->file_len){
		if(form_alert(1,fehler)==2) return(M_INVALID);
		defekt=1;
	}
	
	strcpy(smurf_struct->smurf_pic->format_name, "B&W256 Format");

	{
		unsigned char *f=(unsigned char *)smurf_struct->smurf_pic->palette;
		int i;
		for( i=0; i<256; i++){
			*f++=i;
			*f++=i;
			*f++=i;
		}
	}

	if( defekt==0){
		memcpy(smbuffer,smbuffer+10,(long)x*y);
		Mshrink(0,smbuffer,(long)x*y);
	}else{
		if( (long)x*y+10<smurf_struct->smurf_pic->file_len){
			memcpy(smbuffer,smbuffer+10,(long)x*y);
			Mshrink(0,smbuffer,(long)x*y);
		}else{
			void *z;
			z=Malloc((long)x*y);
			if(z==0) return(M_MEMORY);
			memset(z,0,(long)x*y);
			memcpy(z,smbuffer+10,smurf_struct->smurf_pic->file_len-10);
			Mfree(smbuffer);
			smbuffer=z;
		}
	}
	
	smurf_struct->smurf_pic->pic_width=x;
	smurf_struct->smurf_pic->pic_height=y;
	smurf_struct->smurf_pic->depth=8;
	smurf_struct->smurf_pic->col_format=GREY;
	smurf_struct->smurf_pic->format_type=FORM_PIXELPAK;
	smurf_struct->smurf_pic->pic_data=smbuffer;

	return(M_PICDONE);
}

