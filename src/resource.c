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

/*	Dieses Modul enthÑlt einige Fuktionen zum Initialisieren
	der Resource und der GEM-OberflÑche von Smurf,
	z.B. Popup-Strukturen, WINDOW-Strukturen, etc. */

#include <tos.h>
#include <multiaes.h>
#include <vdi.h>
#include <string.h>
#include <stdio.h>
#include <ext.h>
#include <stdlib.h>

#include "..\sym_gem.h"
#include "..\mod_devl\import.h"
#include "smurfine.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurf_f.h"

#include "portab.h"
#include "xrsrc.h"
#include "popdefin.h"
#include "globdefs.h"
#include "ext_rsc.h"

#include "smurfobs.h"
#include "ext_obs.h"


/*------ Icondaten fÅr die Mauszeiger ------*/
#include "lrarrow.icn"
#include "lrarrow.icm"
#include "udarrow.icn"
#include "udarrow.icm"
#include "lrud_arr.icn"
#include "lrud_arr.icm"

extern void 	shutdown_smurf(char while_startup);

int add_flags[40];			/* zusÑtzliche Fensterelemente */

extern int resource_global[100];

extern SYSTEM_INFO Sys_info;
extern int gl_wchar, gl_hchar, gl_wbox, gl_hbox;

extern POP_UP popups[25];
extern SLIDER sliders[15];
extern WINDOW wind_s[25];

extern	MFORM	lr_arrow, ud_arrow, lrud_arrow;


/* f_init_popup */
/* Diese Funktion generiert die Popup-strukturen fÅr jedes
	einzelne POPUP, das in Smurf irgendwo vorkommt */
