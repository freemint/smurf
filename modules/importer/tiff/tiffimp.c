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
/*							TIFF-Importer					*/
/* Version 1.0  --  25.01.97								*/
/*	  xxx													*/
/* Version 1.1  --  15.03.97 - 26.04.98						*/
/*	  Dutzende von Bugfixes und Sicherheitsabfragen ein-	*/
/*	  gebaut. Besonders LZW-Decoding verbessert.			*/
/*	  Die TIC-Erkennung funktioniert nun auch endlich.		*/
/* Version 1.2  --  11.05.98								*/
/*	  min-is-black und min-is-white wurde zwar beim			*/
/*	  Erstellen der Farbpalette beachtet, jedoch wurde das	*/
/*	  Bild (1 Bit) nicht invertiert wenn min-is-black kam.	*/
/* Version 1.3  --  13.05.98 - 17.05.98						*/
/*	  LZW-Dekoder etwas geÑndert, damit auch Bilder von En-	*/
/*	  codern gelesen werden kînnen, die nicht schon bei		*/
/*	  ftable >= LCC - 1 auf die nÑchste Bittiefe gehen,		*/
/*	  sondern erst bei ftable >= LCC.						*/
/* Version 1.4  --  6.11.99									*/
/*	  Fehler, daû LZW-gepackte TIFF im Standardformat		*/
/*	  Probleme machen wenn sie nicht aligned sind, behoben.	*/
/* Version 1.5  --  2.11.2000								*/
/*	  Fehler daû fÅr 32 Bit-Bilder weder im LZW- noch im	*/
/*	  Packbitsdecoder die richtige LÑnge berechnet wurde	*/
/*	  und daû schluûendlich nur ein leerer 24 Bit Speicher-	*/
/*	  block an Smurf Åbergeben wurde behoben. Auûerdem		*/
/*	  eine 32 Bit-Variante des Difference-Decoders hinzuge-	*/
/*	  fÅgt.													*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"

#define MOTOROLA 1
#define INTEL 2

#define DEBUG	0
#define PRG		1

extern int check_tiff(char *buffer);
extern void tiff24Planes_24Normal(GARGAMEL *smurf_struct,char *source1, char *dest, long Width, long Height, long DataCount, long *DataArray);
extern void tiff32CMYK_24Normal(GARGAMEL *smurf_struct, char *source, char *dest, long Width, long Height, long DataOffset);
extern void tiff4_to_stf(GARGAMEL *smurf_struct,char *source, char *dest, long Width, long Height);
extern void tiff2_to_stf(GARGAMEL *smurf_struct,char *source, char *dest, long Width, long Height);
extern void tiff24_colormap(unsigned int *source, char *dest, long colors, int format);
extern void tiff24_min_is_black(char *dest, int Depth);
extern void tiff24_min_is_white(char *dest, int Depth);
extern int tiff32773_depack(GARGAMEL *smurf_struct,char *buffer, char *ziel, long width, long height, int BitsPerPixel, long RowPerStrip, long *DataArray);
extern int tiffCCITT3_depack(GARGAMEL *smurf_struct,char *source, char *dest, long Width, long Height, int Depth);
extern int tiffLZW_depack(GARGAMEL *smurf_struct,char *source, char *dest, long Width, long Height, int Depth, long DataCount, long *DataArray, long RowPerStrip, int Difference);
void invert_1Bit(char *data, long length);

#if !PRG
void *(*SMalloc)(long amount);
int	(*SMfree)(void *ptr);
#else
void *SMalloc(long amount);
int SMfree(void *ptr);
#endif


/* Dies bastelt direct ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"TIFF-Importer",
						0x0150,
						"Dale Russell, Christian Eyrich",
						"TIF", "TIFF", "TIC", "TIH", "TIM",
						"TIP", "", "", "", "",
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
/*			Taged Image File Format (TIF)			*/
/*		1, 2, 4, 8, 24, 32 Bit,						*/
/*		unkomprimiert, Pixelpacked (Byte & Word),	*/
/*		LZW, 32773-Packbits, CCITT3-Huffman,		*/
/*		Min-is-black/whit, RGB, CMY, CMYK			*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer;
	char *Palette24, *NewBuf, *help;
	char invertTIC, ExtraSamples;

	int format;
	int cnt,shl1, shl2, shl3, shl4, ahl1, ahl2;
	int BYTE, WORD, LONG, Depth, meth, maxR, W1, W2;
	int DirEntrys,TagId, TagFormat, PaletteColors, errcode, Difference;

	long HeaderOffset, MapOffset, DataOffset, TagOff, Compression,TagCount;
	long PlanarConfig, BitsPerSample, Photo, SamplePerPixel, Width, Height;
	long BakOffset, RowsPerStrip, tc, *DataArray, DataCount;
	long FillOrder, NewLength;

#if !PRG
	SMalloc = smurf_struct->services->SMalloc;
	SMfree = smurf_struct->services->SMfree;
#endif

/* Header checken */
	buffer=smurf_struct->smurf_pic->pic_data;
	if((format=check_tiff(buffer))==-1)
		return(M_INVALID);
 
	/* Header Check */
	help = strrchr(smurf_struct->smurf_pic->filename, '.');
	if(help && stricmp(help + 1, "TIC") == 0)
		invertTIC = 1;
	else
		invertTIC = 0;

