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
/*#include <multiaes.h>*/
#include <aes.h>
#include <vdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "smurf.h"
#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "popdefin.h"
#include "smurf_st.h"
#include "smurf_f.h"
#include "destruct.h"
#include "vaproto.h"
#include "smurfobs.h"
#include "ext_obs.h"
#include "demolib.h"


extern	void blockfunctions_off(void);
extern	void f_display_bwh(WINDOW *pic_window);

extern	SMURF_PIC *smurf_picture[25];
extern	int active_pic;
extern	WINDOW picture_windows[25];

extern	SYSTEM_INFO	Sys_info;			/* Systemkonfiguration */
extern	DISPLAY_MODES Display_Opt;
extern	int	mouse_xpos, mouse_ypos;		/* Mausposition */
extern	int	openmode;					/* Dialog neu geîffnet (0) oder buttonevent? (!=0) */
extern	POP_UP	popups[25];
extern	OBJECT	*blockpopup;

extern	int	mouse_button, key_at_event;
extern	int	key_scancode;				/* Scancode beim letzten Keyboard-Event */

extern	WINDOW	wind_s[25];

extern	int	obj;

/*------------ kloake Funktionen -----------*/
void rearrange_line2(char *src, char *dst, long bytes, unsigned int pixels);
void *copyblock(SMURF_PIC *old_pic);
void get_blockcoords(SMURF_PIC *pic, int button);
void insert_blockcoords(SMURF_PIC *pic);
int clear_scrap(void);
int insert_block(WINDOW *picwindow);
int intersect_block(SMURF_PIC *picture);

long block_opac, opac2;

GRECT block;

void insertline_replace(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2);
void insertline_add(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2);
void insertline_clipadd(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2);
void insertline_sub(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2);
void insertline_clipsub(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2);
void insertline_mult(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2);

/******************************************************************	*/
/*							Block freistellen						*/
/******************************************************************	*/
void block_freistellen(WINDOW *pwindow)
{
	char *dest_pic, *picdata;

	SMURF_PIC *old_pic;


	if(pwindow->picture->blockwidth == 0 || pwindow->picture->blockheight == 0)
		return;

	old_pic = pwindow->picture;
	picdata = old_pic->pic_data;

	if(old_pic->block)
	{
		destroy_smurfpic(old_pic->block);
		old_pic->block = NULL;
	}

	dest_pic = copyblock(old_pic);
	if(dest_pic == NULL)
		return;

	old_pic->pic_width = block.g_w;
	old_pic->pic_height = block.g_h;

	old_pic->blockwidth = 0;
	old_pic->blockheight = 0;
	old_pic->pic_data = dest_pic;

	SMfree(picdata);

	if(!Sys_info.realtime_dither)
		f_dither(old_pic, &Sys_info, 1, NULL, &Display_Opt);
	else
	{
		if(old_pic->screen_pic)
		{
			SMfree(old_pic->screen_pic->fd_addr);
			SMfree(old_pic->screen_pic);
		}

		if(old_pic->local_nct)
		{
			SMfree(old_pic->local_nct);
			old_pic->local_nct = NULL;
		}
		old_pic->screen_pic=NULL;
	}
		
	pwindow->xoffset = pwindow->yoffset = 0;
	imageWindow.clipPicwin(pwindow);
	Window.redraw(pwindow, NULL, 0, 0);
	f_pic_changed(pwindow, 1);

/*	f_activate_pic(pwindow->wnum); */

	Dialog.busy.ok();
	Dialog.busy.dispRAM();

	return;
} /* block_freistellen */


/* ------------------------------------------------------------	*/
/*					Block -> Clipboard kopieren					*/
/*	mode=0: kopieren, mode=1: ausschneiden						*/
/* ------------------------------------------------------------	*/
int block2clip(SMURF_PIC *picture, int mode, char *path)
{
	char *dest_pic;
	char AND_byte1, AND_byte2;

	int bx, by, bh, bw;
	int plane, y, msdiff;

	long depth;
	long bytes_blockline, bytes_per_line, bytes_set, yoff, planeoff, planelen;

	EXPORT_PIC *pic_to_save;



	if(encode_block(picture, &pic_to_save) != 0)
		return(-1);

	save_block(pic_to_save, path);
	
	Dialog.busy.enable();

	depth = picture->depth;

	/*
	 * Soll der Block ausgeschnitten werden? (->lîschen im Bild) 
	 */
	if(mode == 1 && picture->block == NULL)
	{
		bx = block.g_x;
		by = block.g_y;
		bw = block.g_w;
		bh = block.g_h;

		if(picture->format_type == FORM_STANDARD) 
		{
			bytes_blockline = (bw + 7) >> 3;						/* Bytes pro Blocklinie */
			bytes_per_line = (picture->pic_width + 7) >> 3;			/* Bytes pro Bildlinie */
			planelen = bytes_per_line*picture->pic_height;
			
			msdiff = (bx + bw) - (((bx + 7) >> 3) << 3);
			/* Bytes die gesetzt werden mÅssen */
			bytes_set = msdiff >> 3;
			
			AND_byte1 = 0xff << ((((bx + 7) >> 3) << 3) - bx);
			AND_byte2 = 0xff >> (msdiff - ((msdiff >> 3) << 3));

			dest_pic = picture->pic_data;
			dest_pic += (((bx + 7) >> 3)) + (by * bytes_per_line);

			planeoff = 0;
			for(plane = 0; plane < picture->depth; plane++)
			{
				yoff = 0;
				for(y = 0; y < bh; y++)
				{
					if(AND_byte1 != 0xFF) 
						*(dest_pic + yoff + planeoff - 1) &= AND_byte1;

					if(bytes_set > 0)
						memset(dest_pic + yoff + planeoff, 0, bytes_set);

					if(AND_byte2 != 0xFF)
						*(dest_pic + yoff + planeoff + bytes_set) &= AND_byte2;

					yoff += bytes_per_line;
				}

				planeoff += planelen;
			}
		}
		else
		{
			bytes_blockline = bw * (depth >> 3);
			bytes_per_line = (picture->pic_width) * (depth >> 3);

			dest_pic = picture->pic_data;
			dest_pic += (long)(bx*(depth >> 3)) + ((long)by * bytes_per_line);

			yoff = 0;
			for(y = 0; y < bh; y++)
			{
				memset(dest_pic + yoff, 0x0, bytes_blockline);
				yoff += bytes_per_line;
			}
		}
	}
	else
		if(mode == 1 && picture->block != NULL)
		{
			destroy_smurfpic(picture->block);
			picture->block = NULL;
		}
		
	Dialog.busy.ok();
	Dialog.busy.dispRAM();

	return(0);
} /* block2clip */


/* ------------------------------------------------------------	*/
/*		clear_scrap - Clipboard komplett lîschen 				*/
/* ------------------------------------------------------------	*/
int clear_scrap(void)
{
	char *filename;

	DTA *dta;


	if(Sys_info.scrp_path == NULL)
		return(-1);
	
	filename = malloc(257);

	strcpy(filename, Sys_info.scrp_path);
	strcat(filename, "scrap.*");

	/*------------ und jetzt lîschen */
	dta = Fgetdta();
	if(Fsfirst(filename, 0) == 0)
		do
		{
			strcpy(filename, Sys_info.scrp_path);
			strcat(filename, dta->d_fname);
			Fdelete(filename);
		} while(Fsnext() == 0);
	
	free(filename);
	
	return(0);
} /* clear_scrap */


