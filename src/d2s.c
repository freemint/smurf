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
 * Olaf Piesche, Christian Eyrich, Dale Russell and Jîrg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

#include <tos.h>
#include <ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\sym_gem.h"
#include "..\mod_devl\import.h"
#include "smurfine.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "nvdi5ras.h"

#include "smurfobs.h"
#include "ext_obs.h"

extern int handle;
void vq_scrninfo(int handle, int *work_out);
void getpix_std_line(char *std, char *buf, int planes, long planelen, int howmany);
void nvdi5_raster(SMURF_PIC *picture, char *ziel, int zoom);

/* Dies bastelt direkt ein rol.w #8,d0 inline ein. */
unsigned int swap_word(unsigned int w)
	0xE058;

extern	int bplanes;

typedef struct
{
	char Intel;
	char skiph;
	unsigned long skipv;
	char v;
	char rs, gs, gm;
} SERVICE;


void d24_to_16(char *buffer, unsigned int *ziel, unsigned int width, unsigned int height, SERVICE *service);
void d24_to_24(char *buffer, char *ziel, unsigned int width, unsigned int height, SERVICE *service);
void d16_to_16(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height, SERVICE *service);
void d16_to_24(unsigned int *buffer, char *ziel, unsigned int width, unsigned int height, SERVICE *service);
void d8pp_to_16(char *buffer, unsigned int *ziel, unsigned int width, unsigned int height, SERVICE *service, char *palette);
void d8pp_to_24(char *buffer, char *ziel, unsigned int width, unsigned int height, SERVICE *service, char *palette);
void dstd_to_16(char *buffer, unsigned int *ziel, unsigned int width, unsigned int height, unsigned int realwidth, unsigned int realheight, SERVICE *service, char *palette, char BitsPerPixel);
void dstd_to_24(char *buffer, char *ziel, unsigned int width, unsigned int height, unsigned int realwidth, unsigned int realheight, SERVICE *service, char *palette, char BitsPerPixel);


