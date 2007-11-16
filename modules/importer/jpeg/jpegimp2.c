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
 * The Initial Developer of the Original Code are
 * Olaf Piesche, Christian Eyrich, Dale Russell and J”rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* =========================================================*/
/*			JPEG File Interchange Format Decoder			*/
/* Version 0.1  --  27.09.97								*/
/*	  8 und 24 Bit											*/
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "..\..\import.h"
#include "..\..\..\src\smurfine.h"
/* #include <demolib.h> */

#include "jpeglib.h"
#include <setjmp.h>

char *read_JPEG_file(char *inpic);
int is_jpeg(char *buffer);

char BitsPerPixel;
char dummy[3], impmessag[21];

unsigned int width, height;

SERVICE_FUNCTIONS *Services;

long f_len_global;

struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;		/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/* Infostruktur fr Hauptmodul */
MOD_INFO module_info = {"JPEG",
						0x0010,
						"Christian Eyrich",
						"JPG", "JPEG", "JFIF", "", "",
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
/*		JPEG File Interchange Format (JPG)			*/
/*		8 und 24 Bit								*/
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
	char *buffer, *ziel, *pal;

	unsigned int i;


/* wie schnell sind wir? */
/*  init_timer(); */

	Services = smurf_struct->services;

	buffer = smurf_struct->smurf_pic->pic_data;

	f_len_global = smurf_struct->smurf_pic->file_len;

/*	printf("JFIF-Importer hier\n");
	getch(); */

	if(is_jpeg(buffer) < 0)
		return(M_INVALID);
	else
	{
		if((ziel = read_JPEG_file(buffer)) == NULL)
			return(M_PICERR);
		else
			if((int)ziel == M_MEMORY)
				return(M_MEMORY);

		strncpy(smurf_struct->smurf_pic->format_name, "JPEGs JFIF .JPG", 21);
	
		smurf_struct->smurf_pic->pic_width = width;
		smurf_struct->smurf_pic->pic_height = height;
		smurf_struct->smurf_pic->depth = BitsPerPixel;

		Mfree(smurf_struct->smurf_pic->pic_data);
		smurf_struct->smurf_pic->pic_data = ziel;

		smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;

		pal = smurf_struct->smurf_pic->palette;
		if(BitsPerPixel == 8)
			for(i = 0; i < 256; i++)
			{
				*pal++ = (char)i;
				*pal++ = (char)i;
				*pal++ = (char)i;
			}

		if(BitsPerPixel == 8)
			smurf_struct->smurf_pic->col_format = GREY;
		else
			smurf_struct->smurf_pic->col_format = RGB;
	} /* Erkennung */

/* wie schnell waren wir? */
/*  printf("%lu", get_timer);
	getch(); */

	return(M_PICDONE);
}


/* Here's the routine that will replace the standard error_exit method: */
METHODDEF(void)my_error_exit(j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr)cinfo->err;

/*	printf("Hier Fehler\n");
	getch(); */

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
} /* my_error_exit */


char *read_JPEG_file(char *inpic)
{
  char *outpic, *ooutpic;

  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;

  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;

  /* More stuff */
  JSAMPARRAY buffer;	/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */
  int y, bh, bl;


  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  /* Establish the setjmp return context for my_error_exit to use. */
  if(setjmp(jerr.setjmp_buffer))
  {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    Mfree(inpic);
    return(NULL);
  }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */
  jpeg_stdio_src(&cinfo, inpic);

  /* Step 3: read file parameters with jpeg_read_header() */
  (void)jpeg_read_header(&cinfo, TRUE);

  width = cinfo.image_width;
  height = cinfo.image_height;
  if(cinfo.num_components == 3)
	  BitsPerPixel = 24;
  else
	  BitsPerPixel = 8;
  strcpy(impmessag, "JFIF ");
  strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
  strcat(impmessag, " Bit");
  Services->reset_busybox(0, impmessag);

  /* Step 4: set parameters for decompression */
  cinfo.dct_method = JDCT_IFAST;			/* fast integer dct */

  /* Step 5: Start decompressor */
  (void)jpeg_start_decompress(&cinfo);

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 

  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW));
  buffer[0] = (JSAMPROW)malloc(row_stride);

  if((outpic = (char *)Malloc((long)row_stride * (long)cinfo.output_height)) == 0)
  {
    jpeg_destroy_decompress(&cinfo);
    return((char *)M_MEMORY);
  }
  ooutpic = outpic;
  memset(outpic, 0x0, (long)row_stride * (long)cinfo.output_height);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

	bh = height / 16; 				/* busy-height */
	if(bh == 0)
		bh = height;
	bl = 0;							/* busy-length */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */

  y = 0;
  while(cinfo.output_scanline < cinfo.output_height)
  {
	if(!(y%bh))
	{
		Services->busybox(bl);
		bl += 8;
	}

    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

	memcpy(outpic, buffer[0], row_stride);
	outpic += row_stride;

	y++;
  }

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);

  /* Step 8: Release JPEG decompression object */
  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  free(buffer[0]);
  free(buffer);

  outpic = ooutpic;

  return(outpic);
} /* read_JPEG_file */


int is_jpeg(char *buffer)
{
	long x = 0;


/*	printf("Steht Marker 0xffd8 und anschliežendes 0xff am Fileanfang?\n"); */

	if(*(unsigned int *)buffer != 0xffd8 || *(buffer + 2) != 0xff)
	{
/*		printf("Kombination steht nicht am Fileanfang\n"); */
		return(-1);
	}
	else
/*		printf("Kombination steht am Fileanfang\n"); */
		return(0);
/*
	printf("Suche jetzt bis zum Fileende nach 0xffe0\n");

	while(x < (f_len_global - 8) && !(*(unsigned int *)buffer == 0xffe0))
	{
		buffer++;
		x++;
	}

	if(*(unsigned long *)(buffer + 4) != 'JFIF')
	{
		printf("0xffe0 nicht gefunden\n");
		return(-1);
	}
	else
	{
		printf("0xffe0 gefunden\n");
		return(0);
	}
*/
} /* is_jpeg */