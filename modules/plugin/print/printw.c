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
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\smurf\smurf_st.h"
#include "..\..\..\smurf\globdefs.h"
#include "..\..\..\smurf\smurfine.h"
#include "..\..\..\smurf\plugin\plugin.h"
#include "..\..\..\smurf\smurf.h"
#include "gdos.h"
#include "printw.h"
#include "wdialog.h"

#include "..\..\..\smurf\smurfobs.h"

#define ENGLISCH 0

#if ENGLISCH
    #include "englisch.rsc\printw.rsh"
    #include "englisch.rsc\printw.rh"
#else
    #include "deutsch.rsc\printw.rsh"
    #include "deutsch.rsc\printw.rh"
#endif

void init_rsh(void);
int handle_print_dialog(EVNT *events);
void close_print_dialog(char with_free);
extern int print_with_GDOS(PRN_SETTINGS *prn_settings);
extern int actualize_DevParam(int gdos_dev, DevParamS *DevParam);

extern DevParamS DevParam;
extern OutParamS OutParam;

char name_string[]="GDOS Print-Plugin";

PLUGIN_INFO plugin_info=
{
    name_string,
    0x0101,             /* Plugin-Version 1.01 */
    0x0106,             /* fÅr Smurf-Version 1.06 */
    0                   /* und nicht resident. */
};

static int my_id;
int popup_initialized;

OBJECT *alerts;
SMURF_PIC *pic_active;

SMURF_PIC * *smurf_picture;
int *active_pic;

SMURF_VARIABLES *smurf_vars;
SERVICE_FUNCTIONS *services;
PLUGIN_FUNCTIONS *smurf_functions;

int pdlg_handle;
PRN_DIALOG *prn_dialog;
PRN_SETTINGS *prn_settings;


void plugin_main(PLUGIN_DATA *data)
{
    char *picname;

    int out1, out2, out3, out4, closed = 0;
    static int notify_num = 0;

    EVNT events;


    /* Die Strukturen mit den Variablen und Funktionen holen */
    services = data->services;
    smurf_functions = data->smurf_functions;
    smurf_vars = data->smurf_vars;

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
                        data->event_par[0] = FILE_PRINT;
                        data->message = MENU_ENTRY;     /* MenÅeintrag anfordern */
                        break;

        case PLG_STARTUP:   data->message = M_WAITING;
                            break;

        /*
         * Plugin wurde aus dem MenÅ heraus aufgerufen 
         */
        case PLGSELECTED:   if(appl_find("?AGI") < 0 ||
                               appl_getinfo(7, &out1, &out2, &out3, &out4) != 1 ||
                               (out1&0x10) == 0)
                            {
/*                              services->f_alert("Benîtigtes Programm WDIALOG ist nicht installiert.", NULL, NULL, NULL, 1); */
                                services->f_alert(alerts[NO_WDIALOG].TextCast, NULL, NULL, NULL, 1);
                                data->message = M_EXIT;
                            }
                            else
                            if(*(smurf_vars->picthere) == 0)
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
                                
                                /*
                                 * Druckdialog kreiern, initialisieren und îffnen 
                                 */
                                if((prn_dialog = pdlg_create(PDLG_3D)) != NULL)
                                {
                                    prn_settings = pdlg_new_settings(prn_dialog);

                                    if((picname = strrchr(pic_active->filename, '\\')) != NULL && *(picname + 1) != '\0')
                                        picname++;
                                    else
                                        picname = pic_active->filename;

                                    if((pdlg_handle = pdlg_open(prn_dialog, prn_settings, picname, PDLG_PRINT, -1, -1)) == 0)
                                    {
                                        pdlg_delete(prn_dialog);
                                        prn_dialog = 0L;
                                    }
                                    else
                                        if(notify_num == 0)
                                        {
                                            data->event_par[0] = pdlg_handle;
                                            data->message = ALL_MSGS;
                                            notify_num++;
                                            return;
                                        }
                                }

                                data->message = M_WAITING;
                            }
                            break;

        case DONE:          if(notify_num == 1)
                            {
                                data->event_par[0] = pdlg_handle;
                                data->message = MBEVT;
                                notify_num++;
                            }
                            else
                                if(notify_num == 2)
                                {
                                    data->event_par[0] = pdlg_handle;
                                    data->message = MKEVT;
                                    notify_num++;
                                }
                                else
                                    data->message = M_WAITING;
                            break;

        case MBEVT:         events.mwhich = MU_BUTTON;
                            events.mx = data->mousex;
                            events.my = data->mousey;
                            events.mclicks = data->klicks;
                            events.mbutton = data->event_par[1];
                            closed = handle_print_dialog(&events);
                            data->message = M_WAITING;
                            break;

        case MKEVT:         events.mwhich = MU_KEYBD;
                            events.kstate = data->event_par[3];
                            events.key = data->event_par[1];
                            closed = handle_print_dialog(&events);
                            data->message = M_WAITING;
                            break;

        case AES_MESSAGE:   events.mwhich = MU_MESAG;
                            memcpy(events.msg, data->event_par, 16);
                            closed = handle_print_dialog(&events);
                            data->message = M_WAITING;
                            break;

        case MPIC_UPDATE:   pic_active = smurf_picture[data->event_par[0]];
                            if((picname = strchr(pic_active->filename, '\\')) != NULL && *(picname + 1) != '\0')
                                picname++;
                            else
                                picname = pic_active->filename;
                            pdlg_update(prn_dialog, picname);
                            data->message = M_WAITING;
                            break;

        case MTERM:         close_print_dialog(1);
                            data->message = M_TERMINATED;
                            break;

        default:            data->message = M_WAITING;
                            break;
    }

    if(closed)
        data->message = M_EXIT;

    return;
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

    alerts = rs_trindex[ALERT_STRINGS];         /* Resourcebaum holen */

    return;
}


int handle_print_dialog(EVNT *events)
{
    int button;


    if(prn_dialog)
    {
        if(pdlg_evnt(prn_dialog, prn_settings, events, &button) == 0)   /* Dialog schlieûen? */
        {
            close_print_dialog(0);                                      /* Dialog schlieûen */

            if(button == PDLG_OK)                                       /* "Drucken" angewÑhlt? */
            {
                actualize_DevParam(prn_settings->driver_id, &DevParam);
                print_with_GDOS(prn_settings);
            }

            pdlg_free_settings(prn_settings);

            return(1);
        }
    }

    return(0);
} /* handle_print_dialog */


void close_print_dialog(char with_free)
{
    int lastx, lasty;


    if(prn_dialog)
    {
        pdlg_close(prn_dialog, &lastx, &lasty);
        pdlg_delete(prn_dialog);
        if(with_free)
            pdlg_free_settings(prn_settings);
        prn_dialog = 0L;
    }

    return;
} /* close_print_dialog */
