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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdi.h>
#include <tos.h>
#include <ext.h>
#include <math.h>
#include <screen.h>
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\smurf\smurf_st.h"
#include "..\..\..\smurf\globdefs.h"
#include "..\..\..\smurf\smurfine.h"
#include "..\..\..\smurf\plugin\plugin.h"
#include "..\..\..\smurf\smurf.h"
#include "gdos.h"
#include "print.h"

#include "..\..\..\smurf\smurfobs.h"

#define ENGLISCH 0

#if ENGLISCH
    #include "englisch.rsc\print.rsh"
    #include "englisch.rsc\print.rh"
#else
    #include "deutsch.rsc\print.rsh"
    #include "deutsch.rsc\print.rh"
#endif

void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);

void init_rsh(void);
void init_driver_popup(void);
void init_windstruct(void);
void actualize_dialog(int start_object);
DevInfoS *get_devinfo(int ID);
void handle_print_dialog(PLUGIN_DATA *data);
int handle_aesmsg(int *msgbuf);
void drag_posbox(int mousex, int mousey, int object);
int SM_wind_set(int wi_ghandle, int wi_gfield, int wi_gw1,
                int wi_gw2, int wi_gw3, int wi_gw4);
float convert_units_from(int unit, int mode);
int convert_units_to(int unit, int mode);
void insert_float(OBJECT *tree, int object, float val);
char clip_values(void);
void actualize_xywh(void);
char *shorten_name(char *string, char newlen);


extern  int actualize_DevParam(int gdos_dev, DevParamS *DevParam);
extern  int scan_devs(void);
extern  int set_DevParam(int gdos_dev, DevParamS *DevParam);
extern  int print_with_GDOS(void);
extern  int get_DevInfo(char devID, DevInfoS *DevInfo);

char name_string[]="GDOS Print-Plugin";

extern  DevInfoS DevInfo[20];
extern  DevParamS DevParam;
extern  OutParamS OutParam;
extern  char dev_anzahl;

PLUGIN_INFO plugin_info =
{
    name_string,
    0x0102,             /* Plugin-Version 1.02 */
    0x0106,             /* fÅr Smurf-Version 1.06 */
    0                   /* und nicht resident. */
};

SERVICE_FUNCTIONS *services;

int my_id, dialog_open;
int popup_initialized;
int unit = UNIT_PIXELS;

POP_UP driver_popup, paper_popup, unit_popup;
WINDOW print_window;
OBJECT *print_dialog, *alerts;
OBJECT *drivpop_rsc, *pappop_rsc, *unitpop_rsc;
SMURF_PIC *pic_active;
int Button2DevID[30];

SMURF_PIC * *smurf_picture;
int *active_pic;

int curr_deviceID = 91;
float scale, temp;

PLUGIN_FUNCTIONS *smurf_functions;
SMURF_VARIABLES *smurf_vars;


