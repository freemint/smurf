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

/************************************************************************/
/************************************************************************/
/*																		*/
/*								WINDOWZø								*/
/*				 Gem-Library fÅr SMURF Grafikkonverter					*/
/*		 Programmiert vom 01.08.1995 bis heute von Olaf Piesche			*/
/*																		*/
/************************************************************************/
/************************************************************************/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*	Benutzt fast nur Original AES-Funktionen, einzige Ausnahme: Radio-	*/
/*	buttons und Checkboxen (Userdefined). 								*/
/*	Schieberegler, Scrollbare Listfelder mit Autolocator, Object-Drag,	*/
/*	Popups (menu_popup und Selbergebastelte), Hide- und Showtree.		*/
/*	bis auf Popups und Dragfunktion (klar) ALLES Åber die AES-Rechteck-	*/
/*	liste und somit voll Hintergrundbedienbar und sauber.				*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/* AES-Erweiterungs- und Vereinfachungsfunktionen. 
	01.08.1995, Olaf Piesche. Benîtigt RADIOCHK.C!						*/

/* Funktionsliste:
int button_ev(OBJECT *tree)						- 	Evnt-button-Erweiterung.
int f_numedit(int obj, OBJECT *tree)			-	Editpopup zum einfacheren Editieren von FTEXTs.
int f_pop(int obj, int popup, OBJECT *tree, int redraw_ob) - Popupmanager
void f_drag(int obj, int parent, OBJECT *tree) 	- Usergesteuertes Draggen von Objekten in einem Formular.
void f_txtinsert(int num, OBJECT *tree, int txt_obj) - EinfÅgen von Zahlen in Textobjekte
void f_hidetree(OBJECT *tree, int object)	- Verstecken eines Objektes mit allen Kindern
void f_showtree(OBJECT *tree, int object)	- Wiederanzeigen eines versteckten Objekts mit allen Kindern
*/
/*----------------------------------------------------------------------*/

#include <tos.h>
/*#include <multiaes.h>*/
#include <aes.h>
#include <ext.h>
#include <vdi.h>
#include <string.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include "src\smurf_st.h"
#include "sym_gem.h"
#include "modules\import.h"
#include "src\smurf.h"
#include "src\smurf_f.h"

#include "src\xrsrc.h"
#include "src\globdefs.h"

#include "src\smurfine.h"
#include "src\smurfobs.h"
#include "src\ext_obs.h"

void f_update_listfield(LIST_FIELD *lfstruct, OBJECT *tree);

extern int resource_global[20];

extern int handle;						/* VDI-Handle */
extern int klicks;						/* Wieviele Klicks (button_ev) */
extern int mouse_xpos, mouse_ypos;		/* Button_ev Mausposition */
/*extern int mouse_button; */
extern MFORM *dummy_ptr;

extern int moveable;			/* f_drag moveable-Flag */
extern int maxx, maxy;			/* Koordinaten des Hauptformulars */
extern int minx, miny;			
extern int scrwd, scrht;		/* Bildschirmbreite / -hîhe		*/

extern int *messagebuf;			/* fÅr EVNT_MULTI und EVNT_MESAG */
extern int appl_id;

extern int mouse_button, key_at_event;

extern SYSTEM_INFO Sys_info;

extern GRECT screen;


extern OBJECT 	*u_tree;				/* Zeiger auf Radiobutton/Checkbox-Formular*/


/* Leereintrag fÅr Listenfelder */
	char empty_entry[128]="";


/* **************************************************************** */
/* ---------------------- Auf Buttonevent warten ------------------	*/
/* EVNT_BUTTON-Erweiterung. Das angeklickte Objekt wird zurÅck-		*/
/* gegeben, Buttons selektiert. Klicks mit links werden Åberwacht.	*/
/* **************************************************************** */
int button_ev(OBJECT *tree)
{
	int ob_number;
	int newedit;
	int scancode;


	evnt_multi(MU_BUTTON|MU_MESAG,1,3,1, 0,0,0,0,0,0,0,0,0,0, messagebuf, 0,0, &mouse_xpos, &mouse_ypos, &mouse_button, &key_at_event, &scancode, &klicks);
	ob_number = objc_find(tree, 0, MAX_DEPTH, mouse_xpos, mouse_ypos);

	if(ob_number != -1)
		form_button(tree, ob_number, klicks, &newedit);

	return(ob_number);
}


/* ****************************************************************	*/
/* ------------ ôffnen und editieren eines Nummernpopups ----------	*/
/* tree = Zeiger auf Formular, in dem das Popup dargestellt wd soll	*/
/* obj = Index des Klickobjektes									*/
/* Die Funktion îffnet das Nummernpopup NUM_POPUP (eigenes Formular)*/
/* und wartet auf Texteingabe und Return-Druck. Die eingegebene		*/
/* Zahl wird zurÅckgegeben. 										*/
/* ****************************************************************	*/
int f_numedit(int obj, OBJECT *tree, int deflt)
{
	char defstr[10];
	static int editx=1;

	int xpos=0, ypos=0;
	int width, height;
	int event_type;
	int key_scancode, nextob, nextchar;


	objc_offset(tree, obj, &xpos, &ypos);	/* X-und Y-Koordinate des klickobj. ermitteln */
	width=u_tree[NUMEDIT_BOX].ob_width;
	height=u_tree[NUMEDIT_BOX].ob_height;
	
	u_tree[NUMEDIT_BOX].ob_x=xpos;			/* popup Xpos setzen */
	u_tree[NUMEDIT_BOX].ob_y=ypos;			/* ypos setzen */
	
	strcpy(u_tree[NUMEDIT].TextCast, itoa(deflt, defstr, 10));
	
	wind_update(BEG_UPDATE);
	form_dial(FMD_START, xpos-4, ypos-4, width+8, height+8, xpos-4, ypos-4, width+8, height+8);

	objc_draw(u_tree, NUMEDIT_BOX, MAX_DEPTH, 0,0,scrwd,scrht);		/* und Popup zeichnen */
	objc_edit(u_tree, NUMEDIT, 0, &editx, ED_INIT);
	
	do
	{
		event_type=evnt_multi(MU_BUTTON|MU_KEYBD,0x102,0x3,0x0,	  0,0,0,0,0, 0,0,0,0,0,  messagebuf, 
					0,0, &mouse_xpos, &mouse_ypos, &mouse_button, &key_at_event, &key_scancode, &klicks);

		if(event_type&MU_KEYBD)
		{
			form_keybd(u_tree, NUMEDIT, 0, key_scancode, &nextob, &nextchar);
			if(nextchar) objc_edit(u_tree, NUMEDIT, nextchar, &editx, ED_CHAR);
		}
	} while((key_scancode&0xFF)!=KEY_RETURN && (key_scancode>>8)!=KEY_UNDO && event_type!=MU_BUTTON);

	objc_edit(u_tree, NUMEDIT, 0, &editx, ED_END);

	form_dial(FMD_FINISH, xpos-4, ypos-4, width+8, height+8, xpos-4, ypos-4, width+8, height+8);
	wind_update(END_UPDATE);

	if(event_type==MU_BUTTON || (key_scancode>>8)==KEY_UNDO) return(deflt);

	return( atoi(u_tree[NUMEDIT].TextCast) );
}


