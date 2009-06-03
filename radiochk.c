/* ******************************************************************** */
/* Handler fr Userdefined Objects: Radiobuttons & Checkboxen. 			*/
/*																		*/
/* (C) MAI 1992 Stefan Piesche 											*/
/*		Umgeschrieben fr Farbicons: 29.07.95, Olaf.																	*/
/* ******************************************************************** */
/*																		*/
/* Die Routinen erwarten lediglich zwei Globale Parameter, die im 		*/
/* Hauptmodul definiert sein mssen.									*/
/* OBJECT *u_tree:	Zeiger auf den Objektbaum in dem die User-IMG sind.	*/
/* int	handle:		VDI-handle											*/
/*																		*/
/* Die Routinen installieren sich selbst, es muž allerdings vor jedem	*/
/* objc_draw der Dialogbox einmal (!) die Routine f_treewalk aufgerufen */
/* werden.	(Nur vor dem ersten objc_draw! Olaf.)						*/
/*																		*/
/* Die Objekte fr Radibuttons mssen folgendermažen aufgebaut sein:	*/
/* Zuerst sollte der TEXT geschrieben werden.							*/
/* Dann wird ber den Text eine transparente I-BOX gelegt, die links	*/
/* Genau 2 Zeichen breit ber den Text hinaussteht (NICHT Adoptieren!).	*/
/* Der Status SELECTED dieser I-BOXEN kann nachher berprft werden		*/
/* Die I-BOXEN mssen definiert sein:									*/
/*																		*/
/* a) Radio-Buttons: SELECTABLE,RADIOBUTTON; das obere Byte des Typs	*/
/*						ist 18 (0x12). Ein (!) Radiobutton muž bereits	*/
/*						als SELECTED definiert sein.					*/
/*																		*/
/*																		*/
/* b) Check-Boxen:	SELECTABLE; oberes Byte: 19 (0x13)					*/
/*																		*/
/*																		*/
/* ******************************************************************** */

/* Statusabfrage:	if( OBJEKTBAUM [BUTTON-OBJEKT].ob_state & SELECTED) */
/*					-> 1=selektiert, 0=unselektiert						*/

/*#include <multiaes.h> why both originally ? */
#include <aes.h>
#include <vdi.h>
#include <stdio.h>
#include "sym_gem.h"
#include "smurf.h"
#include "modules\import.h"

#define RADIOBUTTON   	43		/* Oberes Byte.... */
#define CHECKBOX  		44		/* Fr den Res-Editor */
#define CYCLEBUTTON		45		/* Fr den Res-Editor */
#define CORNER_IMAGE	46		/* Fr den Res-Editor */

USERBLK check_user;
USERBLK radio_user;
USERBLK cycle_user;
USERBLK corner_user;
USERBLK angle_user;

extern OBJECT *u_tree;
extern int handle;

extern int Radio, SelectedRadio;
extern int Check, SelectedCheck;
extern int Cycle, SelectedCycle;
extern SYSTEM_INFO Sys_info;

void f_init_tree(OBJECT *baum,int element);
void f_treewalk	(OBJECT *tree,int start);
int	cdecl f_do_radio	(PARMBLK *parm);
int	cdecl f_do_checkbox	(PARMBLK *parm);
int	cdecl f_do_cycle	(PARMBLK *parm);
int	cdecl f_do_corner(PARMBLK *parm);




void f_treewalk	(OBJECT *tree,int start)
{
	int index = 0;


	for(index=tree[start].ob_head; index!=-1 && index!=start; index=tree[index].ob_next)
	{
		f_init_tree(tree,index);
		f_treewalk(tree,index);		/* rekursiv fr alle Kinder! */
	}

	return;
}