void plugin_main(PLUGIN_DATA *data)
{
    char *picname;

    int back, i;

    DevInfoS *devinfo;


    /* Die Strukturen mit den Variablen und Funktionen holen */
    services = data->services;
    smurf_functions = data->smurf_functions;
    smurf_vars = data->smurf_vars;

    redraw_window = services->redraw_window;                        /* Redrawfunktion */

    if(data->message==AES_MESSAGE)
    {
        data->message = handle_aesmsg(data->event_par);
        return;
    }

    switch(data->message)
    {
        /* 
         * Startup des Plugins: MenÅeintrag anfordern 
         */
        case MSTART:    my_id = data->id;
#if ENGLISCH
                        strcpy(data->plugin_name, "Print...");
#else
                        strcpy(data->plugin_name, "Drucken...");
#endif
                        init_rsh();
                        init_windstruct();
                        data->wind_struct = &print_window;
                        data->event_par[0] = FILE_PRINT;
                        data->message = MENU_ENTRY;     /* MenÅeintrag anfordern */
                        break;

        /*
         * Plugin wurde aus dem MenÅ heraus aufgerufen 
         */
        case PLGSELECTED:   if( *(smurf_vars->picthere) == 0 )
                            {
/*                              services->f_alert("Kein Bild geladen!", NULL, NULL, NULL, 1); */
                                services->f_alert(alerts[NO_PIC].TextCast, NULL, NULL, NULL, 1);
                                data->message = M_EXIT;
                            }
                            else
                            {
                                smurf_picture = smurf_vars->smurf_picture;
                                active_pic = smurf_vars->active_pic;
                    
                                pic_active = smurf_picture[*active_pic];
                                
                                OutParam.zx = 0;
                                OutParam.zy = 0;
                                OutParam.picwidth = pic_active->pic_width;
                                OutParam.picheight = pic_active->pic_height;
                                
                                strcpy(print_dialog[XPOS].TextCast, "0");
                                strcpy(print_dialog[YPOS].TextCast, "0");
                                itoa(OutParam.picwidth, print_dialog[WIDTH].TextCast, 10);
                                itoa(OutParam.picheight, print_dialog[HEIGHT].TextCast, 10);

                                /*
                                 * Devices scannen und Infos Åber aktuelles GerÑt holen 
                                 */
                                if(scan_devs() == -1)
                                {
                                    data->message = M_EXIT;
                                    return;
                                }

                                for(i = 0; i < dev_anzahl; i++)
                                {
                                    if(DevInfo[i].devID == 91)
                                    {
                                        curr_deviceID = 91;
                                        break;
                                    }
                                }

                                if(curr_deviceID != 91)
                                    curr_deviceID = DevInfo[0].devID;

                                if(actualize_DevParam(curr_deviceID, &DevParam) != -1)
                                    actualize_dialog(PAPER_REDRAW);

                                /*
                                 *  Fenster îffnen und Dialog initialisieren
                                 */
                                pic_active = smurf_picture[*active_pic];
                                if((picname = strrchr(pic_active->filename, '\\')) != NULL/* && *(picname + 1) != '\0'*/)
                                    picname++;
                                else
                                    picname = pic_active->filename;
#if ENGLISCH
                                strcpy(print_window.wtitle, "Print: \"");
#else
                                strcpy(print_window.wtitle, "Drucken: \"");
#endif
                                strcat(print_window.wtitle, shorten_name(picname, 41 - (char)strlen(print_window.wtitle)));
/*                              strcat(print_window.wtitle, picname); */
/*                              strcat(print_window.wtitle, "\""); */
                                SM_wind_set(print_window.whandlem, WF_NAME, LONG2_2INT((long)print_window.wtitle), 0,0);

                                if(services->f_module_window(&print_window) < 0)
                                {
                                    data->message = M_EXIT;
                                    break;
                                }
                                dialog_open = 1;
                                
                                if(popup_initialized==0)
                                    init_driver_popup();
                                
                            /*printf("\nDriver Preselection");*/
                                devinfo = get_devinfo(curr_deviceID);
                                if(devinfo!=NULL)
                                    strcpy(print_dialog[DRIVER_POPBUT].ob_spec.tedinfo->te_ptext, devinfo->devName);

                            /*printf("\nPaper Preselection");*/
                                /* Papierformat ermitteln */
                                back = DevParam.paperform;
                            /*printf("\nvoreingestelltes Papierformat: %i", back);*/
                                if(back>=PAGE_DEFAULT && back<=PAGE_B5)
                                    paper_popup.item = back+PAPER1;
                                else
                                    paper_popup.item = back+PAPER6-16;
                                strcpy(print_dialog[PAPER_POPBUT].TextCast, pappop_rsc[paper_popup.item].TextCast);

                                strcpy(print_dialog[UNIT_POPBUT].TextCast, unitpop_rsc[UNIT_PIXELS].TextCast);

                                data->message = M_WAITING;
                            }
                            break;

        /*
         * Fenster wurde geschlossen
         */
        case MWINDCLOSED:   dialog_open=0;
                            data->message = M_WAITING;
                            break;
                            
        /*
         * aktives Bild hat sich geÑndert
         */
        case MPIC_UPDATE:   if(dialog_open)
                            {
                                pic_active = smurf_picture[data->event_par[0]];
                                actualize_dialog(POSITION_PAPER);
                                if((picname = strchr(pic_active->filename, '\\')) != NULL && *(picname + 1) != '\0')
                                    picname++;
                                else
                                    picname = pic_active->filename;
                                strcpy(print_window.wtitle, "Drucken: \"");
                                strcat(print_window.wtitle, picname);
                                strcat(print_window.wtitle, "\"");
                                SM_wind_set(print_window.whandlem, WF_NAME, LONG2_2INT((long)print_window.wtitle), 0,0);
                                data->message = M_WAITING;
                            }
                            break;
                            
        /*
         * Buttonevent
         */                 
        case MKEVT:
        case MBEVT:         handle_print_dialog(data);
                            data->message = M_WAITING;
                            break;

        case MTERM:         data->message = M_TERMINATED;
                            break;

        default:            data->message = M_WAITING;
                            break;
    }

    return;
}


void init_windstruct(void)
{

    print_window.whandlem = 0;                      /* evtl. Handle lîschen */
    print_window.module = my_id;
    print_window.wnum = 1;                          /* Das wievielte Fenster des Moduls? */

    print_window.wx = -1;
    print_window.wy = -1;
    print_window.ww = print_dialog->ob_width;
    print_window.wh = print_dialog->ob_height;

    strcpy(print_window.wtitle, "Bild drucken");    /* Fenstertitel */

    print_window.resource_form = print_dialog;      /* Modulresource-Formular */
    print_window.picture = NULL;                    /* Zeigerfeld fÅr Bild/Animation */
    print_window.editob = XPOS;
    print_window.nextedit = YPOS;
    print_window.editx = 0;
    print_window.pflag = 0;

    print_window.prev_window = NULL;    /* vorheriges Fenster (WINDOW*) */
    print_window.next_window = NULL;    /* nÑxtes Fenster (WINDOW*) */
}


/* init_rsh -----------------------------------------------------
    Initialisiert die Resource 
    -----------------------------------------------------------*/