/*--------------------------------------------------------------*/
/*			Konvertierung fÅr TC- und HC-Screenmodi				*/
/* Falls Bilder <= 16 Bit in einer 16 Bit Auflîsung, oder		*/
/* Bilder <= 24 Bit (also alle) in einer 24 Bit Auflîsung		*/
/* angezeigt werden sollen und nicht Systempalette ausgewÑhlt	*/
/* ist.															*/
/*--------------------------------------------------------------*/
void direct2screen(SMURF_PIC *picture, char *where_to, GRECT *part)
{
	char *buffer, *ziel, *palette,
		 depth, bits;

	unsigned int width, height, endwid, endhgt;

	unsigned long offset;

	int work_out[272];
	int (* func)(int);

	SERVICE service;

/*
	vq_extnd(handle, 1, work_out);
	if(work_out[30]&0x02)				/* Bit 1 gesetzt - neue Rasterfunktionen vorhanden? */
	{
		nvdi5_raster(picture, where_to, picture->zoom);
		return;
	}
*/
	buffer = picture->pic_data;
	width = picture->pic_width;
	height = picture->pic_height;
	depth = picture->depth;
	ziel = where_to;
	palette = picture->palette;

	if(get_cookie('EdDI', (unsigned long *)&func) != 0 && (*func)(0) >= 0x0100)
	{
		vq_scrninfo(handle, work_out);

		/* Anzahl benutzter Bits fÅr Farben */
		if((((unsigned long)work_out[3] << 16) | (unsigned int)work_out[4]) == 32768L)
			bits = 15;
		else
			bits = work_out[2];

		/* Organisations-Dispatcher*/
		switch(work_out[16])
		{
			/* VGA 24 Bit Intel oder */
			/* VGA 32 Bit Intel xbgr */
			case 0: service.Intel = TRUE;
					if(bits == 24)
					{
						service.rs = 0;		/* wird hier miûbraucht und unten als xfirst benutzt */
						service.gs = 0;		/* wird hier miûbraucht und unten als jump benutzt */
					}
					else
					{
						service.rs = 1;		/* wird hier miûbraucht und unten als xfirst benutzt */
						service.gs = 1;		/* wird hier miûbraucht und unten als jump benutzt */
					}
					break;
			/* VGA 15 Bit Intel */
			case 2: service.Intel = TRUE;
					service.rs = 7;
					service.gm = 0xf8;
					service.gs = 2;
					break;
			/* VGA 16 Bit Intel */
			case 3: service.Intel = TRUE;
					service.rs = 8;
					service.gm = 0xfc;
					service.gs = 3;
					break;
			/* VGA 32 Bit Intel bgrx */
			case 8:	service.Intel = TRUE;
					service.rs = 0;			/* wird hier miûbraucht und unten als xfirst benutzt */
					service.gs = 1;			/* wird hier miûbraucht und unten als jump benutzt */
					break;
			/* VGA 15 Bit Motorola */
			case 10:service.Intel = FALSE;
					service.rs = 7;
					service.gm = 0xf8;
					service.gs = 2;
					break;
			case 11:service.Intel = FALSE;
					if(bits == 15)
					/* Falcon */
					{
						service.rs = 8;
						service.gm = 0xf8;
						service.gs = 3;
					}
					else
					/* VGA 16 Bit Motorola */
					{
						/* Wird zwar gefÅllt, aber eigentlich nie gebraucht */
						service.rs = 8;
						service.gm = 0xfc;
						service.gs = 3;
					}
					break;
			/* VGA 24 Bit Motorola oder */
			/* VGA 32 Bit Motorola xrgb */
			case 16:service.Intel = FALSE;
					if(bits == 24)
					{
						service.rs = 0;		/* wird hier miûbraucht und unten als xfirst benutzt */
						service.gs = 0;		/* wird hier miûbraucht und unten als jump benutzt */
					}
					else
					{
						service.rs = 1;		/* wird hier miûbraucht und unten als xfirst benutzt */
						service.gs = 1;		/* wird hier miûbraucht und unten als jump benutzt */
					}
					break;
			/* VGA 32 Bit Motorola rgbx */
			case 24:service.Intel = FALSE;
					service.rs = 0;			/* wird hier miûbraucht und unten als xfirst benutzt */
					service.gs = 1;			/* wird hier miûbraucht und unten als jump benutzt */
					break;
			default:break;
		}
	}

	if(depth == 24)
	{
		/* nach jedem Pixel Åberspringen */
		service.skiph = picture->zoom * 3;

		/* nach jeder Zeile Åberspringen */
		service.skipv = (unsigned long)picture->zoom * (unsigned long)width * 3L;

		if(part != NULL)
		{
			/* Offset zum Bildanfang */
			offset = ((long)part->g_y * (long)width + (long)part->g_x) * 3L;
			buffer += offset;

			service.skipv += (width - part->g_w) * 3L;
			width = part->g_w;
			height = part->g_h;
		}

		/* unter den Tisch gefallene Bytes nachholen */
		service.skipv += (width%(picture->zoom + 1)) * 3;
	}
	else
	{
		/* nach jedem Pixel Åberspringen */
		service.skiph = picture->zoom;

		/* nach jeder Zeile Åberspringen */
		if(picture->format_type == FORM_PIXELPAK)
			service.skipv = (unsigned long)picture->zoom * (unsigned long)width;
		else
			service.skipv = (unsigned long)picture->zoom * ((unsigned long)(width + 7) / 8);

		if(part != NULL)
		{
			/* Offset zum Bildanfang */
			if(picture->format_type == FORM_PIXELPAK)
				offset = (long)part->g_y * (long)width + (long)part->g_x;
			else
				offset = (long)part->g_y * (long)((width + 7) / 8) + (long)(part->g_x + 7) / 8;
			buffer += offset;

			if(picture->format_type == FORM_PIXELPAK)
				service.skipv += width - part->g_w;
			width = part->g_w;
			height = part->g_h;
		}

		/* unter den Tisch gefallene Bytes nachholen */
		if(picture->format_type == FORM_PIXELPAK)
			service.skipv += width%(picture->zoom + 1);
	}

	endwid = width / (picture->zoom + 1);
	endhgt = height / (picture->zoom + 1);

/* wie schnell sind wir? */
/*	init_timer(); */

	/* 16 Bit Bildschirmmodus */
	if(bplanes == 16)
	{
		if(!part)
			Dialog.busy.reset(0, "-> 16 Bit");

		service.v = ((endwid + 15) / 16) * 16L - endwid;			/* Padpixel um auf Vielfaches von 16 zu kommen */

		if(picture->format_type == FORM_STANDARD)
			dstd_to_16(buffer, (unsigned int *)ziel, endwid, endhgt, picture->pic_width, picture->pic_height, &service, palette, depth);
		else
		{
			if(depth == 8)
				d8pp_to_16(buffer, (unsigned int *)ziel, endwid, endhgt, &service, palette);
			else
				if(depth == 16)
					d16_to_16((unsigned int *)buffer, (unsigned int *)ziel, endwid, endhgt, &service);
				else
					if(depth == 24)
						d24_to_16(buffer, (unsigned int *)ziel, endwid, endhgt, &service);
		}
	}
	else
	{
		if(!part)
			if(bplanes == 24)
				Dialog.busy.reset(0, "-> 24 Bit");
			else
				Dialog.busy.reset(0, "-> 32 Bit");

		service.v = (((endwid + 15) / 16) * 16L - endwid) * (bplanes >> 3);		/* Padpixel um auf Vielfaches von 16 zu kommen */

		if(picture->format_type == FORM_STANDARD)
			dstd_to_24(buffer, ziel, endwid, endhgt, picture->pic_width, picture->pic_height, &service, palette, depth);
		else
		{
			if(depth == 8)
				d8pp_to_24(buffer, ziel, endwid, endhgt, &service, palette);
			else
				if(depth == 16)
					d16_to_24((unsigned int *)buffer, ziel, endwid, endhgt, &service);
				else
					if(depth == 24)
						d24_to_24(buffer, ziel, endwid, endhgt, &service);
		}
	}


/* wie schnell waren wir? */
/*	printf("\n%lu", get_timer());
	getch(); */

	return;	
} /* direct2screen */