/* ****************************************************************	*/
/* ------------------- Funktion fÅr popup -------------------------	*/
/* Funktion zum Bedienen von Popup-MenÅs.							*/
/* popup = der Index des Popup-Formulars							*/
/* obj = das Objekt, an	dessen Koordinaten das popup dargestellt	*/
/* werden soll (und in das der Popuptext eingefÅgt wird)			*/
/*																	*/
/* Diese Funktion benutzt sein dem 1.8.95 auch die AES-Popup-		*/
/* Funktion. Schînen Dank an die TOS-Entwickler dafÅr (coole Sache).*/
/*																	*/
/* ZurÅckgegeben wird der Index des Popup-Buttons, der angeklickt	*/
/* wurde. Wird -1 zurÅckgegeben, wurde das Popup geschlossen!		*/
/******************************************************************	*/
int f_pop(POP_UP *popup_struct, int mouseflag, int button, OBJECT *poptree)
{
	int xpos, ypos;
	int pop_button=1, backval;
	int mx,my;
	int popwidth, popheight;
	int dummy=0;
	int pbuf[50];
	int item, popup, obj;
	int popup_head, popup_tail, cycled_item, obutton;
	int em_x, em_y, mbutt;
	int abx,aby, abw, abh;
	int curr_ob;
	int mb[32];

	OBJECT *tree, *popup_form;
	MENU popup_menu;


	tree=popup_struct->display_tree;
	item=popup_struct->item;
	popup=popup_struct->popup_tree;
	obj=popup_struct->display_obj;
	
	if(poptree==NULL)
		xrsrc_gaddr(0, popup, &popup_form, resource_global);		/* Popup-Formular holen */
	else
		popup_form=poptree;

	/* Online-Resourceanpassung fÅr MagiC */
	curr_ob = popup_form[0].ob_head;
	if(Sys_info.OS&MATSCHIG)
	{
		do
		{
			if(popup_form[curr_ob].ob_type!=G_STRING)
			{
				popup_form[curr_ob].ob_flags &= ~ACTIVATOR;
				popup_form[curr_ob].ob_flags |= INDICATOR;
			}

			curr_ob = popup_form[curr_ob].ob_next;
		} while(curr_ob!=0 && curr_ob!=-1);
	}
	else
	{
		do
		{
			if(popup_form[curr_ob].ob_type!=G_STRING)
				popup_form[curr_ob].ob_spec.tedinfo->te_thickness = 0;
			curr_ob = popup_form[curr_ob].ob_next;
		} while(curr_ob!=0 && curr_ob!=-1);
	}


	if(!mouseflag && obj!=-1)
		objc_offset(tree, obj, &xpos, &ypos);		/* Koordiaten fÅr popup holen */

	evnt_multi(MU_TIMER,0,0,0,
	0, dummy, dummy, dummy, dummy, 0, dummy, dummy, dummy, dummy,
	pbuf, 20,0, &mouse_xpos, &mouse_ypos, &dummy, &dummy, &dummy, &dummy);

	xpos-=3;

	if(mouseflag)
	{ 
		xpos=mouse_xpos - (popup_form[0].ob_width/2);
		ypos=mouse_ypos - (popup_form[0].ob_height/2); 
	}

	/*
	 * Position clippen
	 */
	Window.windGet(0, WF_WORKXYWH, &abx, &aby, &abw, &abh);

	if(xpos + popup_form[0].ob_width > abw)
		xpos = abw - popup_form->ob_width;
	else
		if(xpos < abx)
			xpos = abx;

	if(ypos + popup_form[0].ob_height > abh + aby)
		ypos = abh+ aby - popup_form->ob_height;
	else
		if(ypos<aby + 3)
			ypos = aby + 3;

	popup_form->ob_x = xpos;
	popup_form->ob_y = ypos;
	
	if(button == REDRAW)
	{
		strcpy(tree[obj].TextCast, popup_form[popup_struct->item].TextCast);
		return(popup_struct->item);
	}

	/*----------------------------- Cyclebutton angeklickt */
	if(button==popup_struct->Cyclebutton)
	{
		popup_head=popup_form->ob_head;
		popup_tail=popup_form->ob_tail;

		/*----------------- nÑchstes nicht DISABLEDes Item ermitteln */
		cycled_item=item;
		do
		{
			if(mouse_button&0x01)
				cycled_item++;
			else
				cycled_item--;

			if(cycled_item>popup_tail)
				cycled_item=popup_head;
			else
				if(cycled_item<popup_head)
					cycled_item=popup_tail;
		} while((popup_form[cycled_item].ob_state&DISABLED) && cycled_item!=item);

		popup_struct->item=cycled_item;

		if(obj!=-1)
			strcpy(tree[obj].TextCast, popup_form[cycled_item].TextCast);

		return(cycled_item);
	}
	else
		graf_mouse(ARROW, dummy_ptr);

	/*********** Popup îffnen: *************/
	/*----------------------------------- menu_popup mîglich... ----------------*/
	if(Sys_info.OSFeatures&MPOPUP)
	{
		graf_mouse(ARROW, dummy_ptr);
		popup_menu.mn_tree=popup_form;		/* Popup-Formular*/
		popup_menu.mn_menu=0;				/* Vaterobjekt */
		popup_menu.mn_scroll=1;				/* Scrolling?	*/
		popup_menu.mn_item=item;			/* GedrÅckten Button holen */
		
		f_set_syspal();						/* Systempalette! */

		wind_update(BEG_UPDATE);
		backval=menu_popup(&popup_menu, xpos, ypos, &popup_menu);
		wind_update(END_UPDATE);
		
		if(backval!=0)
		{
			pop_button=popup_menu.mn_item;	/* gedrÅckten Button holen */

			if(obj!=-1)	
				if(tree!=NULL)
					strcpy(tree[obj].TextCast, popup_form[pop_button].TextCast);

			popup_struct->item=pop_button;
		}
		
		if(backval==0)
			pop_button=-1;					/* Popup abgebrochen -> -1 zurÅck */

		return(pop_button);
	}
	/*------------------------------------ kein menu_popup? ----------------*/
	else
	{
		popwidth=popup_form->ob_width;
		popheight=popup_form->ob_height;

		ypos-=3;
		popup_form->ob_x=xpos;
		popup_form->ob_y=ypos;
		
		f_set_syspal();						/* Systempalette! */

		wind_update(BEG_MCTRL);
		wind_update(BEG_UPDATE);
		form_dial(FMD_START, xpos-2,ypos-2,popwidth+4,popheight+4, xpos-2,ypos-2,popwidth+4,popheight+4);
		
		objc_draw(popup_form, 0, MAX_DEPTH, 0,0,Sys_info.screen_width, Sys_info.screen_height);
		obutton=1;
		
		/*----------- Eingangsbutton selektieren */
		graf_mkstate(&mouse_xpos, &mouse_ypos, &dummy, &dummy);
		mx=mouse_xpos;
		my=mouse_ypos;
		pop_button=objc_find(popup_form, 0, MAX_DEPTH, mx,my);
		if(pop_button>0 && !(popup_form[pop_button].ob_state&DISABLED))
		{
			objc_offset(popup_form, pop_button, &em_x, &em_y);
			em_x--;
			em_y--;
			popup_form[pop_button].ob_state |= SELECTED;
			objc_draw(popup_form, pop_button, MAX_DEPTH, 0,0,Sys_info.screen_width, Sys_info.screen_height);
			obutton=pop_button;
		}

		do
		{
			graf_mkstate(&em_x, &em_y, &dummy, &dummy);
			backval = evnt_multi(MU_BUTTON|MU_M1, 1,1,1, 1,em_x,em_y,1,1, 0,0,0,0,0,
						mb, 0,0, &mx, &my, &mbutt, &dummy, &dummy, &dummy);

			pop_button=objc_find(popup_form, 0, MAX_DEPTH, mx,my);

			if(obutton!=pop_button)
			{
				if(obutton>0)
				{
					popup_form[obutton].ob_state &= ~SELECTED;
					objc_draw(popup_form, obutton, MAX_DEPTH, 0,0,Sys_info.screen_width, Sys_info.screen_height);
				}

				if(pop_button>0 && !(popup_form[pop_button].ob_state&DISABLED))
				{
					popup_form[pop_button].ob_state |= SELECTED;
					objc_draw(popup_form, pop_button, MAX_DEPTH, 0,0,Sys_info.screen_width, Sys_info.screen_height);
				}

				obutton=pop_button;
			}

			if(backval&MU_BUTTON && pop_button==-1)
				break;
		} while(popup_form[pop_button].ob_state&DISABLED || pop_button==0 || !(backval&MU_BUTTON) );

		if(pop_button>0)
			popup_form[pop_button].ob_state &= ~SELECTED;
		
		form_dial(FMD_FINISH, xpos-2,ypos-2,popwidth+4,popheight+4, xpos-2,ypos-2,popwidth+4,popheight+4);
		wind_update(END_UPDATE);
		wind_update(END_MCTRL);
		
		if(obj!=-1 && tree!=NULL && pop_button!=-1)
			strcpy(tree[obj].TextCast, popup_form[pop_button].TextCast);
			
		if(pop_button > 0)
			popup_struct->item=pop_button;

		return(pop_button);
	}

}