void init_rsh(void)
{
    int t;


    /* Resource Umbauen */
    for(t=0; t<NUM_OBS; t++)
        rsrc_obfix(&rs_object[t], 0);

    print_dialog = rs_trindex[PRINT_MAIN];      /* Resourcebaum holen */
    drivpop_rsc = rs_trindex[DRIVERS];
    pappop_rsc = rs_trindex[PAPERS];
    unitpop_rsc = rs_trindex[UNITS];
    alerts = rs_trindex[ALERT_STRINGS];

    return;
}


/* init_driver_popup ----------------------------------------------
    Bereitet das driver-Popup vor 
    -----------------------------------------------------------*/
void init_driver_popup(void)
{
    int t,tt;
    int curr_obj;

    /*
     * Driver-Popup vorbereiten (POP_UP)
     */
    driver_popup.popup_tree = DRIVERS;                  /* Objektbaum-Index des Popups */
    driver_popup.item = 0;                              /* Item (=letzter Eintrag)      */
    driver_popup.display_tree = print_dialog;           /* Tree, in dems dargestellt werden soll */
    driver_popup.display_obj = DRIVER_POPBUT;           /* Objekt in display_tree, bei dems dargestellt werden soll */
    driver_popup.Cyclebutton = DRIVER_CB;               /* Index des Cyclebuttons */

    curr_obj = DRIVER1;
    
    for(t=0; t<dev_anzahl; t++)
    {
        strcpy(drivpop_rsc[curr_obj].ob_spec.tedinfo->te_ptext, DevInfo[t].devName);
        drivpop_rsc[curr_obj].ob_y = 3+t*20;
        curr_obj++;
    }

    for(tt=t; tt<30; tt++)
    {
        objc_delete(drivpop_rsc, curr_obj);
        curr_obj++;
    }
    
    drivpop_rsc[0].ob_height = dev_anzahl*20+3;
    
    for(t=0; t<dev_anzahl; t++)
    {
        if(DevInfo[t].devID == curr_deviceID)
            break;
    }

    driver_popup.item = t + DRIVER1;

    /*
     * Papier-Popup vorbereiten (POP_UP)
     */
    paper_popup.popup_tree = PAPERS;                    /* Objektbaum-Index des Popups */
    paper_popup.item = 1;                               /* Item (=letzter Eintrag)      */
    paper_popup.display_tree = print_dialog;            /* Tree, in dems dargestellt werden soll */
    paper_popup.display_obj = PAPER_POPBUT;             /* Objekt in display_tree, bei dems dargestellt werden soll */
    paper_popup.Cyclebutton = PAPER_CB;                 /* Index des Cyclebuttons */

    /*
     * Einheiten-Popup vorbereiten (POP_UP)
     */
    unit_popup.popup_tree = UNITS;                      /* Objektbaum-Index des Popups */
    unit_popup.item = 1;                                /* Item (=letzter Eintrag)      */
    unit_popup.display_tree = print_dialog;             /* Tree, in dems dargestellt werden soll */
    unit_popup.display_obj = UNIT_POPBUT;               /* Objekt in display_tree, bei dems dargestellt werden soll */
    unit_popup.Cyclebutton = UNIT_CB;                   /* Index des Cyclebuttons */

    popup_initialized=1;
}


/* actualize_dialog ----------------------------------------------
    Aktualisiert den Druckdialog (Previews)
    ------------------------------------------------------------*/
void actualize_dialog(int start_object)
{
    int pagewidth, pageheight;                          /* Page in Realmaûen (Pixel) */
    int val;


    pagewidth = DevParam.prtwidth + DevParam.leftBorder + DevParam.rightBorder;
    pageheight = DevParam.prtheight + DevParam.upperBorder + DevParam.lowerBorder;

    if(OutParam.zx + OutParam.picwidth > DevParam.leftBorder + DevParam.prtwidth)
    {
        OutParam.picwidth = DevParam.prtwidth - OutParam.zx;

        val = convert_units_from(unit, WIDTH);
        itoa(val, print_dialog[WIDTH].TextCast, 10);
        redraw_window(&print_window, NULL, WIDTH, 0);
    }

    if(OutParam.zy + OutParam.picheight > DevParam.upperBorder + DevParam.prtheight)
    {
        OutParam.picheight = DevParam.prtheight - OutParam.zy;

        val = convert_units_from(unit, HEIGHT);
        itoa(val, print_dialog[HEIGHT].TextCast, 10);
        redraw_window(&print_window, NULL, HEIGHT, 0);
    }

    scale = (float)pagewidth/(float)(print_dialog[PAPER_REDRAW].ob_width - 6);
    temp = (float)pageheight/(float)(print_dialog[PAPER_REDRAW].ob_height - 6);
    if(temp > scale)
        scale = temp;
    scale += 0.001;                                     /* zur Sicherheit etwas grîûer machen */
    print_dialog[POSITION_PAPER].ob_width = pagewidth / scale + 0.5;
    print_dialog[POSITION_PAPER].ob_height = pageheight / scale + 0.5;

    print_dialog[POSITION_FRAME].ob_x = DevParam.leftBorder / scale + 0.5;
    print_dialog[POSITION_FRAME].ob_y = DevParam.upperBorder / scale + 0.5;
    print_dialog[POSITION_FRAME].ob_width = DevParam.prtwidth / scale + 0.5;
    print_dialog[POSITION_FRAME].ob_height = DevParam.prtheight / scale + 0.5;

    print_dialog[POSITION_BOX].ob_x = OutParam.zx / scale + 0.5;
    print_dialog[POSITION_BOX].ob_y = OutParam.zy / scale + 0.5;
    print_dialog[POSITION_BOX].ob_width = OutParam.picwidth / scale + 0.5;
    print_dialog[POSITION_BOX].ob_height = OutParam.picheight / scale + 0.5;

    print_dialog[SIZE_BOX].ob_x = /*print_dialog[POSITION_BOX].ob_x + */print_dialog[POSITION_BOX].ob_width - print_dialog[SIZE_BOX].ob_width;
    print_dialog[SIZE_BOX].ob_y = /*print_dialog[POSITION_BOX].ob_y + */print_dialog[POSITION_BOX].ob_height - print_dialog[SIZE_BOX].ob_height;

    if(print_dialog[SIZE_BOX].ob_x < 0)
        print_dialog[SIZE_BOX].ob_x = 0;
    if(print_dialog[SIZE_BOX].ob_y < 0)
        print_dialog[SIZE_BOX].ob_y = 0;

    if(print_dialog[POSITION_BOX].ob_x < print_dialog[POSITION_FRAME].ob_x)
        print_dialog[POSITION_BOX].ob_x = print_dialog[POSITION_FRAME].ob_x;
    if(print_dialog[POSITION_BOX].ob_y < print_dialog[POSITION_FRAME].ob_y)
        print_dialog[POSITION_BOX].ob_y = print_dialog[POSITION_FRAME].ob_y;

    redraw_window(&print_window, NULL, start_object, 0);

    return;
}