/* ------------------------------------------------------------	*/
/*			copyblock - kopiert einen Block aus einer			*/
/*	SMURF_PIC-Struktur in einen neuen Speicherblock (return).	*/
/* ------------------------------------------------------------	*/
void *copyblock(SMURF_PIC *old_pic)
{
	char *picdata, *dest_pic;
	char format;

	int bx, by, bh, bw, y, plane;
	int depth;

	long bytes_per_pixel;
	long bytes_per_line, bytes_blockline;
	long syoff, src_off, dest_off=0; 
	long src_planelen, dest_planelen;
	long new_mem;
	long xoff, bytes_copy, pixels, soff, doff;


	if(!intersect_block(old_pic))
		return(NULL);

	/*--------------------------- Strukturen auslesen */
	depth = old_pic->depth;
	format = old_pic->format_type;
	bx = block.g_x;
	by = block.g_y;
	bw = block.g_w;
	bh = block.g_h;
	picdata = old_pic->pic_data;
	bytes_per_pixel = (long)depth / 8L;

	/*--------------------------- Vorberechnungen */
	if(format == FORM_PIXELPAK)
	{
		bytes_per_line = bytes_per_pixel * (long)(old_pic->pic_width);
		bytes_blockline = bytes_per_pixel * (long)(bw);

		new_mem = (long)bh * bytes_blockline;
	}
	else
	{
		src_planelen = (long)((old_pic->pic_width + 7) >> 3) * (long)old_pic->pic_height;
		bytes_blockline = (bw + 7) >> 3;
		bytes_copy = bytes_blockline + 1;
		dest_planelen = (long)bytes_blockline * (long)bh;
		bytes_per_line = (old_pic->pic_width + 7) >> 3;
	
		pixels = 8 - bx%8;

		new_mem = (long)bh * bytes_copy * (long)depth;
	}
	
	dest_pic = SMalloc(new_mem + 1024);		/* wie immer 1KB mehr */
	if(dest_pic == NULL)
	{
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_MEM].TextCast, NULL, NULL, NULL, 1);
		return(NULL);
	}

	if(format == FORM_PIXELPAK)
	{
		syoff = (long)by * bytes_per_line;
		src_off = (long)syoff + ((long)bx * bytes_per_pixel);

		for(y = 0; y < bh; y++)
		{
			memcpy(dest_pic + dest_off, picdata + src_off, bytes_blockline);
			
			src_off += bytes_per_line;
			dest_off += bytes_blockline;
		}
	}
	else
	{
		xoff = bx >> 3;
		syoff = (long)by * bytes_per_line;
		src_off = (long)syoff + (long)xoff;

		for(plane=0; plane<depth; plane++)
		{
			soff = src_off;
			doff = dest_off;

			for(y = 0; y < bh; y++)
			{
				rearrange_line2(picdata + soff, dest_pic + doff, bytes_copy, (unsigned int)pixels);
				soff += bytes_per_line;
				doff += bytes_blockline;
			}

			src_off += src_planelen;
			dest_off += dest_planelen;
		}
	}

	Dialog.busy.ok();
	Dialog.busy.dispRAM();

	return(dest_pic);
} /* copyblock */


/* ------------------------------------------------------------	*/
/*			Dialog zum Eingeben der Blockkoordinaten						*/
/* ------------------------------------------------------------	*/
void block_type_in(void)
{
	int button=0;
	int my_scancode;

	extern	void blockfunctions_on(void);

	OBJECT *bwindow;
	SMURF_PIC *pic;
	WINDOW *blockwin;


	my_scancode=(key_scancode&0xff00) >> 8;

	pic = smurf_picture[active_pic];

	blockwin = &wind_s[WIND_BTYPEIN];
	bwindow = wind_s[WIND_BTYPEIN].resource_form;

	/*----- Blockkoordinaten ins Formular einfÅgen ----*/
	if(!openmode || openmode == 2)
		insert_blockcoords(pic);

	if(my_scancode != KEY_UP && my_scancode != KEY_DOWN)
			button = Dialog.init(WIND_BTYPEIN, BLOCK_OK);

	if(button != -1)
	{
		if(button == BLOCK_XLO || button == BLOCK_YLO ||
		   button == BLOCK_WID || button == BLOCK_HGT ||
		   button == BLOCK_XRU || button == BLOCK_YRU)
			get_blockcoords(pic, button);

		switch(button)
		{
			case BLOCK_CLR:	Window.cursorOff(blockwin);
							imageWindow.removeBlock(&picture_windows[active_pic]);
							insert_blockcoords(pic);
							change_object(blockwin, BLOCK_CLR, UNSEL, 1);
							blockfunctions_off();
							break;

			case BLOCK_OK:	change_object(blockwin, BLOCK_OK, UNSEL, 1);
							Dialog.close(WIND_BTYPEIN);
							break;

			case BLOCK_XRU:	f_txtinsert(pic->blockwidth, bwindow, BLOCK_WID, blockwin);
							break;

			case BLOCK_YRU:	f_txtinsert(pic->blockheight, bwindow, BLOCK_HGT, blockwin);
							break;
	
			case BLOCK_XLO:
			case BLOCK_WID:	if(pic->blockwidth)
								f_txtinsert(pic->blockwidth + pic->blockx - 1, bwindow, BLOCK_XRU, blockwin);
							else 
								f_txtinsert(0, bwindow, BLOCK_XRU, blockwin);
							break;

			case BLOCK_YLO:
			case BLOCK_HGT:	if(pic->blockheight)
								f_txtinsert(pic->blockheight + pic->blocky - 1, bwindow, BLOCK_YRU, blockwin);
							else 
								f_txtinsert(0, bwindow, BLOCK_YRU, blockwin);
							break;
		}
	}

	f_display_bwh(&picture_windows[active_pic]);

	if(pic->blockwidth == 0 || pic->blockheight == 0)
		blockfunctions_off();
	else
		blockfunctions_on();

	return;
} /* block_type_in */


/* get_blockcoords --------------------------------------------------------------
	Åbernimmt die im Blockeingabefenster eingetragenen Blockkoordinaten in die
	Åbergebene SMURF_PIC-Struktur und zeichnet das aktive Bildfenster neu.	
	-----------------------------------------------------------------------------*/