/* **************************************************************** */
/* -------------- Funktion zum Draggen von Objekten ---------------	*/
/* obj = zu draggendes objekt										*/
/* parent = Parent von obj (innerhalb dessen gedragged wird)		*/
/* *tree = Zeiger auf Formular										*/
/*																	*/
/* Die Funktion Åbernimmt das Ziehen eines Objektes innerhalb eines	*/
/* Formulars oder Objekts mit der Maus, trÑgt die neue Position ein	*/
/* und zeichnet es neu. Erwartet ein globales Flag 'moveable', das	*/
/* angibt, ob ein Objekt gezogen werden darf (1) oder nicht (0).	*/
/* ****************************************************************	*/
void f_drag(int obj, int parent, OBJECT *tree)
{
	int xkoo, ykoo;
	int oxkoo, oykoo;
	int xmin, ymin;
	int xpos, ypos;
	int w, h;
	int clipw, cliph;


	w=tree[obj].ob_width;			/* Breite und... */
	h=tree[obj].ob_height+1;		/* Hîhe ermitteln (fÅr Dragbox) */
	oxkoo=tree[obj].ob_x;			/* Breite und... */
	oykoo=tree[obj].ob_y;			/* Breite und... */
	
	clipw=tree[0].ob_width;			/* Breite und... */
	cliph=tree[0].ob_height+1;		/* Hîhe ermitteln (fÅr Dragbox) */
	
	if(moveable)
	{
		objc_offset(tree, 0, &xmin, &ymin);			/* Hauptformular - Koo's ermitteln 	*/
		
		objc_change(tree, obj, 0, 0,0,scrwd,scrht, SHADOWED, 0); 	/* 'Hochheben' */
		objc_draw(tree, obj, MAX_DEPTH, oxkoo+w-5+xmin,oykoo+ymin,10,h+10);		/* Neuzeichnen rechter Rand */
		objc_draw(tree, obj, MAX_DEPTH, oxkoo+xmin,oykoo+h-5+ymin,w+10,10);		/* Neuzeichnen unterer Rand */
		
		graf_mouse(FLAT_HAND, dummy_ptr);			/* Zeigehand - Mauszeiger */
		objc_offset(tree, obj, &xpos, &ypos);		/* Dragobjekt - Koo's ermitteln		*/
		graf_dragbox(w,h, xpos,ypos, xmin+4,ymin+4,clipw-8,cliph-8, &xkoo, &ykoo);		/* Objekt verschieben */
		xkoo=((xkoo+4)>>3) <<3;												/* Neue Koordinaten SNAPpen */
		ykoo=((ykoo+4)>>3) <<3;
		xkoo-=xmin;
		ykoo-=ymin;
		if((xkoo+tree[obj].ob_width) > clipw-4) xkoo-=((xkoo+tree[obj].ob_width)-(clipw-4));
		if((ykoo+tree[obj].ob_height) >cliph-4) ykoo-=((ykoo+tree[obj].ob_height)-(cliph-4));
		tree[obj].ob_x=xkoo;				/* Xpos setzen */
		tree[obj].ob_y=ykoo;				/* ypos setzen */
		objc_order(tree, obj, -1);			/* Objekt 'toppen' */
		objc_change(tree, obj, 0, 0,0,scrwd,scrht, NORMAL, 0); 		/* 'Absetzen' */
		
		if(xkoo+xmin!=oxkoo+xmin || ykoo+ymin!=oykoo+ymin)
			objc_draw(tree, obj, MAX_DEPTH, xkoo+xmin-5,ykoo+ymin-5,w+10,h+10);			/* Neuzeichnen des neuen Bereichs */
		
		objc_draw(tree, parent, MAX_DEPTH, oxkoo+xmin-5,oykoo+ymin-5,w+10,h+10);	/* Neuzeichnen des alten Bereichs */
	}

}