void f_init_tree(OBJECT *baum,int element)
{
	switch(baum[element].ob_type >> 8)
	{
		case CHECKBOX: 	baum[element].ob_type=(baum[element].ob_type&0xff00)|G_USERDEF;
						baum[element].ob_spec.userblk=&check_user;
						check_user.ub_code=f_do_checkbox;
						break;
		case RADIOBUTTON: 	baum[element].ob_type=(baum[element].ob_type&0xff00)|G_USERDEF;
							baum[element].ob_spec.userblk=&radio_user;	
							radio_user.ub_code=f_do_radio;
							break;
		case CYCLEBUTTON: 	baum[element].ob_type=(baum[element].ob_type&0xff00)|G_USERDEF;
							baum[element].ob_spec.userblk=&cycle_user;	
							cycle_user.ub_code=f_do_cycle;
							break;
		case CORNER_IMAGE: 	baum[element].ob_type=(baum[element].ob_type&0xff00)|G_USERDEF;
							baum[element].ob_spec.userblk=&corner_user;	
							corner_user.ub_code=f_do_corner;
							break;
/*		case ANGLE_OBJECT: 	baum[element].ob_type=(baum[element].ob_type&0xff00)|G_USERDEF;
							baum[element].ob_spec.userblk = angle_user;	
							angle_user.ub_code = drawAngleObject;
							break;*/
	}

	return;
}


int cdecl f_do_radio(PARMBLK *parm)
{
	int pxy[8];
	int ob_x, ob_y;
	int color_index[2];
	int color_index2[2];
	int Radiobutton;
	int write_mode;

	CICONBLK *quell_img;			/* jetzt Farbicon (Struktur: siehe AES.H!) */
	USERBLK *buttonimg;
	MFDB source, dest;


	if(Sys_info.bitplanes>8)
		write_mode=S_AND_D;
	else
		write_mode=S_OR_D;

	color_index[0]=1;
	color_index[1]=0;

	color_index2[0]=0;
	color_index2[1]=1;

	ob_x = parm->pb_x;
	ob_y = parm->pb_y;

	if(parm->pb_currstate&SELECTED)
		Radiobutton = SelectedRadio;
	else
		Radiobutton = Radio;

	buttonimg = u_tree[Radiobutton].ob_spec.userblk;
	if(buttonimg && Sys_info.Max_col >= 4)
		quell_img = (CICONBLK *)buttonimg->ub_parm;
	else
		quell_img = u_tree[Radiobutton].ob_spec.ciconblk;

	dest.fd_addr = NULL;
	source.fd_w = 16;										/* Radiobuttons/Checkboxen mit 16*16 */
	source.fd_h = 16;
	source.fd_wdwidth = 1;									/* 16 Pixel einer Plane = 1 Word */
	source.fd_stand = 0;

	/* AES-Hintergrundfarbe ermitteln und Box zeichnen */
	pxy[0] = ob_x;
	pxy[1] = ob_y;
	pxy[2] = ob_x + source.fd_w - 1;
	pxy[3] = ob_y + source.fd_h - 1;
	vsf_color(handle, Sys_info.AES_bgcolor);
	vswr_mode(handle, MD_REPLACE);
	vr_recfl(handle, pxy);

	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = source.fd_w - 1;
	pxy[3] = source.fd_h - 1;
	pxy[4] = ob_x;
	pxy[5] = ob_y;
	pxy[6] = ob_x + pxy[2];
	pxy[7] = ob_y + pxy[3];

	if(Sys_info.Max_col >= 4)
	{
		source.fd_nplanes = 1;									/* Bitplanes des Quellimages */
		source.fd_addr = quell_img->mainlist->col_mask;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		vrt_cpyfm(handle, S_AND_NOTD, pxy, &source, &dest, color_index2);  	/* Farbe->Farbe - Rastercopy */

		source.fd_addr = quell_img->mainlist->col_data;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes = quell_img->mainlist->num_planes;	/* Bitplanes des Quellimages */
		vro_cpyfm(handle, write_mode, pxy, &source, &dest); 	/* Farbe->Farbe - Rastercopy */
	}
	else
	{
		source.fd_addr = quell_img->monoblk.ib_pdata;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes = 1;									/* Bitplanes des Quellimages */
		vrt_cpyfm(handle, MD_REPLACE, pxy, &source, &dest, color_index);  	/* s/w->Farbe - Rastercopy */
	}

	return (parm->pb_currstate & ~SELECTED);
}