/* <= 8 Bit Standardformat -> 16 Bit */
void dstd_to_16(char *buffer, unsigned int *ziel, unsigned int width, unsigned int height, unsigned int realwidth, unsigned int realheight, SERVICE *service, char *palette, char BitsPerPixel)
{
	char *line, *pixbuf, *pal,
		 Intel, rs, gs, gm, skiph, v;

	unsigned int *convtab, *oconvtab,
				 x, y, count;

	unsigned long w, planelength, skipv;


	Intel = service->Intel;
	skiph = service->skiph;
	skipv = service->skipv;
	v = service->v;
	rs = service->rs;
	gs = service->gs;
	gm = service->gm;

	count = 1 << BitsPerPixel;
	convtab = (unsigned int *)calloc(1, count << 1);
	oconvtab = convtab;

	/* Vorberechnung */
	x = 0;
	do
	{
		pal = palette + x + x + x;

		if(Intel)
			*convtab++ = swap_word(((*pal++ & 0xf8) << rs) | ((*pal++ & gm) << gs) | (*pal >> 3));
		else
			*convtab++ = ((*pal++ & 0xf8) << rs) | ((*pal++ & gm) << gs) | (*pal >> 3);
	} while(++x < count);

	convtab = oconvtab;


    w = (realwidth + 7) / 8;
    planelength = w * (unsigned long)realheight;   /* LÑnge einer Plane in Bytes */

	pixbuf = (char *)SMalloc(realwidth + 7);
			
	/* Anwendung */
	y = 0;
	do
	{
		memset(pixbuf, 0x0, realwidth);
		getpix_std_line(buffer, pixbuf, BitsPerPixel, planelength, realwidth);
		buffer += w;
		buffer += skipv;
		line = pixbuf;

		x = 0;
		do
		{
			*ziel++ = convtab[*line++];

			line += skiph;
		} while(++x < width);

		ziel += v;
	} while(++y < height);

	free(convtab);
	SMfree(pixbuf);

	return;
} /* dstd_to_16 */