/* Defaultwerte einsetzen */
	if(format==INTEL)
	{
		shl1=24; shl2=16; shl3=8; shl4=0;
		ahl1=8; ahl2=0;
		W1=8; W2=9;
	}
	else
	{
		shl1=0; shl2=8; shl3=16; shl4=24;
		ahl1=0; ahl2=8;	
		W1=10; W2=11;
	}

	DataArray=(long *)NULL;
	DataOffset=MapOffset=RowsPerStrip=0;
	Width=Height=BitsPerSample=SamplePerPixel=PlanarConfig=0;
	Difference=1;
	DataCount=1;
	FillOrder=1;
	Compression=0;

/************************************************************/
/*	Tag-Identification v0.1									*/
/************************************************************/
	if((Palette24 = SMalloc(1024L)) == NULL)
		return(M_MEMORY);
	memset(Palette24, 0x0, 1024L);

	HeaderOffset=(long)buffer[4]<<shl4;
	HeaderOffset+=(long)buffer[5]<<shl3;
	HeaderOffset+=(long)buffer[6]<<shl2;
	HeaderOffset+=(long)buffer[7]<<shl1;
	DirEntrys=(int)buffer[HeaderOffset]<<ahl2;
	DirEntrys+=(int)buffer[HeaderOffset+1]<<ahl1;
	
	if(DirEntrys==0)
	{
		form_alert(1,"[1][ Getting a ZERO | as IFD-counter (?)][ Crash! ]");
		SMfree(Palette24);
		return(M_PICERR);
	}
	
	TagOff=HeaderOffset+2;
	for (cnt=0; cnt<DirEntrys; cnt++)
	{
		WORD=LONG=FALSE;
		TagId=(int)buffer[TagOff+0]<<ahl2;
		TagId+=(int)buffer[TagOff+1]<<ahl1;
		TagFormat=(int)buffer[TagOff+2]<<ahl2;
		TagFormat+=(int)buffer[TagOff+3]<<ahl1;
	
		switch(TagFormat)
		{
			case 1: BYTE=TRUE;
					break;
			case 3: WORD=TRUE;
					break;
			case 4: LONG=TRUE;
					break;
			default:
#if DEBUG
					printf("\n Tag-Format is unknown (%i)", (int)TagFormat);
#endif
					break;
		}

		TagCount =  (long) buffer[TagOff+4] << shl4;
		TagCount += (long) buffer[TagOff+5] << shl3;
		TagCount += (long) buffer[TagOff+6] << shl2;
		TagCount += (long) buffer[TagOff+7] << shl1;

		switch(TagId)
		{
			case 0x0100:if(LONG)				/*	Width */
						{
							Width=(long)buffer[TagOff+8]<<shl4;
							Width+=(long)buffer[TagOff+9]<<shl3;
							Width+=(long)buffer[TagOff+10]<<shl2;
							Width+=(long)buffer[TagOff+11]<<shl1;
						}
						else
						{
							Width=(long)buffer[TagOff+8]<<ahl2;
							Width+=(long)buffer[TagOff+9]<<ahl1;
						}	
						break;
			case 0x0101:if(LONG)				/*	Height */
						{
							Height=(long)buffer[TagOff+8]<<shl4;
							Height+=(long)buffer[TagOff+9]<<shl3;
							Height+=(long)buffer[TagOff+10]<<shl2;
							Height+=(long)buffer[TagOff+11]<<shl1;
						}
						else
						{
							Height=(long)buffer[TagOff+8]<<ahl2;
							Height+=(long)buffer[TagOff+9]<<ahl1;
						}	
						break;
			case 0x0102:if(TagCount == 1)
							BakOffset = TagOff + 8;
						else
						{
							BakOffset=(long)buffer[TagOff+8]<<shl4;
							BakOffset+=(long)buffer[TagOff+9]<<shl3;
							BakOffset+=(long)buffer[TagOff+10]<<shl2;
							BakOffset+=(long)buffer[TagOff+11]<<shl1;
						}

						if(LONG)				/* BitsPerSample */
						{
							BitsPerSample=(long)buffer[BakOffset]<<shl4;
							BitsPerSample+=(long)buffer[BakOffset+1]<<shl3;
							BitsPerSample+=(long)buffer[BakOffset+2]<<shl2;
							BitsPerSample+=(long)buffer[BakOffset+3]<<shl1;
						}
						else
						{
							BitsPerSample=(long)buffer[BakOffset]<<ahl2;
							BitsPerSample+=(long)buffer[BakOffset+1]<<ahl1;
						}

						break;
			case 0x0111:if(LONG)				/* Strip Offsets */
						{
							DataOffset=(long)buffer[TagOff+8]<<shl4;
							DataOffset+=(long)buffer[TagOff+9]<<shl3;
							DataOffset+=(long)buffer[TagOff+10]<<shl2;
							DataOffset+=(long)buffer[TagOff+11]<<shl1;
						}
						else
						{
							DataOffset=(long)buffer[TagOff+W1]<<ahl2;
							DataOffset+=(long)buffer[TagOff+W2]<<ahl1;
						}

						if(TagCount>1)
						{
							BakOffset=DataOffset;
							DataArray = SMalloc(TagCount*4L);
							if(DataArray==NULL)
							{
								form_alert(1,"[ Can't allocate | RAM for DataRows][Sorry!]");
								SMfree(Palette24);
								return(M_PICERR);
							}

							DataCount=TagCount;

							for(tc=0; tc<TagCount; tc++)
							{
								if(LONG)
								{
									DataOffset=(long)buffer[BakOffset+0]<<shl4;
									DataOffset+=(long)buffer[BakOffset+1]<<shl3;
									DataOffset+=(long)buffer[BakOffset+2]<<shl2;
									DataOffset+=(long)buffer[BakOffset+3]<<shl1;
									BakOffset+=4;
								}
								else
								{
									DataOffset=(long)buffer[BakOffset+0]<<ahl2;
									DataOffset+=(long)buffer[BakOffset+1]<<ahl1;
									BakOffset+=2;
								}

								DataArray[tc] = DataOffset;
							}
						} 
						break;

			case 0x013d:Difference=(int)buffer[TagOff+8]<<ahl2;
						Difference+=(int)buffer[TagOff+9]<<ahl1;
						break;
	
			case 0x0117:break;					/* Strip Counts, not necessary for decoding... */
			case 0x0106:if(LONG)				/* Photometric Interpretation */
						{
							Photo=(long)buffer[TagOff+8]<<shl4;
							Photo+=(long)buffer[TagOff+9]<<shl3;
							Photo+=(long)buffer[TagOff+10]<<shl2;
							Photo+=(long)buffer[TagOff+11]<<shl1;
						}
						else
						{
							Photo=(long)buffer[TagOff+8]<<ahl2;
							Photo+=(long)buffer[TagOff+9]<<ahl1;
						}	
						break;
			case 0x0140:						/* Colormap Offset */
#if DEBUG
					  	printf("\nColormap-Tag found (Counter:%li) Word=%i Long=%i", TagCount, WORD, LONG);
#endif
						PaletteColors=(int)TagCount / 3;
						MapOffset=(long)buffer[TagOff+8]<<shl4;
						MapOffset+=(long)buffer[TagOff+9]<<shl3;
						MapOffset+=(long)buffer[TagOff+10]<<shl2;
						MapOffset+=(long)buffer[TagOff+11]<<shl1;
						break;
			case 0x0152:ExtraSamples = buffer[TagOff+8];
						break;
			case 0x0115:if(LONG)				/* SamplePerPixel */
						{
							SamplePerPixel=(long)buffer[TagOff+8]<<shl4;
							SamplePerPixel+=(long)buffer[TagOff+9]<<shl3;
							SamplePerPixel+=(long)buffer[TagOff+10]<<shl2;
							SamplePerPixel+=(long)buffer[TagOff+11]<<shl1;
						}
						else
						{
							SamplePerPixel=(long)buffer[TagOff+8]<<ahl2;
							SamplePerPixel+=(long)buffer[TagOff+9]<<ahl1;
						}	
						break;
			case 0x011c:						/* Plane Configuration */
						PlanarConfig=(long)buffer[TagOff+8]<<ahl2;
						PlanarConfig+=(long)buffer[TagOff+9]<<ahl1;
						break;
			case 0x010a:						/* Fill Order */
						FillOrder=(long)buffer[TagOff+8]<<ahl2;
						FillOrder+=(long)buffer[TagOff+9]<<ahl1;
						break;
			case 0x0103:if(LONG)				/* Compression */
						{
							Compression=(long)buffer[TagOff+8]<<shl4;
							Compression+=(long)buffer[TagOff+9]<<shl3;
							Compression+=(long)buffer[TagOff+10]<<shl2;
							Compression+=(long)buffer[TagOff+11]<<shl1;
						}
						else
						{
							Compression=(long)buffer[TagOff+8]<<ahl2;
							Compression+=(long)buffer[TagOff+9]<<ahl1;
						}
						break;
			case 0x0116:if(LONG)				/* Rows per Strip */
						{
							RowsPerStrip=(long)buffer[TagOff+8]<<shl4;
							RowsPerStrip+=(long)buffer[TagOff+9]<<shl3;
							RowsPerStrip+=(long)buffer[TagOff+10]<<shl2;
							RowsPerStrip+=(long)buffer[TagOff+11]<<shl1;
						}
						else
						{
							RowsPerStrip=(long)buffer[TagOff+8]<<ahl2;
							RowsPerStrip+=(long)buffer[TagOff+9]<<ahl1;
						}
						break;
			default:
#if DEBUG
						printf("\n	Unknown or unsupported tag #%i ($%X)",(int)TagId,(int)TagId);
#endif
						break; 
		}
		TagOff+=12;		/* Next Tag, please */
	}	