int	cdecl f_do_checkbox	(PARMBLK *parm)
{
	int pxy[8];
	int ob_x,ob_y;
	int color_index[2];
	int color_index2[2];
	int Checkbox;
	int write_mode;

	CICONBLK *quell_img;
	USERBLK	*buttonimg;
	MFDB source,dest;


	if(Sys_info.bitplanes > 8)
		write_mode = S_AND_D;
	else
		write_mode = S_OR_D;

	color_index[0] = 1;
	color_index[1] = 0;

	color_index2[0] = 0;
	color_index2[1] = 1;

	ob_x = parm->pb_x;
	ob_y = parm->pb_y;


	if(parm->pb_currstate&SELECTED)
		Checkbox = SelectedCheck;
	else
		Checkbox = Check;

	buttonimg = u_tree[Checkbox].ob_spec.userblk;

	if(buttonimg && Sys_info.Max_col>=4)
		quell_img = (CICONBLK *)buttonimg->ub_parm;
	else
		quell_img = u_tree[Checkbox].ob_spec.ciconblk; 

	dest.fd_addr = NULL;

	source.fd_w = 16;
	source.fd_h = 16;
	source.fd_wdwidth = 1;
	source.fd_stand = 0;

	pxy[0] = ob_x;
	pxy[1] = ob_y;
	pxy[2] = ob_x + source.fd_w - 1;
	pxy[3] = ob_y + source.fd_h - 1;
	vsf_color(handle, Sys_info.AES_bgcolor);
	vswr_mode(handle, MD_REPLACE);
	vr_recfl(handle, pxy);

	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = source.fd_w - 1;
	pxy[3] = source.fd_h - 1;
	pxy[4] = ob_x;
	pxy[5] = ob_y;
	pxy[6] = ob_x + pxy[2];
	pxy[7] = ob_y + pxy[3];

	if(Sys_info.Max_col >= 4)
	{
		source.fd_nplanes = 1;
		source.fd_addr = quell_img->mainlist->col_mask;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		vrt_cpyfm(handle,S_AND_NOTD,pxy,&source,&dest, color_index2);  		/* Farb-Rastercopy */

		source.fd_addr = quell_img->mainlist->col_data;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes = quell_img->mainlist->num_planes;	/* Bitplanes des Quellimages */
		vro_cpyfm(handle, write_mode, pxy, &source, &dest);  	/* Farb-Rastercopy */
	}
	else
	{
		source.fd_addr = quell_img->monoblk.ib_pdata;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes = 1;									/* Bitplanes des Quellimages */
		vrt_cpyfm(handle, MD_REPLACE, pxy, &source, &dest, color_index);  		/* Farb-Rastercopy */
	}

	return (parm->pb_currstate & ~SELECTED);
}