void f_init_popups(void)
{

/*	Auûenfarbe im Options-MenÅ */
	popups[POPUP_OUTCOL].popup_tree = OUT_COL_POPUP;
	popups[POPUP_OUTCOL].item = 2;
	popups[POPUP_OUTCOL].display_tree = Dialog.smurfOpt.tree;
	popups[POPUP_OUTCOL].display_obj = OUTCOL;
	popups[POPUP_OUTCOL].Cyclebutton = OCOL_CB;
	
/* Die Ditherpopups fÅrs Displayoptions-Formular */
	popups[POPUP_DITHER].popup_tree = DITHERPOPUP;
	popups[POPUP_DITHER].item = 1;
	popups[POPUP_DITHER].display_tree = Dialog.dispOpt.tree;
	popups[POPUP_DITHER].display_obj = DITHER_POPBUT;
	popups[POPUP_DITHER].Cyclebutton = DITHER_CB;

	popups[POPUP_COLRED].popup_tree = COL_RED_POPUP;
	popups[POPUP_COLRED].item = 1;
	popups[POPUP_COLRED].display_tree = Dialog.dispOpt.tree;
	popups[POPUP_COLRED].display_obj = COLRED_POPBUT;
	popups[POPUP_COLRED].Cyclebutton = COLRED_CB;

	popups[POPUP_PIC].popup_tree = PIC_POPUP;
	popups[POPUP_PIC].item = 1;
	popups[POPUP_PIC].display_tree = pic_form;
	popups[POPUP_PIC].display_obj = -1;
	popups[POPUP_PIC].Cyclebutton = -11;

	popups[POPUP_ZOOM].popup_tree = ZOOM_POP;
	popups[POPUP_ZOOM].item = 1;
	popups[POPUP_ZOOM].display_tree = pic_form;
	popups[POPUP_ZOOM].display_obj = ZOOM_FACTOR;
	popups[POPUP_ZOOM].Cyclebutton = -11;

	popups[POPUP_NPDEPTH].popup_tree = DEPTH_POPUP;
	popups[POPUP_NPDEPTH].item = 1;
	popups[POPUP_NPDEPTH].display_tree = newpic_window;
	popups[POPUP_NPDEPTH].display_obj = NP_DEPTHPOP;
	popups[POPUP_NPDEPTH].Cyclebutton = NPDEPTH_CB;
	
	popups[POPUP_KODAK].popup_tree = KODAK_POPUP;
	popups[POPUP_KODAK].item = 2;
	popups[POPUP_KODAK].display_tree = Dialog.smurfOpt.tree;
	popups[POPUP_KODAK].display_obj = PCD_DEF_PB;
	popups[POPUP_KODAK].Cyclebutton = PCD_DEF_CB;

	popups[POPUP_EXP_DEPTH].popup_tree = EXPORT_DEPTH_POP;
	popups[POPUP_EXP_DEPTH].item = 2;
	popups[POPUP_EXP_DEPTH].display_tree = export_form;
	popups[POPUP_EXP_DEPTH].display_obj = EXP_DEPTH;
	popups[POPUP_EXP_DEPTH].Cyclebutton = EXP_DEPTHC;

	popups[POPUP_PICORDER].popup_tree = PICORDER_POPUP;
	popups[POPUP_PICORDER].item = 2;
	popups[POPUP_PICORDER].display_tree = export_form;
	popups[POPUP_PICORDER].display_obj = -1;
	popups[POPUP_PICORDER].Cyclebutton = -11;

	popups[POPUP_BLOCK].popup_tree = BLOCK_POPUP;
	popups[POPUP_BLOCK].item = 1;
	popups[POPUP_BLOCK].display_tree = pic_form;
	popups[POPUP_BLOCK].display_obj = -1;
	popups[POPUP_BLOCK].Cyclebutton = -11;

	popups[POPUP_CONVDEPTH].popup_tree = DEPTH_POPUP;
	popups[POPUP_CONVDEPTH].item = 1;
	popups[POPUP_CONVDEPTH].display_tree = transform_window;
	popups[POPUP_CONVDEPTH].display_obj = CONV_DEPTHPB;
	popups[POPUP_CONVDEPTH].Cyclebutton = CONV_DEPTHCB;

	popups[POPUP_CONVDITHER].popup_tree = DITHERPOPUP;
	popups[POPUP_CONVDITHER].item = 1;
	popups[POPUP_CONVDITHER].display_tree = transform_window;
	popups[POPUP_CONVDITHER].display_obj = CONV_DITHPB;
	popups[POPUP_CONVDITHER].Cyclebutton = CONV_DITHCB;

	popups[POPUP_CONVPAL].popup_tree = COL_RED_POPUP;
	popups[POPUP_CONVPAL].item = 1;
	popups[POPUP_CONVPAL].display_tree = transform_window;
	popups[POPUP_CONVPAL].display_obj = CONV_PALPB;
	popups[POPUP_CONVPAL].Cyclebutton = CONV_PALCB;

	popups[POPUP_PREVDIT].popup_tree = DITHERPOPUP;
	popups[POPUP_PREVDIT].item = 1;
	popups[POPUP_PREVDIT].display_tree = Dialog.smurfOpt.tree;
	popups[POPUP_PREVDIT].display_obj = PREVDIT_PB;
	popups[POPUP_PREVDIT].Cyclebutton = PREVDIT_CB;

	popups[POPUP_UNIT].popup_tree = UNIT_POPUP;
	popups[POPUP_UNIT].item = 1;
	popups[POPUP_UNIT].display_tree = newpic_window;
	popups[POPUP_UNIT].display_obj = NP_UNITPB;
	popups[POPUP_UNIT].Cyclebutton = NP_UNITCB;

	popups[POPUP_MOVEDIT].popup_tree = DITHERPOPUP;
	popups[POPUP_MOVEDIT].item = 1;
	popups[POPUP_MOVEDIT].display_tree = Dialog.smurfOpt.tree;
	popups[POPUP_MOVEDIT].display_obj = MOVE_PREVDIT_PB;
	popups[POPUP_MOVEDIT].Cyclebutton = MOVE_PREVDIT_CB;

	popups[POPUP_PICMANDIT].popup_tree = DITHERPOPUP;
	popups[POPUP_PICMANDIT].item = 1;
	popups[POPUP_PICMANDIT].display_tree = Dialog.smurfOpt.tree;
	popups[POPUP_PICMANDIT].display_obj = PICMANDIT_PB;
	popups[POPUP_PICMANDIT].Cyclebutton = PICMANDIT_CB;

	popups[POPUP_AUTODITHER].popup_tree = DITHERPOPUP;
	popups[POPUP_AUTODITHER].item = 1;
	popups[POPUP_AUTODITHER].display_tree = Dialog.smurfOpt.tree;
	popups[POPUP_AUTODITHER].display_obj = AUTODIT_PB;
	popups[POPUP_AUTODITHER].Cyclebutton = AUTODIT_CB;

	popups[POPUP_AUTOPAL].popup_tree = COL_RED_POPUP;
	popups[POPUP_AUTOPAL].item = 1;
	popups[POPUP_AUTOPAL].display_tree = Dialog.smurfOpt.tree;
	popups[POPUP_AUTOPAL].display_obj = AUTOPAL_PB;
	popups[POPUP_AUTOPAL].Cyclebutton = AUTOPAL_CB;
}