void get_blockcoords(SMURF_PIC *pic, int button)
{
	long val;

	WINDOW *blockwin;
	OBJECT *bwindow;


	blockwin = &wind_s[WIND_BTYPEIN];
	bwindow = blockwin->resource_form;

	/*
	 * alte Blockbox lîschen
	 */
	Window.redraw(&picture_windows[active_pic], NULL, 0, DRAWNOTREE|BLOCK_ONLY|DRAWNOBLOCK);

	switch(button)
	{
		case BLOCK_XLO:
			val	= atol(bwindow[BLOCK_XLO].TextCast);
			if(val > 32767 || val + pic->blockwidth > 32767)
			{
				val = pic->blockx;
		
				if(!(Sys_info.profi_mode&SELECTED))
					Dialog.winAlert.openAlert("Blîcke nur mit Maûen und Koordinaten bis 32767!", NULL, NULL, NULL, 1);
				f_txtinsert((int)val, bwindow, BLOCK_XLO, blockwin);
			}
			pic->blockx = (int)val;
			break;

		case BLOCK_YLO:
			val = atol(bwindow[BLOCK_YLO].TextCast);
			if(val > 32767 || val + pic->blockheight > 32767)
			{
				val = pic->blocky;
		
				if(!(Sys_info.profi_mode&SELECTED))
					Dialog.winAlert.openAlert("Blîcke nur mit Maûen und Koordinaten bis 32767!", NULL, NULL, NULL, 1);
				f_txtinsert((int)val, bwindow, BLOCK_YLO, blockwin);
			}
			pic->blocky = (int)val;
			break;

		case BLOCK_WID:
			val = atol(bwindow[BLOCK_WID].TextCast);
			if(val > 32767 || pic->blockx + val > 32767)
			{
				val = pic->blockwidth;
		
				if(!(Sys_info.profi_mode&SELECTED))
					Dialog.winAlert.openAlert("Blîcke nur mit Maûen und Koordinaten bis 32767!", NULL, NULL, NULL, 1);
				f_txtinsert((int)val, bwindow, BLOCK_WID, blockwin);
			}
			pic->blockwidth = (int)val;
			break;

		case BLOCK_HGT:
			val = atol(bwindow[BLOCK_HGT].TextCast);
			if(val > 32767 || pic->blocky + val > 32767)
			{
				val = pic->blockheight;
				if(!(Sys_info.profi_mode&SELECTED))
					Dialog.winAlert.openAlert("Blîcke nur mit Maûen und Koordinaten bis 32767!", NULL, NULL, NULL, 1);
				f_txtinsert((int)val, bwindow, BLOCK_HGT, blockwin);
			}
			pic->blockheight = (int)val;
			break;

		case BLOCK_XRU:
			val = atol(bwindow[BLOCK_XRU].TextCast);
			if(val > 32767)
			{
				val = pic->blockx + pic->blockwidth - 1;
				if(!(Sys_info.profi_mode&SELECTED))
					Dialog.winAlert.openAlert("Blîcke nur mit Maûen und Koordinaten bis 32767!", NULL, NULL, NULL, 1);
				f_txtinsert((int)val, bwindow, BLOCK_XRU, blockwin);
			}
			val = val + 1 - pic->blockx;
			pic->blockwidth = (int)val;
			break;

		case BLOCK_YRU:
			val = atol(bwindow[BLOCK_YRU].TextCast);
			if(val > 32767)
			{
				val = pic->blocky + pic->blockheight - 1;
				if(!(Sys_info.profi_mode&SELECTED))
					Dialog.winAlert.openAlert("Blîcke nur mit Maûen und Koordinaten bis 32767!", NULL, NULL, NULL, 1);
				f_txtinsert((int)val, bwindow, BLOCK_YRU, blockwin);
			}
			val = val + 1 - pic->blocky;
			pic->blockheight = (int)val;
			break;
	}

	/*
	 * Blockbox neu zeichnen
	 */
	Window.redraw(&picture_windows[active_pic], NULL, 0, DRAWNOTREE|BLOCK_ONLY|DRAWNOBLOCK);

	return;
} /* get_blockcoords */


/* insert_blockcoords --------------------------------------------------------------
	fÅgt die Blockkoordinaten aus pic in den Blockeingabe-Dialog ein. Redraw der
	Editfelder wird von f_txtinsert Åbernommen.
	-------------------------------------------------------------------------------*/
void insert_blockcoords(SMURF_PIC *pic)
{
	WINDOW *blockwin;
	OBJECT *bwindow;


	blockwin = &wind_s[WIND_BTYPEIN];
	bwindow = blockwin->resource_form;

	f_txtinsert(pic->blockx, bwindow, BLOCK_XLO, blockwin);
	f_txtinsert(pic->blocky, bwindow, BLOCK_YLO, blockwin);
	f_txtinsert(pic->blockwidth, bwindow, BLOCK_WID, blockwin);
	f_txtinsert(pic->blockheight, bwindow, BLOCK_HGT, blockwin);

	if(pic->blockwidth)
		f_txtinsert(pic->blockwidth + pic->blockx - 1, bwindow, BLOCK_XRU, blockwin);
	else
		f_txtinsert(0, bwindow, BLOCK_XRU, blockwin);
	
	if(pic->blockheight)
		f_txtinsert(pic->blockheight + pic->blocky - 1, bwindow, BLOCK_YRU, blockwin);
	else
		f_txtinsert(0, bwindow, BLOCK_YRU, blockwin);

	return;
} /* insert_blockcoords */


/* void block_dklick(WINDOW *picwindow) ------------------------------------
	handlet den Doppelklick in einen Blockrahmen (Aufnehmen bzw. Ablegen
	des Blocks inkl. Rauskopieren, evtl. Neudithern usw.
	------------------------------------------------------------------------ */
void block_dklick(WINDOW *picwindow)
{
	char *block_data;

	int back;

	SMURF_PIC *picture, *picblock;

	extern BLOCKMODE blockmode_conf;


	picture = picwindow->picture;

	/*
	 * aufnehmen in leeren Blockrahmen 
	 */
	if(picture->block == NULL)
	{
		block_data = copyblock(picture);						/* Daten kopieren */
		if(block_data == NULL)
			return;

		picture->blockx = block.g_x;
		picture->blocky = block.g_y;
		picture->blockwidth = block.g_w;
		picture->blockheight = block.g_h;

		picblock = SMalloc(sizeof(SMURF_PIC));
		memcpy(picblock, picture, sizeof(SMURF_PIC));

		picblock->pic_width = picture->blockwidth;
		picblock->pic_height = picture->blockheight;
		picblock->palette = malloc(1025);
		memcpy(picblock->palette, picture->palette, 1024);
		memcpy(picblock->red, picture->red, 256 * 2);
		memcpy(picblock->grn, picture->grn, 256 * 2);
		memcpy(picblock->blu, picture->blu, 256 * 2);
		picblock->local_nct = picture->local_nct;
		picblock->not_in_nct = picture->not_in_nct;
		picblock->block = NULL;
		picblock->zoom = picture->zoom;
		picblock->screen_pic = NULL;
		picblock->pic_data = block_data;
		picblock->changed = 255;
		picture->block = picblock;

		/*
		 * wenn RT-dither an und nicht ersetzen oder transparent,
		 * dann muû der Blockbereich fÅrs Preview neu gezeichnet werden.
		 */
		if(Sys_info.realtime_dither && (blockmode_conf.mode != BCONF_REPLACE || blockmode_conf.opacity != 100))
		{
			Window.redraw(picwindow, NULL, 0, BLOCK_ONLY|DRAWNOTREE|DRAWNOBLOCK);
			Window.redraw(picwindow, NULL, 0, BLOCK_ONLY);
		}
		else
			f_dither(picblock, &Sys_info, 0, NULL, &Display_Opt);
			
		Dialog.busy.ok();
	}
	/*
	 * Oder einfÅgen und neu dithern. Der Block selbst wird nur mit ReDithered, wenn sich
	 * seine Zielpalette (Darstellung) von der neuen des Bildes unterscheidet, also
	 * wohl meistens beim Median Cut, und auch nur wenn der RTdither aus ist.
	 */
	else
	{
		back = insert_block(picwindow);
		if(back == 0)
		{
			if(!Sys_info.realtime_dither)
				f_dither(picture, &Sys_info, 1, NULL, &Display_Opt);
			else
			{
/*
				redraw.g_x = picture->blockx;
				redraw.g_y = picture->blocky;
				redraw.g_w = picture->blockwidth;
				redraw.g_h = picture->blockheight;

				Window.redraw(picwindow, redraw, 0, 0);
*/
				Window.redraw(picwindow, NULL, 0, BLOCK_ONLY|DRAWNOTREE|DRAWNOBLOCK);
				Window.redraw(picwindow, NULL, 0, BLOCK_ONLY);
			}
				
			if(active_pic == picwindow->wnum)
				f_pic_changed(picwindow, 1);
		
			imageWindow.toggleAsterisk(picwindow, 1);
		}

		if(Sys_info.realtime_dither &&
		   (memcmp(picture->red, picture->block->red, 512) != 0 ||
		    memcmp(picture->grn, picture->block->grn, 512) != 0 ||
		    memcmp(picture->blu, picture->block->blu, 512) !=0 ))
		{
			memcpy(picture->block->red, picture->red, 512);
			memcpy(picture->block->grn, picture->grn, 512);
			memcpy(picture->block->blu, picture->blu, 512);
			picture->block->local_nct = picture->local_nct;
			picture->block->not_in_nct = picture->not_in_nct;
			
			f_dither(picture->block, &Sys_info, 1, NULL, &Display_Opt);
		}

		Dialog.busy.ok();
	}

	return;
} /* block_dklick */