/* <= 8 Bit Standardformat -> 24 Bit */
void dstd_to_24(char *buffer, char *ziel, unsigned int width, unsigned int height, unsigned int realwidth, unsigned int realheight, SERVICE *service, char *palette, char BitsPerPixel)
{
	char *line, *pixbuf, *pal,
		 Intel, val, skiph, v, xfirst, jump;

	unsigned int x, y;

	unsigned long w, planelength, skipv;


	Intel = service->Intel;
	skiph = service->skiph;
	skipv = service->skipv;
	v = service->v;
	xfirst = service->rs;
	jump = service->gs;

    w = (realwidth + 7) / 8;
    planelength = w * (unsigned long)realheight;   /* LÑnge einer Plane in Bytes */

	pixbuf = (char *)SMalloc(realwidth + 7);

	if(xfirst)
		ziel++;
			
	y = 0;
	do
	{
		memset(pixbuf, 0x0, realwidth);
		getpix_std_line(buffer, pixbuf, BitsPerPixel, planelength, realwidth);
		buffer += w;
		buffer += skipv;
		line = pixbuf;

		x = 0;
		do
		{
			val = *line++;					
			pal = palette + val + val + val;

			if(Intel)
			{
				*ziel++ = *(pal + 2);
				*ziel++ = *(pal + 1);
				*ziel++ = *pal;
			}
			else
			{
				*ziel++ = *pal++;
				*ziel++ = *pal++;
				*ziel++ = *pal;
			}

			ziel += jump;								/* 32 Bit? */

			line += skiph;
		} while(++x < width);

		ziel += v;
	} while(++y < height);

	SMfree(pixbuf);

	return;
} /* dstd_to_24 */


/* 8 Bit pixelpacked -> 16 Bit */
void d8pp_to_16(char *buffer, unsigned int *ziel, unsigned int width, unsigned int height, SERVICE *service, char *palette)
{
	char *pal,
		 Intel, rs, gs, gm, skiph, v;

	unsigned int *convtab, *oconvtab,
				 x, y;

	unsigned long skipv;


	Intel = service->Intel;
	skiph = service->skiph;
	skipv = service->skipv;
	v = service->v;
	rs = service->rs;
	gs = service->gs;
	gm = service->gm;

	convtab = (unsigned int *)calloc(1, 256 * 2);
	oconvtab = convtab;

	/* Vorberechnung */
	x = 0;
	do
	{
		pal = palette + x + x + x;

		if(Intel)
			*convtab++ = swap_word(((*pal++ & 0xf8) << rs) | ((*pal++ & gm) << gs) | (*pal++ >> 3));
		else
			*convtab++ = ((*pal++ & 0xf8) << rs) | ((*pal++ & gm) << gs) | (*pal++ >> 3);
	} while(++x < 256);

	convtab = oconvtab;

	/* Anwendung */
	y = 0;
	do
	{
		x = 0;
		do
		{
			*ziel++ = convtab[*buffer++];

			buffer += skiph;
		} while(++x < width);

		buffer += skipv;
		ziel += v;
	} while(++y < height);

	free(convtab);

	return;
} /* d8pp_to_16 */


/* 8 Bit pixelpacked -> 24 Bit */
void d8pp_to_24(char *buffer, char *ziel, unsigned int width, unsigned int height, SERVICE *service, char *palette)
{
	char *pal,
		 Intel, skiph, v, val, xfirst, jump;

	unsigned int x, y;

	unsigned long skipv;


	Intel = service->Intel;
	skiph = service->skiph;
	skipv = service->skipv;
	v = service->v;
	xfirst = service->rs;
	jump = service->gs;


	if(xfirst)
		ziel++;

	y = 0;
	do
	{
		x = 0;
		do
		{
			val = *buffer++;
			pal = palette + val + val + val;

			if(Intel)
			{
				*ziel++ = *(pal++ + 2);
				*ziel++ = *pal++;
				*ziel++ = *(pal++ - 2);
			}
			else
			{
				*ziel++ = *pal++;
				*ziel++ = *pal++;
				*ziel++ = *pal++;
			}

			ziel += jump;								/* 32 Bit? */

			buffer += skiph;
		} while(++x < width);

		buffer += skipv;
		ziel += v;
	} while(++y < height);

	return;
} /* d8pp_to_24 */