#if DEBUG
	printf("\n\n	PHOTOMETRIC:%li",Photo);
	printf("\n	DataOffset:%li",DataOffset);
	printf("\n	MapOffset:%li",MapOffset);
	printf("\n	SamplePerPixel:%li",SamplePerPixel);
	printf("\n	BitsPerSample:%li",BitsPerSample);
	printf("\n	Width in Pixel:%li",Width);
	printf("\n	Height in Pixel:%li",Height);
	printf("\n	Planar Config:%li",PlanarConfig);
	printf("\n	Differencing Predictor:%i",Difference);
	printf("\n	Kompression:%li",Compression);
	printf("\n	RowsPerStrip:%li",RowsPerStrip);
	printf("\n	Fill-Order:%li",FillOrder);
	getch();
#endif

	if(Width==0 || Height==0)
	{
		form_alert(1,"[1][ * Missing:|'Width/Height-Tag'][ Stop! ]");	
		SMfree(Palette24);
		return(M_PICERR);
	}

	if(PlanarConfig==0 && BitsPerSample==0 && SamplePerPixel==0)
	{
		form_alert(1,"[1][ * Missing:|'Configuration-Tags'][ Stop! ]");
		SMfree(Palette24);
		return(M_PICERR);
	}

/****************************************************************/
/*																*/
/*		   		Welches TIFF hÑttens denn gern?					*/
/*																*/
/****************************************************************/
	
	if(Photo==4 || Photo==6)
	{
		if(Photo==4)
			form_alert(1,"[1][ Photometric-Set | 'Transparency mask' | isn't supported! ][ Oh-No! ]");
		else
			if(Photo==6)
				form_alert(1,"[1][ Photometric-Set | 'YCbCr-Colorsystem' | isn't supported! ][ Oh-No! ]");
			else
				if(Photo==8)
					form_alert(1,"[1][ Photometric-Set | 'CIE LAB-Colorsystem' | isn't supported! ][ Oh-No! ]");
		SMfree(Palette24);
		SMfree(DataArray);
		return(M_PALERR);
	}
	
	if(DataCount > 1 || DataArray != (long *)NULL) 
		DataOffset=DataArray[0];
	else
		if(DataOffset != 0) 
		{	
			DataArray=SMalloc(4L);
			DataArray[0]=DataOffset;
		}

	if(Compression<=0)
		Compression=1;

	if(PlanarConfig<=0)
		PlanarConfig=1;

	if(BitsPerSample<=0)
		BitsPerSample=0;
	else
		if(BitsPerSample>8)
			BitsPerSample=8;

	if(SamplePerPixel<=0)
		SamplePerPixel=1;
	else
		if(SamplePerPixel>4)
			SamplePerPixel=4;

	if(RowsPerStrip == 0 || RowsPerStrip > Height)
		RowsPerStrip = Height;

	if(SamplePerPixel==4 && BitsPerSample==0 && Photo==5)	/* 32 Bit CMYK-TIFs */
		BitsPerSample=8;
	if(SamplePerPixel==3 && BitsPerSample==0) 				/* 24 Bit TIFs */
		BitsPerSample=8;

	Depth = (int)BitsPerSample * (int)SamplePerPixel;

	if(Depth>32) 
	{	
		if(SamplePerPixel==3 || Photo==2)
			Depth=24;
		else
			if(SamplePerPixel==1 || Photo==3 || Photo==0 || Photo==1)	
				Depth=8;
	}	

	if(Depth==0) 
	{
		if(SamplePerPixel==0 || SamplePerPixel==1)
			switch((int)BitsPerSample)
			{
				case 8:	Depth=256; break;
				case 4: Depth=16; break;
				case 2: Depth=4; break;
				case 1: Depth=2; break;
				default: break;	
			}
		else
			if(SamplePerPixel==3)
				Depth=24;
		
		if(Depth==0)
		{ 
			form_alert(1,"[1][ Can't computing | Picture-Depth ][ Oh-No! ]");
			SMfree(Palette24);
			SMfree(DataArray);
			return (M_PICERR); 
		}
	}

	if(Photo==0 || Photo==1 || Photo==3)
	{
		switch(Depth)
		{
			case 8:	maxR=256; break;
			case 4: maxR=16; break;
			case 2: maxR=4; break;
			case 1: maxR=2; break;
			default: maxR=256; break;	
		}

		if(PaletteColors<1 || PaletteColors>maxR)
			PaletteColors=maxR;
	}
	
	if(PaletteColors<1 || PaletteColors>256)
		PaletteColors = 1<<Depth;
	
	if(Photo==0 || Photo==1 || Photo==3)
	{
		if(Depth==1)
		{
			if(!MapOffset)
			{
				Palette24[0]=255; Palette24[1]=255; Palette24[2]=255;
				Palette24[3]=0; Palette24[4]=0; Palette24[5]=0;
			}	
		}
		else
		{
			if(Photo==3)
				if (MapOffset==0)
					tiff24_min_is_black(Palette24,Depth);
				else 
					tiff24_colormap((unsigned int *)(buffer + MapOffset), Palette24, (long)PaletteColors, format);
			else
				if(Photo==0)
					tiff24_min_is_white(Palette24, Depth);
				else
					if(Photo==1)
						tiff24_min_is_black(Palette24, Depth);
		}
	}

	switch((int)Compression)
	{
		case 0x01:	meth = 0;				/* No compression or pixelpacked: BYTE */
					break;
		case 0x02:	meth = 1;				/* CCITT Group 3: 1-dimensional Huffman */
					break;
		case 0x03:	meth = 2;				/* CCITT Group 3/T.4: Facsimile 1/2-dimensional Huffmann */
					break;
		case 0x04:	meth = 3;				/* CCITT Group 4/T.6: S/W Compression */
					break;
		case 0x05:	meth = 4;				/* Lempel, Ziv and Welch (LZW) Compression */
					break;
		case 0x06:	meth = 5;				/* Joint Photographic Experts Group (JPeG) Compression */
					break;
		case 32773L:meth = 6;				/* RLE: 32773 Packbits Compression */
					break;
		case 32771L:meth = 0;				/* No compression or pixelpacked: WORD */
					break;
		default:	form_alert(1,"[1][Unknown | compression algorithm][ What? ]");
					SMfree(Palette24);
					SMfree(DataArray);
					return(M_UNKNOWN_TYPE);
	}

