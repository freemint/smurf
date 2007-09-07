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
 * The Initial Developer of the Original Code is
 * Christian Eyrich
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* =========================================================*/
/*	Photo CD-Format "den Importer schreib' ich mal schnell"	*/
/*	Ein Bild, fÅnf Auflîsungen, das YCbCr-Farbsystem, eine	*/
/*	kranke Kodierung und ein kaputter Abend					*/
/* Version 0.1  --  10.01.96								*/
/*    BASE/16, BASE/4 und BASE - Mehr gibt's nicht, weil	*/
/*	  Kodak zu geldgeil ist.								*/
/* Version 0.2  --  20.01.96								*/
/*	  Einige Optimierungen im YCbCr -> RGB (brought to you	*/
/*	  by Dale) und das Modul schleicht nicht mehr (Faktor	*/
/*	  60 zur 0.1)											*/
/* Version 0.3  --  31.01.96								*/
/*	  Klaus verwirrte mich sehr, nun endlich ist es voll-	*/
/*	  bracht und die Farbkonvertierung aus hpcdtoppm ge-	*/
/*	  klaut - nur sind 429 ticks fÅr ein Base4-Bild noch	*/
/*	  zuviel												*/ 
/* Version 0.4  --  27.03.96								*/
/*	  Quellspeicher wurde nicht freigegeben					*/
/* Version 0.5  --  01.06.96								*/
/*	  Der PixelmÅll an der untersten Zeile und der rechten	*/
/*	  Spalte ist weg und die Routine wieder 50 Ticks		*/
/*	  schneller.											*/
/* Version 0.6  --  15.01.97								*/
/*	  Zeiger werden nun direkt angesprochen					*/
/* Version 0.7  --  01.10.97								*/
/*	  YCC->RGB-Umrechnung total Åberarbeitet und um ca. 31% */
/*	  beschleunigt. Die Ergebnisse sind auch korrekter.		*/
/* Version 0.8  --  04.10.97								*/
/*	  Endlich eine 68000er Version.							*/
/* Version 0.9  --  xx.xx.xx								*/
/*	  Bilder werden zurechtgedreht: Byte 0x0E02				*/
/* =========================================================*/
 
#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);

int (*busybox)(int pos);
SERVICE_FUNCTIONS *Services;

static void initctable(void);static void ycctorgb(char *ziel, unsigned int width, unsigned int height);
extern void PCD_decode00(long *par);	/* 68000er Assembler Routine */
extern void PCD_decode20(long *par);	/* 68020er Assembler Routine */

MOD_INFO module_info = {"PCD Image-Modul",
						0x0080,
						"Christian Eyrich",
						"PCD", "", "", "", "",
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
/*		Kodak Photo CD-Dekomprimierer (PCD)			*/
/*		24 Bit, datenreduziert						*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/

const char PCDMap[512] =	{/* 128 mal 0 gegen Unterlauf */
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,
/* 256 Kernzahlen */
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,
       14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
       28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,
       42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,       56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
       70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
       84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,
       98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
      112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125,
      126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
      140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153,
      154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
      168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
      182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195,
      196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
      210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
      224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
      238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
      252, 253, 254, 255,/* 128 mal 255 gegen öberlauf */
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,	  255, 255
	};

static long T_L[256],T_R2[256],T_G1[256],T_G2[256],T_B1[256]; 

