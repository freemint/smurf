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
#include "smurf.h"
#include "..\sym_gem.h"
#include "..\modules\import.h"
#include "smurfine.h"
#include "globdefs.h"
#include "popdefin.h"
#include "smurf_st.h"
#include "smurf_f.h"

#include "smurfobs.h"
#include "ext_obs.h"

void check_clipping(void);
void applyConfig(long *loadcnf);

#define	ABS(i)	(i>0? i : -i)

extern	CROSSHAIR	position_markers[20];		/* Positionsmarker fÅr die Editmodule */

extern	SYSTEM_INFO Sys_info;			/* Systemkonfiguration */
extern	SERVICE_FUNCTIONS global_services;
extern	DISPLAY_MODES Display_Opt;

extern	int openmode;					/* Dialog neu geîffnet (0) oder buttonevent? (!=0) */
extern	int edit_mod_num;				/* Modul-ID des Moduls, das das Einstellformular benutzt */
extern	int	obj;						/* Objekt beim loslassen des Buttons */
extern	int	key_scancode;				/* Scancode beim letzten Keyboard-Event */
extern	int key_ascii;

extern	char *edit_modules[100];		/* Pfade fÅr bis zu 100 Edit-Module */

extern	SMURF_PIC *smurf_picture[MAX_PIC];
extern	int active_pic;
extern	int picwindthere, dialwindthere, picthere;
extern	key_at_event;

extern	long sx1, sx2, sx3, sx4;		/* Maxima */
extern	long sn1, sn2, sn3, sn4;		/* Minima */
extern	int	sy1,sy2,sy3,sy4;			/* eingestellte Sliderwerte */
	
/*----- Window-Variablen --------*/
extern	WINDOW wind_s[25];
extern	WINDOW picture_windows[MAX_PIC];
extern	POP_UP	popups[25];
extern	SLIDER sliders[15];
extern	char module_pics[21][7];

extern MFORM *dummy_ptr;

void f_make_preview(int redraw_flag);
void f_insert_prefs(GARGAMEL *smurf_st, SMURF_PIC *picture);
void check_prevzoom(void);

SMURF_PIC *module_preview = NULL;
SMURF_PIC move_prev;
SMURF_PIC *oldpic;
int oldxoff, oldyoff;
int prev_zoom=1;

/*------------------- Modul-Einstell-Formular îffnen ------------------	*/
/* Vom Modul aufgerufen: mod_id (GARGAMEL->module_number) ist			*/
/* Die Nummer des Moduls, das gerade das Einstellformular benutzt.		*/
/*---------------------------------------------------------------------	*/
void f_module_prefs(MOD_INFO *infostruct, int mod_id)
{
	char edd[6];
	char cmp_modname[30];

	int sd1,sd2,sd3,sd4;
	int min_val, max_val, redraw_me = 0, back;
	int t;
	int endwidth, endheight, zoom, index;

	long editval1, editval2, editval3, editval4, *cnfblock;

	OBJECT *pref_form;

	extern void *edit_cnfblock[100];


	/* evtl. altes Modul terminieren */
	if(edit_mod_num != -1)
	{
		module.comm.startEdit("", module.bp[edit_mod_num], MTERM, edit_mod_num, module.smStruct[edit_mod_num]);
		check_and_terminate(module.smStruct[edit_mod_num]->module_mode, edit_mod_num);
		for(t = 0; t < 7; t++)
			module_pics[edit_mod_num][t] = 0;

		edit_mod_num = -1;
		Dialog.busy.dispRAM();
		redraw_me = 1;
	}

	if(mod_id < 0 || mod_id > 20)
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[MODINIT_ID_ERR].TextCast, NULL, NULL, NULL, 1);


	/* altes Preview freigeben */
	if(module_preview != NULL)
	{
		if(module_preview->screen_pic != NULL)
			SMfree(module_preview->screen_pic->fd_addr);

		free(module_preview->screen_pic);
		SMfree(module_preview);
	}

	module_preview = NULL;


	/* neues Modul holen */
	edit_mod_num = mod_id;
	pref_form = wind_s[WIND_MODFORM].resource_form;
	strncpy(wind_s[WIND_MODFORM].wtitle, infostruct->mod_name, 40);
	if(wind_s[WIND_MODFORM].whandlem > 0)
		Window.windSet(wind_s[WIND_MODFORM].whandlem, WF_NAME, LONG2_2INT((long)wind_s[WIND_MODFORM].wtitle), 0,0);

	/*
	 * unbenutzte Elemente verstecken
	 */
	if(strlen(infostruct->slide1)==0)
	{
		pref_form[S1_F].ob_flags|=HIDETREE;
		pref_form[S1_A].ob_flags|=HIDETREE;
		pref_form[S1_R].ob_flags|=HIDETREE;
	}
	else
	{
		pref_form[S1_F].ob_flags&=~HIDETREE;
		pref_form[S1_A].ob_flags&=~HIDETREE;
		pref_form[S1_R].ob_flags&=~HIDETREE;
	}

	if(strlen(infostruct->slide2)==0)
	{
		pref_form[S2_F].ob_flags|=HIDETREE;
		pref_form[S2_A].ob_flags|=HIDETREE;
		pref_form[S2_R].ob_flags|=HIDETREE;
	}
	else
	{
		pref_form[S2_F].ob_flags&=~HIDETREE;
		pref_form[S2_A].ob_flags&=~HIDETREE;
		pref_form[S2_R].ob_flags&=~HIDETREE;
	}

	if(strlen(infostruct->slide3)==0)
	{
		pref_form[S3_F].ob_flags|=HIDETREE;
		pref_form[S3_A].ob_flags|=HIDETREE;
		pref_form[S3_R].ob_flags|=HIDETREE;
	}
	else
	{
		pref_form[S3_F].ob_flags&=~HIDETREE;
		pref_form[S3_A].ob_flags&=~HIDETREE;
		pref_form[S3_R].ob_flags&=~HIDETREE;
	}

	if(strlen(infostruct->slide4)==0)
	{
		pref_form[S4_F].ob_flags|=HIDETREE;
		pref_form[S4_A].ob_flags|=HIDETREE;
		pref_form[S4_R].ob_flags|=HIDETREE;
	}
	else
	{
		pref_form[S4_F].ob_flags&=~HIDETREE;
		pref_form[S4_A].ob_flags&=~HIDETREE;
		pref_form[S4_R].ob_flags&=~HIDETREE;
	}

	if(strlen(infostruct->check1)==0)
		pref_form[CHECK1].ob_flags|=HIDETREE;
	else
		pref_form[CHECK1].ob_flags&=~HIDETREE;

	if(strlen(infostruct->check2)==0)
		pref_form[CHECK2].ob_flags|=HIDETREE;
	else
		pref_form[CHECK2].ob_flags&=~HIDETREE;

	if(strlen(infostruct->check3)==0)
		pref_form[CHECK3].ob_flags|=HIDETREE;
	else
		pref_form[CHECK3].ob_flags&=~HIDETREE;

	if(strlen(infostruct->check4)==0)
		pref_form[CHECK4].ob_flags|=HIDETREE;
	else
		pref_form[CHECK4].ob_flags&=~HIDETREE;

	if(strlen(infostruct->edit1)==0)
		pref_form[ED1].ob_flags|=HIDETREE;
	else
		pref_form[ED1].ob_flags&=~HIDETREE;

	if(strlen(infostruct->edit2)==0)
		pref_form[ED2].ob_flags|=HIDETREE;
	else
		pref_form[ED2].ob_flags&=~HIDETREE;

	if(strlen(infostruct->edit3)==0)
		pref_form[ED3].ob_flags|=HIDETREE;
	else
		pref_form[ED3].ob_flags&=~HIDETREE;

	if(strlen(infostruct->edit4)==0)
		pref_form[ED4].ob_flags|=HIDETREE;
	else
		pref_form[ED4].ob_flags&=~HIDETREE;

	if(infostruct->how_many_pix > 1)
		pref_form[PICS_INFO].ob_flags&=~HIDETREE;
	else
		pref_form[PICS_INFO].ob_flags|=HIDETREE;

	/*
	 * und jetzt alle abgeschalteten Objekte disablen 
	 */
	for(t=pref_form[0].ob_head; t<pref_form[0].ob_tail; t++)
	{
		if(pref_form[t].ob_flags & HIDETREE)
			pref_form[t].ob_state |= DISABLED;
		else
			pref_form[t].ob_state &= ~DISABLED;
	}

	/* 
	 * Muû der Cursor komplett ausgeschaltet werden (kein Editobjekt da)?
	 */
	if(strlen(infostruct->edit1)==0 && strlen(infostruct->edit2)==0 && 
	   strlen(infostruct->edit3)==0 && strlen(infostruct->edit4)==0)
		wind_s[WIND_MODFORM].editob=0;
	else
		wind_s[WIND_MODFORM].editob=ED1;