int	cdecl f_do_cycle(PARMBLK *parm)
{
	int pxy[8];
	int ob_x,ob_y;
	int color_index[2];
	int color_index2[2];
	int Cyclebutton;
	int write_mode;

	CICONBLK *quell_img;
	USERBLK	*buttonimg;
	MFDB source,dest;

	
	if(Sys_info.bitplanes > 8)
		write_mode = S_AND_D;
	else
		write_mode = S_OR_D;

	color_index[0] = 1;
	color_index[1] = 0;
	color_index2[0] = 0;
	color_index2[1] = 1;
	
	ob_x = parm->pb_x;
	ob_y = parm->pb_y;
	
	if(parm->pb_currstate&SELECTED)
		Cyclebutton = SelectedCycle;
	else
		Cyclebutton = Cycle;
	
	buttonimg = u_tree[Cyclebutton].ob_spec.userblk;
	if(buttonimg && Sys_info.Max_col >= 4)
		quell_img = (CICONBLK *)buttonimg->ub_parm;
	else
		quell_img = u_tree[Cyclebutton].ob_spec.ciconblk; 
	
	dest.fd_addr = NULL;

	source.fd_w = 32;
	source.fd_h = 24;
	source.fd_wdwidth = 2;
	source.fd_stand = 0;

	pxy[0] = ob_x;
	pxy[1] = ob_y;
	pxy[2] = ob_x + source.fd_w - 1;
	pxy[3] = ob_y + source.fd_h - 1;
	vsf_color(handle, Sys_info.AES_bgcolor);
	vswr_mode(handle, MD_REPLACE);
	vr_recfl(handle, pxy);

	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = 23 /*source.fd_w - 1*/;
	pxy[3] = 23 /*source.fd_h - 1*/;
	pxy[4] = ob_x;
	pxy[5] = ob_y;
	pxy[6] = ob_x + pxy[2];
	pxy[7] = ob_y + pxy[3];
	
	if(Sys_info.Max_col >= 4)
	{
		source.fd_nplanes = 1;
		source.fd_addr = quell_img->mainlist->col_mask;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		vrt_cpyfm(handle, S_AND_NOTD, pxy, &source, &dest, color_index2);	/* Farb-Rastercopy */

		source.fd_addr = quell_img->mainlist->col_data;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes = quell_img->mainlist->num_planes;	/* Bitplanes des Quellimages */
		vro_cpyfm(handle, write_mode, pxy, &source, &dest);  	/* Farb-Rastercopy */
	}
	else
	{
		source.fd_addr = quell_img->monoblk.ib_pdata;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes = 1;									/* Bitplanes des Quellimages */
		vrt_cpyfm(handle, MD_REPLACE, pxy, &source, &dest, color_index);	/* Farb-Rastercopy */
	}
	
	return (parm->pb_currstate&~SELECTED);
}


int	cdecl f_do_corner(PARMBLK *parm)
{
	int pxy[8];
	int ob_x,ob_y;
	int color_index[2];
	int color_index2[2];
	int Corner;
	int write_mode;

	USERBLK	*buttonimg;
	CICONBLK *quell_img;
	MFDB source,dest;

	
	if(Sys_info.bitplanes > 8)
		write_mode = S_AND_D;
	else
		write_mode = S_OR_D;

	color_index[0] = 1;
	color_index[1] = 0;
	color_index2[0] = 0;
	color_index2[1] = 1;
	
	ob_x = parm->pb_x;
	ob_y = parm->pb_y;
	
	Corner = CORNER2;
	
	
	buttonimg = u_tree[Corner].ob_spec.userblk;
	if(buttonimg && Sys_info.Max_col >= 4)
		quell_img = (CICONBLK *)buttonimg->ub_parm;
	else
		quell_img = u_tree[Corner].ob_spec.ciconblk; 
	
	
	dest.fd_addr = NULL;
	
	source.fd_w = 48;
	source.fd_h = 42;
	source.fd_wdwidth = 3;
	source.fd_stand = 0;

	pxy[0] = ob_x;
	pxy[1] = ob_y;
	pxy[2] = ob_x + source.fd_w - 1;
	pxy[3] = ob_y + source.fd_h - 1;
	vsf_color(handle, Sys_info.AES_bgcolor);
	vswr_mode(handle, MD_REPLACE);
	vr_recfl(handle, pxy);

	
	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = 47 /*source.fd_w - 1*/;
	pxy[3] = 41 /*source.fd_h - 1*/;
	pxy[4] = ob_x;
	pxy[5] = ob_y;
	pxy[6] = ob_x + pxy[2];
	pxy[7] = ob_y + pxy[3];
	
	if(Sys_info.Max_col >= 4)
	{
		source.fd_nplanes = 1;
		source.fd_addr = quell_img->mainlist->col_mask;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		vrt_cpyfm(handle, S_AND_NOTD, pxy, &source, &dest, color_index2);  		/* Farb-Rastercopy */

		source.fd_addr = quell_img->mainlist->col_data;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes = quell_img->mainlist->num_planes;	/* Bitplanes des Quellimages */
		vro_cpyfm(handle, write_mode, pxy, &source, &dest);  	/* Farb-Rastercopy */
	}
	else
	{
		source.fd_addr = quell_img->monoblk.ib_pdata;			/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes = 1;									/* Bitplanes des Quellimages */
		vrt_cpyfm(handle, MD_REPLACE, pxy, &source, &dest, color_index);  		/* Farb-Rastercopy */
	}
	
	
	return (parm->pb_currstate&~SELECTED);
}