void f_init_sliders(void)
{
	sliders[PDSLIDER1].regler = S1_R;
	sliders[PDSLIDER1].schiene = S1_F;
	sliders[PDSLIDER1].rtree = module_form;
	sliders[PDSLIDER1].txt_obj = S1_A;
	sliders[PDSLIDER1].min_val = 0;
	sliders[PDSLIDER1].max_val = 0;
	sliders[PDSLIDER1].window = &wind_s[WIND_MODFORM];

	sliders[PDSLIDER2].regler = S2_R;
	sliders[PDSLIDER2].schiene = S2_F;
	sliders[PDSLIDER2].rtree = module_form;
	sliders[PDSLIDER2].txt_obj = S2_A;
	sliders[PDSLIDER2].min_val = 0;
	sliders[PDSLIDER2].max_val = 0;
	sliders[PDSLIDER2].window = &wind_s[WIND_MODFORM];

	sliders[PDSLIDER3].regler = S3_R;
	sliders[PDSLIDER3].schiene = S3_F;
	sliders[PDSLIDER3].rtree = module_form;
	sliders[PDSLIDER3].txt_obj = S3_A;
	sliders[PDSLIDER3].min_val = 0;
	sliders[PDSLIDER3].max_val = 0;
	sliders[PDSLIDER3].window = &wind_s[WIND_MODFORM];

	sliders[PDSLIDER4].regler = S4_R;
	sliders[PDSLIDER4].schiene = S4_F;
	sliders[PDSLIDER4].rtree = module_form;
	sliders[PDSLIDER4].txt_obj = S4_A;
	sliders[PDSLIDER4].min_val = 0;
	sliders[PDSLIDER4].max_val = 0;
	sliders[PDSLIDER4].window = &wind_s[WIND_MODFORM];

	sliders[BLOCK_SLIDER].regler = BCONF_R;
	sliders[BLOCK_SLIDER].schiene = BCONF_F;
	sliders[BLOCK_SLIDER].rtree = blockmode_window;
	sliders[BLOCK_SLIDER].txt_obj = BCONF_A;
	sliders[BLOCK_SLIDER].min_val = 1;
	sliders[BLOCK_SLIDER].max_val = 100;
	sliders[BLOCK_SLIDER].window = &wind_s[WIND_BLOCKMODE];
}



