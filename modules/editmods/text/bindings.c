#include <stdlib.h>
#include <string.h>
#include <aes.h>
#include <vdi.h>
#include "fontsel.h"

AESPB aespb;


/* Binding fÅr die neue VDI-Funktion v_opnbm(), Offscreen-Bitmap îffnen */
/* VDI 100, 1 */
void v_opnbm(int *work_in, MFDB *bitmap, int *handle, int *work_out)
{
    /* Wenn work_in[15..19] 0 enthalten, wird eine Bitmap im gerÑtespezifischen
       Format oder mit nur 1 Ebene erzeugt (hÑngt vom MFDB ab). Anderfalls wird
       versucht eine Bitmap mit der Farbanzahl <work_in[15..16]>, <work_in[17]>
       Ebenen, dem Pixelformat <work_in[18]> und der Bitreihenfolge
		<work_in[19]> anzulegen. Falls kein passender Offscreen-Treiber vorhanden 
		ist, kann die Bitmap nicht geîffnet werden. */

    VDIPB vdipb;


    vdipb.contrl = _VDIParBlk.contrl;
    vdipb.intin = work_in;
    vdipb.ptsin = _VDIParBlk.ptsin;
    vdipb.intout = work_out;
    vdipb.ptsout = work_out + 45;

    vdipb.contrl[0] = 100;
    vdipb.contrl[1] = 0;
    vdipb.contrl[3] = 20;
    vdipb.contrl[5] = 1;
    *(MFDB **)&vdipb.contrl[7] = bitmap;

    vdi(&vdipb);

    *handle = vdipb.contrl[6];

    return;
} /* v_opnbm */


/* Binding fÅr die neue VDI-Funktion v_clsbm(), Offscreen-Bitmap schlieûen */
/* VDI 101, 1 */
void v_clsbm(int handle)
{
    VDIPB vdipb;


    vdipb.contrl = _VDIParBlk.contrl;
    vdipb.intin = _VDIParBlk.intin;
    vdipb.ptsin = _VDIParBlk.ptsin;
    vdipb.intout = _VDIParBlk.intout;
    vdipb.ptsout = _VDIParBlk.ptsout;

    vdipb.contrl[0] = 101;
    vdipb.contrl[1] = 0;
    vdipb.contrl[3] = 0;
    vdipb.contrl[5] = 1;
    vdipb.contrl[6] = handle;

    vdi(&vdipb);

    return;
} /* v_clsbm */


/* Binding fÅr die neue VDI-Funktion vqt_xfntinfo(), erweiterte Fontinformation */
/* VDI 229, 0 */
void vqt_xfntinfo(int handle, int flags, int id, int index, XFNT_INFO *info)
{
    VDIPB vdipb;


    vdipb.contrl = _VDIParBlk.contrl;
    vdipb.intin = _VDIParBlk.intin;
    vdipb.ptsin = _VDIParBlk.ptsin;
    vdipb.intout = _VDIParBlk.intout;
    vdipb.ptsout = _VDIParBlk.ptsout;

    vdipb.contrl[0] = 229;
    vdipb.contrl[1] = 0;
    vdipb.contrl[3] = 5;
    vdipb.contrl[5] = 0;
    vdipb.contrl[6] = handle;

	vdipb.intin[0] = flags;
	vdipb.intin[1] = id;
	vdipb.intin[2] = index;
    
/*    *((long *) (&(vdipb.intin[3]))) = info; */
	*(XFNT_INFO **)&vdipb.intin[3] = info;

    vdi(&vdipb);

    return;
}


/* Binding fÅr die neue VDI-Funktion vqt_real_extent(), umgebendes Textrechteck berechnen*/
/* VDI 240, 4200 */
/*
	Diese Funktion wird z.Zt. nur von NVDI bereitgestellt.
	Es wird das umgebende Viereck (es muû sich nicht immer um ein Rechteck handeln)
	fÅr Textausgabe an der Stelle x,y zurÅckgeliefert. Dabei werden sÑmtliche
	Texteffekte, Rotation, SchrÑgstellung, Pair-Kerning, Track-Kerning,
	ZeichenÅberhÑnge, horizontale und vertikale Ausrichtung berÅcksichtigt.
*/
void vqt_real_extent( int handle, int x, int y, char *string,  int *extent )
{
	/*Aufruf: vqt_real_extent( handle, x, y, string, extent )*/

    VDIPB vdipb;
	int *intin, t;

    vdipb.contrl = _VDIParBlk.contrl;
    vdipb.intin = _VDIParBlk.intin;
    vdipb.ptsin = _VDIParBlk.ptsin;
    vdipb.intout = _VDIParBlk.intout;
    vdipb.ptsout = _VDIParBlk.ptsout;
 
	vdipb.contrl[0] = 240;
	vdipb.contrl[1] = 1;
	vdipb.contrl[3] = (int)strlen(string);
	vdipb.contrl[5] = 4200;
	vdipb.contrl[6] = handle;
	vdipb.ptsin[0] = x;
	vdipb.ptsin[1] = y;
	
	intin = malloc(strlen(string)*2L);
	for(t=0; t<strlen(string); t++)
		intin[t]=string[t];
	
	vdipb.intin = intin;

    vdi(&vdipb);

	free(intin);
	memcpy(extent, vdipb.ptsout, 8*2);

    return;
}