/* clip2block() ------------------------------------------------------------------
	fÅgt ein IMG aus dem Clipboard in den Block des Bildes picture ein.
	Ein Blockrahmen wird von der Funktion erzeugt bzw. ein vorhandener angepaût.
	Ein evtl. im vorhandenen Blockrahmen liegender Block wird verworfen.
	Der Ditherdispatcher wird mit gefakter DISPLAY_OPT aufgerufen, um die Dithereinstellungen
	des Zielbildes zu Åbernehmen, die Zielpalette wird direkt aus diesem kopiert.
	Ist data = NULL wird das Clipboard selbstÑnding ausgelesen, bei != NULL die Åbergebenen
	Daten als Block genommen.
	---------------------------------------------------------------------------------*/
void clip2block(SMURF_PIC *picture, char *data, int mx, int my)
{
	char clip_path[256];

	int pic_dithermode, pic_palmode;
	DISPLAY_MODES myDisplay;
	

	if(!data)
	{
		strcpy(clip_path, Sys_info.scrp_path);			/* Clipboardpfad holen */
		strcat(clip_path, "scrap.img");
	
		data = fload(clip_path, 0);
		if(data == NULL)
			return;
	}
	else
		strcpy(clip_path, "block.img");

	if(picture->block != NULL)							/* evtl. vorhandenen Block verwerfen */
		destroy_smurfpic(picture->block);

	picture->block = SMalloc(sizeof(SMURF_PIC));		/* Speicher fÅr neuen Block anfordern */

	picture->block->pic_data = data;
	picture->block->local_nct = picture->local_nct;
	picture->block->not_in_nct = picture->not_in_nct;
	memcpy(picture->block->red, picture->red, 256 * 2);	/* Palette aus dem Zielbild kopieren */
	memcpy(picture->block->grn, picture->grn, 256 * 2);
	memcpy(picture->block->blu, picture->blu, 256 * 2);

	picture->block->changed = 255;						/* Kennzeichnung als Block */

	strcpy(picture->block->filename, clip_path);

	/*
	 * importieren des Clipboards
	 */
	if(f_import_pic(picture->block, "img") == M_PICDONE)
	{
		if(picture->blockwidth==0 && picture->blockheight==0)	/* Block zentrieren, wenn neu */
		{
			if(mx == -1 || my == -1)
			{
				picture->blockx = picture->pic_width / 2 - picture->block->pic_width / 2;
				picture->blocky = picture->pic_height / 2 - picture->block->pic_height / 2;
			}
			else
			{
				picture->blockx = mx;
				picture->blocky = my;
			}
		}

		/*
		 * Dither- und Palettenmodus des Zielbildes ermitteln
		 */
		if(picture->depth <= 24 && picture->depth >= 16)
		{
			pic_palmode = Display_Opt.syspal_24;
			pic_dithermode = Display_Opt.dither_24;
		}
		else
			if(picture->depth <= 8 && picture->depth >= 5)
			{
				pic_palmode = Display_Opt.syspal_8;
				pic_dithermode = Display_Opt.dither_8;
			}
			else
				if(picture->depth <= 4)
				{
					pic_palmode = Display_Opt.syspal_4;
					pic_dithermode = Display_Opt.dither_4;
				}
		
		/*
		 * Display auf den Modus des Bildes anpassen (myDisplay.[modus] = Zielbildmodus)
		 */
		if(picture->block->depth <= 24 && picture->block->depth >= 16)
		{
			myDisplay.syspal_24 = pic_palmode;
			myDisplay.dither_24 = pic_dithermode;
		}
		else
			if(picture->block->depth <= 8 && picture->block->depth >= 5)
			{
				myDisplay.syspal_8 = pic_palmode;
				myDisplay.dither_8 = pic_dithermode;
			}
			else
				if(picture->block->depth <= 4)
				{
					myDisplay.syspal_4 = pic_palmode;
					myDisplay.dither_4 = pic_dithermode;
				}

		picture->blockwidth = picture->block->pic_width;
		picture->blockheight = picture->block->pic_height;
		picture->block->zoom = picture->zoom;
		f_dither(picture->block, &Sys_info, 0, NULL, &myDisplay);
	}

	Dialog.busy.ok();
	Dialog.busy.dispRAM();

	return;
} /* clip2block */


/* insert_block() -------------------------------------------------------------
	Rechnet den Block aus dem Bild picwindow->picture in selbiges ein.
	Der Blockmodus wird berÅcksichtigt. RÅckgabe: 0=eingefÅgt, 1=Fehler
	---------------------------------------------------------------------------*/