/* get_devinfo ------------------------------------------------
    Sucht aus den bis zu 30 DevInfoS-Strukturen die zu ID passende raus.
    ----------------------------------------------------------*/
DevInfoS *get_devinfo(int ID)
{
    int t;
    
    for(t=0; t<30; t++)
    {
        if(DevInfo[t].devID == ID) 
            return(&DevInfo[t]);
    }

    return(NULL);
}



/* handle_print_dialog ----------------------------------------
    öbernimmt die Userbedienung des Print-Dialogs
    ----------------------------------------------------------*/
void handle_print_dialog(PLUGIN_DATA *data)
{
    char new_paperform;

    int button, back;

    float val;


/*  Goto_pos(1, 0); */
    
    button = data->event_par[0];

    if(data->message == MBEVT)
    {
        switch(button)
        {
            case DRIVER_POPBUT:
            case DRIVER_CB:     back = services->popup(&driver_popup, 0, button, drivpop_rsc);
                                if(back != -1)
                                {
                                    curr_deviceID = DevInfo[back - DRIVER1].devID;

/*                                  printf("Drucker gewechselt\n"); */
                                    back = actualize_DevParam(curr_deviceID, &DevParam);
/*                                  printf("Druckerinfo aktualisiert\n"); */
                                    if(back == -1)
/*                                      services->f_alert("Drucker konnte nicht initialisiert werden!", NULL, NULL, NULL, 1); */
                                        services->f_alert(alerts[NO_INIT].TextCast, NULL, NULL, NULL, 1);
                                    else
                                    {
                                        clip_values();

                                        actualize_dialog(PAPER_REDRAW);

                                        back = DevParam.paperform;
                                        if(back >= PAGE_DEFAULT && back <= PAGE_B5)
                                            paper_popup.item = back + PAPER1;
                                        else
                                            paper_popup.item = back + PAPER6 - 16;

                                        strcpy(print_dialog[PAPER_POPBUT].TextCast, pappop_rsc[paper_popup.item].TextCast);
                                        services->deselect_popup(&print_window, PAPER_POPBUT, PAPER_CB);
                                    }

                                    if(DevParam.can_scale == 0)
                                    {
                                        print_window.editob = XPOS;
                                        print_dialog[WIDTH_UP].ob_state |= DISABLED;
                                        print_dialog[WIDTH_DOWN].ob_state |= DISABLED;
                                        print_dialog[WIDTH].ob_state |= DISABLED;
                                        print_dialog[HEIGHT_UP].ob_state |= DISABLED;
                                        print_dialog[HEIGHT_DOWN].ob_state |= DISABLED;
                                        print_dialog[HEIGHT].ob_state |= DISABLED;
                                    }
                                    else
                                    {
                                        print_dialog[WIDTH_UP].ob_state &= ~DISABLED;
                                        print_dialog[WIDTH_DOWN].ob_state &= ~DISABLED;
                                        print_dialog[WIDTH].ob_state &= ~DISABLED;
                                        print_dialog[HEIGHT_UP].ob_state &= ~DISABLED;
                                        print_dialog[HEIGHT_DOWN].ob_state &= ~DISABLED;
                                        print_dialog[HEIGHT].ob_state &= ~DISABLED;
                                    }

                                    redraw_window(&print_window, NULL, ADJUST_PARENT,0);
                                }

                                services->deselect_popup(&print_window, DRIVER_POPBUT, DRIVER_CB);
                                actualize_xywh();
                                break;

            case PAPER_POPBUT:
            case PAPER_CB:      back = services->popup(&paper_popup, 0, button, pappop_rsc);
                                if(back != -1)
                                {
                                    if(back >= PAPER1 && back <= PAPER5)
                                        new_paperform = back - PAPER1;
                                    else
                                        new_paperform = back - PAPER6 + 16;
                                    DevParam.paperform = new_paperform;

                                    back = actualize_DevParam(curr_deviceID, &DevParam);
                                    if(back == -1)
/*                                      services->f_alert("Drucker konnte nicht initialisiert werden!", NULL, NULL, NULL, 1); */
                                        services->f_alert(alerts[NO_INIT].TextCast, NULL, NULL, NULL, 1);
                                    else
                                    { 
                                        clip_values();

                                        actualize_dialog(PAPER_REDRAW);
                                    }

                                    if(DevParam.paperform != new_paperform)
                                    {
/*                                      services->f_alert("Dieses Papierformat steht nicht zur VerfÅgung!", NULL, NULL, NULL, 1); */
                                        services->f_alert(alerts[PAPERFORMAT].TextCast, NULL, NULL, NULL, 1);
                                        back = DevParam.paperform;
                                        if(back >= PAGE_DEFAULT && back <= PAGE_B5)
                                            paper_popup.item = back + PAPER1;
                                        else
                                            paper_popup.item = back + PAPER6 - 16;

                                        strcpy(print_dialog[PAPER_POPBUT].TextCast, pappop_rsc[paper_popup.item].TextCast);
                                    }
                                }

                                services->deselect_popup(&print_window, PAPER_POPBUT, PAPER_CB);
                                actualize_xywh();
                                break;

            case PRINT_START:   if(DevParam.depth == 1 && smurf_picture[*active_pic]->depth > 1)
/*                                  services->f_alert("Ausgabe von Farbbildern auf s/w-Drucker ist noch nicht mîglich!", NULL, NULL, NULL, 1); */
                                    services->f_alert(alerts[NO_COLOUR].TextCast, NULL, NULL, NULL, 1);
                                else
                                    if(DevParam.depth == 3)
/*                                      services->f_alert("Ausgabe in 8 Farben ist leider noch nicht mîglich!", NULL, NULL, NULL, 1); */
                                        services->f_alert(alerts[NO_EIGHT].TextCast, NULL, NULL, NULL, 1);
                                    else
                                    {
                                        OutParam.advance = 1;                                       /* Vorschub nach Seite? */
                                        print_with_GDOS();
                                    }
                                break;

            case X_UP:
            case X_DOWN:        val = atof(print_dialog[XPOS].TextCast);
                                if(button == X_UP)
                                    val++;
                                else
                                    val--;
                                insert_float(print_dialog, XPOS, val);
                                break;

            case Y_UP:
            case Y_DOWN:        val = atof(print_dialog[YPOS].TextCast);
                                if(button == Y_UP)
                                    val++;
                                else
                                    val--;
                                insert_float(print_dialog, YPOS, val);
                                break;

            case WIDTH_UP:
            case WIDTH_DOWN:    val = atof(print_dialog[WIDTH].TextCast);
                                if(button == WIDTH_UP)
                                    val++;
                                else
                                    val--;
                                insert_float(print_dialog, WIDTH, val);
                                break;

            case HEIGHT_UP:
            case HEIGHT_DOWN:   val = atof(print_dialog[HEIGHT].TextCast);
                                if(button == HEIGHT_UP)
                                    val++;
                                else
                                    val--;
                                insert_float(print_dialog, HEIGHT, val);
                                break;

            case SIZE_BOX:
            case POSITION_BOX:  drag_posbox(data->mousex, data->mousey, button);
                                break;

            case UNIT_POPBUT:
            case UNIT_CB:       back = services->popup(&unit_popup, 0, button, unitpop_rsc);
                                if(back != -1)
                                {
                                    unit = back;

                                    strcpy(print_dialog[UNIT_POPBUT].TextCast, unitpop_rsc[unit_popup.item].TextCast);

                                    actualize_xywh();
                                }

                                services->deselect_popup(&print_window, UNIT_POPBUT, UNIT_CB);
                                break;
        }
    }

    if(data->message==MKEVT)
    {
        if(button==PRINT_START)
        {
            if(DevParam.depth==3)
/*              services->f_alert("Ausgabe in 8 Farben ist leider noch nicht mîglich!", NULL, NULL, NULL, 1); */
                services->f_alert(alerts[NO_EIGHT].TextCast, NULL, NULL, NULL, 1);
            else
            {
                OutParam.advance = 1;                           /* Vorschub nach Seite? */
                print_with_GDOS();
            }
        }
    }

    /* Breite leer? */
    if(*(print_dialog[WIDTH].TextCast) == '\0' && button != WIDTH)
    {
        val = (float)pic_active->pic_width * (float)OutParam.picheight / (float)pic_active->pic_height;
        OutParam.picwidth = val;
        val  = convert_units_from(unit, WIDTH);
        insert_float(print_dialog, WIDTH, val);
    }       

    /* Hîhe leer? */
    if(*(print_dialog[HEIGHT].TextCast) == '\0' && button != HEIGHT)
    {
        val = (float)pic_active->pic_height * (float)OutParam.picwidth / (float)pic_active->pic_width;
        OutParam.picheight = val;
        val  = convert_units_from(unit, HEIGHT);
        insert_float(print_dialog, HEIGHT, val);
    }

    /*
     * Position geÑndert?
     */
    if(button == X_UP || button == X_DOWN || button == XPOS)
    {
        button = XPOS;
        OutParam.zx = convert_units_to(unit, XPOS);
    }
    else
    if(button == Y_UP || button == Y_DOWN || button == YPOS)
    {
        button = YPOS;
        OutParam.zy = convert_units_to(unit, YPOS);
    }
    else
    if(button == WIDTH_UP || button == WIDTH_DOWN || button == WIDTH)
    {
        button = WIDTH;
        OutParam.picwidth = convert_units_to(unit, WIDTH);
    }
    else
    if(button == HEIGHT_UP || button == HEIGHT_DOWN || button == HEIGHT)
    {
        button = HEIGHT;
        OutParam.picheight = convert_units_to(unit, HEIGHT);
    }

    if(clip_values())
    {
        val  = convert_units_from(unit, button);
        insert_float(print_dialog, button, val);
        redraw_window(&print_window, NULL, button, 0);
    }

    actualize_dialog(POSITION_PAPER);

    return;
}