/*
	/*------------------------------------ Formulartexte lîschen */	
	memset(pref_form[S1_TIT].TextCast, 0x0, 8);
	memset(pref_form[S2_TIT].TextCast, 0x0, 8);
	memset(pref_form[S3_TIT].TextCast, 0x0, 8);
	memset(pref_form[S4_TIT].TextCast, 0x0, 8);
	memset(pref_form[CHECK1_TEXT].TextCast, 0x0, 15);
	memset(pref_form[CHECK1_TEXT].TextCast, 0x0, 15);
	memset(pref_form[CHECK1_TEXT].TextCast, 0x0, 15);
	memset(pref_form[CHECK1_TEXT].TextCast, 0x0, 15);
	memset(pref_form[ED1_TIT].TextCast, 0x0, 10);
	memset(pref_form[ED2_TIT].TextCast, 0x0, 10);
	memset(pref_form[ED3_TIT].TextCast, 0x0, 10);
	memset(pref_form[ED4_TIT].TextCast, 0x0, 10);
*/
	/*----------------------------- neue Formulartexte eintragen */	
	strcpy(pref_form[S1_TIT].TextCast, infostruct->slide1);
	strcpy(pref_form[S2_TIT].TextCast, infostruct->slide2);
	strcpy(pref_form[S3_TIT].TextCast, infostruct->slide3);
	strcpy(pref_form[S4_TIT].TextCast, infostruct->slide4);
	strcpy(pref_form[ED1_TIT].TextCast, infostruct->edit1);
	strcpy(pref_form[ED2_TIT].TextCast, infostruct->edit2);
	strcpy(pref_form[ED3_TIT].TextCast, infostruct->edit3);
	strcpy(pref_form[ED4_TIT].TextCast, infostruct->edit4);
	strcpy(pref_form[CHECK1_TEXT].TextCast, infostruct->check1);
	strcpy(pref_form[CHECK2_TEXT].TextCast, infostruct->check2);
	strcpy(pref_form[CHECK3_TEXT].TextCast, infostruct->check3);
	strcpy(pref_form[CHECK4_TEXT].TextCast, infostruct->check4);

	/*------------------------------ Slider-Mins / Maxs auslesen */
	max_val = (int)infostruct->smax1;
	min_val = (int)infostruct->smin1;
	sliders[PDSLIDER1].max_val = max_val;
	sliders[PDSLIDER1].min_val = min_val;
	max_val = (int)infostruct->smax2;
	min_val = (int)infostruct->smin2;
	sliders[PDSLIDER2].max_val = max_val;
	sliders[PDSLIDER2].min_val = min_val;
	max_val = (int)infostruct->smax3;
	min_val = (int)infostruct->smin3;
 	sliders[PDSLIDER3].max_val = max_val;
	sliders[PDSLIDER3].min_val = min_val;
	max_val = (int)infostruct->smax4;
	min_val = (int)infostruct->smin4;
	sliders[PDSLIDER4].max_val = max_val;
	sliders[PDSLIDER4].min_val = min_val;

	/*------------------------------- Slider-Defaults auslesen */
	sy1 = sd1 = (int)infostruct->sdef1;
	sy2 = sd2 = (int)infostruct->sdef2;
	sy3 = sd3 = (int)infostruct->sdef3;
	sy4 = sd4 = (int)infostruct->sdef4;

	/*--------------------------------- Edit-Defaults setzen */
	editval1 = infostruct->edef1;
	if(editval1 < infostruct->emin1)
		editval1 = infostruct->emin1;
	else if(editval1 > infostruct->emax1)
		editval1 = infostruct->emax1;

	editval2 = infostruct->edef2;
	if(editval2 < infostruct->emin2) 
		editval2 = infostruct->emin2;
	else if(editval2 > infostruct->emax2)
		editval2 = infostruct->emax2;

	editval3 = infostruct->edef3;
	if(editval3 < infostruct->emin3)
		editval3 = infostruct->emin3;
	else if(editval3 > infostruct->emax3)
		editval3 = infostruct->emax3;

	editval4 = infostruct->edef4;
	if(editval4 < infostruct->emin4) 
		editval4 = infostruct->emin4;
	else if(editval4 > infostruct->emax4)
			editval4 = infostruct->emax4;

	ltoa(editval1, edd, 10);
	strncpy(pref_form[ED1].TextCast, edd, 5);
	ltoa(editval2, edd, 10);
	strncpy(pref_form[ED2].TextCast, edd, 5);
	ltoa(editval3, edd, 10);
	strncpy(pref_form[ED3].TextCast, edd, 5);
	ltoa(editval4, edd, 10);
	strncpy(pref_form[ED4].TextCast, edd, 5);

	/*---------------------------- Check-Defaults setzen */
	/* Radiobutton oder Checkbox? */
	pref_form[CHECK1].ob_type &= ~0xFF00;
	pref_form[CHECK2].ob_type &= ~0xFF00;
	pref_form[CHECK3].ob_type &= ~0xFF00;
	pref_form[CHECK4].ob_type &= ~0xFF00;

	if(infostruct->cdef1 >= 2)
	{
		pref_form[CHECK1].ob_type &= ~(CHECKBOX << 8);
		pref_form[CHECK1].ob_type |= RADIOBUTTON << 8;
		pref_form[CHECK1].ob_flags |= RBUTTON;
	}
	else
	{
		pref_form[CHECK1].ob_type &= ~(RADIOBUTTON << 8);
		pref_form[CHECK1].ob_type |= CHECKBOX << 8;
		pref_form[CHECK1].ob_flags &= ~RBUTTON;
	}

	if(infostruct->cdef2 >= 2)
	{
		pref_form[CHECK2].ob_type &= ~(CHECKBOX << 8);
		pref_form[CHECK2].ob_type |= RADIOBUTTON << 8;
		pref_form[CHECK2].ob_flags |= RBUTTON;
	}
	else
	{
		pref_form[CHECK2].ob_type &= ~(RADIOBUTTON << 8);
		pref_form[CHECK2].ob_type |= CHECKBOX << 8;
		pref_form[CHECK2].ob_flags &= ~RBUTTON;
	}

	if(infostruct->cdef3 >= 2)
	{
		pref_form[CHECK3].ob_type &= ~(CHECKBOX << 8);
		pref_form[CHECK3].ob_type |= RADIOBUTTON << 8;
		pref_form[CHECK3].ob_flags |= RBUTTON;
	}
	else
	{	
		pref_form[CHECK3].ob_type &= ~(RADIOBUTTON << 8);
		pref_form[CHECK3].ob_type |= CHECKBOX << 8;
		pref_form[CHECK3].ob_flags &= ~RBUTTON;
	}

	if(infostruct->cdef4 >= 2)
	{
		pref_form[CHECK4].ob_type &= ~(CHECKBOX << 8);
		pref_form[CHECK4].ob_type |= RADIOBUTTON << 8;
		pref_form[CHECK4].ob_flags |= RBUTTON;
	}
	else
	{	
		pref_form[CHECK4].ob_type &= ~(RADIOBUTTON << 8);
		pref_form[CHECK4].ob_type |= CHECKBOX << 8;
		pref_form[CHECK4].ob_flags &= ~RBUTTON;
	}

	f_treewalk(pref_form, 0);

	if(infostruct->cdef1 != 0 && infostruct->cdef1 != 3)
		pref_form[CHECK1].ob_state |= SELECTED;
	else
		pref_form[CHECK1].ob_state &= ~SELECTED;
	if(infostruct->cdef2 != 0 && infostruct->cdef2 != 3)
		pref_form[CHECK2].ob_state |= SELECTED;
	else
		pref_form[CHECK2].ob_state &= ~SELECTED;
	if(infostruct->cdef3 != 0 && infostruct->cdef3 != 3)
		pref_form[CHECK3].ob_state |= SELECTED;
	else
		pref_form[CHECK3].ob_state &= ~SELECTED;
	if(infostruct->cdef4 != 0 && infostruct->cdef4 != 3)
		pref_form[CHECK4].ob_state |= SELECTED;
	else
		pref_form[CHECK4].ob_state &= ~SELECTED;


	/*
	 * evtl. gemerkte Konfiguration wieder in den Dialog Åbertragen
	 */
	strcpy(cmp_modname, infostruct->mod_name);
	if(strlen(cmp_modname) < 28)
		strncat(cmp_modname, "                           ", 28-strlen(cmp_modname));
	for(index = 0; index < Dialog.emodList.anzahl; index++)
	{
		if(strcmp(Dialog.emodList.modNames[index], cmp_modname) == 0)
			break;
	}

	if(edit_cnfblock[index] != NULL)
	{
		cnfblock = (long *)edit_cnfblock[index];
		applyConfig(cnfblock);
		sd1 = cnfblock[0];
		sd2 = cnfblock[1];
		sd3 = cnfblock[2];
		sd4 = cnfblock[3];
	}

	/*
	 * Windowstruktur fertigmachen
	 */
	wind_s[WIND_MODFORM].module = edit_mod_num;
	
	wind_s[WIND_MODFORM].pic_xpos = pref_form[PREV_OUTER].ob_x + pref_form[PREV_BOX].ob_x;
	wind_s[WIND_MODFORM].pic_ypos = pref_form[PREV_OUTER].ob_y + pref_form[PREV_BOX].ob_y;
	wind_s[WIND_MODFORM].clipwid = pref_form[PREV_BOX].ob_width;
	wind_s[WIND_MODFORM].cliphgt = pref_form[PREV_BOX].ob_height;

	zoom = prev_zoom = 1;
	if(smurf_picture[active_pic]->block == NULL)
	{
		endwidth = smurf_picture[active_pic]->pic_width / zoom;
		endheight = smurf_picture[active_pic]->pic_height / zoom;
	}
	else
	{
		endwidth = smurf_picture[active_pic]->block->pic_width / zoom;
		endheight = smurf_picture[active_pic]->block->pic_height / zoom;
	}

	if(pref_form[PREV_BOX].ob_width > endwidth)
		wind_s[WIND_MODFORM].clipwid = endwidth;
	if(pref_form[PREV_BOX].ob_height > endheight)
		wind_s[WIND_MODFORM].cliphgt = endheight;
	
	/*
	 * Move-Preview - Struktur vorbereiten
	 */
	if(smurf_picture[active_pic]->block == NULL)
	{
		memcpy(&move_prev, smurf_picture[active_pic], sizeof(SMURF_PIC));
		move_prev.local_nct = NULL;
	}
	else
	{
		memcpy(&move_prev, smurf_picture[active_pic]->block, sizeof(SMURF_PIC));
		move_prev.local_nct = NULL;
	}

	/*
	 * Echtzeitdithering erzwingen und move_prev ins Fenster hÑngen
	 */
	move_prev.screen_pic = NULL;
	move_prev.zoom = 0;
	wind_s[WIND_MODFORM].picture = &move_prev;

	check_prevzoom();								/* Zoombuttons fÅrs Preview ein/ausschalten */
	
	if(wind_s[WIND_MODFORM].whandlem <= 0)			/* Fenster neu? -> kein weiterer Redraw nîtig. */
		redraw_me = 0;
	
	back = Window.open(&wind_s[WIND_MODFORM]);

	if(back != -1)
	{
		if(redraw_me)
			Window.redraw(&wind_s[WIND_MODFORM], NULL, 0, 0);

		/* Slider drauf setzen */
		setslider(&sliders[PDSLIDER1], sd1);
		setslider(&sliders[PDSLIDER2], sd2);
		setslider(&sliders[PDSLIDER3], sd3);
		setslider(&sliders[PDSLIDER4], sd4);
	}

	module.smStruct[edit_mod_num]->wind_struct = &wind_s[WIND_MODFORM];

	Dialog.busy.ok();

	return;
} /* f_module_prefs */