#if DEBUG
	printf("\n\nPHOTOMETRIC: %li", Photo);
	printf("\nDataOffset: %li", DataOffset);
	printf("\nMapOffset: %li", MapOffset);
	printf("\nSamplePerPixel: %li", SamplePerPixel);
	printf("\nBitsPerSample: %li", BitsPerSample);
	printf("\nWidth in Pixel: %li", Width);
	printf("\nHeight in Pixel: %li", Height);
	printf("\nPlanar Config: %li", PlanarConfig);
	printf("\nDifferencing Predictor:%i", Difference);
	printf("\nKompression: %li", Compression);
	printf("\nRowsPerStrip: %li", RowsPerStrip);
	printf("\nFill-Order: %li", FillOrder);
	getch();
#endif

	if(meth!=0 && meth!=1 && meth!=4 && meth!=6)
	{
/*		form_alert(1,"[1][Nicht unterstÅtzter|Packalgorithmus!][Mist]"); */
		SMfree(Palette24);
		SMfree(DataArray);
		return(M_UNKNOWN_TYPE);
	}


/********************************************************/
/*		Decrunch-Dispatcher								*/
/********************************************************/
	if(Depth>=8)
	{
		NewLength = Width * Height * Depth / 8 + 1024;
		NewBuf = (char *)SMalloc(NewLength);
	}
	else
	{
		NewLength = (Width + 7) / 8 * Height * Depth + 1024;
		NewBuf = (char *)SMalloc(NewLength);
	}
	
	if(NewBuf == 0)
	{
		form_alert(1,"[1][ Couldn't allocate | RAM for depacking ][ Pitty! ]");
		SMfree(Palette24); 
		SMfree(NewBuf);	
		SMfree(DataArray);
		return(M_MEMORY);

	}

	if(meth > 0)
	{
		errcode = 0;
		switch((int)meth)
		{
			case 0x01:	errcode=tiffCCITT3_depack(smurf_struct,buffer + DataOffset,NewBuf, Width, Height, Depth);
						Depth=8; PlanarConfig=1; PaletteColors=2;
						if(Depth==24 && PlanarConfig==1 && Photo==2 ||
						   Depth==8 && PlanarConfig==1 ||
						   Depth==1 && PlanarConfig==1)
							break;
						else
						{
							SMfree(buffer);
							DataOffset = 0;
							buffer = NewBuf;			
							NewBuf = SMalloc(NewLength);
							break;
						}
			case 0x04:	errcode = tiffLZW_depack(smurf_struct, buffer, NewBuf, Width, Height, Depth, DataCount, DataArray, RowsPerStrip, Difference);
						if(Depth == 24 && PlanarConfig == 1 && Photo == 2 ||
						   Depth == 8 && PlanarConfig == 1 ||
						   Depth == 1 && PlanarConfig == 1)
							break;
						else
						{
							SMfree(buffer);
							DataOffset = 0;
							buffer = NewBuf;			
							NewBuf = SMalloc(NewLength);
							break;
						}
			case 0x06: 	errcode=tiff32773_depack(smurf_struct,buffer,NewBuf, Width, Height, Depth,RowsPerStrip,DataArray);
						if(Depth==24 && PlanarConfig==1 && Photo==2 ||
						   Depth==8 && PlanarConfig==1 ||
						   Depth==1 && PlanarConfig==1)
							break;
						else
						{
							SMfree(buffer);
							DataOffset = 0;
							buffer = NewBuf;			
							NewBuf = SMalloc(NewLength);
							break;
						}
			default:	break;
		}

		if(errcode < 0)
		{
			form_alert(1,"[1][ Fehler beim Entpacken ][ Abbruch ]");
			SMfree(Palette24); 
			SMfree(NewBuf);	
			SMfree(DataArray);
			return(M_PICERR);
		}
	}