/* ****************************************************************	*/
/* Wandelt eine Zahl in einen String und fÅgt sie ins Objekt txt_obj*/
/* im Baum *tree ein (muû TEDINFO sein) und redrawed es wenn nîtig.	*/
/* ****************************************************************	*/
void f_txtinsert(int num, OBJECT *tree, int txt_obj, WINDOW *ws)
{
	char vol_disp[10];

	
	itoa(num, vol_disp, 10);
	strcpy(tree[txt_obj].TextCast, vol_disp);

	if(!ws)
		objc_draw(tree, txt_obj, 1, 0,0,scrwd,scrht);
	else
		Window.redraw(ws, NULL, txt_obj, DRAWNOPICTURE);

	return;
}


/* ****************************************************************	*/
/* -- Verstecken eines ganzen Objektbaums inklusive aller Kinder --	*/
/* *tree ist der Baum, in dem und object das parentobjekt, das		*/
/* versteckt werden soll. 											*/
/* ****************************************************************	*/
void f_hidetree(OBJECT *tree, int object)
{
	int startob, endob, ob_t;
	OBJECT *treepointer;


	xrsrc_gaddr(1, object, &treepointer, resource_global);
	startob=treepointer->ob_head;
	endob=treepointer->ob_tail;
	
	tree[object].ob_flags|=HIDETREE;	
	
	for(ob_t=startob; ob_t<=endob; ob_t++)
		tree[ob_t].ob_flags|=HIDETREE;	

	return;
}


/* ****************************************************************	*/
/* --- Aufdecken eines ganzen Objektbaums inklusive aller Kinder --	*/
/* *tree ist der Baum, in dem und object das parentobjekt, das		*/
/* aufgedeckt werden soll. 											*/
/* ****************************************************************	*/
void f_showtree(OBJECT *tree, int object)
{
	OBJECT *treepointer;
	int startob, endob, ob_t;


	xrsrc_gaddr(1, object, &treepointer, resource_global);
	startob=treepointer->ob_head;
	endob=treepointer->ob_tail;

	tree[object].ob_flags&=~HIDETREE;	

	for(ob_t=startob; ob_t<=endob; ob_t++)
		tree[ob_t].ob_flags&=~HIDETREE;	

	return;
}

/* ****************************************************************	*/
/* ------------ Funktion zum Bedienen von Schiebereglern ----------	*/
/* regler = Reglerobjekt											*/
/* fhr = ReglerfÅhrung (Parent von regler)							*/
/* *tree = Zeiger auf Formular										*/
/*																	*/
/* Die Funktion Åbernimmt das Ziehen des Reglers mit der Maus und	*/
/* gibt seine neue Position zurÅck (0=ganz oben, 1000=ganz unten)	*/
/* ****************************************************************	*/
int f_rslid(SLIDER *slider_struct)
{
	int regler, fhr, txt_obj;
	int min_val, max_val;
	int mousex, mousey, mbutt, oldy;
	int ypos, fhr_x, fhr_y;
	int slidval;
	int klickobj;
	int dummy;
	int reg_x, reg_y, ev;

	float differ;

	OBJECT *rtree;
	WINDOW *windst;
	GRECT wsize;


	regler = slider_struct->regler;
	fhr = slider_struct->schiene;
	rtree = slider_struct->rtree;
	txt_obj = slider_struct->txt_obj;
	min_val = (int)slider_struct->min_val;
	max_val = (int)slider_struct->max_val;
	windst = slider_struct->window;
	differ = (float)(max_val-min_val);

	wsize.g_x = windst->wx;
	wsize.g_y = windst->wy;
	wsize.g_w = windst->ww;
	wsize.g_h = windst->wh;

	if(rtree[regler].ob_state&DISABLED)
		return(0);

	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);

	graf_mkstate(&mousex, &mousey, &dummy, &dummy);
	klickobj = objc_find(rtree, 0, MAX_DEPTH, mousex, mousey);

	if(klickobj == regler)
	{
		objc_offset(rtree, fhr, &fhr_x, &fhr_y);
		graf_mouse(POINT_HAND, dummy_ptr);						/* Zeigehand - Mauszeiger */

		do
		{
			oldy = mousey;
			ev = evnt_multi(MU_BUTTON|MU_M1, 1,1,0, 1,0,mousey,scrwd,1, 0,0,0,0,0, NULL, dummy,dummy, &dummy, &mousey, &mbutt, &dummy, &dummy, &dummy);

			if(oldy != mousey)
			{
				ypos = mousey - fhr_y;
				if(ypos < 0)
					ypos = 0;
				else
					if(ypos > 122)
						ypos = 122;

				if(rtree[regler].ob_y == ypos)
					continue;

				rtree[regler].ob_y = ypos;

				if(windst == NULL)
					objc_draw(rtree, fhr, 2, 0,0,scrwd,scrht);		/* Neuzeichnen */
				else
					Window.redraw(windst, &wsize, fhr, DRAWNOPICTURE);

				slidval = (long)(121 - ypos) * (differ / 121F) + min_val;
				if(slidval < min_val)
					slidval = min_val;
				else
					if(slidval > max_val)
						slidval = max_val;

				f_txtinsert(slidval, rtree, txt_obj, windst);
			}
		} while(ev != MU_BUTTON);

		graf_mouse(ARROW, dummy_ptr);						/* Pfeil - Mauszeiger */
	}
	else
	if(klickobj == fhr)
	{
		objc_offset(rtree, regler, &reg_x, &reg_y);

		ypos = rtree[regler].ob_y;
		if(mousey < reg_y - 1)
			ypos--;
		else
			if(mousey > reg_y + rtree[regler].ob_height)
				ypos++;

		if(ypos < 0)
			ypos = 0;
		else
			if(ypos > 122)
				ypos = 122;

		rtree[regler].ob_y = ypos;

		slidval = (long)(121 - ypos) * (differ / 121F) + min_val;
		if(slidval < min_val)
			slidval = min_val;
		else
			if(slidval > max_val)
				slidval = max_val;

		f_txtinsert(slidval, rtree, txt_obj, windst);
		Window.redraw(windst, &wsize, fhr, DRAWNOPICTURE);
	}

	wind_update(END_UPDATE);
	wind_update(END_MCTRL);

	return((int)slidval);
}