/* 16 Bit -> 16 Bit */
void d16_to_16(unsigned int *buffer, unsigned int *ziel, unsigned int width, unsigned int height, SERVICE *service)
{
	char Intel, rs, gs, gm, skiph, v, r, g, b;

	unsigned int x, y, val;

	unsigned long skipv;


	Intel = service->Intel;
	skiph = service->skiph;
	skipv = service->skipv;
	v = service->v;
	rs = service->rs;
	gs = service->gs;
	gm = service->gm;


	if(rs == 8 && !Intel)				/* Falcon oder VGA 16 Bit Motorola */
	{
		y = 0;
		do
		{
			x = 0;
			do
			{
				*ziel++ = *buffer++;

				buffer += skiph;
			} while(++x < width);
			buffer += skipv;
			ziel += v;
		} while(++y < height);
	} /* Falcon-Sonderfall */
	else								/* VGA 15 Bit Motorola und Intel sowie VGA 16 Bit Intel */
	{
		y = 0;
		do
		{
			x = 0;
			do
			{
				val = *buffer++;
				r = (val & 0xf800) >> 8;
				g = (val & 0x7e0) >> 3;
				b = (val & 0x1f) << 3;

				if(Intel)
					*ziel++ = swap_word(((r & 0xf8) << rs) | ((g & gm) << gs) | (b >> 3));
				else
					*ziel++ = ((r & 0xf8) << rs) | ((g & gm) << gs) | (b >> 3);
				
				buffer += skiph;
			} while(++x < width);

			buffer += skipv;
			ziel += v;
		} while(++y < height);
	}

	return;
} /* d16_to_16 */


/* 16 Bit -> 24 Bit */
void d16_to_24(unsigned int *buffer, char *ziel, unsigned int width, unsigned int height, SERVICE *service)
{
	char Intel, skiph, v, xfirst, jump;

	unsigned int x, y, val;

	unsigned long skipv;


	Intel = service->Intel;
	skiph = service->skiph;
	skipv = service->skipv;
	v = service->v;
	xfirst = service->rs;
	jump = service->gs;


	if(xfirst)
		ziel++;

	y = 0;
	do
	{
		x = 0;
		do
		{
			val = *buffer++;
			if(Intel)
			{
				*ziel++ = (val & 0x1f) << 3;
				*ziel++ = (val & 0x7e0) >> 3;
				*ziel++ = (val & 0xf800) >> 8;
			}
			else
			{
				*ziel++ = (val & 0xf800) >> 8;
				*ziel++ = (val & 0x7e0) >> 3;
				*ziel++ = (val & 0x1f) << 3;
			}

			ziel += jump;								/* 32 Bit? */
			
			buffer += skiph;
		} while(++x < width);

		buffer += skipv;
		ziel += v;
	} while(++y < height);

	return;
} /* d16_to_24 */


/* 24 Bit -> 16 Bit */
void d24_to_16(char *buffer, unsigned int *ziel, unsigned int width, unsigned int height, SERVICE *service)
{
	char Intel, rs, gs, gm, skiph, v;

	unsigned int x, y;

	unsigned long skipv;


	Intel = service->Intel;
	skiph = service->skiph;
	skipv = service->skipv;
	v = service->v;
	rs = service->rs;
	gs = service->gs;
	gm = service->gm;


	y = 0;
	do
	{
		x = 0;
		do
		{
			if(Intel)
				*ziel++ = swap_word(((*buffer++ & 0xf8) << rs) | ((*buffer++ & gm) << gs) | (*buffer++ >> 3));
			else
				*ziel++ = ((*buffer++ & 0xf8) << rs) | ((*buffer++ & gm) << gs) | (*buffer++ >> 3);

			buffer += skiph;
		} while(++x < width);

		buffer += skipv;
		ziel += v;
	} while(++y < height);

	return;
} /* d24_to_16 */


/* 24 Bit -> 24 Bit */
void d24_to_24(char *buffer, char *ziel, unsigned int width, unsigned int height, SERVICE *service)
{
	char Intel, skiph, v, xfirst, jump;

	unsigned int x, y;

	unsigned long skipv;


	Intel = service->Intel;
	skiph = service->skiph;
	skipv = service->skipv;
	v = service->v;
	xfirst = service->rs;
	jump = service->gs;


	if(xfirst)
		ziel++;

	y = 0;
	do
	{
		x = 0;
		do
		{
			if(Intel)
			{
				*ziel++ = *(buffer++ + 2);
				*ziel++ = *buffer++;
				*ziel++ = *(buffer++ - 2);
			}
			else
			{
				*ziel++ = *buffer++;
				*ziel++ = *buffer++;
				*ziel++ = *buffer++;
			}

			ziel += jump;								/* 32 Bit? */

			buffer += skiph;
		} while(++x < width);

		buffer += skipv;
		ziel += v;
	} while(++y < height);

	return;
} /* d24_to_24 */