/* ------ Resource laden und initialisieren, AES Initial.	---- */
void init_smurfrsc(char *rscpath)
{
	int back, t, dummy;
	GRECT desk;

	extern OBJECT *confsave_dialog, *modconf_popup;


	/* ----- Startup-Dialog initialisieren --- */
	desk.g_x = 0;
	desk.g_y = 0;
	desk.g_w = Sys_info.screen_width;
	desk.g_h = Sys_info.screen_height;
	init_xrsrc(Sys_info.vdi_handle, &desk, gl_wchar, gl_hchar);
	back = xrsrc_load(rscpath, resource_global);
	if(back == FALSE)
	{
		form_alert(1, "[3][Resource-File SMURF.RSC nicht|gefunden! PrÅfen Sie, ob sich|die Datei im Smurf-Pfad|befindet.][ Ups ]");
		exit(-1);
	}

	xrsrc_gaddr(0, MENU1, &menu_tree, resource_global);

	xrsrc_gaddr(0, DEPTH_POPUP, &form_pop, resource_global);					/* Farbtiefenpopup */
	xrsrc_gaddr(0, OUT_COL_POPUP, &edit_pop, resource_global);					/* Auûenfarbe */
	xrsrc_gaddr(0, DITHERPOPUP, &col_pop, resource_global);						/* Ditheralgo */
	xrsrc_gaddr(0, DISPLAY_OPTIONS, &Dialog.dispOpt.tree, resource_global);		/* Display-Opt-Form */
	xrsrc_gaddr(0, PIC_WIND_FORM, &pic_form, resource_global);					/* Bildfenster-Form */
	xrsrc_gaddr(0, IMAGE_INFO, &pic_info_form, resource_global);				/* BildinfoForm */
	xrsrc_gaddr(0, SMURF_OPTIONS, &Dialog.smurfOpt.tree, resource_global);		/* Options-Form */
	xrsrc_gaddr(0, WINDOW_ALERT, &alert_form, resource_global);					/* Windowalert */
	xrsrc_gaddr(0, MOD_FORM, &module_form, resource_global);					/* Preferences-Form */
	xrsrc_gaddr(0, BUSY_WINDOW, &Dialog.busy.busyTree, resource_global);		/* Busybox */
	xrsrc_gaddr(0, MODULES, &Dialog.emodList.tree, resource_global);			/* Edit-Modulliste */
	xrsrc_gaddr(0, SMURF_INFO, &info_window, resource_global);					/* Smurf Info */
	xrsrc_gaddr(0, NEWPIC_FORM, &newpic_window, resource_global);				/* Neues Bild */
	xrsrc_gaddr(0, BLOCK_TYPE, &blocktype_window, resource_global);				/* Ordner laden */
	xrsrc_gaddr(0, EXPORT_MODS, &Dialog.expmodList.tree, resource_global);		/* Export-Modulliste */
	xrsrc_gaddr(0, PIC_MANAGER, &Dialog.picMan.tree, resource_global);			/* Bildmanager */
	xrsrc_gaddr(0, EXPORT_FORM, &export_form, resource_global);					/* Export-Formular */
	xrsrc_gaddr(0, PICORDER_POPUP, &picorder_popup, resource_global);			/* Bildreihenfolge-Popup*/
	xrsrc_gaddr(0, COL_RED_POPUP, &colred_popup, resource_global);				/* Farbreduktion */
	xrsrc_gaddr(0, BLOCK_POPUP, &blockpopup, resource_global);					/* Blockpopup */
	xrsrc_gaddr(0, BLOCK_CONF, &blockmode_window, resource_global);				/* Blockpopup */
	xrsrc_gaddr(0, EXPORT_DEPTH_POP, &exp_dp_popup, resource_global);			/* Export-Tiefe */
	xrsrc_gaddr(0, MODULE_INFO, &Dialog.emodList.infoTree, resource_global);	/* Edit-Modul-Info */
	xrsrc_gaddr(0, EXP_MODULE_INFO, &Dialog.expmodList.infoTree, resource_global);				/* Export-Modul-Info */
	xrsrc_gaddr(0, TRANSFORM_PIC, &transform_window, resource_global);			/* Bild wandeln */

	xrsrc_gaddr(0, ALERT_STRINGS, &Dialog.winAlert.alerts, resource_global);	/* Alertstrings */
	xrsrc_gaddr(0, WINDOW_TITLES, &Window.titles, resource_global);				/* Fenstertitel */
	xrsrc_gaddr(0, SAVE_MODCONFIG, &confsave_dialog, resource_global);			/* Modulkonfiguration speichern */
	xrsrc_gaddr(0, MODCONF_POPUP, &modconf_popup, resource_global);				/* Modulkonfiguration aufpopen */

	xrsrc_gaddr(0, BUTTONS, &u_tree, resource_global);							/* Userdefs */

	f_treewalk(form_pop, 0);
	f_treewalk(edit_pop, 0);
	f_treewalk(col_pop, 0);
	f_treewalk(Dialog.dispOpt.tree, 0);
	f_treewalk(u_tree, 0);
	f_treewalk(Dialog.smurfOpt.tree, 0);
	f_treewalk(module_form, 0);
	f_treewalk(Dialog.emodList.infoTree, 0);
	f_treewalk(Dialog.emodList.tree, 0);
	f_treewalk(blocktype_window, 0);
	f_treewalk(newpic_window, 0);
	f_treewalk(Dialog.picMan.tree, 0);
	f_treewalk(export_form, 0);
	f_treewalk(Dialog.expmodList.tree, 0);
	f_treewalk(Dialog.expmodList.infoTree, 0);
	f_treewalk(pic_info_form, 0);
	f_treewalk(transform_window, 0);
	f_treewalk(blockmode_window, 0);
	f_treewalk(confsave_dialog, 0);
	f_treewalk(modconf_popup, 0);

	/* Formularkoordinaten in Felder eintragen */
	form_center(Dialog.dispOpt.tree, &(wind_s[WIND_DOPT].wx), &(wind_s[WIND_DOPT].wy), &(wind_s[WIND_DOPT].ww), &(wind_s[WIND_DOPT].wh));

	form_center(pic_form, &(wind_s[WIND_PIC].wx), &(wind_s[WIND_PIC].wy), &(wind_s[WIND_PIC].ww), &(wind_s[WIND_PIC].wh));
	form_center(pic_info_form, &(wind_s[WIND_PICINFO].wx), &(wind_s[WIND_PICINFO].wy), &(wind_s[WIND_PICINFO].ww), &(wind_s[WIND_PICINFO].wh));
	form_center(Dialog.smurfOpt.tree, &(wind_s[WIND_OPTIONS].wx), &(wind_s[WIND_OPTIONS].wy), &(wind_s[WIND_OPTIONS].ww), &(wind_s[WIND_OPTIONS].wh));
	form_center(alert_form, &(wind_s[WIND_ALERT].wx), &(wind_s[WIND_ALERT].wy), &(wind_s[WIND_ALERT].ww), &(wind_s[WIND_ALERT].wh));
	form_center(module_form, &(wind_s[WIND_MODFORM].wx), &(wind_s[WIND_MODFORM].wy), &(wind_s[WIND_MODFORM].ww), &(wind_s[WIND_MODFORM].wh));
	form_center(Dialog.emodList.tree, &(wind_s[WIND_MODULES].wx), &(wind_s[WIND_MODULES].wy), &(wind_s[WIND_MODULES].ww), &(wind_s[WIND_MODULES].wh));
	form_center(info_window, &(wind_s[WIND_INFO].wx), &(wind_s[WIND_INFO].wy), &(wind_s[WIND_INFO].ww), &(wind_s[WIND_INFO].wh));
	form_center(newpic_window, &(wind_s[WIND_NEWPIC].wx), &(wind_s[WIND_NEWPIC].wy), &(wind_s[WIND_NEWPIC].ww), &(wind_s[WIND_NEWPIC].wh));
	form_center(blocktype_window, &(wind_s[WIND_BTYPEIN].wx), &(wind_s[WIND_BTYPEIN].wy), &(wind_s[WIND_BTYPEIN].ww), &(wind_s[WIND_BTYPEIN].wh));
	form_center(Dialog.expmodList.tree, &(wind_s[WIND_EXPORT].wx), &(wind_s[WIND_EXPORT].wy), &(wind_s[WIND_EXPORT].ww), &(wind_s[WIND_EXPORT].wh));
	form_center(Dialog.picMan.tree, &(wind_s[WIND_PICMAN].wx), &(wind_s[WIND_PICMAN].wy), &(wind_s[WIND_PICMAN].ww), &(wind_s[WIND_PICMAN].wh));
	form_center(export_form, &(wind_s[FORM_EXPORT].wx), &(wind_s[FORM_EXPORT].wy), &(wind_s[FORM_EXPORT].ww), &(wind_s[FORM_EXPORT].wh));
	form_center(Dialog.emodList.infoTree, &(wind_s[WIND_MODULES].wx), &(wind_s[WIND_MODULES].wy), &(wind_s[WIND_MODULES].ww), &(wind_s[WIND_MODULES].wh));
	form_center(Dialog.expmodList.infoTree, &(wind_s[WIND_EXPORT].wx), &(wind_s[WIND_EXPORT].wy), &(wind_s[WIND_EXPORT].ww), &(wind_s[WIND_EXPORT].wh));
	form_center(transform_window, &(wind_s[WIND_TRANSFORM].wx), &(wind_s[WIND_TRANSFORM].wy), &(wind_s[WIND_TRANSFORM].ww), &(wind_s[WIND_TRANSFORM].wh));
	form_center(blockmode_window, &(wind_s[WIND_BLOCKMODE].wx), &(wind_s[WIND_BLOCKMODE].wy), &(wind_s[WIND_BLOCKMODE].ww), &(wind_s[WIND_BLOCKMODE].wh));
	form_center(confsave_dialog, &dummy,&dummy,&dummy,&dummy);
	form_center(modconf_popup, &dummy,&dummy,&dummy,&dummy);

	wind_s[WIND_MODFORM].wnum = WIND_MODFORM;
	strcpy(wind_s[WIND_MODFORM].wtitle, Window.titles[WT_MODCONF].TextCast);
	wind_s[WIND_MODFORM].resource_form = module_form;
	wind_s[WIND_MODFORM].picture = NULL;

	wind_s[WIND_MODFORM].editob = ED1;
	wind_s[WIND_MODFORM].nextedit = ED1;
	wind_s[WIND_MODFORM].editx = 0;
	wind_s[WIND_MODFORM].pflag = 0;
	
	wind_s[WIND_ALERT].wnum = WIND_ALERT;
	strcpy(wind_s[WIND_ALERT].wtitle, Window.titles[WT_ALERT].TextCast);
	wind_s[WIND_ALERT].resource_form = alert_form;
	wind_s[WIND_ALERT].picture = NULL;

	wind_s[WIND_ALERT].editob = 0;
	wind_s[WIND_ALERT].pflag = 0;
	
	wind_s[WIND_DOPT].wnum = WIND_DOPT;
	strcpy(wind_s[WIND_DOPT].wtitle, Window.titles[WT_DISPOPTS].TextCast);
	wind_s[WIND_DOPT].resource_form = Dialog.dispOpt.tree;
	wind_s[WIND_DOPT].picture = NULL;

	wind_s[WIND_DOPT].editob = PAL_TIMER;
	wind_s[WIND_DOPT].nextedit = PAL_TIMER;
	wind_s[WIND_DOPT].editx = 0;
	wind_s[WIND_DOPT].pflag = 0;
	
	wind_s[WIND_PICINFO].wnum = WIND_PICINFO;
	strcpy(wind_s[WIND_PICINFO].wtitle, Window.titles[WT_PICINFO].TextCast);
	wind_s[WIND_PICINFO].resource_form = pic_info_form;
	wind_s[WIND_PICINFO].picture = NULL;

	wind_s[WIND_PICINFO].editob = 0;
	wind_s[WIND_PICINFO].nextedit = 0;
	wind_s[WIND_PICINFO].editx = 0;
	wind_s[WIND_PICINFO].pflag = 0;
	
	wind_s[WIND_OPTIONS].wnum = WIND_OPTIONS;
	strcpy(wind_s[WIND_OPTIONS].wtitle, Window.titles[WT_OPTIONS].TextCast);
	wind_s[WIND_OPTIONS].resource_form = Dialog.smurfOpt.tree;
	wind_s[WIND_OPTIONS].picture = NULL;

	wind_s[WIND_OPTIONS].editob = 0;
	wind_s[WIND_OPTIONS].pflag = 0;
	
	wind_s[WIND_BUSY].wnum = WIND_BUSY;
	strcpy(wind_s[WIND_BUSY].wtitle, Window.titles[WT_STATUS].TextCast);
	wind_s[WIND_BUSY].resource_form = Dialog.busy.busyTree;
	wind_s[WIND_BUSY].picture = NULL;

	wind_s[WIND_BUSY].editob = 0;
	wind_s[WIND_BUSY].pflag = 0;
	
	wind_s[WIND_MODULES].wnum = WIND_MODULES;
	strcpy(wind_s[WIND_MODULES].wtitle, Window.titles[WT_EDITMODS].TextCast);
	wind_s[WIND_MODULES].resource_form = Dialog.emodList.tree;
	wind_s[WIND_MODULES].picture = NULL;

	wind_s[WIND_MODULES].editob = 0;
	wind_s[WIND_MODULES].pflag = 0;

	wind_s[WIND_EXPORT].wnum = WIND_EXPORT;
	strcpy(wind_s[WIND_EXPORT].wtitle, Window.titles[WT_EXPFORMAT].TextCast);
	wind_s[WIND_EXPORT].resource_form = Dialog.expmodList.tree;
	wind_s[WIND_EXPORT].picture = NULL;

	wind_s[WIND_EXPORT].editob = 0;
	wind_s[WIND_EXPORT].pflag = 0;
	
	wind_s[WIND_INFO].wnum = WIND_INFO;
	strcpy(wind_s[WIND_INFO].wtitle, Window.titles[WT_INFO].TextCast);
	wind_s[WIND_INFO].resource_form = info_window;
	wind_s[WIND_INFO].picture = NULL;

	wind_s[WIND_INFO].editob = 0;
	wind_s[WIND_INFO].pflag = 0;
	
	wind_s[WIND_NEWPIC].wnum = WIND_NEWPIC;
	strcpy(wind_s[WIND_NEWPIC].wtitle, Window.titles[WT_NEWPIC].TextCast);
	wind_s[WIND_NEWPIC].resource_form = newpic_window;
	wind_s[WIND_NEWPIC].picture = NULL;

	wind_s[WIND_NEWPIC].editob = NEWPIC_WID;
	wind_s[WIND_NEWPIC].nextedit = NEWPIC_WID;
	wind_s[WIND_NEWPIC].editx = 0;
	wind_s[WIND_NEWPIC].pflag = 0;
	strcpy(wind_s[WIND_NEWPIC].resource_form[NEWPIC_WID].TextCast, "640");
	strcpy(wind_s[WIND_NEWPIC].resource_form[NEWPIC_HGT].TextCast, "480");
	strcpy(wind_s[WIND_NEWPIC].resource_form[NEWPIC_DPI].TextCast, "300");

	wind_s[WIND_PICMAN].wnum = WIND_PICMAN;
	strcpy(wind_s[WIND_PICMAN].wtitle, Window.titles[WT_PICMAN].TextCast);
	wind_s[WIND_PICMAN].resource_form = Dialog.picMan.tree;
	wind_s[WIND_PICMAN].picture = NULL;

	wind_s[WIND_PICMAN].editob = 0;
	wind_s[WIND_PICMAN].pflag = 0;

	wind_s[FORM_EXPORT].wnum = FORM_EXPORT;
	strcpy(wind_s[FORM_EXPORT].wtitle, Window.titles[WT_EXPFORM].TextCast);
	wind_s[FORM_EXPORT].resource_form = export_form;
	wind_s[FORM_EXPORT].picture = NULL;

	wind_s[WIND_TRANSFORM].wnum = WIND_TRANSFORM;
	strcpy(wind_s[WIND_TRANSFORM].wtitle, Window.titles[WT_TRANSFORM].TextCast);
	wind_s[WIND_TRANSFORM].resource_form = transform_window;
	wind_s[WIND_TRANSFORM].picture = NULL;

	wind_s[WIND_TRANSFORM].editob = 0;
	wind_s[WIND_TRANSFORM].nextedit = 0;
	wind_s[WIND_TRANSFORM].editx = 0;
	wind_s[WIND_TRANSFORM].pflag = 0;

	wind_s[WIND_BTYPEIN].wnum = WIND_BTYPEIN;
	strcpy(wind_s[WIND_BTYPEIN].wtitle, Window.titles[WT_BTYPEIN].TextCast);
	wind_s[WIND_BTYPEIN].resource_form = blocktype_window;
	wind_s[WIND_BTYPEIN].picture = NULL;

	wind_s[WIND_BTYPEIN].editob = BLOCK_XLO;
	wind_s[WIND_BTYPEIN].nextedit = BLOCK_XLO;
	wind_s[WIND_BTYPEIN].editx = 0;
	wind_s[WIND_BTYPEIN].pflag = 0;

	wind_s[WIND_BLOCKMODE].wnum = WIND_BLOCKMODE;
	strcpy(wind_s[WIND_BLOCKMODE].wtitle, Window.titles[WT_BLOCKMODE].TextCast);
	wind_s[WIND_BLOCKMODE].resource_form = blockmode_window;
	wind_s[WIND_BLOCKMODE].picture = NULL;

	wind_s[WIND_BLOCKMODE].editob = 0;
	wind_s[WIND_BLOCKMODE].nextedit = 0;
	wind_s[WIND_BLOCKMODE].editx = 0;
	wind_s[WIND_BLOCKMODE].pflag = 0;


	/*----- Verkettung der Window-Liste vorinitialisieren ----*/
	for(t = 0; t < 25; t++)
	{
		wind_s[t].prev_window = NULL;
		wind_s[t].next_window = NULL;
	}


	/*----------- zusÑtzliche Window-Flags -----*/
	add_flags[WIND_MODULES] = INFO;
	add_flags[WIND_EXPORT] = INFO;

	/*----------- MFORMs fertigmachen --------*/
	lr_arrow.mf_xhot = 7;
	lr_arrow.mf_yhot = 7;
	lr_arrow.mf_nplanes = 1;
	lr_arrow.mf_fg = 0;
	lr_arrow.mf_bg = 1;
	memcpy(lr_arrow.mf_mask, lrarrow_mask, 16 * 2);
	memcpy(lr_arrow.mf_data, lrarrow, 16 * 2);

	ud_arrow.mf_xhot = 7;
	ud_arrow.mf_yhot = 7;
	ud_arrow.mf_nplanes = 1;
	ud_arrow.mf_fg = 0;
	ud_arrow.mf_bg = 1;
	memcpy(ud_arrow.mf_mask, udarrow_mask, 16 * 2);
	memcpy(ud_arrow.mf_data, udarrow, 16 * 2);

	lrud_arrow.mf_xhot = 7;
	lrud_arrow.mf_yhot = 7;
	lrud_arrow.mf_nplanes = 1;
	lrud_arrow.mf_fg = 0;
	lrud_arrow.mf_bg = 1;
	memcpy(lrud_arrow.mf_mask, lrud_arr_mask, 16 *2);
	memcpy(lrud_arrow.mf_data, lrud_arr, 16 * 2);
}