/******************************************************************	*/
/*			Slider aus *sliderstruct auf Wert value setzen			*/
/******************************************************************	*/
void setslider(SLIDER *sliderstruct, long value)
{
	char string[8];

	int ypos;
	int sliderob, slidertxt, sliders;
	int gx,gy,gw,gh;

	long max_val, min_val;

	float div;

	OBJECT *tree;


	max_val = sliderstruct->max_val;
	min_val = sliderstruct->min_val;
	sliderob = sliderstruct->regler;
	slidertxt = sliderstruct->txt_obj;
	sliders = sliderstruct->schiene;
	tree = sliderstruct->rtree;
	
	if(value > max_val)
		value = max_val;
	else
		if(value < min_val)
			value = min_val;

	div = (max_val - min_val) / 121F;
	if(div == 0)
		ypos = 0;
	else
		ypos = 121 - ((value - min_val) / div);

	if(ypos > 122)
		ypos = 122;
	else
		if(ypos < 0)
			ypos = 0;

	sliderstruct->rtree[sliderob].ob_y = ypos;

	ltoa(value, string, 10);
	strcpy(tree[slidertxt].TextCast, string);

	if(sliderstruct->window)
	{
		Window.redraw(sliderstruct->window, NULL, sliders, DRAWNOPICTURE);
		Window.redraw(sliderstruct->window, NULL, slidertxt, DRAWNOPICTURE);
	}
	else
	{
		wind_update(BEG_UPDATE);

		objc_offset(tree, sliders, &gx, &gy);
		gx -= 3;
		gy -= 3;
		gw = tree[sliders].ob_width + 6;
		gh = tree[sliders].ob_height + 6;
		objc_draw(tree, sliders, MAX_DEPTH, gx,gy,gw,gh);

		objc_offset(tree, slidertxt, &gx, &gy);
		gx -= 3;
		gy -= 3;
		gw = tree[slidertxt].ob_width + 6;
		gh = tree[slidertxt].ob_height + 6;
		objc_draw(tree, slidertxt, MAX_DEPTH, gx,gy,gw,gh);

		wind_update(END_UPDATE);
	}

	return;
}


/*************************************************************	*/
/*--------------------- Listenfeld-Funktion -------------------	*/
/*************************************************************	*/
int f_listfield(long *pwindow, int klick_obj, int key_scancode, LIST_FIELD *lfstruct)
{
int listparent;
int first_entry, last_entry, entryc=0;
OBJECT *tree;
WINDOW *window;
char * *entries;
char *myentry;
int sl_parent, sl_ob;
int spar_height, spar_y, spar_x, number_of_entries, maxentries;
int oldoffset;
int mbutt, oldy, mousey=0;
int gx,gy;
int next_obj, first_sel;
int oldkey_scan, ent_found;
int scan_ascii;
int old_rtoff, old_slpos, dummy=0;
char *al_string, *al_last, len;

window = (WINDOW *)pwindow;

tree=window->resource_form;

listparent = lfstruct->parent_obj;
first_entry = tree[listparent].ob_head;
last_entry = tree[listparent].ob_tail;
sl_ob = lfstruct->slider_obj;
sl_parent = lfstruct->slider_parent;
entries = (char* *)(lfstruct->entry);

number_of_entries = lfstruct->number_entries;
maxentries = lfstruct->max_entries;


gx = tree[sl_parent].ob_x-2+tree[0].ob_x;
gy = tree[sl_parent].ob_y-2+tree[0].ob_y;

oldoffset = lfstruct->scroll_offset;

oldkey_scan = key_scancode;
key_scancode>>=8;

if(key_scancode!=0 || klick_obj!=0)
{	
	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);
}

/*---------------------- Tastenbedienung ------------------------*/
if(key_scancode!=0)
{
	/* erstmal Cursortasten ---------------------------------*/
	if(key_scancode==KEY_DOWN || key_scancode==KEY_UP)
	{

		entryc=get_selected_object(tree, first_entry, last_entry);
		tree[entryc].ob_state &= ~SELECTED;		/* Deselektieren! */
		next_obj=entryc;
	
		if(key_scancode==KEY_DOWN)
		{
			if(key_at_event&KEY_SHIFT)
			{
				objc_offset(tree, sl_ob, &gx, &gy);
				mouse_ypos=gy+1;
				klick_obj=sl_parent;
			}
			else
			{
				myentry=(char*)*((char* *)(entries+lfstruct->scroll_offset+entryc-first_entry+1));
				if( entryc<last_entry && myentry )
					next_obj=entryc+1;
				else klick_obj=lfstruct->slar_dn;
			}

			tree[next_obj].ob_state |= SELECTED;
			if(entryc<last_entry)
				Window.redraw(window, NULL, listparent, DRAWNOPICTURE);

		}
		else if(key_scancode == KEY_UP)
		{
			if(key_at_event&KEY_SHIFT)
			{
				objc_offset(tree, sl_ob, &gx, &gy);
				mouse_ypos = gy - 1;
				klick_obj = sl_parent;
			}
			else
			{
				myentry = (char *)*((char **)(entries + lfstruct->scroll_offset + entryc-first_entry - 1));
				if(entryc>first_entry && myentry)
					next_obj = entryc - 1;
				else
					klick_obj = lfstruct->slar_up;
			}

			tree[next_obj].ob_state |= SELECTED;
			if(entryc > first_entry)
				Window.redraw(window, NULL, listparent, DRAWNOPICTURE);
		}
	
	} /* Cursor hoch/runter */

	/* und jetzt alle anderen --------------------------------*/
	else
	{
		scan_ascii=scan_2_ascii(oldkey_scan, 0);
		
		/*------ lîschen im Autolocator */
		if(lfstruct->autolocator && lfstruct->auto_len>0)
		{
			if(key_scancode==KEY_ESCAPE || key_scancode==KEY_HOME)
				memset(lfstruct->autolocator, 0x0, lfstruct->auto_len);
			else
				if(key_scancode==KEY_BACKSPACE)
				{
					len=strlen(lfstruct->autolocator);
					al_last=lfstruct->autolocator+len;
					if(len>0)
						*(al_last-1)=0;
				}
			
			if(key_scancode==KEY_HOME)
			{ 
				if(key_at_event&KEY_SHIFT) 
				{
					myentry= (char*) *((char* *)(entries+(lfstruct->number_entries-1)));
					if(lfstruct->autolocator && myentry && strlen(myentry))
						strncpy(lfstruct->autolocator, myentry, strlen(myentry));
				}
				else
				{
					myentry= (char*) *((char* *)(entries+0));
					if(lfstruct->autolocator && myentry && strlen(myentry))
						strncpy(lfstruct->autolocator, myentry, strlen(myentry));
				}
			}
		}

		/*------------- Autolocator  -----------------------*/
		if(lfstruct->autolocator!=NULL && lfstruct->auto_len>0
			&& (((scan_ascii>='A' && scan_ascii<='Z') 
			  || (scan_ascii>='0' && scan_ascii<'9')
			  || scan_ascii==' ') || key_scancode==KEY_BACKSPACE
			  || key_scancode==KEY_ESCAPE || key_scancode==KEY_HOME) )
		{
		/* Buchstabe suchen */
			entryc=0;
			ent_found=0;

			len=strlen(lfstruct->autolocator);
			al_last=lfstruct->autolocator+len;
			if(len<lfstruct->auto_len && key_scancode!=KEY_BACKSPACE && key_scancode!=KEY_ESCAPE) 
				*(al_last)=(char)scan_ascii;
			
			al_string = lfstruct->autolocator;

			do
			{
				myentry=(char*) *((char* *)(entries+entryc));
				if(myentry)
				{
					if( strnicmp(al_string, myentry, strlen(al_string))==0 ) ent_found=1;
					entryc++;
				}
			} while(entryc<lfstruct->number_entries && ent_found==0);


		/* Eintrag gefunden? Hinspringen */
			if(entryc<=lfstruct->number_entries && ent_found)
			{
				first_sel=get_selected_object(tree, first_entry, last_entry);
				/* damit auch wirklich nur was passiert wenn sich der ausgewÑhlte Eintrag geÑndert hat */
				if(entryc - lfstruct->scroll_offset - 1 != first_sel - first_entry)
				{
					tree[first_sel].ob_state &= ~SELECTED;
				
					lfstruct->scroll_offset=entryc-1;
								
					if(lfstruct->scroll_offset+maxentries > number_of_entries)
						lfstruct->scroll_offset = number_of_entries-maxentries;
				
					if(lfstruct->scroll_offset < 0) lfstruct->scroll_offset = 0;
				 
					tree[(entryc-lfstruct->scroll_offset+first_entry)-1].ob_state=SELECTED;
					
					klick_obj=REDRAW;
				}
			}


		}
	} /* andere Taste */

}