int insert_block(WINDOW *picwindow)
{
	SMURF_PIC *pic, *block, blockLinePic;
	MOD_ABILITY	conv_abs;
	int	y, plane;
	int	picwid, pichgt, blockwid, blockhgt;
	int	picdepth, blockdepth;
	int	endx, endy, begx, begy;
	int	FULLR, FULLG, FULLB;
	unsigned char *picdata, *blockdata, *currpic, *currblock;
	int bpp_pic, bpp_block;

	char *buf, *buf_copy;
	long plen_block;

	int dbits_left, dbits_right;
	char copy_byte, right_byte = 0xFF, left_byte = 0xFF;		/* und fÅr Standardformat-Ziel */
	int st_piclinelen, st_blocklinelen;
	long plen_pic;
	char *linebuf;

	extern BLOCKMODE blockmode_conf;
	extern	void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);


	/*
	 * Strukturen auslesen
	 */
	pic = picwindow->picture;
	block = pic->block;

	picwid = pic->pic_width;
	pichgt = pic->pic_height;
	picdepth = pic->depth;

	blockwid = block->pic_width;
	blockhgt = block->pic_height;
	blockdepth = block->depth;
	
	if(block->format_type == FORM_STANDARD)
		plen_block = (blockwid + 7) / 8 * blockhgt;
	if(pic->format_type == FORM_STANDARD)
		plen_pic = (picwid + 7) / 8 * pichgt;


	block_opac = ((long)blockmode_conf.opacity * 255L) / 100L;
	opac2 = 255 - block_opac;

	/*
	 * Blockbeschneidung (aua...) 
	 */
	endx = blockwid;
	endy = blockhgt;
	begx = begy = 0;
	if(pic->blockx + blockwid > picwid)
		endx = picwid - pic->blockx;
	if(pic->blocky + blockhgt > pichgt)
		endy = pichgt - pic->blocky;
	if(pic->blockx < 0)
		begx = -(pic->blockx);
	if(pic->blocky < 0)
		begy = -(pic->blocky);

	memset(&conv_abs, 0x0, sizeof(MOD_ABILITY));

	/*
	 * Bild ggfs. konvertieren
	 */
	if(block->col_format == GREY && pic->col_format == GREY)
	{
		if(blockdepth <= 8 &&
		   (picdepth < 8 || picdepth == 8 && pic->format_type == FORM_STANDARD))
		{
			if(!(Sys_info.profi_mode&SELECTED))
			{
				f_set_syspal();
				if(Dialog.winAlert.openAlert("Das Bild muû zum EinfÅgen des Blocks in eine hîhere Farbtiefe konvertiert werden!", "Abbruch", "Weiter", NULL, 1) == 1)
					return(-1);
			}
			
			conv_abs.depth1 = 8;
			conv_abs.form1 = FORM_PIXELPAK;
			if(f_convert(pic, &conv_abs, block->col_format, SAME, 1) < 0)
				return(-1);

			picdepth = pic->depth;						/* die Farbtiefe neu holen */
		}
		else
			if(blockdepth > picdepth)
			{
				if(!(Sys_info.profi_mode&SELECTED))
				{
					f_set_syspal();
					if(Dialog.winAlert.openAlert("Das Bild muû zum EinfÅgen des Blocks in eine hîhere Farbtiefe konvertiert werden!", "Abbruch", "Weiter", NULL, 1) == 1)
						return(-1);
				}

				conv_abs.depth1 = blockdepth;
				conv_abs.form1 = FORM_PIXELPAK;
				if(f_convert(pic, &conv_abs, block->col_format, SAME, 1) < 0)
					return(-1);

				picdepth = pic->depth;					/* die Farbtiefe neu holen */
			}
	}
	else
		if(picdepth < 16 || blockdepth > picdepth)
		{
			if(!(Sys_info.profi_mode&SELECTED))
			{
				f_set_syspal();
				if(Dialog.winAlert.openAlert("Das Bild muû zum EinfÅgen des Blocks in eine hîhere Farbtiefe konvertiert werden!", "Abbruch", "Weiter", NULL, 1) == 1)
					return(-1);
			}

			if(blockdepth == 16)
				conv_abs.depth1 = 16;
			else
				conv_abs.depth1 = 24;
			conv_abs.form1 = FORM_PIXELPAK;
			if(f_convert(pic, &conv_abs, block->col_format, SAME, 1) < 0)
				return(-1);

			picdepth = pic->depth;						/* die Farbtiefe neu holen */
		}


	/*
	 * Blockzeilenpuffer mit genÅgend Platz fÅr evtl. 24 Bit
	 */
	buf = SMalloc((block->pic_width + 32) * 3);

	/*
	 * Variablen fÅr weiû transparent
	 */
	if(blockdepth <= 8 || blockdepth == 24)
		FULLR = FULLG = FULLB = 255;
	else
		if(blockdepth == 16)
		{
			FULLR = FULLB = 31;
			FULLG = 63;
		}

	/*
	 * und jetzt einfÅgen. 
	 */
	Dialog.busy.reset(0, "Block einrechnen");

	if(pic->format_type == FORM_STANDARD)
	{
		st_piclinelen = ((picwid + 7) >> 3);
		st_blocklinelen = ((blockwid + 7) >> 3);
		picdata = (char *)(pic->pic_data);
		picdata += (long)((pic->blockx) / 8) + (long)st_piclinelen * (long)pic->blocky;
		blockdata = block->pic_data;

		linebuf = (char *)SMalloc(st_blocklinelen + 5);

		dbits_right = 8 - ((st_blocklinelen << 3) - blockwid);		/* Anzahl an Bits, die rechts noch fehlen */
		right_byte >>= dbits_right;

		dbits_left = (((pic->blockx + 7) >> 3) << 3) - pic->blockx;
		left_byte <<= dbits_left;

		for(y = begy; y < endy; y++)
		{
			if(!(y&63))
				Dialog.busy.draw(((long)(y - begy) << 7) / (long)(endy - begy));

			for(plane = 0; plane < pic->depth; plane++)
			{
				currpic = picdata + plane * plen_pic;
				currblock = blockdata + plane * plen_block;

				/* Mitte... */
				memset(linebuf, 0x0, st_blocklinelen + 5);
				rearrange_line2(currblock, linebuf, st_blocklinelen + 1, 8 - dbits_left);

				/* ...linker Rand... */
				copy_byte = *currpic & left_byte;
				*(linebuf) |= copy_byte;

				/* ...rechter Rand... */
				copy_byte = *(currpic + st_blocklinelen - 1)&right_byte;
				*(linebuf+st_blocklinelen-1) |= copy_byte;

				memcpy(currpic, linebuf, st_blocklinelen);
			}

			blockdata += st_blocklinelen;
			picdata += st_piclinelen;
		}

		SMfree(linebuf);
	}
	else
	{
		/*
		 * Startadresse berechnen
		 */
		bpp_pic = picdepth / 8;
		bpp_block = blockdepth / 8;
		picdata = (char *)(pic->pic_data);
		picdata += (long)pic->blockx * (long)bpp_pic + (long)picwid * (long)bpp_pic * (long)pic->blocky;
		blockdata = block->pic_data;
		st_blocklinelen = ((blockwid + 7) >> 3);

		Dialog.busy.fullDisable();

		for(y = begy; y < endy; y++)
		{
			blockdepth = block->depth;
			blockLinePic.depth = block->depth;

			if(!(y&15))
				Dialog.busy.draw(((long)(y - begy) << 7) / (long)(endy - begy));

			if(block->format_type == FORM_PIXELPAK)
				currblock = blockdata + ((long)y * (long)blockwid * (long)bpp_block) + ((long)begx * (long)bpp_block);
			else
				if(block->format_type == FORM_STANDARD)
					currblock = blockdata + ((long)y * st_blocklinelen);

			currpic = picdata + (y * (long)picwid * (long)bpp_pic) + ((long)begx * (long)bpp_pic);

			buf_copy = currblock;

			/*
			 * Zeile aus dem Originalblock nach buf auslesen
			 */
			if(block->format_type == FORM_STANDARD)
			{
				memset(buf, 0x0, block->pic_width);
				getpix_std_line(currblock, buf, blockdepth, plen_block, block->pic_width);
				buf_copy = buf + begx;
				blockdepth = 8;
				blockLinePic.depth = blockdepth;
			}
			else
			{
				memcpy(buf, currblock, block->pic_width * bpp_block);
				buf_copy = buf;
			}

			/*
			 * Blockzeile in Bildfarbtiefe & Datenformat konvertieren
			 */
			if(blockdepth < picdepth)
			{
				memcpy(&blockLinePic, block, sizeof(SMURF_PIC));
				blockLinePic.pic_data = buf_copy;
				blockLinePic.depth = blockdepth;
				blockLinePic.format_type = FORM_PIXELPAK;				/* ist garantiert von vorherigem if */
				blockLinePic.pic_height = 1;

				conv_abs.depth1 = picdepth;
				conv_abs.form1 = pic->format_type;

				if(f_convert(&blockLinePic, &conv_abs, pic->col_format, NEW, 1) < 0)
				{
					SMfree(buf);
					return(-1);
				}

				buf_copy = blockLinePic.pic_data;
			}
			
			/*
			 * konvertierte Zeile (Amen) ins Bild einrechnen
			 */
			if(blockmode_conf.mode == BCONF_REPLACE)
				insertline_replace(buf_copy, currpic, blockLinePic.depth, endx - begx, block_opac, opac2);
			else
				if(blockmode_conf.mode == BCONF_ADD)
					insertline_add(buf_copy, currpic, blockLinePic.depth, endx - begx, block_opac, opac2);
				else
					if(blockmode_conf.mode == BCONF_CLIPADD)
						insertline_clipadd(buf_copy, currpic, blockLinePic.depth, endx - begx, block_opac, opac2);
					else
						if(blockmode_conf.mode == BCONF_SUB)
							insertline_sub(buf_copy, currpic, blockLinePic.depth, endx - begx, block_opac, opac2);
						else
							if(blockmode_conf.mode == BCONF_CLIPSUB)
								insertline_clipsub(buf_copy, currpic, blockLinePic.depth, endx - begx, block_opac, opac2);
							else
								if(blockmode_conf.mode == BCONF_MULT)
									insertline_mult(buf_copy, currpic, blockLinePic.depth, endx - begx, block_opac, opac2);


			/* stopft das Speicherleck */
			if(blockdepth < picdepth)
				SMfree(buf_copy);
		}
	}
	
	SMfree(buf);
	Dialog.busy.ok();
	Dialog.busy.dispRAM();

	return(0);
} /* insert_block */