void fix_rsc(void)
{
	OBJECT *ob;


	/*--------------- ObjektfÅllmuster und -farben ----------------*/
	if(Sys_info.Max_col >= 15)
	{
		ob = wind_s[WIND_EXPORT].resource_form;
		ob[EMODSL_PAR].ob_spec.obspec.interiorcol = 9;
		ob[EMODSL_PAR].ob_spec.obspec.fillpattern = 7;

		ob = wind_s[WIND_MODULES].resource_form;
		ob[SBACK_MODULE].ob_spec.obspec.interiorcol = 9;
		ob[SBACK_MODULE].ob_spec.obspec.fillpattern = 7;

		ob = Dialog.picMan.tree;
		ob[PMSL_PAR].ob_spec.obspec.interiorcol = 9;
		ob[PMSL_PAR].ob_spec.obspec.fillpattern = 7;
	}

	if(Sys_info.Max_col == 1)
	{
		ob = pic_form;
		/* Bits 8-11 auf 0 setzten = Textfarbe auf 0 setzen */
		ob[COL_IDX].ob_spec.tedinfo->te_color &= ~0x0f00;
	}

	/*--------------- Objektrahmen (wenn kein 3D-AES) ----------------*/
	if(Sys_info.Max_col < 15 || Sys_info.OS&MATSCHIG || !(Sys_info.OSFeatures&AES3D))
	{
		ob = wind_s[WIND_EXPORT].resource_form;
		ob[EMOD_SLIDER].ob_spec.obspec.framesize = -1;
		ob[EMOD_UP].ob_spec.obspec.framesize = -1;
		ob[EMOD_DN].ob_spec.obspec.framesize = -1;
		ob[EMOD_OUTER_BOX].ob_spec.obspec.framesize = -1;
		ob[EMOD_OUTER_BOX].ob_spec.obspec.framesize = -1;
		ob[EMODSL_PAR].ob_spec.obspec.framesize = 1;

		ob = wind_s[WIND_MODULES].resource_form;
		ob[SLID_MODULE].ob_spec.obspec.framesize = -1;
		ob[UP_MODULE].ob_spec.obspec.framesize = -1;
		ob[DN_MODULE].ob_spec.obspec.framesize = -1;
		ob[MODULES_BOX].ob_spec.obspec.framesize = -1;
		ob[SBACK_MODULE].ob_spec.obspec.framesize = 1;
		ob[GNAME_BOX].ob_spec.obspec.framesize = -1;
		ob[GBACK_BOX].ob_spec.obspec.framesize = 1;

		ob = Dialog.picMan.tree;
		ob[PM_SLIDER].ob_spec.obspec.framesize = -1;
		ob[PM_PREVBOX].ob_spec.obspec.framesize = -1;
		ob[PM_UP].ob_spec.obspec.framesize = -1;
		ob[PM_DN].ob_spec.obspec.framesize = -1;
		ob[HW_BOX].ob_spec.obspec.framesize = -1;
		ob[PM_OUTER_BOX].ob_spec.obspec.framesize = -1;
		ob[PMSL_PAR].ob_spec.obspec.framesize = 1;
		
		ob = wind_s[WIND_OPTIONS].resource_form;
		ob[MISC_BUTTON].ob_spec.tedinfo->te_thickness = 1;
		ob[PREVIEWS_BUTTON].ob_spec.tedinfo->te_thickness = 1;
		ob[INTERFACE_BUTTON].ob_spec.tedinfo->te_thickness = 1;
		ob[CONVERS_BUTTON].ob_spec.tedinfo->te_thickness = 1;

		u_tree[DRAGTXT_FRAME].ob_spec.obspec.framesize = -1;
		Dialog.busy.busyTree[BW_BOX].ob_spec.obspec.framesize = -1;
	}

	if(!(Sys_info.OS&MATSCHIG))
	{
		ob = wind_s[WIND_OPTIONS].resource_form;
		ob[MISC_BUTTON].ob_spec.tedinfo->te_thickness = 1;
		ob[PREVIEWS_BUTTON].ob_spec.tedinfo->te_thickness = 1;
		ob[INTERFACE_BUTTON].ob_spec.tedinfo->te_thickness = 1;
		ob[CONVERS_BUTTON].ob_spec.tedinfo->te_thickness = 1;
	}

	return;
}