/* ------------------------------------------------------------------------	*/
/* --------------------------- MODUL- FORMULAR ----------------------------	*/
/* Nimmt Einstellungen im Modulformular vor und Åbergibt ggfs. ans Modul 	*/
/* ------------------------------------------------------------------------ */
void f_mpref_change(void)
{
	char str[10], confname[33] = "";
	char *textbeg;

	int button, back, picnum, oldtop;
	int oldw, oldh;
	int newwid, newhgt;
	int clip, close_me=0, mem=0, dummy, staste;
	unsigned int w2;

	long editval1, editval2, editval3, editval4;
	long cnfblock[12], *loadcnf;
	long *cnf_save;
	unsigned long w1;

	OBJECT *modtree;
	SMURF_PIC *edit_picture;
	SMURF_PIC *pic;
	MOD_ABILITY *mod_abs;
	MOD_INFO *mod_inf;


	modtree = wind_s[WIND_MODFORM].resource_form;

	button = Dialog.init(WIND_MODFORM, START_MOD);

	if((key_scancode >> 8) == KEY_UP || (key_scancode >> 8) == KEY_DOWN)
	{
		textbeg = module.bp[edit_mod_num]->p_tbase;
		
		mod_inf = (MOD_INFO *)*((MOD_INFO **)(textbeg + MOD_INFO_OFFSET));
		mod_abs = *((MOD_ABILITY **)(textbeg + MOD_ABS_OFFSET));

		editval1 = atol(modtree[ED1].TextCast);
		if(editval1 < mod_inf->emin1)
		{
			editval1 = mod_inf->emin1;
			clip = 1;
		}
		else
			if(editval1 > mod_inf->emax1) 
			{
				editval1 = mod_inf->emax1;
				clip = 1;
			}

		editval2 = atol(modtree[ED2].TextCast);
		if(editval2 < mod_inf->emin2) 
		{
			editval2 = mod_inf->emin2;
			clip = 2;
		}
		else
			if(editval2 > mod_inf->emax2) 
			{
				editval2 = mod_inf->emax2;
				clip = 2;
			}

		editval3 = atol(modtree[ED3].TextCast);
		if(editval3 < mod_inf->emin3)
		{
			editval3 = mod_inf->emin3;
			clip = 3;
		}
		else
			if(editval3 > mod_inf->emax3) 
			{
				editval3 = mod_inf->emax3;
				clip = 3;
			}

		editval4 = atol(modtree[ED4].TextCast);
		if(editval4 < mod_inf->emin4) 
		{
			editval4 = mod_inf->emin4;
			clip = 4;
		}
		else
			if(editval4 > mod_inf->emax4) 
			{
				editval4 = mod_inf->emax4;
				clip = 4;
			}


		if(clip == 1)
		{
			ltoa(editval1, str, 10);
			strncpy(modtree[ED1].TextCast, str, 6);
			Window.redraw(&wind_s[WIND_MODFORM], NULL, ED1, DRAWNOPICTURE);
		}
		else
		if(clip == 2)
		{
			ltoa(editval2, str, 10);
			strncpy(modtree[ED2].TextCast, str, 6);
			Window.redraw(&wind_s[WIND_MODFORM], NULL, ED2,DRAWNOPICTURE);
		}
		else
		if(clip == 3)
		{
			ltoa(editval3, str, 10);
			strncpy(modtree[ED3].TextCast, str, 6);
			Window.redraw(&wind_s[WIND_MODFORM], NULL, ED3, DRAWNOPICTURE);
		}
		else
		if(clip == 4)
		{
			ltoa(editval4, str, 10);
			strncpy(modtree[ED4].TextCast, str, 6);
			Window.redraw(&wind_s[WIND_MODFORM], NULL, ED4, DRAWNOPICTURE);
		}
	}

	switch(button)
	{
		case PREVZOOM_DEC:	prev_zoom++; 
								check_clipping();
								f_make_preview(0);
								change_object(&wind_s[WIND_MODFORM], PREVZOOM_DEC, UNSEL, 1);
								break;

		case PREVZOOM_INC:	if(prev_zoom>1)
								prev_zoom--; 
								check_clipping();
								f_make_preview(0);
								change_object(&wind_s[WIND_MODFORM], PREVZOOM_INC, UNSEL, 1);
								break;

		case PREVZOOM_RESET:	prev_zoom=1; 
								check_clipping();
								f_make_preview(0);
								change_object(&wind_s[WIND_MODFORM], PREVZOOM_RESET, UNSEL, 1);
								break;


		/*---- Schiebereglerbedienung: ----*/
		case S1_F:
		case S1_R:	sy1 = f_rslid(&sliders[PDSLIDER1]); 
					break;
		case S2_F:
		case S2_R:	sy2 = f_rslid(&sliders[PDSLIDER2]); 
					break;
		case S3_F:
		case S3_R:	sy3 = f_rslid(&sliders[PDSLIDER3]); 
					break;
		case S4_F:
		case S4_R:	sy4 = f_rslid(&sliders[PDSLIDER4]); 
					break;

		case S1_A:	sy1 = f_numedit(S1_A, modtree, sy1);
					if(sy1 > sliders[PDSLIDER1].max_val)
						sy1 = (int)sliders[PDSLIDER1].max_val;
					else
						if(sy1 < sliders[PDSLIDER1].min_val)
							sy1 = (int)sliders[PDSLIDER1].min_val;
					setslider(&sliders[PDSLIDER1], sy1);
					break;
		case S2_A:	sy2 = f_numedit(S2_A, modtree, sy2); 
					if(sy2 > sliders[PDSLIDER2].max_val)
						sy2 = (int)sliders[PDSLIDER2].max_val;
					else
						if(sy2 < sliders[PDSLIDER2].min_val)
							sy2 = (int)sliders[PDSLIDER2].min_val;
					setslider(&sliders[PDSLIDER2], sy2);
					break;
		case S3_A:	sy3 = f_numedit(S3_A, modtree, sy3); 
					if(sy3 > sliders[PDSLIDER3].max_val)
						sy3 = (int)sliders[PDSLIDER3].max_val;
					else
						if(sy3 < sliders[PDSLIDER3].min_val) sy3 = (int)sliders[PDSLIDER3].min_val;
					setslider(&sliders[PDSLIDER3], sy3);
					break;
		case S4_A:	sy4 = f_numedit(S4_A, modtree, sy4);
					if(sy4 > sliders[PDSLIDER4].max_val)
						sy4 = (int)sliders[PDSLIDER4].max_val;
					else
						if(sy4 < sliders[PDSLIDER4].min_val)
							sy4 = (int)sliders[PDSLIDER4].min_val;
					setslider(&sliders[PDSLIDER4], sy4);
					break;

		/* Preview-Icon angeklickt! */
		case PREV_BOX:	if(picthere)
						{
							pic = smurf_picture[active_pic];
							if(pic->block != NULL)
								pic = pic->block;
							f_move_preview(&wind_s[WIND_MODFORM], pic, PREV_ACTION);
							f_make_preview(DRAWNOTREE);
						}
						break;

		case PICS_INFO:	ready_modpics_popup(&wind_s[WIND_MODFORM]);
						f_pop(&popups[POPUP_PICORDER], 1, 0, NULL);		/* Bildreihenfolge-popup îffnen */
						change_object(&wind_s[WIND_MODFORM], PICS_INFO, UNSEL, 1);
						break;
			
		case MCONF_STDSAVE:	textbeg = module.bp[edit_mod_num]->p_tbase;
							mod_inf = (MOD_INFO *)*((MOD_INFO **)(textbeg + MOD_INFO_OFFSET));
							cnfblock[0] = sy1;
							cnfblock[1] = sy2;
							cnfblock[2] = sy3;
							cnfblock[3] = sy4;
							cnfblock[4] = modtree[CHECK1].ob_state;
							cnfblock[5] = modtree[CHECK2].ob_state;
							cnfblock[6] = modtree[CHECK3].ob_state;
							cnfblock[7] = modtree[CHECK4].ob_state;
							cnfblock[8] = atol(modtree[ED1].TextCast);
							cnfblock[9] = atol(modtree[ED2].TextCast);
							cnfblock[10] = atol(modtree[ED3].TextCast);
							cnfblock[11] = atol(modtree[ED4].TextCast);
							mconfSave(mod_inf, edit_mod_num, cnfblock, 12 * 4, confname);
							change_object(&wind_s[WIND_MODFORM], MCONF_STDSAVE, UNSEL, 1);
							break;
								
		case MCONF_STDLOAD:	textbeg = module.bp[edit_mod_num]->p_tbase;
							mod_inf = (MOD_INFO *)*((MOD_INFO **)(textbeg + MOD_INFO_OFFSET));
								loadcnf = (long *)mconfLoad(mod_inf, edit_mod_num, confname);
							if(loadcnf != NULL)
							{
								applyConfig(loadcnf);
								SMfree(loadcnf);
								Window.redraw(&wind_s[WIND_MODFORM], NULL, 0,0);
								setslider(&sliders[PDSLIDER1], sy1);
								setslider(&sliders[PDSLIDER2], sy2);
								setslider(&sliders[PDSLIDER3], sy3);
								setslider(&sliders[PDSLIDER4], sy4);
							}
							change_object(&wind_s[WIND_MODFORM], MCONF_STDLOAD, UNSEL, 1);
							break;

		/* Los! (Start gedrÅckt): */
		case START_MOD:
			change_object(&wind_s[WIND_MODFORM], START_MOD, UNSEL, 1);

			textbeg=module.bp[edit_mod_num]->p_tbase;
			mod_inf=(MOD_INFO *)*((MOD_INFO **)(textbeg + MOD_INFO_OFFSET));

			/*
			 * Editfeld-Werte erneut clippen, da diese vor dem Modulstart
			 * noch geÑndert werden kînnen
			 */
			editval1 = atol(modtree[ED1].TextCast);
			editval2 = atol(modtree[ED2].TextCast);
			editval3 = atol(modtree[ED3].TextCast);
			editval4 = atol(modtree[ED4].TextCast);
			if(editval1<mod_inf->emin1)			editval1 = mod_inf->emin1;
			else if(editval1>mod_inf->emax1)		editval1 = mod_inf->emax1;
			if(editval2<mod_inf->emin2)			editval2 = mod_inf->emin2;
			else if(editval2>mod_inf->emax2)		editval2 = mod_inf->emax2;
			if(editval3<mod_inf->emin3)			editval3 = mod_inf->emin3;
			else if(editval3>mod_inf->emax3)		editval3 = mod_inf->emax3;
			if(editval4<mod_inf->emin4)			editval4 = mod_inf->emin4;
			else if(editval4>mod_inf->emax4)		editval4 = mod_inf->emax4;
			ltoa(editval1, modtree[ED1].TextCast, 10);
			ltoa(editval2, modtree[ED2].TextCast, 10);
			ltoa(editval3, modtree[ED3].TextCast, 10);
			ltoa(editval4, modtree[ED4].TextCast, 10);
				

			/*
			 * Default-Konfigurationsblock anfordern
			 */
			cnf_save = SMalloc(50);
			cnf_save[0] = sy1;
			cnf_save[1] = sy2;
			cnf_save[2] = sy3;
			cnf_save[3] = sy4;
			cnf_save[4] = modtree[CHECK1].ob_state;
			cnf_save[5] = modtree[CHECK2].ob_state;
			cnf_save[6] = modtree[CHECK3].ob_state;
			cnf_save[7] = modtree[CHECK4].ob_state;
			cnf_save[8] = editval1;
			cnf_save[9] = editval2;
			cnf_save[10] = editval3;
			cnf_save[11] = editval4;
			w1 = (long)cnf_save>>16;
			w2 = (int)cnf_save;
			module.smStruct[edit_mod_num]->event_par[0] = w1;
			module.smStruct[edit_mod_num]->event_par[1] = w2;
			module.smStruct[edit_mod_num]->event_par[2] = 12*4;
			memorize_emodConfig(module.bp[edit_mod_num], module.smStruct[edit_mod_num]);

			if(!picthere)
				break;
	
			/*
			 * Gargamel-Struktur fÅllen
			 */
			Window.topNow(&wind_s[WIND_BUSY]);
			textbeg = module.bp[edit_mod_num]->p_tbase;
			mod_inf = (MOD_INFO *)*((MOD_INFO **)(textbeg + MOD_INFO_OFFSET));
			mod_abs = *((MOD_ABILITY **)(textbeg + MOD_ABS_OFFSET));

			pic = smurf_picture[active_pic];

			if(mod_inf->how_many_pix > 1)
			{
				back = f_give_pics(mod_inf, mod_abs, edit_mod_num);
				if(back != 0)
					return;
			}
			else
			{
				if(pic->block != NULL)
					pic = pic->block;

				back = f_convert(pic, mod_abs, RGB, SAME, 0);
				if(back == M_MEMORY) 
				{
					mem =1;
					Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_MEM].TextCast, NULL, NULL, NULL, 1);
					Dialog.busy.reset(128, "Speicher ...");
				}
			}
			
			if(mem == 0)
			{
				f_insert_prefs(module.smStruct[edit_mod_num], pic);
	
				oldw = pic->pic_width;
				oldh = pic->pic_height;
				
				/*
				 * Modul starten... 
				 */
				Window.windGet(0, WF_TOP, &oldtop, 0,0,0);
				Window.topNow(&wind_s[WIND_BUSY]);
				Window.redraw(&wind_s[WIND_BUSY], NULL, 0, 0);
	
				module.smStruct[edit_mod_num]->event_par[0] = position_markers[edit_mod_num&0xFF].xpos[0];
				module.smStruct[edit_mod_num]->event_par[1] = position_markers[edit_mod_num&0xFF].ypos[0];
				module.comm.startEdit("", module.bp[edit_mod_num], MCH_COORDS, edit_mod_num, module.smStruct[edit_mod_num]);

				Dialog.busy.reset(0, "Modul arbeitet...");
				if(key_at_event&KEY_ALT)
					close_me = 1;
				
				module.comm.startEdit(edit_modules[edit_mod_num], module.bp[edit_mod_num], MEXEC, module.smStruct[edit_mod_num]->module_number, module.smStruct[edit_mod_num]);
				f_handle_modmessage(module.smStruct[edit_mod_num]);
		
				Dialog.busy.ok();
		
				/*
				 * alles erledigt?
				 */
				if(module.smStruct[edit_mod_num]->module_mode == M_PICDONE)
				{
					if(mod_inf->how_many_pix > 1)
					{
						picnum = module.smStruct[edit_mod_num]->event_par[0];
						picnum = module_pics[edit_mod_num][picnum];
						edit_picture = smurf_picture[picnum];
					}
					else
					{
						edit_picture = module.smStruct[edit_mod_num]->smurf_pic;
						if(edit_picture->block)
						{	
							edit_picture->blockwidth = edit_picture->block->pic_width;
							edit_picture->blockheight = edit_picture->block->pic_width;
							edit_picture = edit_picture->block;
						}

						picnum = active_pic;
					}

					newwid = picture_windows[picnum].picture->pic_width;
					newhgt = picture_windows[picnum].picture->pic_height;
					if(oldw != newwid || oldh != newhgt)
						imageWindow.clipPicwin(&picture_windows[picnum]);

					picture_windows[picnum].clipwid = newwid;
					picture_windows[picnum].cliphgt = newhgt;

					/* damit bei BildÑnderungen im Fall von Median-Cut */
					/* auch die Palette neu berechnet wird und sowieso */
					if(edit_picture->local_nct)
					{
						SMfree(edit_picture->local_nct);
						edit_picture->local_nct = NULL;
					}

					if(!Sys_info.realtime_dither)
						f_dither(edit_picture, &Sys_info, 1, NULL, &Display_Opt);
	
					Window.redraw(&picture_windows[picnum], NULL, 0, 0);
					f_pic_changed(&picture_windows[picnum], 1);

					Dialog.busy.ok();
				} /* picdone */
				
				Window.topHandle(oldtop);
			} /* genug Speicher */

			if(close_me)
				Window.close(wind_s[WIND_MODFORM].whandlem);
			break;
	}

	check_prevzoom();

	/*------------ Immediate-Preview? -------------*/
	graf_mkstate(&dummy, &dummy, &dummy, &staste);
	if(Sys_info.immed_prevs == SELECTED && !(staste&0x03) && 
	   (button == S1_R || button == S2_R || button == S3_R || button == S4_R))
	{
		wind_s[WIND_MODFORM].xoffset=oldxoff;
		wind_s[WIND_MODFORM].yoffset=oldyoff;
		f_make_preview(DRAWNOTREE);
	}

	if(!close_me)
		check_and_terminate(module.smStruct[edit_mod_num]->module_mode, edit_mod_num);

	return;
} /* f_mpref_change */