/* Binding fÅr die neue VDI-Funktion v_opnbm(), Offscreen-Bitmap îffnen */
/* VDI 246 */
long vst_arbpt32( int handle, long height,
                           int *char_width, int *char_height,
                           int *cell_width, int *cell_height )
{
    VDIPB vdipb;


    vdipb.contrl = _VDIParBlk.contrl;
    vdipb.intin = _VDIParBlk.intin;
    vdipb.ptsin = _VDIParBlk.ptsin;
    vdipb.intout = _VDIParBlk.intout;
    vdipb.ptsout = _VDIParBlk.ptsout;
 
	vdipb.contrl[0] = 246;
	vdipb.contrl[1] = 0;
	vdipb.contrl[3] = 2;
	vdipb.contrl[6] = handle;
	*( (long*)&(vdipb.intin[0])) = height;

	*char_width = vdipb.ptsout[0];
	*char_height = vdipb.ptsout[1];
	*cell_width = vdipb.ptsout[2];
	*cell_height = vdipb.ptsout[3];

    vdi(&vdipb);

	return( *((long*)&(vdipb.intout[0])) );
}


FNT_DIALOG *fnts_create(int handle, int no_fonts,
						int font_flags, int dialog_flags,
						char *sample, char *opt_button)
{
	aespb.contrl = _GemParBlk.contrl;
	aespb.global = _GemParBlk.global;
	aespb.intin = _GemParBlk.intin;
	aespb.intout = _GemParBlk.intout;
	aespb.addrin = (void *)_GemParBlk.addrin;
	aespb.addrout = (void *)_GemParBlk.addrout;

	aespb.contrl[0] = 180;
	aespb.contrl[1] = 4;
	aespb.contrl[3] = 2;
	aespb.contrl[2] = 0;
	aespb.contrl[4] = 1;

	aespb.intin[0] = handle;
	aespb.intin[1] = no_fonts;
	aespb.intin[2] = font_flags;
	aespb.intin[3] = dialog_flags;

	_GemParBlk.addrin[0] = sample;
	_GemParBlk.addrin[1] = opt_button;

	_crystal(&aespb);

	return(_GemParBlk.addrout[0]);
} /* fnts_create */


int fnts_delete(FNT_DIALOG *fnt_dialog, int handle)
{
	aespb.contrl = _GemParBlk.contrl;
	aespb.global = _GemParBlk.global;
	aespb.intin = _GemParBlk.intin;
	aespb.intout = _GemParBlk.intout;
	aespb.addrin = (void *)_GemParBlk.addrin;
	aespb.addrout = (void *)_GemParBlk.addrout;

	aespb.contrl[0] = 181;
	aespb.contrl[1] = 1;
	aespb.contrl[3] = 1;
	aespb.contrl[2] = 1;
	aespb.contrl[4] = 0;

	aespb.intin[0] = handle;

	_GemParBlk.addrin[0] = fnt_dialog;

	_crystal(&aespb);

	return(aespb.intout[0]);
} /* fnts_delete */


/* leicht verÑndertes Binding fÅr fnts_do() */
/* button_flags und check_boxes werden nicht nach auûen gefÅhrt */
int fnts_do(FNT_DIALOG *fnt_dialog, long id_in, long pt_in, 
			long ratio_in, long *id, long *pt, long *ratio)
{
	aespb.contrl = _GemParBlk.contrl;
	aespb.global = _GemParBlk.global;
	aespb.intin = _GemParBlk.intin;
	aespb.intout = _GemParBlk.intout;
	aespb.addrin = (void *)_GemParBlk.addrin;
	aespb.addrout = (void *)_GemParBlk.addrout;

	aespb.contrl[0] = 187;
	aespb.contrl[1] = 7;
	aespb.contrl[3] = 1;
	aespb.contrl[2] = 8;
	aespb.contrl[4] = 0;

/*	aespb.intin[0] = button_flags; */
	aespb.intin[0] = 0;
	aespb.intin[1] = (int)(id_in>>16);
	aespb.intin[2] = (int)id_in;
	aespb.intin[3] = (int)(pt_in>>16);
	aespb.intin[4] = (int)pt_in;
	aespb.intin[5] = (int)(ratio_in>>16);
	aespb.intin[6] = (int)ratio_in;

	_GemParBlk.addrin[0] = fnt_dialog;

	_crystal(&aespb);

/*	*check_boxes = aespb.intout[1]; */
	*id = ((long)aespb.intout[2]<<16) | aespb.intout[3];
	*pt = ((long)aespb.intout[4]<<16) | aespb.intout[5];
	*ratio = ((long)aespb.intout[6]<<16) | aespb.intout[7];

	return(aespb.intout[0]);
} /* fnts_do */