/*---------------------- Mausbedienung -------------------------*/
if(klick_obj!=0)
{
	oldoffset=lfstruct->scroll_offset;

	/*----- runterscrollen -----------*/
	if(klick_obj==lfstruct->slar_dn)
	{
		if(lfstruct->scroll_offset+lfstruct->max_entries < number_of_entries)
		{
			lfstruct->scroll_offset++;
			tree[sl_ob].ob_height = tree[sl_parent].ob_height*((float)maxentries/(float)number_of_entries);
			spar_height= tree[sl_parent].ob_height - tree[sl_ob].ob_height - 4;
			tree[sl_ob].ob_y =(lfstruct->scroll_offset * spar_height / (number_of_entries-maxentries)) +2;
		}
	}
	
	/*----- raufscrollen -----------*/
	else
	if(klick_obj==lfstruct->slar_up)
	{
		if(lfstruct->scroll_offset > 0)
		{
			lfstruct->scroll_offset--;
			tree[sl_ob].ob_height = tree[sl_parent].ob_height*((float)lfstruct->max_entries/(float)lfstruct->number_entries);
			spar_height= tree[sl_parent].ob_height - tree[sl_ob].ob_height - 4;
			tree[sl_ob].ob_y =(lfstruct->scroll_offset * spar_height / (number_of_entries-maxentries)) +2;
		}
	}

	/*--------------- RT-Sliden ----------------*/
	else
	if(klick_obj==sl_ob)
	{
		graf_mouse(FLAT_HAND, dummy_ptr);
		
		change_object(window, sl_ob, SEL, 1);
		
		objc_offset(tree, sl_parent, &spar_x, &spar_y);
		spar_height=tree[sl_parent].ob_height;

		graf_mkstate(&dummy, &mousey, &dummy, &dummy);
		
		do
		{
			oldy = mousey;
			evnt_multi(MU_BUTTON|MU_M1, 1,1,0, 1,0,mouse_ypos,scrwd,1, 0,0,0,0,0, NULL, dummy,dummy, &dummy, &mousey, &mbutt, &dummy, &dummy, &dummy);

			if(oldy != mousey)
			{
				old_slpos = tree[sl_ob].ob_y;
				tree[sl_ob].ob_y = (mousey-spar_y)-(tree[sl_ob].ob_height/2);
				if(tree[sl_ob].ob_y < 2)
					tree[sl_ob].ob_y = 2;
				else
					if(tree[sl_ob].ob_y > spar_height-tree[sl_ob].ob_height-2)
						tree[sl_ob].ob_y = spar_height-tree[sl_ob].ob_height-2;

				if(tree[sl_ob].ob_y == old_slpos)
					continue;
			
				old_rtoff=lfstruct->scroll_offset;
				
				lfstruct->scroll_offset=(tree[sl_ob].ob_y*(number_of_entries-maxentries))/(spar_height-tree[sl_ob].ob_height-2);
				
				if(lfstruct->scroll_offset+lfstruct->max_entries > number_of_entries)
					lfstruct->scroll_offset=number_of_entries-lfstruct->max_entries;
				else
					if(lfstruct->scroll_offset<0)
						lfstruct->scroll_offset=0;
		
				if(number_of_entries<=maxentries)
				{
					tree[sl_ob].ob_y=2;
					lfstruct->scroll_offset=0;
				}
		
		
				/* Listenupdate ist nur notwendig, wenn sich Scrolloffset geÑndert hat. */
		
				if(old_rtoff!=lfstruct->scroll_offset)
				{
					f_update_listfield(lfstruct, tree);
					Window.redraw(window, NULL, listparent, DRAWNOPICTURE);
				} /* Scrolloffset geÑndert? */
		
				if((mousey>spar_y) && (mousey<spar_y+spar_height))
					Window.redraw(window, NULL, sl_parent, DRAWNOPICTURE);
			
			}
		} while(mbutt!=0);

		if(lfstruct->scroll_offset==0)
		{
			tree[sl_ob].ob_y=2;
			Window.redraw(window, NULL, sl_parent, DRAWNOPICTURE);
		}
		
		change_object(window, sl_ob, UNSEL, 1);
		graf_mouse(ARROW, dummy_ptr);
	}


	/*------------------ BlÑttern / Redraw ----------------*/
	else
	if(klick_obj==sl_parent || klick_obj == REDRAW)
	{
		if(klick_obj==sl_parent)
		{
			objc_offset(tree, sl_ob, &gx, &gy);
	
			if(mouse_ypos<gy) lfstruct->scroll_offset-=lfstruct->max_entries;
			else if(mouse_ypos>gy) lfstruct->scroll_offset+=lfstruct->max_entries;
		
			if(lfstruct->scroll_offset+lfstruct->max_entries > number_of_entries)
				lfstruct->scroll_offset = number_of_entries-lfstruct->max_entries;
		
			if(lfstruct->scroll_offset<0)
				lfstruct->scroll_offset=0;
		}
	
		f_update_listfield(lfstruct, tree);
	
		if(number_of_entries>maxentries) tree[sl_ob].ob_height = tree[sl_parent].ob_height*((float)maxentries/(float)number_of_entries) -2;
		else tree[sl_ob].ob_height = tree[sl_parent].ob_height-4;
	
		if(tree[sl_ob].ob_height > (tree[sl_parent].ob_height-2)) tree[sl_ob].ob_height=tree[sl_parent].ob_height-3;
		else if(tree[sl_ob].ob_height <10) tree[sl_ob].ob_height=10;
	
		spar_height= tree[sl_parent].ob_height - tree[sl_ob].ob_height - 2;
	
		if(number_of_entries-maxentries==0) tree[sl_ob].ob_y=2;
		else tree[sl_ob].ob_y =(lfstruct->scroll_offset * (spar_height-2) / (number_of_entries-maxentries))+2;
	
		Window.redraw(window, NULL, listparent, DRAWNOPICTURE);
		Window.redraw(window, NULL, sl_parent, DRAWNOPICTURE);
	}

	if((klick_obj==lfstruct->slar_up || klick_obj==lfstruct->slar_dn) && oldoffset!=lfstruct->scroll_offset)
	{
		f_update_listfield(lfstruct, tree);
	
		Window.redraw(window, NULL, listparent, DRAWNOPICTURE);
		Window.redraw(window, NULL, sl_parent, DRAWNOPICTURE);
	}
}

	/*---- Selektiertes Eintrags-Objekt ermitteln! */
	entryc=get_selected_object(tree, first_entry, last_entry);

	if(key_scancode==KEY_HOME && lfstruct->autolocator!=NULL && lfstruct->auto_len>0)	
		memset(lfstruct->autolocator, 0x0, lfstruct->auto_len);

	if(key_scancode!=0 || klick_obj!=0)
	{
		wind_update(END_UPDATE);
		wind_update(END_MCTRL);
	}

	return(entryc-first_entry+lfstruct->scroll_offset);
}