/* ------------------------------------------------------------------------	*/
/*	En-/Disablen der Preview-Zoom-Buttons nach PrÅfung der Grîûen			*/
/* ------------------------------------------------------------------------	*/
void check_prevzoom(void)
{
	OBJECT *modtree;
	SMURF_PIC *pic;

	
	pic = smurf_picture[active_pic];
	if(pic->block != NULL)
		pic = pic->block;

	modtree = wind_s[WIND_MODFORM].resource_form;

	if((modtree[PREV_BOX].ob_width) * (prev_zoom + 1) >= pic->pic_width &&
	   (modtree[PREV_BOX].ob_height) * (prev_zoom + 1) >= pic->pic_height)
		change_object(&wind_s[WIND_MODFORM], PREVZOOM_DEC, DISABLED, 1);
	else
		change_object(&wind_s[WIND_MODFORM], PREVZOOM_DEC, ENABLED, 1);

	if(prev_zoom <= 1)
	{
		change_object(&wind_s[WIND_MODFORM], PREVZOOM_INC, DISABLED, 1);
		change_object(&wind_s[WIND_MODFORM], PREVZOOM_RESET, DISABLED, 1);
	}
	else
	{
		change_object(&wind_s[WIND_MODFORM], PREVZOOM_INC, ENABLED, 1);
		change_object(&wind_s[WIND_MODFORM], PREVZOOM_RESET, ENABLED, 1);
	}

	return;
} /* check_prevzoom */