/* handle_aesmsg ----------------------------------------------
    kÅmmert sich um von Smurf weitergeleitete AES-Messages
    ----------------------------------------------------------*/
int handle_aesmsg(int *msgbuf)
{
    int driver_id;
    DevInfoS *info;
    int t;

    if(msgbuf[0]==82)       /* Treiber wurde geÑndert! */
    {
        driver_id = msgbuf[3];
        
        info = get_devinfo(driver_id);
        if(info!=NULL)
        {
            get_DevInfo(driver_id, info);

            for(t=0; t<dev_anzahl; t++) 
                if(DevInfo[t].devID==driver_id) break;
                
            strcpy(drivpop_rsc[t+DRIVER1].TextCast, DevInfo[t].devName);

            if(driver_popup.item == t+DRIVER1)
                strcpy(print_dialog[DRIVER_POPBUT].TextCast, DevInfo[t].devName);
            
            services->deselect_popup(&print_window, DRIVER_POPBUT, DRIVER_CB);
        }

        if(driver_id == curr_deviceID)
            actualize_DevParam(curr_deviceID, &DevParam);

        actualize_dialog(PAPER_REDRAW);
    }

    return(M_WAITING);
}



void drag_posbox(int mousex, int mousey, int object)
{
    int inx,iny,inw,inh;
    int outx,outy,outw,outh;
    int endx,endy, papx, papy, button, key, oldx,oldy;
    float val, aspect;

    aspect = (float)pic_active->pic_width/(float)pic_active->pic_height;

    objc_offset(print_dialog, POSITION_PAPER, &papx, &papy);

    objc_offset(print_dialog, POSITION_BOX, &inx, &iny);
    inw = print_dialog[POSITION_BOX].ob_width;
    inh = print_dialog[POSITION_BOX].ob_height;

    objc_offset(print_dialog, POSITION_FRAME, &outx, &outy);
    outw = print_dialog[POSITION_FRAME].ob_width;
    outh = print_dialog[POSITION_FRAME].ob_height;

    oldx = mousex;
    oldy = mousey;

    /*
     * Verkleinern/Vergrîûern?
     */
    if(object==SIZE_BOX)
    {
        do
        {
            graf_mkstate(&endx, &endy, &button, &key);
            
            if(oldx==endx && oldy==endy)
                continue;
            
            oldx = endx;
            oldy = endy;
            
            if(endx<outx+outw && endx>inx)
            {
                print_dialog[POSITION_BOX].ob_width = endx-inx;
                
                OutParam.picwidth = (endx - inx) * scale;

                val = convert_units_from(unit, WIDTH);
                insert_float(print_dialog, WIDTH, val);
            }

            if(endy<outy+outh && endy>iny)
            {
                print_dialog[POSITION_BOX].ob_height = endy-iny;

                if(key==KEY_CTRL || key&KEY_SHIFT)
                    OutParam.picheight = OutParam.picwidth/aspect;
                else
                    OutParam.picheight = (endy - iny) * scale;

                val = convert_units_from(unit, HEIGHT);
                insert_float(print_dialog, HEIGHT, val);
            }

            print_dialog[SIZE_BOX].ob_x = print_dialog[POSITION_BOX].ob_width - print_dialog[SIZE_BOX].ob_width;
            print_dialog[SIZE_BOX].ob_y = print_dialog[POSITION_BOX].ob_height - print_dialog[SIZE_BOX].ob_height;
        
            actualize_dialog(POSITION_PAPER);
        } while(button!=0);
    }

    /*
     * Draggen?
     */
    else
    {   
        graf_dragbox(inw,inh,inx,iny, outx,outy,outw,outh, &endx,&endy);

        print_dialog[POSITION_BOX].ob_x = endx-papx;
        print_dialog[POSITION_BOX].ob_y = endy-papy;
        print_dialog[SIZE_BOX].ob_x = (endx-papx)+print_dialog[POSITION_BOX].ob_width-print_dialog[SIZE_BOX].ob_width;
        print_dialog[SIZE_BOX].ob_y = (endx-papx)+print_dialog[POSITION_BOX].ob_height-print_dialog[SIZE_BOX].ob_height;

        OutParam.zx = (endx - papx) * scale + 0.5;
        if(OutParam.zx + OutParam.picwidth > DevParam.leftBorder + DevParam.prtwidth)
            OutParam.zx = DevParam.leftBorder + DevParam.prtwidth - OutParam.picwidth;
        else
            if(print_dialog[POSITION_BOX].ob_x <= print_dialog[POSITION_FRAME].ob_x)
                OutParam.zx = DevParam.leftBorder;

        OutParam.zy = (endy - papy) * scale + 0.5;
        if(OutParam.zy + OutParam.picheight > DevParam.upperBorder + DevParam.prtheight)
            OutParam.zy = DevParam.upperBorder + DevParam.prtheight - OutParam.picheight;
        else
            if(print_dialog[POSITION_BOX].ob_y <= print_dialog[POSITION_FRAME].ob_y)
                OutParam.zy = DevParam.upperBorder;

        val  = convert_units_from(unit, XPOS);
        insert_float(print_dialog, XPOS, val);
        val  = convert_units_from(unit, YPOS);
        insert_float(print_dialog, YPOS, val);

        actualize_dialog(POSITION_PAPER);
    }

    return;
}