void insertline_replace(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2)
{
	unsigned char *pic;
	unsigned int *p16, *b16, x;
	long or,og,ob;


	pic = (unsigned char *)pdata2;
	p16 = (unsigned int *)pdata2;
	b16 = (unsigned int *)pdata1;

	if(block_opac == 255)
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = *pdata1++;
							*pic++ = *pdata1++;
							*pic++ = *pdata1++;
							break;
	
				case 16:	*p16++ = *b16++;
							break;
	
				case 8:		*pic++ = *pdata1++;
							break;
			}
		}
	}
	else
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = ((long)(*pdata1++)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(*pdata1++)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(*pdata1++)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
	
				case 16:	or = (((*p16 & 0xf800)>>11) * opac2) >> 8;
							og = (((*p16 & 0x7e0)>>6) * opac2) >> 8;
							ob = ((*p16 & 0x1f) * opac2) >> 8;
							or += ((*b16 & 0xf800)>>11)*opac1 >> 8;
							og += ((*b16 & 0x7e0)>>6)*opac1 >> 8;
							ob += (*b16 & 0x1f)*opac1 >> 8;
							*p16++ = (or<<11)|(og<<6)|ob;
							b16++;
							break;
	
				case 8:		*pic++ = ((long)(*pdata1++)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
			}
		}
	}

	return;
} /* insertline_replace */


void insertline_add(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2)
{
	unsigned char *pic;
	unsigned int *p16, *b16, x;
	long or,og,ob, r,g,b;

	
	pic = (unsigned char *)pdata2;
	p16 = (unsigned int *)pdata2;
	b16 = (unsigned int *)pdata1;

	if(block_opac == 255)
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ += *pdata1++;
							*pic++ += *pdata1++;
							*pic++ += *pdata1++;
							break;
	
				case 16:	or = ((*p16&0xf800)>>11) + ((*b16&0xf800)>>11);
							og = ((*p16&0x7e0)>>6) + ((*b16&0x7e0)>>6);
							ob = (*p16&0x1f) + (*b16&0x1f);
							*p16++ = (or<<11)|(og<<6)|ob;
							b16++;
							break;
	
				case 8:		*pic++ += *pdata1++;
							break;
			}
		}
	}
	else
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = ((long)(*pdata1++ + *pic)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(*pdata1++ + *pic)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(*pdata1++ + *pic)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
	
				case 16:	or = ((*p16&0xf800)>>11) + ((*b16 & 0xf800)>>11);
							og = ((*p16&0x7e0)>>6) + ((*b16 & 0x7e0)>>6);
							ob = (*p16&0x1f) + (*b16&0x1f);
							r = (((*p16&0xf800)>>11)*opac1>>8) + (or*opac2>>8);
							g = (((*p16&0x7e0)>>6)*opac1>>8) + (og*opac2>>8);
							b = ((*p16&0x1f)*opac1>>8) + (ob*opac2>>8);
							*p16++ = (r<<11)|(g<<6)|b;
							b16++;
							break;
	
				case 8:		*pic++ = ((long)(*pdata1++ + *pic)*opac1 >> 8) + ((long)(*pic)*opac2 >>8);
							break;
			}
		}
	}

	return;
} /* insertline_add */


void insertline_clipadd(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2)
{
	unsigned char *pic;
	unsigned int *p16, *b16, x;
	long or,og,ob, r,g,b;
	char clip24[513];
	int clip16rb[65];
	int clip16g[129];

	
	pic = (unsigned char *)pdata2;
	p16 = (unsigned int *)pdata2;
	b16 = (unsigned int *)pdata1;

	/*
	 * clip-arrays fÅllen (warum zum Teufel
	 * habe ich das nicht eigentlich schon immer mit arrays gemacht?)
	 */
	for(x=0; x<256; x++) clip24[x] = x;
	for(x=256; x<513; x++) clip24[x] = 255;
	for(x=0; x<32; x++) clip16rb[x] = x;
	for(x=32; x<65; x++) clip16rb[x] = 31;
	for(x=0; x<64; x++) clip16g[x] = x;
	for(x=64; x<129; x++) clip16g[x] = 63;

	if(block_opac == 255)
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = clip24[(int)(*pic) + (int)(*pdata1++)];
							*pic++ = clip24[(int)(*pic) + (int)(*pdata1++)];
							*pic++ = clip24[(int)(*pic) + (int)(*pdata1++)];
							break;
	
				case 16:	or = ((*p16&0xf800)>>11) + ((*b16&0xf800)>>11);
							og = ((*p16&0x7e0)>>6) + ((*b16&0x7e0)>>6);
							ob = (*p16&0x1f) + (*b16&0x1f);
							*p16++ = (clip16rb[or]<<11) | (clip16g[og]<<6) | clip16rb[ob];
							b16++;
							break;
	
				case 8:		*pic++ = clip24[(int)(*pic) + (int)(*pdata1++)];
							break;
			}
		}
	}
	else
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = ((long)(clip24[*pdata1++ + *pic])*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(clip24[*pdata1++ + *pic])*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(clip24[*pdata1++ + *pic])*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
	
				case 16:	or = clip16rb[((*p16&0xf800)>>11) + ((*b16 & 0xf800)>>11)];
							og = clip16g[((*p16&0x7e0)>>6) + ((*b16 & 0x7e0)>>6)];
							ob = clip16rb[(*p16&0x1f) + (*b16&0x1f)];
							r = (((*p16&0xf800)>>11)*opac1>>8) + (or*opac2>>8);
							g = (((*p16&0x7e0)>>6)*opac1>>8) + (og*opac2>>8);
							b = ((*p16&0x1f)*opac1>>8) + (ob*opac2>>8);
							*p16++ = (r<<11) | (g<<6) | b;
							b16++;
							break;
	
				case 8:		*pic++ = ((long)(clip24[*pdata1++ + *pic]) * opac1 >> 8) + ((long)(*pic) * opac2 >> 8);
							break;
			}
		}
	}

	return;
} /* insertline_clipadd */


void insertline_sub(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2)
{
	unsigned char *pic;
	unsigned int *p16, *b16, x;
	long or,og,ob, r,g,b;

	
	pic = (unsigned char *)pdata2;
	p16 = (unsigned int *)pdata2;
	b16 = (unsigned int *)pdata1;

	if(block_opac == 255)
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ -= *pdata1++;
							*pic++ -= *pdata1++;
							*pic++ -= *pdata1++;
							break;
	
				case 16:	or = ((*p16&0xf800)>>11) - ((*b16&0xf800)>>11);
							og = ((*p16&0x7e0)>>6) - ((*b16&0x7e0)>>6);
							ob = (*p16&0x1f) - (*b16&0x1f);
							*p16++ = (or<<11)|(og<<6)|ob;
							b16++;
							break;
	
				case 8:		*pic++ -= *pdata1++;
							break;
			}
		}
	}
	else
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = ((long)(*pdata1++ - *pic)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(*pdata1++ - *pic)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(*pdata1++ - *pic)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
	
				case 16:	or = ((*p16&0xf800)>>11) - ((*b16 & 0xf800)>>11);
							og = ((*p16&0x7e0)>>6) - ((*b16 & 0x7e0)>>6);
							ob = (*p16&0x1f) - (*b16&0x1f);
							r = (((*p16&0xf800)>>11)*opac1>>8) + (or*opac2>>8);
							g = (((*p16&0x7e0)>>6)*opac1>>8) + (og*opac2>>8);
							b = ((*p16&0x1f)*opac1>>8) + (ob*opac2>>8);
							*p16++ = (r<<11)|(g<<6)|b;
							b16++;
							break;
	
				case 8:		*pic++ = ((long)(*pdata1++ - *pic)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
			}
		}
	}

	return;
} /* insertline_sub */