/* ----------------------------------------------------------------------------	*/
/*	öberprÅft das Clipping des Previewausschnittes im Module-Preferences-Dialog	*/
/*	anhand Bildbreite, Zoom und PrevBox-Abmessungen und paût es ggfs. an.		*/
/* ----------------------------------------------------------------------------	*/
void check_clipping(void)
{
	OBJECT *modtree = wind_s[WIND_MODFORM].resource_form;


	wind_s[WIND_MODFORM].clipwid = modtree[PREV_BOX].ob_width;
	wind_s[WIND_MODFORM].cliphgt = modtree[PREV_BOX].ob_height;

	if(smurf_picture[active_pic]->pic_width / prev_zoom < modtree[PREV_BOX].ob_width)
		wind_s[WIND_MODFORM].clipwid = smurf_picture[active_pic]->pic_width / prev_zoom;
	if(smurf_picture[active_pic]->pic_height / prev_zoom < modtree[PREV_BOX].ob_height)
		wind_s[WIND_MODFORM].cliphgt = smurf_picture[active_pic]->pic_height / prev_zoom;

	return;
} /* check_clipping */


/* ------------------------------------------------------------------------		*/
/*							Preview bewegen										*/
/* Handled das Verschieben des Previews von bild orig_pic im Fenster			*/
/* window mit der Maus. Zum Redraw wird Objekt redraw_object benutzt.			*/
/* Der Redraw wird in jedem Fall mit Online-Dithering ausgefÅhrt, um			*/
/* voneinander verschiedene Preview- und Bild-Zooms richtig handlen zu			*/
/* kînnen und immer den Preview-Dither zu sehen. Es wird je ein waagrechtes		*/
/* und ein senkrechtes Redraw-Rechteck erzeugt, die die Breite bzw. Hîhe		*/
/* der Mauszeigerbewegung und die Hîhe bzw. Breite des gesamten Preview-		*/
/* ausschnittes haben.															*/
/* Die beim Ausschnittsweise Dithern entstehenden Fehler sind hier relativ		*/
/* wurscht, da der Ausschnitt sowieso gleich nach dem Loslassen vom Modul		*/
/*	bearbeitet und komplett neu dargestellt wird.								*/
/* Das Dithermodul schreibt dann die Darstelung des angeforderten Ausschnitts	*/
/* an die in der Åbergebenen DITHER_DATA stehenden Koordinaten der Bildschirm-	*/
/* Darstellung. Also wird immer nur das in die Darstellung reingedithert, was	*/
/* verschoben wurde.															*/
/* ------------------------------------------------------------------------		*/
void f_move_preview(WINDOW *window, SMURF_PIC *orig_pic, int redraw_object)
{
	int dummy, mbutt;
	int mx,my, omx, omy;
	int max_xoff, max_yoff;		

	SMURF_PIC move_pic, *old_pic;
	GRECT redraw;
	DISPLAY_MODES old;


	graf_mkstate(&omx, &omy, &mbutt, &dummy);		/* immer noch gedrÅckt? */

	graf_mouse(FLAT_HAND, dummy_ptr);
	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);

	if(window->wnum == WIND_MODFORM)
	{
		window->xoffset = oldxoff;
		window->yoffset = oldyoff;
	}

	/*
	 * Jetzt das aktive Bild nach move_pic kopieren und den Screenzeiger freigeben.
	 */
	memcpy(&move_pic, orig_pic, sizeof(SMURF_PIC));
	move_pic.screen_pic = NULL;
	move_pic.zoom = prev_zoom-1;
	move_pic.local_nct = NULL;
	old_pic = window->picture;
	window->picture = &move_pic;

	max_xoff = (window->picture->pic_width/prev_zoom)-window->clipwid;
	max_yoff = (window->picture->pic_height/prev_zoom)-window->cliphgt;

	if(max_xoff < 0)
		max_yoff = 0;
	if(max_yoff < 0)
		max_yoff = 0;

	graf_mkstate(&mx, &my, &mbutt, &dummy);
	omx = mx;
	omy = my;

	/*
	 * Display-Options faken
	 */
	make_singular_display(&old, Sys_info.PreviewMoveDither, CR_SYSPAL);

	do
	{
		graf_mkstate(&mx, &my, &mbutt, &dummy);		/* immer noch gedrÅckt? */
		
		if(mx != omx || my != omy)
		{
			window->xoffset += (omx-mx)/**prev_zoom*/*2;
			window->yoffset += (omy-my)/**prev_zoom*/*2;
			
			if(window->xoffset < 0)
				window->xoffset = 0;
			if(window->xoffset>max_xoff)
				window->xoffset = max_xoff;
			
			if(window->yoffset < 0)
				window->yoffset = 0;
			if(window->yoffset > max_yoff)
				window->yoffset = max_yoff;

			redraw.g_x = window->wx + window->pic_xpos;
			redraw.g_y = window->wy + window->pic_ypos;
			redraw.g_w = window->clipwid;
			redraw.g_h = window->cliphgt;
			Window.redraw(window, &redraw, redraw_object, DRAWNOTREE);

			omx = mx;
			omy = my;
		}	
	} while(mbutt);

	restore_display(&old);

	wind_update(END_UPDATE);
	wind_update(END_MCTRL);
	graf_mouse(ARROW, dummy_ptr);

	window->picture = old_pic;

	return;
} /* f_move_preview */