/*----------------------------------------------------------------- */
/* convert_units_from                                               */
/*  Konvertiert Pixel in Pixel, Millimeter, Zoll und Prozent        */
/*  RÅckgabewert ist ein Flieûkommawert zur Ausgabe.                */
/*----------------------------------------------------------------- */
float convert_units_from(int unit, int mode)
{
    float val;


    switch(mode)
    {
        case WIDTH:
                switch(unit)
                {
                    case UNIT_PIXELS:   val = (float)OutParam.picwidth;
                                        break;
                    case UNIT_MM:       val = (float)OutParam.picwidth * 25.4 / (float)DevParam.horres;
                                        break;
                    case UNIT_INCH:     val = (float)OutParam.picwidth / (float)DevParam.horres;
                                        break;
                    case UNIT_PERCENT:  val = (float)OutParam.picwidth * 100.0 / (float)pic_active->pic_width;
                                        break;
                }
                break;

        case HEIGHT:
                switch(unit)
                {
                    case UNIT_PIXELS:   val = (float)OutParam.picheight;
                                        break;
                    case UNIT_MM:       val = (float)OutParam.picheight * 25.4 / (float)DevParam.verres;
                                        break;
                    case UNIT_INCH:     val = (float)OutParam.picheight / (float)DevParam.verres;
                                        break;
                    case UNIT_PERCENT:  val = (float)OutParam.picheight * 100.0 / (float)pic_active->pic_height;
                                        break;
                }
                break;

        case XPOS:
                switch(unit)
                {
                    case UNIT_PIXELS:   val = (float)OutParam.zx;
                                        break;
                    case UNIT_MM:       val = (float)OutParam.zx * 25.4 / (float)DevParam.verres;
                                        break;
                    case UNIT_INCH:     val = (float)OutParam.zx / (float)DevParam.verres;
                                        break;
                    case UNIT_PERCENT:  val = (float)OutParam.zx * 100.0 / (float)DevParam.prtwidth;
                                        break;
                }
                break;

        case YPOS:
                switch(unit)
                {
                    case UNIT_PIXELS:   val = (float)OutParam.zy;
                                        break;
                    case UNIT_MM:       val = (float)OutParam.zy * 25.4 / (float)DevParam.verres;
                                        break;
                    case UNIT_INCH:     val = (float)OutParam.zy / (float)DevParam.verres;
                                        break;
                    case UNIT_PERCENT:  val = (float)OutParam.zy * 100.0 / (float)DevParam.prtheight;
                                        break;
                }
                break;
    }

    return(val);
} /* convert_units_from */