int imp_module_main(GARGAMEL *smurf_struct)
{
	char *ziel, *ziely, *zielcb, *zielcb1, *zielcb2,
		 *zielcr, *zielcr1, *zielcr2, *buffer, *obuffer;

    unsigned int bh, bl, y, width, realwidth, height, i;
 
    unsigned long help;
 

/* wie schnell sind wir? */
/*	init_timer(); */

	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;

	busybox = smurf_struct->services->busybox;
	Services = smurf_struct->services;

	buffer = smurf_struct->smurf_pic->pic_data;
 
/* Das Bild liegt im YCbCr-Farbsystem vor. Durch Datenreduktion */
/* sind fÅr jeden Pixel nur die Helligkeitswerte (Y) enthalten. */
/* Die Farbwerte sind nur in einem von vier Pixel gespeichert und */
/* werden fÅr die Darstellung interpoliert. */

/* Hier wird wegen der Grîûe von PCD-Files die Ermittlung der */
/* wichtigen Daten vom Smurf erledigt und nur Rohdaten ab dem */
/* passenden Offset Åbergeben. */

	width = smurf_struct->smurf_pic->pic_width;
	height = smurf_struct->smurf_pic->pic_height;

	strncpy(smurf_struct->smurf_pic->format_name, "Photo CD .PCD", 21);
	smurf_struct->smurf_pic->depth = 24;

	smurf_struct->services->reset_busybox(0, "Photo CD 24 Bit");

	if((ziel = SMalloc((long)width * (long)height * 3)) == 0)
		return(M_MEMORY);
	else
	{
		obuffer = buffer;
		ziely = ziel;
		zielcb = ziel + 1;
		zielcr = ziel + 2;

		realwidth = width * 3;

/* *** */
		help = width;
		while(help--) /* Hole BPERZ Byte aus dem Datenstrom ;-)) */
		{
			*ziely = *buffer++;
			ziely += 3;

			*ziely = *buffer++;
			ziely += 3;
		}
		*(ziely - realwidth - 3) = *(ziely - realwidth - 6);
		*(ziely - 3) = *(ziely - 6);

		help = width >> 1;
		while(help--)
		{
			*zielcb = *buffer++;
			zielcb += 3;

			*zielcb = (*(zielcb - 3) + *buffer) >> 1;
			zielcb += 3;
		}
		*(zielcb - 3) = *(zielcb - 6);
		zielcb += realwidth;

		help = width >> 1;
		while(help--)
		{
			*zielcr = *buffer++;
			zielcr+= 3;

			*zielcr = (*(zielcr - 3) + *buffer) >> 1;
			zielcr+= 3;
		}
		*(zielcr - 3) = *(zielcr - 6);
		zielcr += realwidth;

/* *** */

		bh = height / 20; 				/* busy-height */
		bl = 0;							/* busy-length */

		y = 0;
		do
		{
			if(!(y%bh))
			{
				busybox(bl);
				bl += 12;
			}

			help = width;
			while(help--) /* Hole BPERZ Byte aus dem Datenstrom ;-)) */
			{
				*ziely = *buffer++;
				ziely += 3;

				*ziely = *buffer++;
				ziely += 3;
			}
			*(ziely - realwidth - 3) = *(ziely - realwidth - 6);
			*(ziely - 3) = *(ziely - 6);

			help = width >> 1;
			while(help--)
			{
				*zielcb = *buffer++;
				zielcb += 3;

				*zielcb = (*(zielcb - 3) + *buffer) >> 1;
				zielcb += 3;
			}
			*(zielcb - 3) = *(zielcb - 6);

			help = width >> 1;
			while(help--)
			{
				*zielcr = *buffer++;
				zielcr+= 3;

				*zielcr = (*(zielcr - 3) + *buffer) >> 1;
				zielcr+= 3;
			}
			*(zielcr - 3) = *(zielcr - 6);

	/* Versuch' doch mal, mit nur einem Zeiger auszukommen */
	/* und adressiere alles relativ */
			zielcb -= realwidth;
			zielcb2 = zielcb - realwidth;
			zielcb1 = zielcb2 - realwidth;
			zielcr -= realwidth;
			zielcr2 = zielcr - realwidth;
			zielcr1 = zielcr2 - realwidth;
			i = width;
			do
			{
				*zielcb2 = (*zielcb1 + *zielcb) >> 1;
				zielcb += 3; 
				zielcb1 += 3;
				zielcb2 += 3;

				*zielcr2 = (*zielcr1 + *zielcr) >> 1;
				zielcr += 3;
				zielcr1 += 3;
				zielcr2 += 3;
			} while(--i); /* Interpolation Zeilen Y, Y-1 und Y-2 */

			zielcb += realwidth;
			zielcr += realwidth;

			y += 2;
		} while(y < height - 2); /* y */
		memcpy(ziel + (long)realwidth * (height - 1), ziel + (long)realwidth * (height - 2), realwidth);

		smurf_struct->services->reset_busybox(128, "YCbCr -> RGB");

/* wie schnell sind wir? */
/*	init_timer(); */

		ycctorgb(ziel, width, height);

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */

		smurf_struct->smurf_pic->pic_data = ziel;
		smurf_struct->smurf_pic->format_type = 0;
		smurf_struct->smurf_pic->col_format = RGB;

		buffer = obuffer;

	} /* Malloc */

/* wie schnell waren wir? */
/*	printf("%lu\n", get_timer());
	getch(); */

	SMfree(buffer);
	return(M_PICDONE);
}