/* ------------------------------------------------------------------------	*/
/*								Preview erzeugen							*/
/* ------------------------------------------------------------------------	*/
void f_make_preview(int redraw_flag)
{
	SMURF_PIC *source_pic;
	SMURF_PIC *add_pix[7];
	GARGAMEL smurf_st;
	char alertstr[70];
	char helpstr[3];
	long PicLen;
	long Awidth;
	int t, w,h, picnum, piccol;
	MOD_ABILITY *mod_abs, new_mod;
	MOD_INFO *mod_inf;
	char *textbeg;
	DISPLAY_MODES thisDisplay;


	textbeg = module.bp[edit_mod_num]->p_tbase;
	mod_abs = *((MOD_ABILITY* *)(textbeg + MOD_ABS_OFFSET));
	mod_inf = (MOD_INFO *)*((MOD_INFO **)(textbeg + MOD_INFO_OFFSET));


	/*
	 * und jetzt prÅfen, ob alle vom Modul benîtigten Bilder 
	 * vorhanden sind.
	 */
	for(t = 0; t < mod_inf->how_many_pix; t++)
	{
		picnum = module_pics[edit_mod_num][t];
	
		if(mod_inf->how_many_pix > 1 && picture_windows[picnum].whandlem == -1)
		{
			strcpy(alertstr, "Modul braucht ");
			strncat(alertstr, itoa(mod_inf->how_many_pix, helpstr, 10), 4);
			strcat(alertstr, " Bilder!");
			Dialog.winAlert.openAlert(alertstr, NULL, NULL, NULL, 1);
			return;
		}
	}


	/* ------------- Preview-Pic-Struktur vorbereiten -----------*/
	source_pic = smurf_picture[active_pic];
	if(source_pic->block != NULL)
		source_pic = source_pic->block;

	/* ---alte Bildschirmdarstellung freigeben--- */
	if(module_preview != NULL)
	{
		SMfree(module_preview->screen_pic->fd_addr);
		free(module_preview->screen_pic);
		free(module_preview->palette);
		SMfree(module_preview);
	}

	module_preview = SMalloc(sizeof(SMURF_PIC));
	memcpy(module_preview, source_pic, sizeof(SMURF_PIC));
	module_preview->palette = malloc(1025);
	memcpy(module_preview->palette, source_pic->palette, 1025);
	module_preview->pic_width = wind_s[WIND_MODFORM].clipwid;
	module_preview->pic_height = wind_s[WIND_MODFORM].cliphgt;
	Awidth = ((((long)module_preview->pic_width+7)/8)<<3);
	PicLen = (Awidth*(long)module_preview->pic_height*(long)module_preview->depth)/8L;
	module_preview->pic_data = SMalloc(PicLen);
	module_preview->local_nct = NULL;

	Dialog.busy.disable();
	
	/*----------- Bildausschnitt kopieren -------------------------------*/
	copy_preview(source_pic, module_preview, &wind_s[WIND_MODFORM]);
	f_convert(module_preview, mod_abs, RGB, SAME, 0);

	/*
	 * jetzt die Bilder ans Modul Åbergeben
	 */
	if(mod_inf->how_many_pix > 1)
	{
		for(t = 0; t < mod_inf->how_many_pix; t++)
		{
			module.smStruct[edit_mod_num]->event_par[0] = t;
			module.comm.startEdit(edit_modules[edit_mod_num], module.bp[edit_mod_num], MPICS, module.smStruct[edit_mod_num]->module_number, module.smStruct[edit_mod_num]);

			if(module.smStruct[edit_mod_num]->module_mode == M_PICTURE)
			{
				picnum = module_pics[edit_mod_num][t];
				if(picture_windows[picnum].whandlem == -1)
				{
					Dialog.winAlert.openAlert("Fehler: zu verwendendes Bild existiert nicht! Weisen Sie die Bilder durch Drag&Drop aus dem Bildmanager neu zu.", NULL, NULL, NULL, 1);
					continue;
				}
					
				source_pic = picture_windows[picnum].picture;
				add_pix[t] = SMalloc(sizeof(SMURF_PIC));
				memcpy(add_pix[t], source_pic, sizeof(SMURF_PIC));
				add_pix[t]->palette = malloc(1025);
				memcpy(add_pix[t]->palette, source_pic->palette, 1025);
				add_pix[t]->pic_width = wind_s[WIND_MODFORM].clipwid;
				add_pix[t]->pic_height = wind_s[WIND_MODFORM].cliphgt;
				Awidth = ((((long)module_preview->pic_width + 7) / 8) << 3);
				PicLen = (Awidth * (long)add_pix[t]->pic_height * (long)add_pix[t]->depth) / 8L;
				add_pix[t]->pic_data = SMalloc(PicLen);
	
				copy_preview(source_pic, add_pix[t], &wind_s[WIND_MODFORM]);
	
				memset(&new_mod, 0x0, sizeof(MOD_ABILITY));
				new_mod.depth1 = module.smStruct[edit_mod_num]->event_par[0];
				new_mod.form1 = module.smStruct[edit_mod_num]->event_par[1];
				piccol = module.smStruct[edit_mod_num]->event_par[2];
				f_convert(add_pix[t], &new_mod, piccol, SAME, 0);
	
				smurf_st.event_par[0] = t;
				smurf_st.smurf_pic = add_pix[t];
				module.comm.startEdit(edit_modules[edit_mod_num], module.bp[edit_mod_num], MPICTURE, module.smStruct[edit_mod_num]->module_number, &smurf_st);
				if(smurf_st.module_mode != M_WAITING)
					break;
			}
		}
	}


	f_insert_prefs(&smurf_st, module_preview);

	Dialog.busy.reset(0, "Preview...");
	graf_mouse(BUSYBEE, dummy_ptr);

	module.comm.startEdit(edit_modules[edit_mod_num], module.bp[edit_mod_num], MEXEC, module.smStruct[edit_mod_num]->module_number, &smurf_st);

	/*-------- verÑndertes Bild kopieren -----*/
	if(mod_inf->how_many_pix > 1)
	{
		picnum = smurf_st.event_par[0];
		w = add_pix[picnum]->pic_width;
		h = add_pix[picnum]->pic_height;
		PicLen = (w * (long)h * (long)add_pix[picnum]->depth) / 8L;
		
		SMfree(module_preview->pic_data);
		module_preview->pic_data = SMalloc(PicLen);
		memcpy(module_preview->pic_data, add_pix[picnum]->pic_data, PicLen);

		module_preview->pic_width = add_pix[picnum]->pic_width;
		module_preview->pic_height = add_pix[picnum]->pic_height;
		module_preview->depth = add_pix[picnum]->depth;
		module_preview->format_type = add_pix[picnum]->format_type;
		module_preview->col_format = add_pix[picnum]->col_format;
		memcpy(module_preview->palette, add_pix[picnum]->palette, 1025);
	}

	module_preview->zoom = 0;

	/* -------- Display-Options temporÑr umbauen --------- */
	/* (Preview wird nur nach SysPal gedithert) */
	thisDisplay.dither_24 = Sys_info.PreviewDither;
	thisDisplay.dither_8 = Sys_info.PreviewDither;
	thisDisplay.dither_4 = Sys_info.PreviewDither;
	thisDisplay.syspal_24 = CR_SYSPAL;
	thisDisplay.syspal_8 = CR_SYSPAL;
	thisDisplay.syspal_4 = CR_SYSPAL;

	f_dither(module_preview, &Sys_info, 0, NULL, &thisDisplay);

	Dialog.busy.enable();

	oldxoff = wind_s[WIND_MODFORM].xoffset;
	oldyoff = wind_s[WIND_MODFORM].yoffset;
	oldpic = wind_s[WIND_MODFORM].picture;

	wind_s[WIND_MODFORM].xoffset = 0;
	wind_s[WIND_MODFORM].yoffset = 0;
	wind_s[WIND_MODFORM].picture = module_preview;

	Window.redraw(&wind_s[WIND_MODFORM], NULL, PREV_OUTER, redraw_flag);

	graf_mouse(ARROW, dummy_ptr);

	if(mod_inf->how_many_pix == 1)
	{
		SMfree(module_preview->pic_data);
		free(module_preview->palette);
	}
	else
		for(t = 0; t < mod_inf->how_many_pix; t++)
		{
			if(add_pix[t])
			{
				SMfree(add_pix[t]->pic_data);
				free(add_pix[t]->palette);
				SMfree(add_pix[t]);
			}
		}

	Dialog.busy.ok();
	Dialog.busy.dispRAM();

	return;
} /* f_make_preview */


