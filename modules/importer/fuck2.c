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
		Smurf Fuckpaint Loader
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

/* Infostruktur fÅr Hauptmodul */
MOD_INFO	module_info={
"F*ckpaint",0x0110,"Bjîrn Spruck",
"PI9","","","","","","","","","",
"","","","","","","","","","","","",
0,128,0,128,0,128,0,128,
0,10,0,10,0,10,0,10,
0,0,0,0,0,0,0,0,0,0,0,0,
};

void convertiere_bild(unsigned int *sc1,unsigned int *sc2)
{
	int i;
	unsigned int *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8;

	s1=sc2;
	s2=s1+4800;
	s3=s2+4800;
	s4=s3+4800;
	s5=s4+4800;
	s6=s5+4800;
	s7=s6+4800;
	s8=s7+4800;
	for( i=0; i<4800; i++ ){
		*s1++=*sc1++;
		*s2++=*sc1++;
		*s3++=*sc1++;
		*s4++=*sc1++;
		*s5++=*sc1++;
		*s6++=*sc1++;
		*s7++=*sc1++;
		*s8++=*sc1++;
	}
}

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*		Fuckpaint Format (by Sage)			.PI9	*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *smbuffer, *buf;

	smbuffer=smurf_struct->smurf_pic->pic_data;

	if( smurf_struct->smurf_pic->file_len!=77824L) return(M_INVALID);
	
	strcpy(smurf_struct->smurf_pic->format_name, "F*ckpaint Format");
	
	{
		unsigned char *f=(unsigned char *)smurf_struct->smurf_pic->palette;
		int i;
		for( i=0; i<256; i++){
			*f++=*smbuffer++;
			*f++=*smbuffer++;
			smbuffer++;
			*f++=*smbuffer++;
		}
	}

	buf=Malloc(76800L);
	if(buf==0) return(M_MEMORY);
	convertiere_bild(smbuffer,buf);
	Mfree(smurf_struct->smurf_pic->pic_data);

	smurf_struct->smurf_pic->pic_width=320;
	smurf_struct->smurf_pic->pic_height=240;
	smurf_struct->smurf_pic->depth=8;
	smurf_struct->smurf_pic->col_format=RGB;
	smurf_struct->smurf_pic->format_type=FORM_STANDARD;
	smurf_struct->smurf_pic->pic_data=buf;

	return(M_PICDONE);
}