/******************************************************************	*/
/*	FÅhrt ein Update der ListfeldeintrÑge eines Listfields durch	*/
/******************************************************************	*/

/*	Jetzt werden die Stringzeiger aus der LIST_FIELD-Struktur		*/
/*	direkt auf die EintrÑge in der Resource gesetzt. Dadurch habe	*/
/*	ich nochmal massenweise Stringoperationen gespart und das Update*/
/*	um weitere ca. 30% beschleunigt...								*/
/*	Olaf, 28.09.96													*/

void f_update_listfield(LIST_FIELD *lfstruct, OBJECT *tree)
{
	register char * * entries;
	register char *myentry;
	register int stringcount, entryc;
	int first_entry, last_entry;
	int number_of_entries;

	OBJECT *entry_obj;

	first_entry = tree[lfstruct->parent_obj].ob_head;
	last_entry = tree[lfstruct->parent_obj].ob_tail;
	number_of_entries = lfstruct->number_entries;

	entries=(char* *)(lfstruct->entry);

	/* 
	 * Strings ins Listfeld kopieren 
	 */
	entryc=first_entry;
	stringcount=lfstruct->scroll_offset;

	for(entryc=first_entry; entryc<=last_entry; entryc++)
	{
		myentry=(char*)*((char* *)(entries+stringcount));
		entry_obj=&(tree[entryc]);

		if(myentry!=NULL && stringcount<number_of_entries)
		{
			entry_obj->TextCast=myentry;
			entry_obj->ob_state &= ~DISABLED;
		}
		else
		{
			entry_obj->TextCast=empty_entry;
			entry_obj->ob_state |= DISABLED;
		}
		stringcount++;
	} 

}


/******************************************************************	*/
/* ermittelt im tree *tree von Objekt first_entry bis last_entry	*/
/* das erste selektierte Objekt.									*/
/******************************************************************	*/
int get_selected_object(OBJECT *tree, int first_entry, int last_entry)
{
	int entryc;


	entryc=first_entry;

	do
	{
		if(tree[entryc].ob_state&SELECTED)				/* selektiert? */
			break;

		entryc++;
	} while(entryc<=last_entry);
	
	/* kein selektiertes Obj? - -1 */
	if(entryc>last_entry)
		entryc=-1;
	
	return(entryc);
}


/******************************************************************	*/
/*--------------------- Listenfeld generieren ----------------------*/
/******************************************************************	*/
void f_generate_listfield(int uparrow, int dnarrow, int sliderparent, int sliderobject,
	int listparent, char *listentries, int num_entries, int max_entries, LIST_FIELD *listfield, int autoloc)
{
	if(listfield == NULL)
		listfield = malloc(sizeof(LIST_FIELD));
	
	listfield->number_entries = num_entries;
	listfield->entry = listentries;
	listfield->max_entries = max_entries;
	listfield->parent_obj = listparent;
	listfield->slider_parent = sliderparent;
	listfield->slider_obj = sliderobject;
	listfield->slar_up = uparrow;
	listfield->slar_dn = dnarrow;
	listfield->scroll_offset = 0;
	listfield->auto_len = autoloc;
	
	if(autoloc != 0 && autoloc != -1)
	{
		listfield->autolocator = malloc(autoloc);
		memset(listfield->autolocator, 0x0, autoloc);
	}

	return;
} /* f_generate_listfield */


/******************************************************************	*/
/* -------- Hintergrundbedienbares KomplementÑr zu OBJC_CHANGE ----	*/
/* window = Zeiger auf Fensterstruktur, in dem das objct liegt		*/
/*																	*/
/* LÑuft Åber die Rechteckliste. status entweder SEL | UNSEL |		*/
/* ENABLED | DISABLED												*/
/* Redraw = 1->gleich neu zeichnen.									*/
/* **************************************************************** */
void change_object(WINDOW *window, int objct, int status, int redraw)
{
	GRECT gred;
	OBJECT *tree;


	tree=window->resource_form;
	
	objc_offset(tree, objct, &gred.g_x, &gred.g_y);
	
	gred.g_x-=3; 
	gred.g_y-=3;
	gred.g_w=tree[objct].ob_width+6; 
	gred.g_h=tree[objct].ob_height+6; 
	
	if(status == SEL)
		tree[objct].ob_state |= SELECTED;
	else
	if(status == UNSEL)
		tree[objct].ob_state &= ~SELECTED;
	else
	if(status == DISABLED)
		tree[objct].ob_state |= DISABLED;
	else
	if(status == ENABLED)
		tree[objct].ob_state &= ~DISABLED;
	
	if(redraw != 0)
		Window.redraw(window, &gred, objct, DRAWNOPICTURE);

	return;
}