/* ------------------------------------------------------------------------	*/
/*				PD-Parameter in GARGAMEL-Struktur einfÅgen					*/
/* ------------------------------------------------------------------------	*/
void f_insert_prefs(GARGAMEL *smurf_st, SMURF_PIC *picture)
{
	OBJECT *modtree;


	modtree=wind_s[WIND_MODFORM].resource_form;

	smurf_st->smurf_pic=picture;
	smurf_st->picwind_x=picture_windows[active_pic].wx;
	smurf_st->picwind_y=picture_windows[active_pic].wy;
	smurf_st->picwind_w=picture_windows[active_pic].ww;
	smurf_st->picwind_h=picture_windows[active_pic].wh;
	smurf_st->slide1=(long)sy1;
	smurf_st->slide2=(long)sy2;
	smurf_st->slide3=(long)sy3;
	smurf_st->slide4=(long)sy4;
	smurf_st->edit1=atol(modtree[ED1].TextCast);
	smurf_st->edit2=atol(modtree[ED2].TextCast);
	smurf_st->edit3=atol(modtree[ED3].TextCast);
	smurf_st->edit4=atol(modtree[ED4].TextCast);
	smurf_st->check1=modtree[CHECK1].ob_state&SELECTED;
	smurf_st->check2=modtree[CHECK2].ob_state&SELECTED;
	smurf_st->check3=modtree[CHECK3].ob_state&SELECTED;
	smurf_st->check4=modtree[CHECK4].ob_state&SELECTED;
}