/*----------------------------------------------------------------- */
/* convert_units_to                                                 */
/*  Konvertiert Pixel, Millimeter, Zoll und Prozent in Pixel        */
/*  RÅckgabewert ist ein Flieûkommawert zur Ausgabe.                */
/*----------------------------------------------------------------- */
int convert_units_to(int unit, int mode)
{
    float val;


    switch(mode)
    {
        case WIDTH:
                val = atof(print_dialog[WIDTH].TextCast);

                switch(unit)
                {
                    case UNIT_MM:       val = val * (float)DevParam.horres / 25.4;
                                        break;
                    case UNIT_INCH:     val = val * (float)DevParam.horres;
                                        break;
                    case UNIT_PERCENT:  val = val * (float)pic_active->pic_width / 100.0;
                                        break;
                }
                break;

        case HEIGHT:
                val = atof(print_dialog[HEIGHT].TextCast);

                switch(unit)
                {
                    case UNIT_MM:       val = val * (float)DevParam.verres / 25.4;
                                        break;
                    case UNIT_INCH:     val = val * (float)DevParam.verres;
                                        break;
                    case UNIT_PERCENT:  val = val * (float)pic_active->pic_height / 100.0;
                                        break;
                }
                break;


        case XPOS:
                val = atof(print_dialog[XPOS].TextCast);

                switch(unit)
                {
                    case UNIT_MM:       val = val * (float)DevParam.horres / 25.4;
                                        break;
                    case UNIT_INCH:     val = val * (float)DevParam.horres;
                                        break;
                    case UNIT_PERCENT:  val = val * (float)DevParam.prtwidth / 100.0;
                                        break;
                }
                break;

        case YPOS:
                val = atof(print_dialog[YPOS].TextCast);

                switch(unit)
                {
                    case UNIT_MM:       val = val * (float)DevParam.verres / 25.4;
                                        break;
                    case UNIT_INCH:     val = val * (float)DevParam.verres;
                                        break;
                    case UNIT_PERCENT:  val = val * (float)DevParam.prtheight / 100.0;
                                        break;
                }
                break;
    }

    return((int)(val + 0.5));
} /* convert_units_to */