void insertline_clipsub(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2)
{
	unsigned char *pic;
	unsigned int *p16, *b16, x;
	long or,og,ob, r,g,b;
	char clip24[513];
	int clip16rb[65];
	int clip16g[129];

	
	pic = (unsigned char *)pdata2;
	p16 = (unsigned int *)pdata2;
	b16 = (unsigned int *)pdata1;

	/*
	 * clip-arrays fÅllen (warum zum Teufel
	 * habe ich das nicht eigentlich schon immer mit arrays gemacht?)
	 */
	for(x=0; x<256; x++) clip24[x] = 0;
	for(x=256; x<513; x++) clip24[x] = x-256;
	for(x=0; x<32; x++) clip16rb[x] = 0;
	for(x=32; x<65; x++) clip16rb[x] = x-32;
	for(x=0; x<64; x++) clip16g[x] = 0;
	for(x=64; x<129; x++) clip16g[x] = x-64;

	if(block_opac == 255)
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = clip24[(int)(*pic) - (int)(*pdata1++) + 256];
							*pic++ = clip24[(int)(*pic) - (int)(*pdata1++) + 256];
							*pic++ = clip24[(int)(*pic) - (int)(*pdata1++) + 256];
							break;
	
				case 16:	or = ((*p16&0xf800)>>11) - ((*b16&0xf800)>>11);
							og = ((*p16&0x7e0)>>6) - ((*b16&0x7e0)>>6);
							ob = (*p16&0x1f) - (*b16&0x1f);
							*p16++ = (clip16rb[or+32]<<11) | (clip16g[og+64]<<6) | clip16rb[ob+32];
							b16++;
							break;
	
				case 8:		*pic++ = clip24[(int)(*pic) + (int)(*pdata1++) + 256];
							break;
			}
		}
	}
	else
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = ((long)(clip24[*pdata1++ - *pic + 256])*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(clip24[*pdata1++ - *pic + 256])*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = ((long)(clip24[*pdata1++ - *pic + 256])*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
	
				case 16:	or = clip16rb[((*p16&0xf800)>>11) - ((*b16 & 0xf800)>>11) + 32];
							og = clip16g[((*p16&0x7e0)>>6) - ((*b16 & 0x7e0)>>6) + 64];
							ob = clip16rb[(*p16&0x1f) - (*b16&0x1f) + 32];
							r = (((*p16&0xf800)>>11)*opac1>>8) + (or*opac2>>8);
							g = (((*p16&0x7e0)>>6)*opac1>>8) + (og*opac2>>8);
							b = ((*p16&0x1f)*opac1>>8) + (ob*opac2>>8);
							*p16++ = (r<<11) | (g<<6) | b;
							b16++;
							break;
	
				case 8:		*pic++ = ((long)(clip24[*pdata1++ - *pic + 256])*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
			}
		}
	}

	return;
} /* insertline_clipsub */


void insertline_mult(char *pdata1, void *pdata2, int depth, unsigned int num, long opac1, long opac2)
{
	unsigned char *pic;
	unsigned int *p16, *b16, x;
	long or,og,ob, r,g,b;

	
	pic = (unsigned char *)pdata2;
	p16 = (unsigned int *)pdata2;
	b16 = (unsigned int *)pdata1;

	if(block_opac == 255)
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = ((long)*pic * (long)(*pdata1++))>>8;
							*pic++ = ((long)*pic * (long)(*pdata1++))>>8;
							*pic++ = ((long)*pic * (long)(*pdata1++))>>8;
							break;
	
				case 16:	or = ((*p16&0xf800)>>11);
							og = ((*p16&0x7e0)>>6);
							ob = (*p16&0x1f);
							r = ((*b16&0xf800)>>11);
							g = ((*b16&0x7e0)>>6);
							b = (*b16&0x1f);
							*p16++ = (((long)or*(long)r >>8)<<11)|(((long)og*(long)g >>8)<<6)|((long)ob*(long)b >>8);
							b16++;
							break;
	
				case 8:		*pic++ = (long)*pic * (long)(*pdata1++) >> 8;
							break;
			}
		}
	}
	else
	{
		for(x=0; x<num; x++)
		{
			switch(depth)
			{
				case 24:	*pic++ = (((long)*pdata1++ * (long)*pic >> 8)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = (((long)*pdata1++ * (long)*pic >> 8)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							*pic++ = (((long)*pdata1++ * (long)*pic >> 8)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
	
				case 16:	or = (long)((*p16&0xf800)>>11) * (long)((*b16 & 0xf800)>>11) >> 5;
							og = ((long)(*p16&0x7e0)>>6) * ((long)(*b16 & 0x7e0)>>6) >> 5;
							ob = (long)(*p16&0x1f) * (long)(*b16&0x1f) >> 5;
							r = (((*p16&0xf800)>>11)*opac1>>8) + (or*opac2>>8);
							g = (((*p16&0x7e0)>>6)*opac1>>8) + (og*opac2>>8);
							b = ((*p16&0x1f)*opac1>>8) + (ob*opac2>>8);
							*p16++ = (r<<11)|(g<<6)|b;
							b16++;
							break;
	
				case 8:		*pic++ = ( ((long)(*pdata1++) * (long)*pic >> 8)*opac1 >>8) + ((long)(*pic)*opac2 >>8);
							break;
			}
		}
	}

	return;
} /* insertline_mult */


void blockmode(void)
{
	int button, t;

	static long block_opac=100;

	OBJECT *tree;

	extern int picwindthere;
	extern SLIDER sliders[15];

	
	button = Dialog.init(WIND_BLOCKMODE, BCONF_OK);
	tree = wind_s[WIND_BLOCKMODE].resource_form;

	if(obj == DIALOG_EXIT)
		button=BCONF_OK;
	
	if(button == BCONF_F || button == BCONF_R)
		block_opac = f_rslid(&sliders[BLOCK_SLIDER]); 
	else
		if(button == BCONF_SET || button == BCONF_OK)
		{
			blockmode_conf.mode = get_selected_object(tree, BCONF_REPLACE, BCONF_MULT);
			blockmode_conf.opacity = block_opac;

			blockmode_conf.transparent = 0;
			if(IsSelected(tree[BCONF_BTRANS]))
				blockmode_conf.transparent |= 0x01;
			if(IsSelected(tree[BCONF_WTRANS]))
				blockmode_conf.transparent |= 0x02;
			
			change_object(&wind_s[WIND_BLOCKMODE], button, UNSEL, 1);

			/*
			 * wenn der RT-dither an ist, mÅssen nun evtl. Blîcke in allen
			 * Fenstern redrawed werden
			 */
			if(Sys_info.realtime_dither)
			{
				for(t = 0; t < picwindthere; t++)
				{
					Window.redraw(&picture_windows[t+1], NULL, 0, DRAWNOTREE|BLOCK_ONLY|DRAWNOBLOCK);
					Window.redraw(&picture_windows[t+1], NULL, 0, BLOCK_ONLY);
				}
			}
		}
	
	if(button == BCONF_OK)
		Dialog.close(WIND_BLOCKMODE);

	return;
} /* blockmode */


void blockfunctions_on(void)
{
	extern OBJECT *menu_tree;


	if(Sys_info.scrp_path != NULL)
	{
		blockpopup[BLOCK_COPY].ob_state &= ~DISABLED;
		blockpopup[BLOCK_CUT].ob_state &= ~DISABLED;
		menu_ienable(menu_tree, EDIT_COPY, 1);
		menu_ienable(menu_tree, EDIT_CUT, 1);
	}

	blockpopup[BLOCK_RELEASE].ob_state &= ~DISABLED;
	blockpopup[BLOCK_FREE].ob_state &= ~DISABLED;
	blockpopup[BLOCK_MODE].ob_state &= ~DISABLED;

	menu_ienable(menu_tree, EDIT_CROP, 1);
	menu_ienable(menu_tree, EDIT_RELEASE, 1);

	return;
} /* blockfunctions_on */


void blockfunctions_off(void)
{
	extern OBJECT *menu_tree;


 	if(Sys_info.scrp_path != NULL)
 	{
		blockpopup[BLOCK_COPY].ob_state |= DISABLED;
		blockpopup[BLOCK_CUT].ob_state |= DISABLED;
		menu_ienable(menu_tree, EDIT_COPY, 0);
		menu_ienable(menu_tree, EDIT_CUT, 0);
	}

	blockpopup[BLOCK_RELEASE].ob_state |= DISABLED;
	blockpopup[BLOCK_FREE].ob_state |= DISABLED;
	blockpopup[BLOCK_SIZE].ob_state |= DISABLED;
	blockpopup[BLOCK_EXPORT].ob_state |= DISABLED;
	blockpopup[BLOCK_MODE].ob_state |= DISABLED;

	menu_ienable(menu_tree, EDIT_CROP, 0);
	menu_ienable(menu_tree, EDIT_RELEASE, 0);

	return;
} /* blockfunctions_off */


int encode_block(SMURF_PIC *picture, EXPORT_PIC **pic_to_save)
{
	char *dest_pic;
	char *textseg_begin;
	char *dummy = NULL;
	char clipexp_path[256];

	int back = 0;

	BASPAG *clx_bp = NULL;
	GARGAMEL clx_struct;
	MOD_ABILITY *clipexp_mabs;
	SMURF_PIC *new_pic;



	/*
	 * Struktur anlegen und Block reinkopieren
	 */
	if(picture->block == NULL)
	{
		new_pic = (SMURF_PIC *)SMalloc(sizeof(SMURF_PIC));
		memcpy(new_pic, picture, sizeof(SMURF_PIC));
		dest_pic = copyblock(picture);

		if(dest_pic == NULL)
		{
			SMfree(new_pic);
			return(-1);
		}

		new_pic->pic_data = dest_pic;
		new_pic->pic_width = block.g_w;
		new_pic->pic_height = block.g_h;
	}
	else
		new_pic = picture->block;

	Dialog.busy.disable();

	/*------------------ Exporter initialisieren und Bild konvertieren --------------*/
	strcpy(clipexp_path, Sys_info.standard_path);
	strcat(clipexp_path, "\\modules\\clipbrd.sxm");

	clx_bp = NULL;
	clx_bp = (BASPAG *)start_exp_module(clipexp_path, MSTART, new_pic, clx_bp, &clx_struct, 0x0101);
	textseg_begin = (char *)(clx_bp->p_tbase);
	clipexp_mabs = (MOD_ABILITY *)*((MOD_ABILITY **)(textseg_begin + MOD_ABS_OFFSET));
	f_convert(new_pic, clipexp_mabs, RGB, SAME, 0);

	/*------------------- Bild speichern --------------------------------------------*/
	*pic_to_save = start_exp_module(clipexp_path, MEXEC, new_pic, clx_bp, &clx_struct, 0x0101);

	/* Fehler aufgetreten? ----------------------------------------------------------*/	
	if(clx_struct.module_mode == M_MEMORY)
	{
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NOMEM_EXPORT].TextCast, NULL, NULL, NULL, 1);
		back = -1;
	}
	else
		if(clx_struct.module_mode == M_PICERR)
		{
			Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NOMEM_PICERR].TextCast, NULL, NULL, NULL, 1);
			back = -1;
		}
		/* 
		 * Bild codiert?
		 */
		else
			if(clx_struct.module_mode == M_DONEEXIT)
				back = 0;


	/*----------- Modul terminieren ------------------------------*/