/*
	printf("Width: %ld, Height: %ld, NewLength: %ld\n", Width, Height, NewLength);
	getch();
*/

	/*	------- 24 Bit, RGB	 --------*/
	if(Depth==24 && PlanarConfig==1 && Photo==2)
	{
		if(meth != 0x01 && meth != 0x04 && meth != 0x06)
			memcpy(NewBuf, buffer + DataOffset, NewLength);
		smurf_struct->smurf_pic->pic_data = NewBuf;
		smurf_struct->smurf_pic->depth = 24;
		if(invertTIC)
			smurf_struct->smurf_pic->col_format = CMY;
		else
			smurf_struct->smurf_pic->col_format = RGB;
		smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
	}

	/*	------- 24 Bit, RGB-Planes	 --------*/
	else if(Depth==24 && PlanarConfig==2 && Photo==2)
	{
		tiff24Planes_24Normal(smurf_struct, buffer, NewBuf, Width, Height, DataCount, DataArray);
		smurf_struct->smurf_pic->pic_data = NewBuf;
		smurf_struct->smurf_pic->depth = 24;
		if(invertTIC)
			smurf_struct->smurf_pic->col_format = CMY;
		else
			smurf_struct->smurf_pic->col_format = RGB;
		smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
	}

	/*	------- 32/24 Bit, CMYK	 --------*/
	else if(Depth>=24 && PlanarConfig==1 && Photo==5)
	{
		tiff32CMYK_24Normal(smurf_struct, buffer, NewBuf, Width, Height, DataOffset);
		smurf_struct->smurf_pic->pic_data = NewBuf;
		/* solange Smurf nur 24 Bit versteht */
		smurf_struct->smurf_pic->depth = 24;
		smurf_struct->smurf_pic->col_format = RGB;
		smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
	}

	/*	------- 8 Bit, RGB-Palette	 --------*/
	else if(Depth==8 && PlanarConfig==1)
	{
		if(meth != 0x01 && meth != 0x04 && meth != 0x06)
			memcpy(NewBuf, buffer + DataOffset, NewLength);
		smurf_struct->smurf_pic->pic_data = NewBuf;
		smurf_struct->smurf_pic->depth = 8; 
		smurf_struct->smurf_pic->col_format = RGB;
		smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
		memcpy(smurf_struct->smurf_pic->palette, Palette24, 1024L);
	}

	/*	------- 4 Bit, RGB-Palette	 --------*/
	else if(Depth==4 && PlanarConfig==1)
	{
		tiff4_to_stf(smurf_struct, buffer + DataOffset, NewBuf, Width, Height);
		smurf_struct->smurf_pic->pic_data = NewBuf;
		smurf_struct->smurf_pic->depth = 4; 
		smurf_struct->smurf_pic->col_format = RGB;
		smurf_struct->smurf_pic->format_type = FORM_STANDARD;
		memcpy(smurf_struct->smurf_pic->palette, Palette24, 1024L);
	}

	/*	------- 2 Bit, RGB-Palette	 --------*/
	else if(Depth==2 && PlanarConfig==1)
	{
		tiff2_to_stf(smurf_struct, buffer + DataOffset, NewBuf, Width, Height);
		smurf_struct->smurf_pic->pic_data = NewBuf;
		smurf_struct->smurf_pic->depth = 2; 
		smurf_struct->smurf_pic->col_format = RGB;
		smurf_struct->smurf_pic->format_type = FORM_STANDARD;
		memcpy(smurf_struct->smurf_pic->palette, Palette24, 1024L);
	}

	/*	------- 1 Bit, RGB-Palette	 --------*/
	else if(Depth==1 && PlanarConfig==1)
	{
		if(Photo == 1)
			invert_1Bit(NewBuf, NewLength);
		if(meth != 0x01 && meth != 0x04 && meth != 0x06)
			memcpy(NewBuf, buffer + DataOffset, NewLength);
		smurf_struct->smurf_pic->pic_data = NewBuf;
		smurf_struct->smurf_pic->depth = 1; 
		smurf_struct->smurf_pic->col_format = RGB;
		smurf_struct->smurf_pic->format_type = FORM_STANDARD;
		memcpy(smurf_struct->smurf_pic->palette, Palette24, 1024L);
	}

	else
	{
		form_alert(1,"[1][ Bildstruktur defekt ][ Abbruch ]");
		SMfree(Palette24); 
		SMfree(NewBuf);	
		SMfree(DataArray);
		return(M_PICERR);
	}

	smurf_struct->smurf_pic->pic_width = (int)Width;
	smurf_struct->smurf_pic->pic_height = (int)Height;
	SMfree(buffer);		
	SMfree(Palette24);		

	return(M_PICDONE);
}