/* --- INITCTABLE --- */
static void initctable(void){
	int i;

	for(i = 0; i < 256; i++)	{  	
		/* i * 1.3584 * 1024 + Rundung + 128 * 1024 gegen Unterlauf */
		T_L[i] = i * 1391L + 512L + 131072L;		/* (i - 137) * 1.340762 * 1.3584 * 1024 */
		T_R2[i] = (i - 137) * 1865L;		/* (i - 156) * 0.317038 * 1.3584 * 1024 */
		T_G1[i] = (i - 156) * -441L;		/* (i - 137) * 0.682243 * 1.3584 * 1024 */
		T_G2[i] = (i - 137) * -949L;		/* (i - 156) * 1.632639 * 1.3584 * 1024 */
		T_B1[i] = (i - 156) * 2271L;
	}
	return;
}/* --- YCCTORGB --- */
static void ycctorgb(char *ziel, unsigned int width, unsigned int height){
	long *par;
	
	initctable();
	par = Malloc(100L);
	memset(par, 0, 100L);	

	*(par) = (long)ziel;
	*(par + 1) = (long)T_B1;
	*(par + 2) = (long)T_G1;
	*(par + 3) = (long)T_R2;
	*(par + 4) = (long)T_G2;
	*(par + 5) = (long)T_L;
	*(par + 6) = (long)width;
	*(par + 7) = (long)height;
	*(par + 8) = (long)PCDMap;

	if(!(Services->CPU_type&MC68000))
		PCD_decode20(par);
	else
		PCD_decode00(par);

	Mfree(par);
}


/*
	Die unteren zwei Bit des 0x0E02-ten Bytes vom Dateianfang an gesehen.
	(Ich hoffe ich habe mich nicht verzÑhlt ;-)
	
	0x00: TURN_NONE
	0x01: TURN_LEFT
	0x03: TURN_RIGHT
	REST: FEHLER "Illegale Orientierung im Kodak Photo-CD-Bild!"
	
	Gilt meines Wissens nicht fÅr die öbersichtsbilder auf der CD!
	
	RR> Huch? Ich lese ab Byteposition 194635. Das kînnte erklÑren, warum
	RR>meine Bilder manchmal auf dem Kopf stehen... :-).


	case BASE_D16:	DatenOffset = 0x2000;
				   	PCDwidth = 192;
				 	PCDheight = 128;
				   	break;
	case BASE_D4:	DatenOffset = 0xB800;
				  	PCDwidth = 384;
				  	PCDheight = 256;
				  	break;
	case BASE: 		DatenOffset = 0x30000L;
			   		PCDwidth = 768;
			   		PCDheight = 512;
			   		break;
*/


/*
	R = Y                 + 1.340762 * C2	G = Y - 0.317038 * C1 - 0.682243 * C2	B = Y + 1.632639 * C1
	YCC is scaled by 1.3584.  C1 zero is 156 and C2 is at 137.*/

/*
Converting PhotoYCC data to RGB 24bit data for display by computers on CRT's
is achieved as follows:
Firstly normal Luma and Chroma data are recovered:
	Luma      = 1.3584 * Luma_8bit
	Chroma1   = 2.2179 * (Chroma1_8bit - 156)
	Chroma2   = 1.8215 * (Chroma2_8bit - 137)
For display primaries that are, or are very close to, CCIR Recommendation 709
primaries in their chromaticities, then 
	R'  = L + Chroma2
	G'  = L - 0.194Chroma1 - 0.509Chroma2
	B'  = L + Chroma1
Two things to watch are:
a) this results in RGB values from 0 to 346 (instead of the more usual 0 to 255)
a look-up-table is usually used to convert these through a non-linear function
to 8 bit data. For example:
	Y  = (255/1.402)*Y' 
	C1 = 111.40*C1'+156 
	C2 = 135.64*C2'+137 
b) if the display phosphors differ from CCIR 709 primaries then further
conversion will be necessary, possibly through anintermediate device
independent colour space such as CIE XYZ.
*/