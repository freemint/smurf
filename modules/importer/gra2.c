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
		Smurf GRA Loader
*/

#include <tos.h>
#include <ext.h>
#include <stdio.h>
#include <string.h>
#include <portab.h>
#include <aes.h>
#include <vdi.h>

#include "..\import.h"
#include "..\..\src\smurfine.h"


/* Infostruktur fr Hauptmodul */
MOD_INFO	module_info={
"GRA Format",0x0130,"Bj”rn Spruck",
"GRA","","","","","","","","","",
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
/*		GRA Format (by Sage) 				.GRA	*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *smbuffer;
	long x, y;

	if( smurf_struct->smurf_pic->file_len<=8) return(M_INVALID);
	smbuffer=smurf_struct->smurf_pic->pic_data;

	x=*((long *)&smbuffer[0]);
	y=*((long *)&smbuffer[4]);
	if( x*y+8!=smurf_struct->smurf_pic->file_len) return(M_INVALID);
	
	strcpy(smurf_struct->smurf_pic->format_name, "GRA Format");
	
	{
		unsigned char *f=(unsigned char *)smurf_struct->smurf_pic->palette;
		int i;
		for( i=0; i<256; i++){
			*f++=i;
			*f++=i;
			*f++=i;
		}
	}

	memcpy(smbuffer,smbuffer+8,(long)x*y);
	Mshrink(0,smbuffer,(long)x*y);

	smurf_struct->smurf_pic->pic_width=x;
	smurf_struct->smurf_pic->pic_height=y;
	smurf_struct->smurf_pic->depth=8;
	smurf_struct->smurf_pic->col_format=GREY;
	smurf_struct->smurf_pic->format_type=FORM_PIXELPAK;
	smurf_struct->smurf_pic->pic_data=smbuffer;

	return(M_PICDONE);
}