int check_tiff(char *buffer)
{
	int v1, v2, format;
	unsigned int magic;


/* Motorola oder Intel ? */

	v1 = buffer[0];
	v2 = buffer[1];

	if(v1 == 0x49 && v2 == 0x49)
		format = INTEL;
	else
		if(v1 == 0x4d && v2 == 0x4d)
			format = MOTOROLA;
		else
			format = -1;

/* Header checken (Magic = 0x002a) */
	magic = *(unsigned int *)(buffer + 2);
	if(format != -1 && (magic == 0x2a00 || magic == 0x002a))
		return(format);
	else
		return(-1);
}


#if PRG
/*-------------- Main-Function fÅr TIFF-Programm zum Debuggen */
void main(void)
{
	int filehandle;
	GARGAMEL sm_struct;
	SMURF_PIC pic;
	char *file;
	long dummy, len;

	void dummybb(int lft);
	void dummyrbb(int lft, char *str);


	dummy = Fopen("K:\\AMBERT.TIF", FO_READ);
	if(dummy >= 0)
		filehandle = (int)dummy;
	else
		return;

	len = Fseek(0L, filehandle, 2);
	Fseek(0L, filehandle, 0L);
	
	file = Malloc(len);

	Fread(filehandle, len, file);
	Fclose(filehandle);

	pic.pic_data = file;
	sm_struct.smurf_pic = &pic;

	imp_module_main(&sm_struct);

	return;	
}


void *SMalloc(long amount)
{
	char *buffer;


	buffer = (char *)Malloc(amount);
	memset(buffer, 0x0, amount);

	return(buffer);
}


/* --- Funktion zum Freigeben von Speicher + Kontrolle ------- */
int SMfree(void *ptr)
{
	if(ptr == NULL)
		return(-1);

	Mfree(ptr);

	return(0);
}
#endif