/* Winkelobjekt zeichnen
 * der eingestellte Winkel muž irgendwo in der OBJECT-Struktur
 * gespeichert werden - einen der bei IBOXes vorhandenen
 * Parameter muž ich also dazu mižbrauchen.
 */
int	cdecl drawAngleObject(PARMBLK *parm)
{
	CICONBLK *quell_img;
	MFDB source,dest;
	int pxy[8];
	int ob_x,ob_y;
	int color_index[2];
	int color_index2[2];
	int Corner;
	USERBLK	*buttonimg;
	int write_mode;
	
	if(Sys_info.bitplanes>8) write_mode=S_AND_D;
	else write_mode=S_OR_D;

	color_index[0]=1;
	color_index[1]=0;
	color_index2[0]=0;
	color_index2[1]=1;
	
	ob_x=parm->pb_x;
	ob_y=parm->pb_y;
	
	Corner=CORNER2;
	
	
	buttonimg=(USERBLK*)(u_tree[Corner].ob_spec.userblk);
	if(buttonimg && Sys_info.Max_col>=4)
		quell_img=(CICONBLK*)(buttonimg->ub_parm);
	else
		quell_img=(CICONBLK*)u_tree[Corner].ob_spec.iconblk; 
	
	
	dest.fd_addr=NULL;
	
	source.fd_w=48;
	source.fd_h=42;
	source.fd_wdwidth=3;
	source.fd_stand=0;

	pxy [0] = ob_x;
	pxy [1] = ob_y;
	pxy [2] = ob_x + source.fd_w-1;
	pxy [3] = ob_y + source.fd_h-1;
	vsf_color(handle, Sys_info.AES_bgcolor);
	vswr_mode(handle, MD_REPLACE);
	vr_recfl(handle, pxy);

	
	pxy [0] = 0;
	pxy [1] = 0;
	pxy [2] = 47 /*source.fd_w-1*/;
	pxy [3] = 41 /*source.fd_h-1*/;
	pxy [4] = ob_x;
	pxy [5] = ob_y;
	pxy [6] = ob_x + pxy [2];
	pxy [7] = ob_y+ pxy [3];
	
	if(Sys_info.Max_col>=4)
	{
		source.fd_nplanes=1;
		source.fd_addr=quell_img->mainlist->col_mask;		/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		vrt_cpyfm(handle,S_AND_NOTD,pxy,&source,&dest, color_index2);  		/* Farb-Rastercopy */

		source.fd_addr=quell_img->mainlist->col_data;		/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes=quell_img->mainlist->num_planes;	/* Bitplanes des Quellimages */
		vro_cpyfm(handle, write_mode, pxy, &source,&dest);  		/* Farb-Rastercopy */
	}
	else
	{
		source.fd_addr=quell_img->monoblk.ib_pdata;		/* *mainlist: Zeiger auf CICON-Struktur, col_data sind die Bilddaten */
		source.fd_nplanes=1;								/* Bitplanes des Quellimages */
		vrt_cpyfm(handle,MD_REPLACE,pxy,&source,&dest, color_index);  		/* Farb-Rastercopy */
	}
	
	
	return (parm->pb_currstate & ~SELECTED) ;
}