/*--------------------------------------------------------------------------------- */
/* insert_32_float                                                                  */
/*  fÅgt eine 64Bit-Flieûkommazahl val als Text in den Objektbaum tree, TEXTobjekt  */
/*  ein. Der String hat hinterher grundsÑtzlich 5 Stellen, bei variabler Mantisse.  */
/*--------------------------------------------------------------------------------- */
void insert_float(OBJECT *tree, int object, float val)
{
    char str[10], str2[10];

    int dec, dummy; 

    double fval;


    memset(str, 0, 10);
    memset(str2, 0, 10);
    fval = (double)val;

    ftoa(&fval, str, 10, 1, &dec, &dummy);

    if(dec > 0)
        strncpy(str2, str, dec);
    else
        strcpy(str2, "0");

    if(dec < 0) dec = 0;
    strcat(str2, ".");

    strcat(str2, str + dec);

    strncpy(tree[object].TextCast, str2, 5);

    redraw_window(&print_window, NULL, object, 0);

    return;
} /* insert_float */


/* Clippt XPOS, YPOS, WIDTH und HEIGHT damit das Bild aufs Blatt paût */
char clip_values()
{
    char changed = 0;


    if(OutParam.zx + OutParam.picwidth > DevParam.leftBorder + DevParam.prtwidth)
    {
        OutParam.zx = DevParam.leftBorder + DevParam.prtwidth - OutParam.picwidth;
        changed = 1;
    }

    if(OutParam.zx < 0)
    {
        OutParam.zx = 0;
        changed = 1;
    }

    if(OutParam.zy + OutParam.picheight > DevParam.upperBorder + DevParam.prtheight)
    {
        OutParam.zy = DevParam.upperBorder + DevParam.prtheight - OutParam.picheight;
        changed = 1;
    }

    if(OutParam.zy < 0)
    {
        OutParam.zy = 0;
        changed = 1;
    }

    if(OutParam.zx + OutParam.picwidth > DevParam.leftBorder + DevParam.prtwidth)
    {
        OutParam.picwidth = DevParam.prtwidth - OutParam.zx;
        changed = 1;
    }

    if(OutParam.zy + OutParam.picheight > DevParam.upperBorder + DevParam.prtheight)
    {
        OutParam.picheight = DevParam.prtheight - OutParam.zy;
        changed = 1;
    }

    return(changed);
} /* clip_values */


void actualize_xywh()
{
    float val;


    val  = convert_units_from(unit, WIDTH);
    insert_float(print_dialog, WIDTH, val);

    val = convert_units_from(unit, HEIGHT);
    insert_float(print_dialog, HEIGHT, val);

    val  = convert_units_from(unit, XPOS);
    insert_float(print_dialog, XPOS, val);

    val  = convert_units_from(unit, YPOS);
    insert_float(print_dialog, YPOS, val);

    return;
} /* actualize_xywh */


/* KÅrzt fÅr beliebige Strings zu lange Filenamen durch */
/* entfernen von Text in der Mitte und ersetzen durch "...". */
/* Aus "Dies ist ein zu langer Filename.img" wÅrde bei KÅrzung */
/* auf 27 Zeichen "Dies ist ein...Filename.img" */
char *shorten_name(char *string, char newlen)
{
    char temp[257] = "";


    /* nichts tun wenn String sowieso passend */
    if(strlen(string) <= newlen)
        return(string);

    strncpy(temp, string, newlen / 2 - 1);          /* auf die HÑlfte und eines weniger */
    strcat(temp, "...");                            /* LÅckenfÅller rein */
    strcat(temp, string + strlen(string) - (newlen - newlen / 2 - 3));  /* und bis newlen LÑnge mit Originalstring affÅllen */

    return(temp);   
} /* shorten_name */