/* ------------------------------------------------------------------------	*/
/*																			*/
/*					Objekt solide mit der Maus ziehen						*/
/*																			*/
/*																			*/
/*	*wind:		Window, in dem sich das zu draggende Objekt befindet		*/
/*																			*/
/*	objct:		Index des zu draggenden Objektes							*/
/*																			*/
/*	*dex, *dey:	End-Mausposition nach dem Loslassen der Maustaste			*/
/*																			*/
/*	call:		Flag, ob funktion *call_me	aufgerufen werden soll			*/
/*																			*/
/*	(*call_me):	Ptr auf Funktion, die wÑhrend des Draggens periodisch auf-	*/
/*				gerufen werden soll, wenn call=1 ist - so wurde z.B. 		*/
/*				das automatische Mitscrollen des Bildmanagers erledigt.		*/
/*																			*/
/* ------------------------------------------------------------------------	*/
void f_drag_object ( WINDOW *wind, int objct, int *dex, int *dey, 
						int call, int (*call_me)(int mx, int my) )
{
int obx, oby, obw, obh;
OBJECT *tree;
int dummy, mousex, mousey, mbutton, keystate;
int nmx, nmy, m_xoff, m_yoff;
int mbuff[16];
MFDB screenm, buffer;
int savepxy[8];
int deletepxy[8];
int cliparray[8];
int obj_xpos, obj_ypos;
int omx, omy, redraw;

	/* Mausposition speichern */
	obx=mousex=mouse_xpos;
	oby=mousey=mouse_ypos;


	/* Objektmaûe holen */
	tree=wind->resource_form;
	objc_offset(tree, objct, &obx, &oby);
	m_xoff=(mousex-obx);
	m_yoff=(mousey-oby);
	obw=u_tree[DRAGTXT].ob_width;
	obh=u_tree[DRAGTXT].ob_height;

	cliparray[0]=screen.g_x;
	cliparray[1]=screen.g_y;
	cliparray[2]=screen.g_w;
	cliparray[3]=screen.g_h;
	vs_clip(Sys_info.vdi_handle, 1, cliparray);

	screenm.fd_addr=NULL;

	buffer.fd_addr=SMalloc(((long)(obw+16)*(long)obh*(long)Sys_info.bitplanes)/8L);
	buffer.fd_w=obw;
	buffer.fd_h=obh;
	buffer.fd_wdwidth=(obw+15)/16;
	buffer.fd_stand=0;
	buffer.fd_nplanes=Sys_info.bitplanes;

	wind_update(BEG_MCTRL);
	wind_update(BEG_UPDATE);

		obj_xpos=mousex-m_xoff;
		if(obj_xpos<0) obj_xpos=0;
		else if(obj_xpos+obw>screen.g_w) obj_xpos=screen.g_w-obw;
		obj_ypos=mousey-m_yoff;
		if(obj_ypos<0) obj_ypos=0;
		else if(obj_ypos+obh>screen.g_h) obj_ypos=screen.g_h-obh;
		
		savepxy[0]=obj_xpos;
		savepxy[1]=obj_ypos;
		savepxy[2]=obj_xpos+obw;
		savepxy[3]=obj_ypos+obh;
		savepxy[4]=0;
		savepxy[5]=0;
		savepxy[6]=obw;
		savepxy[7]=obh;


	/* dafÅr sorgen, daû die Mauspos-Vars am Anfang auf jeden Fall unterschiedlich sind */
	omx = omy = 0;
	if(mousex ==0)
		omx = 1;

	do
	{
		if(call == 1 && call_me != NULL)
			redraw = call_me(mousex,mousey);			/* Åbergebene Funktion aufrufen */

	/* Wenn kein Timerevent, Hintergrund retten und Objekt neu zeichnen */
		if(omx != mousex || omy != mousey || redraw)
		{
			graf_mouse(M_OFF, dummy_ptr);
			vro_cpyfm(Sys_info.vdi_handle, S_ONLY, savepxy, &screenm, &buffer);						/* Hintergrund retten */

			u_tree[DRAGTXT].ob_x = obj_xpos;
			u_tree[DRAGTXT].ob_y = obj_ypos;	
			objc_draw(u_tree, DRAGTXT, MAX_DEPTH, screen.g_x, screen.g_y, screen.g_w, screen.g_h);	/* Objekt neu zeichnen	*/
			graf_mouse(M_ON, dummy_ptr);

			obx = obj_xpos;
			oby = obj_ypos;
		}

		omx = mousex;
		omy = mousey;

		evnt_multi(MU_TIMER|MU_M1, 1,3,0, 1,mousex,mousey,1,1, 0,0,0,0,0, 
					mbuff, 50,0, &nmx, &nmy, &mbutton, &keystate, &dummy, &dummy);
					
		graf_mkstate(&nmx, &nmy, &mbutton, &keystate);

		mousex=nmx;
		mousey=nmy;


	/* Wenn kein Timerevent, Hintergrund restoren	*/
		if(omx != mousex || omy != mousey || mbutton == 0 || redraw)
		{
			deletepxy[0] = 0;
			deletepxy[1] = 0;
			deletepxy[2] = obw;
			deletepxy[3] = obh;
			deletepxy[4] = obx;
			deletepxy[5] = oby;
			deletepxy[6] = obw + obx;
			deletepxy[7] = obh + oby;
			
			obj_xpos = mousex - m_xoff;
			if(obj_xpos < 0)
				obj_xpos = 0;
			else
				if(obj_xpos + obw > screen.g_w)
					obj_xpos = screen.g_w-obw;

			obj_ypos = mousey - m_yoff;
			if(obj_ypos < 0)
				obj_ypos = 0;
			else
				if(obj_ypos + obh > screen.g_h)
					obj_ypos = screen.g_h - obh;
			
			savepxy[0] = obj_xpos;
			savepxy[1] = obj_ypos;
			savepxy[2] = obj_xpos+obw;
			savepxy[3] = obj_ypos+obh;
			savepxy[4] = 0;
			savepxy[5] = 0;
			savepxy[6] = obw;
			savepxy[7] = obh;
			
			graf_mouse(M_OFF, dummy_ptr);
			vro_cpyfm(Sys_info.vdi_handle, S_ONLY, deletepxy, &buffer, &screenm);		/* Hintergrund restoren */
			graf_mouse(M_ON, dummy_ptr);
		}


	} while(mbutton != 0);

	wind_update(END_MCTRL);
	wind_update(END_UPDATE);

	vs_clip(Sys_info.vdi_handle, 0, cliparray);

	*dex = mousex;
	*dey = mousey;

	SMfree(buffer.fd_addr);

	return;
}




int appl_xgetinfo (type, out1, out2, out3, out4)
int type;
int *out1, *out2;
int *out3, *out4;
{
	char has_agi = FALSE;

	unsigned long dummy;


	has_agi = ((get_cookie('MagX', &dummy) || get_cookie('MgMc', &dummy) || get_cookie('MgPC', &dummy)) ||
			   (_GemParBlk.global[0] == 0x400 && type < 4) || (_GemParBlk.global[0] > 0x400));

	if(has_agi)
		return(appl_getinfo (type, out1, out2, out3, out4));
	else
		return(0);
} /* appl_xgetinfo */