void copy_preview(SMURF_PIC *source_pic, SMURF_PIC *preview, WINDOW *prev_window)
{
	long CutOff, BPP;
	long SrcPlanelen, DestPlanelen;
	long SrcLineLen, DestLineLen;
	long Soff=0, Doff=0;
	int y, plane, x;
	char *Destdata, *Srcdata, *sptr, *dptr, *linebuf, *src, *dest;
	int *sptr16, *dptr16;
	int prev_endhgt, prev_endwid, desty_count, destx_count;

	extern	void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);
	extern	int setpix_std_line16(char *buf, char *dest, int depth, long planelen, int howmany);


	Srcdata=source_pic->pic_data;
	Destdata=preview->pic_data;
	
	if(source_pic->format_type == FORM_PIXELPAK)
	{
		SrcLineLen = ((long)source_pic->pic_width*(long)source_pic->depth)/8L;
		DestLineLen = ((long)preview->pic_width*(long)preview->depth)/8L;
		BPP = (long)preview->depth / 8L;
	}
	else
	{
		SrcLineLen = ((long)source_pic->pic_width+7)/8L;
		DestLineLen = ((long)preview->pic_width+7)/8L;
		SrcPlanelen = SrcLineLen * (long)source_pic->pic_height;
		DestPlanelen = DestLineLen * (long)preview->pic_height;
		BPP = 1;
	}

	/*------------------ Bildausschnitt ins Preview kopieren - Pixelpacked */
	if(source_pic->format_type == FORM_PIXELPAK)
	{
		CutOff=(long)(prev_window->yoffset)*SrcLineLen;
		CutOff += (long)(prev_window->xoffset)*BPP;

		prev_endhgt = preview->pic_height * prev_zoom;
		if(prev_endhgt>source_pic->pic_height) 
			prev_endhgt = source_pic->pic_height;

		prev_endwid = preview->pic_width * prev_zoom;
		if(prev_endwid>source_pic->pic_width) 
			prev_endwid = source_pic->pic_width;
		
		desty_count=0;
		for(y=0; y<prev_endhgt; y+=prev_zoom)
		{
			Soff=y*SrcLineLen;
			Doff=desty_count*DestLineLen;

			destx_count=0;
			if(source_pic->depth == 24)
			{
				for(x=0; x<prev_endwid; x+=prev_zoom)
				{
					dptr = Destdata+Doff+destx_count;
					sptr = Srcdata+Soff+CutOff+x*3;
					*(dptr) = *(sptr);
					*(dptr+1) = *(sptr+1);
					*(dptr+2) = *(sptr+2);
					destx_count+=3;
				}
			}
			else if(source_pic->depth == 16)
			{
				for(x=0; x<prev_endwid; x+=prev_zoom)
				{
					dptr16 = (int*)(Destdata+Doff+destx_count);
					sptr16 = (int*)(Srcdata+Soff+CutOff+x+x);
					*(dptr16) = *(sptr16);
					destx_count+=2;
				}
			}
			else if(source_pic->depth == 8)
			{
				for(x=0; x<prev_endwid; x+=prev_zoom)
				{
					dptr = Destdata+Doff+destx_count;
					sptr = Srcdata+Soff+CutOff+x;
					*(dptr) = *(sptr);
					destx_count++;
				}
			}

			desty_count++;
		}	
	}
	/*---------------------------------- Standardformat */
	else
	{
		CutOff = (long)(prev_window->yoffset)*SrcLineLen;
		CutOff += (long)(prev_window->xoffset)/8;

		/*
		 * ohne Zoom
		 */
		if(prev_zoom==1)
		{
			for(plane=0; plane<preview->depth; plane++)
			{
				for(y=0; y<preview->pic_height; y++)
				{
					Soff=y*SrcLineLen;
					Doff=y*DestLineLen;
					memcpy( Destdata+Doff, Srcdata+Soff+CutOff, DestLineLen);
				}
				Destdata+=DestPlanelen;
				Srcdata+=SrcPlanelen;
			}
		}
		/*
		 * mit Zoom
		 */
		else
		{
			linebuf = SMalloc(preview->pic_width * prev_zoom + 32);
			
			for(y = 0; y < preview->pic_height; y++)
			{
				memset(linebuf, 0, preview->pic_width*prev_zoom+32);
				getpix_std_line(Srcdata+Soff+CutOff, linebuf, source_pic->depth, SrcPlanelen, preview->pic_width*prev_zoom);

				src = linebuf;
				dest = linebuf;
				x = 0;
				do
				{
					*dest++ = *src++;
					src += prev_zoom-1;
				} while(++x<preview->pic_width);
				setpix_std_line16(linebuf, Destdata+Doff, source_pic->depth, DestPlanelen, preview->pic_width);

				Soff = y*SrcLineLen*prev_zoom;
				Doff = y*DestLineLen;
			}

			SMfree(linebuf);
		}
	} /* Standardformat */
}


void applyConfig(long *loadcnf)
{
	OBJECT *modtree;
	
	modtree = wind_s[WIND_MODFORM].resource_form;
	
	sy1 = (int)loadcnf[0];
	sy2 = (int)loadcnf[1];
	sy3 = (int)loadcnf[2];
	sy4 = (int)loadcnf[3];
	modtree[CHECK1].ob_state = (int)loadcnf[4];
	modtree[CHECK2].ob_state = (int)loadcnf[5];
	modtree[CHECK3].ob_state = (int)loadcnf[6];
	modtree[CHECK4].ob_state = (int)loadcnf[7];
	ltoa(loadcnf[8], modtree[ED1].TextCast, 10);
	ltoa(loadcnf[9], modtree[ED2].TextCast, 10);
	ltoa(loadcnf[10], modtree[ED3].TextCast, 10);
	ltoa(loadcnf[11], modtree[ED4].TextCast, 10);
}