/*	Pexec(102, dummy, clx_bp, ""); */
	SMfree(clx_bp->p_env);
	SMfree(clx_bp);

	if(picture->block == NULL)
	{
		SMfree(new_pic->pic_data);
		SMfree(new_pic);
	}

	return(back);
} /* encode_block */


/* save_block -------------------------------------------------------
	Speichert ein Bild pic_to_save mit dem Clipboardexporter in den Pfad
	path oder, wenn path==NULL, in den Clipboardpfad.
	----------------------------------------------------------------*/
int save_block(EXPORT_PIC *pic_to_save, char *path)
{
	char *dest_pic;
	char clipexp_path[256];

	int ap_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int fhandle, ok, dummy2;

	long len, dummy;


	dest_pic = (char *)pic_to_save->pic_data;
	len = pic_to_save->f_len;

	if(path == NULL)								/* aufs Clipboard? */
	{
		if(clear_scrap() == -1)
			goto set_free;

		strcpy(clipexp_path, Sys_info.scrp_path);
	}
	else
		strcpy(clipexp_path, path);


	strcat(clipexp_path, "scrap.img");
	dummy = Fcreate(clipexp_path, 0);
	if(dummy < 0 && (Sys_info.profi_mode&SELECTED) == 0)
		Dialog.winAlert.openAlert("Datei konnte nicht erzeugt werden!", NULL, NULL, NULL, 1);
	else
	{
		fhandle = (int)dummy;
		if(Fwrite(fhandle, len, dest_pic) != len)
			Dialog.winAlert.openAlert("Datei konnte nicht vollstÑndig geschrieben werden! Wahrscheinlich ist das Laufwerk voll.", NULL, NULL, NULL, 1);
		else
			if(path == NULL)						/* aufs Clipboard? */
				/* shel_write() vorhanden? */
				if(Sys_info.OSFeatures&GETINFO && appl_getinfo(10, &ok, &dummy2, &dummy2, &dummy2) == 1 &&
				   ((ok&0x00ff) >= SHW_BROADCAST))
				{
					ap_buf[0] = SC_CHANGED;
					ap_buf[1] = Sys_info.app_id;
					ap_buf[2] = 0;
					ap_buf[3] = 0x0008;
					*(char **)&ap_buf[4] = ".IMG";
					ap_buf[6] = 0;
					ap_buf[7] = 0;

					shel_write(SHW_BROADCAST, 0, 0, (char *)ap_buf, NULL);
				}			
		Fclose(fhandle);
	}

set_free:
	if(Mfree(pic_to_save->pic_data) != 0)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[EXP_MFREE_ERR].TextCast, NULL, NULL, NULL, 1);
	if(Mfree(pic_to_save) != 0)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[EXP_MFREE_ERR].TextCast, NULL, NULL, NULL, 1);

	return(0);
} /* save_block */


void block_over_all(WINDOW *window)
{
	if(window->picture->block != NULL)							/* evtl. vorhandenen Block verwerfen */
	{
		destroy_smurfpic(window->picture->block);
		window->picture->block = NULL;
	}

	window->picture->blockx = 0;
	window->picture->blocky = 0;
	window->picture->blockwidth = window->picture->pic_width;
	window->picture->blockheight = window->picture->pic_height;

	imageWindow.drawBlockbox(window);
	Window.redraw(window, NULL, 0, 0);

	blockfunctions_on();

	return;
} /* new_block */


int intersect_block(SMURF_PIC *picture)
{
	int x, y, w, h;


	x = max(0, picture->blockx);
	y = max(0, picture->blocky);
	w = min(picture->pic_width, picture->blockx + picture->blockwidth);
	h = min(picture->pic_height, picture->blocky + picture->blockheight);

	block.g_x = x;
	block.g_y = y;
	block.g_w = w - x;
	block.g_h = h - y;

	return((w > x) && (h > y));
} /* intersect_block */