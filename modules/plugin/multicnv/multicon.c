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

/*---------------------------------------------------------------------------
                                Multikonverter-Plugin
            FÅr SMURF Grafikkonverter, Olaf ab dem 20.03.98
            
    Das Plugin verwendet fÅr die Verwaltung der Exportmodule ausschlieûlich
    Smurf-interne Stukturen, so daû bei MMORE/MMORECANC/MMOREOK Smurf den
    gewohnten Weg geht und das Plugin nicht noch mit den Konfigurationen
    hantieren muû. Smurf merkt also in diesem Punkt gar nicht, daû er das
    Exportmodul gar nicht selber gestartet hat.
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdi.h>
#include <tos.h>
#include <ext.h>
#include <math.h>
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\src\smurf_st.h"
#include "..\..\..\src\smurf.h"
#include "..\..\..\src\smurfine.h"
#include "..\..\..\src\globdefs.h"
#include "..\..\..\src\plugin\plugin.h"
#include "..\..\..\src\popdefin.h"
#include "..\..\..\src\smurfobs.h"

#include "multi.h"

#define ENGLISCH 0

#if ENGLISCH
    #include "englisch.rsc\multi.rsh"
    #include "englisch.rsc\multi.rh"
#else
    #include "deutsch.rsc\multi.rsh"
    #include "deutsch.rsc\multi.rh"
#endif

#define TextCast ob_spec.tedinfo->te_ptext

void init_rsh(void);
void init_windstruct(void);
void actualize_dialog(void);
void handle_dialog(PLUGIN_DATA *data);
int handle_aesmsg(int *msgbuf);
void open_exporter_options(void);
void analyze_module(MOD_ABILITY *expmabs, char *export_path, int mod_num);
void ready_depth_popup(MOD_ABILITY *expmabs);
int give_free_module(void);
void init_exporter(void);
int depth2popbutton(int depth);
void init_colpop(void);
void check_depth(void);

char *load_palfile(char *path, int *red, int *green, int *blue, int max_cols);

extern  void change_object(WINDOW *window, int object, int status, int redraw);
extern  char *fload(char *Path, int header);
extern  int depth_button2depth(int button);

extern  void start_conversion(void);

char name_string[] = "Multikonverter\0";

PLUGIN_INFO plugin_info=
{
    name_string,
    0x091,              /* Plugin-Version 1.00 */
    0x106,              /* fÅr Smurf-Version 1.06 */
    0                   /* und nicht resident. */
};

SERVICE_FUNCTIONS *services;

int my_id, dialog_open, options_open;

WINDOW  window;
OBJECT  *dialog, *alerts;
SMURF_PIC *pic_active;

SMURF_PIC * *smurf_picture;
GARGAMEL * *smurf_struct;
BASPAG * *edit_bp;
int *active_pic;

PLUGIN_FUNCTIONS    *smurf_functions;
SMURF_VARIABLES *smurf_vars;

LIST_FIELD exporter_list;
POP_UP  depth_popup, colsys_popup, dither_popup, palette_popup;

/*
*   Informationen des gewÑhlten Exporters: 
*/
EXPORT_CONFIG   original_exp_conf;
EXPORT_CONFIG *exp_conf;        /* dies ist die Struktur aus Smurf, um ihn beim Dithern zu betrÅgen */
EXPORT_CONFIG my_exp_conf;
MOD_ABILITY expmabs;
char *export_path;
int mod_num, exp_depth;

int fix_red[256], fix_green[256], fix_blue[256];

char Path[256] = "C:\\";

void plugin_main(PLUGIN_DATA *data)
{
    int exp_index;

    /* Die Strukturen mit den Variablen und Funktionen holen */
    services = data->services;
    smurf_functions = data->smurf_functions;
    smurf_vars = data->smurf_vars;
    smurf_struct = smurf_vars->smurf_struct;
    smurf_picture = smurf_vars->smurf_picture;
    edit_bp = smurf_vars->edit_bp;
    active_pic = smurf_vars->active_pic;
    exp_conf = smurf_vars->exp_conf;

    if(data->message == AES_MESSAGE)
    {
        data->message = handle_aesmsg(data->event_par);
        return;
    }

    switch(data->message)
    {
        /* 
         * Startup des Plugins: MenÅeintrag anfordern 
         */
        case MSTART:
                        my_id = data->id;
                        strcpy(data->plugin_name, "Multikonvert...");
                        init_rsh();
                        init_windstruct();
                        data->wind_struct = &window;
                        data->message = MENU_ENTRY;     /* MenÅeintrag anfordern */
                        data->event_par[0]=24;
                        break;

        /*
         * Plugin wurde aus dem MenÅ heraus aufgerufen 
         */
        case PLGSELECTED:   pic_active = smurf_picture[*active_pic];

                            /*
                             * Fenster îffnen und Dialog initialisieren
                             */
                            my_exp_conf.exp_dither = DIT1;
                            my_exp_conf.exp_colred = CR_SYSPAL;
                            
                            strcpy(dialog[DITHER_PB].TextCast, smurf_vars->col_pop[DIT1].TextCast);
                            strcpy(dialog[PALMODE_PB].TextCast, smurf_vars->colred_popup[CR_SYSPAL].TextCast);

                            services->f_module_window(&window);
                            dialog_open = 1;

                            services->listfield((long *)&window, 99, 99, &exporter_list);

                            exp_index = services->listfield((long *)&window, 0, 0, &exporter_list);
                            export_path = smurf_vars->export_modules[exp_index];
                            analyze_module(&expmabs, export_path, mod_num);
                            ready_depth_popup(&expmabs);
                            init_exporter();

                            init_colpop();

                            strcpy(dialog[SOURCE_PATH].TextCast, "");
                            strcpy(dialog[DEST_PATH].TextCast, "");
                            strcpy(dialog[WILDCARDS].TextCast, "*");

                            data->message = M_WAITING;
                            break;

        /*
         * Fenster wurde geschlossen
         */
        case MWINDCLOSED:   smurf_functions->check_and_terminate(MTERM, mod_num&0xFF);  
                            dialog_open = 0;
                            break;
                            
        /*
         * Keyboardevent/Buttonevent
         */
        case MKEVT:
        case MBEVT:     handle_dialog(data);
                        data->message = M_WAITING;
                        break;

        case MTERM:     data->message = M_TERMINATED;
                        break;

        default:        data->message = M_WAITING;
                        break;
    }
}


void init_windstruct(void)
{

    window.module = my_id;
    window.wnum = 1;                            /* Das wievielte Fenster des Moduls? */
    window.wx = -1;                         
    window.wy = -1;                         

    strcpy(window.wtitle, "Multikonverter");    /* Fenstertitel */

    window.resource_form = dialog;          /* Modulresource-Formular */
    window.picture = NULL;                  /* Zeigerfeld fÅr Bild/Animation */
    window.editob = WILDCARDS;
    window.nextedit = 0;
    window.editx = 0;
    window.pflag = 0;

    window.prev_window = NULL;  /* vorheriges Fenster (WINDOW*) */
    window.next_window = NULL;  /* nÑxtes Fenster (WINDOW*) */
    
    
    memcpy(&exporter_list, smurf_vars->export_list, sizeof(LIST_FIELD));
    exporter_list.max_entries = 9;      
    exporter_list.parent_obj = EXPLIST_PARENT;
    exporter_list.slider_parent = EXPLIST_SB;   /* Parentobjekt des Sliders     */
    exporter_list.slider_obj = EXPLIST_SL;      /* Sliderobjekt                 */
    exporter_list.slar_up = EXPLIST_UP;         /* Slider-Arrow hoch            */
    exporter_list.slar_dn = EXPLIST_DN;         /* Slider-Arrow runter          */
    exporter_list.scroll_offset = 0;            /* Scrolloffset der Liste       */
    exporter_list.autolocator = NULL;           /* Zeiger auf Autolocator-String */
    exporter_list.auto_len = 0;
    
    memcpy(&depth_popup, &smurf_vars->pop_ups[POPUP_EXP_DEPTH], sizeof(POP_UP));
    depth_popup.item = 7;                   /* Item (=letzter Eintrag)      */
    depth_popup.display_tree = dialog;      /* Tree, in dems dargestellt werden soll */
    depth_popup.display_obj = DEPTH_PB;     /* Objekt in display_tree, bei dems dargestellt werden soll */
    depth_popup.Cyclebutton = DEPTH_CB;     /* Index des Cyclebuttons */

    memcpy(&palette_popup, &smurf_vars->pop_ups[POPUP_COLRED], sizeof(POP_UP));
    palette_popup.item = 1;                 /* Item (=letzter Eintrag)      */
    palette_popup.display_tree = dialog;        /* Tree, in dems dargestellt werden soll */
    palette_popup.display_obj = PALMODE_PB; /* Objekt in display_tree, bei dems dargestellt werden soll */
    palette_popup.Cyclebutton = PALMODE_CB; /* Index des Cyclebuttons */

    memcpy(&dither_popup, &smurf_vars->pop_ups[POPUP_DITHER], sizeof(POP_UP));
    dither_popup.item = 1;                  /* Item (=letzter Eintrag)      */
    dither_popup.display_tree = dialog;     /* Tree, in dems dargestellt werden soll */
    dither_popup.display_obj = DITHER_PB;   /* Objekt in display_tree, bei dems dargestellt werden soll */
    dither_popup.Cyclebutton = DITHER_CB;   /* Index des Cyclebuttons */
}



/* init_rsh -----------------------------------------------------
    Initialisiert die Resource 
    -----------------------------------------------------------*/
void init_rsh(void)
{
    int t;

    /* Resource Umbauen */
    for (t=0; t<NUM_OBS; t++)   rsrc_obfix (&rs_object[t], 0);

    dialog = rs_trindex[MULTICON];      /* Resourcebaum holen */
    alerts = rs_trindex[ALERT_STRINGS];

    return;
}



/* actualize_dialog ----------------------------------------------
    Aktualisiert den Dialog
    ------------------------------------------------------------*/
void actualize_dialog(void)
{

    services->redraw_window(&window, NULL, 0, 0);
}




/* handle_dialog ----------------------------------------------
    öbernimmt die Userbedienung des Dialogs
    ----------------------------------------------------------*/
void handle_dialog(PLUGIN_DATA *data)
{
    int button, back, exp_index, t;
    char module_name[33]="";
    char *textseg, *pal_loadpath;
    MOD_INFO *minfo;


    if(data->message == MBEVT)
    {

        button = data->event_par[0];
        switch(button)
        {
            case DEPTH_PB:
            case DEPTH_CB:      back = services->popup(&depth_popup, 0, button, NULL);
                                services->deselect_popup(&window, DEPTH_PB, DEPTH_CB);
                                if(back>0)
                                    exp_depth = back;
                                check_depth();
                                break;
                                
            case DITHER_PB:
            case DITHER_CB:     back = services->popup(&dither_popup, 0, button, NULL);
                                services->deselect_popup(&window, DITHER_PB, DITHER_CB);
                                if(back>0)
                                    my_exp_conf.exp_dither = back;
                                check_depth();
                                break;
                                
            case PALMODE_PB:
            case PALMODE_CB:    back = services->popup(&palette_popup, 0, button, NULL);
                                services->deselect_popup(&window, PALMODE_PB, PALMODE_CB);
                                if(back>0) my_exp_conf.exp_colred = back;
                                    
                                if(my_exp_conf.exp_colred==CR_FILEPAL)
                                    change_object(&window, LOADPAL, ENABLED, 1);
                                else
                                    change_object(&window, LOADPAL, DISABLED, 1);
                                break;

            case EXPORTER_OPTIONS:  open_exporter_options();
                                    break;
            
            case SOURCE_PATH:   back = services->f_fsbox(Path, "Quellpfad", 6);
                                strncpy(dialog[SOURCE_PATH].TextCast, Path, 53);
                                dialog[SOURCE_PATH].TextCast[54]=0;
                                services->redraw_window(&window, NULL, SOURCE_PATH, 0);
                                break;
                                    
            case DEST_PATH: back = services->f_fsbox(Path, "Zielpfad", 6);
                            strncpy(dialog[DEST_PATH].TextCast, Path, 53);
                            dialog[DEST_PATH].TextCast[54]=0;
                            services->redraw_window(&window, NULL, DEST_PATH, 0);
                            break;

            case LOADPAL:   pal_loadpath = load_palfile(smurf_vars->Sys_info->standard_path, 
                                                    fix_red, fix_green, fix_blue, 1<<depth_button2depth(exp_depth));
                            if(pal_loadpath != NULL)
                            {
                                strcpy(smurf_vars->colred_popup[CR_FILEPAL].TextCast, strrchr(pal_loadpath, '\\')+1);
                                strcpy(dialog[PALMODE_PB].TextCast, strrchr(pal_loadpath, '\\')+1);
                            }
                            change_object(&window, DITHER_PB, UNSEL, 1);
                            change_object(&window, LOADPAL, UNSEL, 1);
                            break;

            case GO:        start_conversion();
                            break;
        }

        /*
         * Hat sich der gewÑhlte Exporter geÑndert? -> Farbtiefenpopup neu initialisieren
         */
        if(button >= EXPLIST_1 && button <= EXPLIST_9 || 
            button == EXPLIST_UP || button == EXPLIST_DN || button == EXPLIST_SL || button == EXPLIST_SB)
        {
            /*
             * ist der Exporter schon durch Fensterschlieûen
             * terminiert worden? Dann darf hier nix mehr gemacht werden
             */
            if(edit_bp[mod_num&0xFF]!=NULL)     
            {
                /*
                 * bisherigen Optionsdialog schlieûen
                 */
                if(options_open)
                {
                    options_open = 0;
                    
                    textseg = edit_bp[mod_num&0xFF] -> p_tbase;
                    minfo = (MOD_INFO *)*((MOD_INFO **)(textseg + MOD_INFO_OFFSET));    /* Zeiger auf Modulinfostruktur */
                    strncpy(module_name, minfo->mod_name, 30);
                                
                    for(t = 0; t < smurf_vars->anzahl_exporter; t++)
                    {
                        if( strncmp(module_name, smurf_vars->export_module_names[t], strlen(module_name))==0 )
                            break;
                    } 
    
                    *( (long *)smurf_struct[mod_num&0xFF]->event_par) = (long)smurf_vars->export_cnfblock[t];
                    smurf_struct[mod_num&0xFF]->event_par[2] = smurf_vars->export_cnflen[t];
                
                    smurf_functions->start_exp_module(export_path, MMORECANC, NULL, edit_bp[mod_num&0xFF], smurf_struct[mod_num&0xFF], mod_num);
                    wind_close(smurf_struct[mod_num&0xFF]->wind_struct->whandlem);
                    wind_delete(smurf_struct[mod_num&0xFF]->wind_struct->whandlem);
                    smurf_struct[mod_num&0xFF]->wind_struct->whandlem = -1;
                }

                /* momentanen Exporter wieder terminieren
                 */
                smurf_functions->check_and_terminate(MTERM, mod_num&0xFF);
            }
            
            exp_index = services->listfield((long *)&window, button, 0, &exporter_list);
            export_path = smurf_vars->export_modules[exp_index];
            analyze_module(&expmabs, export_path, mod_num);
            ready_depth_popup(&expmabs);
            init_exporter();
            init_colpop();
        }
    }
    
}

/* handle_aesmsg ----------------------------------------------
    kÅmmert sich um von Smurf weitergeleitete AES-Messages
    ----------------------------------------------------------*/
int handle_aesmsg(int *msgbuf)
{
    return(M_WAITING);
}


/* open_exporter options ------------------------------------------------
    Bringt den geladenen Exporter dazu, seinen internen Optionsdialog zu
    îffnen.
    ---------------------------------------------------------------------*/
void open_exporter_options(void)
{
    smurf_functions->start_exp_module(export_path, MMORE, NULL, edit_bp[mod_num&0xFF], smurf_struct[mod_num&0xFF], mod_num);
    options_open=1;
}


/* init_exporter --------------------------------------------------------
    LÑdt den Exporter der globalen ID mod_num und startet diesen mit MSTART
    bei gleichzeitiger öbergabe seines Konfigurationsblocks.
    ----------------------------------------------------------------------*/
void init_exporter(void)
{
    char *txtbeg;
    char module_name[33] = "";

    int t;

    MOD_INFO *modinfo;


    mod_num = give_free_module();
    if(mod_num == -1)
    {
        services->f_alert(smurf_vars->alerts[MOD_LOAD_ERR].TextCast, 0,0,0,1);
        return;
    }
    mod_num |= 0x100;       /* als Exporter kennzeichnen */

    smurf_struct[mod_num&0xFF] = malloc(sizeof(GARGAMEL));
    memset(smurf_struct[mod_num&0xFF], 0x0, sizeof(GARGAMEL));

    edit_bp[mod_num&0xFF] = (BASPAG *)smurf_functions->start_exp_module(export_path, MSTART, NULL, edit_bp[mod_num&0xFF], smurf_struct[mod_num&0xFF], mod_num);
    smurf_struct[mod_num&0xFF]->wind_struct = NULL;

    txtbeg = edit_bp[mod_num&0xFF]->p_tbase;
    modinfo = (MOD_INFO *)*((MOD_INFO **)(txtbeg + MOD_INFO_OFFSET));       /* Zeiger auf Modulinfostruktur */

    strncpy(module_name, modinfo->mod_name, 30);

    for(t = 0; t < smurf_vars->anzahl_exporter; t++)
    {
        if(strncmp(module_name, smurf_vars->export_module_names[t], strlen(module_name)) == 0)
            break;
    } 
    *((long *)smurf_struct[mod_num&0xFF]->event_par) = (long)smurf_vars->export_cnfblock[t];
    smurf_struct[mod_num&0xFF]->event_par[2] = smurf_vars->export_cnflen[t];
}


/* analyze_module --------------------------------------------------------
    LÑdt das Modul mit dem Pfad export_path mit der ID mod_num und kopiert
    die Infostrukturen in expmabs.
    ---------------------------------------------------------------------*/
void analyze_module(MOD_ABILITY *expmabs, char *export_path, int mod_num)
{
    MOD_ABILITY *export_mabs;

    
    mod_num = give_free_module();
    if(mod_num == -1)
    {
        services->f_alert(smurf_vars->alerts[MOD_LOAD_ERR].TextCast, 0,0,0,1);
        return;
    }
    mod_num |= 0x100;       /* als Exporter kennzeichnen */
    
    smurf_struct[mod_num&0xFF] = malloc(sizeof(GARGAMEL));
    memset(smurf_struct[mod_num&0xFF], 0x0, sizeof(GARGAMEL));

    /*
     * Modul analysieren
     */
    export_mabs = (MOD_ABILITY *)smurf_functions->start_exp_module(export_path, MQUERY, NULL, edit_bp[mod_num&0xFF], smurf_struct[mod_num&0xFF], mod_num);
    memcpy(expmabs, export_mabs, sizeof(MOD_ABILITY));

    if(expmabs->ext_flag&0x02)
        dialog[EXPORTER_OPTIONS].ob_state &= ~DISABLED;
    else
        dialog[EXPORTER_OPTIONS].ob_state |= DISABLED;

    services->redraw_window(&window, NULL, EXPORTER_OPTIONS, 0);
    
    smurf_functions->check_and_terminate(MTERM, mod_num&0xFF);  /* Modul wieder terminieren */
}


/* ready_depth_popup ------------------------------------------------------------
    Paût die Farbtiefen im Smurf-Popup fÅr Exportfarbtiefen nach den ModulfÑhigkeiten
    in expmabs an.
    -----------------------------------------------------------------------------*/
void ready_depth_popup(MOD_ABILITY *expmabs)
{
    int export_depth[10], t, ob;

    export_depth[0]=expmabs->depth1;
    export_depth[1]=expmabs->depth2;
    export_depth[2]=expmabs->depth3;
    export_depth[3]=expmabs->depth4;
    export_depth[4]=expmabs->depth5;
    export_depth[5]=expmabs->depth6;
    export_depth[6]=expmabs->depth7;
    export_depth[7]=expmabs->depth8;
    
    /*
    *   Depth-Popup vorbereiten 
    */
    ob=EXP_D1;
    for(t=0; t<=6; t++)
    {
        smurf_vars->exp_dp_popup[ob].ob_state |= DISABLED;
        ob++;
    }
    for(t=0; t<8; t++)
    {
        switch(export_depth[t])
        {
            case 24:    smurf_vars->exp_dp_popup[EXP_D24].ob_state &= ~DISABLED;    break;
            case 16:    smurf_vars->exp_dp_popup[EXP_D16].ob_state &= ~DISABLED;    break;
            case 8:     smurf_vars->exp_dp_popup[EXP_D8].ob_state &= ~DISABLED;     break;
            case 4:     smurf_vars->exp_dp_popup[EXP_D4].ob_state &= ~DISABLED;     break;
            case 2:     smurf_vars->exp_dp_popup[EXP_D2].ob_state &= ~DISABLED;     break;
            case 1:     smurf_vars->exp_dp_popup[EXP_D1].ob_state &= ~DISABLED;     break;
        }
    }
}


/* give_free_module -------------------------------------------------------
    ermittelt eine freie Smurf-Modulstruktur in den Smurf-internen Arrays und
    gibt einen entsprechenden Index zurÅck, der fÅr smurf_struct verwendet werden kann.
    -----------------------------------------------------------------------*/
int give_free_module(void)
{
    int mod;
    
    /* Freie Modulstruktur ermitteln */
    mod=0;
    while(smurf_struct[mod]!=NULL) mod++;
    if(mod>20)
    {
        services->f_alert(smurf_vars->alerts[NO_MORE_MODULES].TextCast, 0,0,0,1); 
        mod=-1;
    }

    return(mod);    
}

int depth2popbutton(int depth)
{
    int exp_depth;
    
    if(depth<2) exp_depth = EXP_D1;
    else if(depth<4) exp_depth = EXP_D2;
    else if(depth<8) exp_depth = EXP_D4;
    else if(depth<16) exp_depth = EXP_D8;
    else if(depth<24) exp_depth = EXP_D16;
    else exp_depth = EXP_D24;
    
    return(exp_depth);
}

void init_colpop(void)
{
    exp_depth = depth2popbutton(expmabs.depth1);
    
    if(expmabs.depth2>expmabs.depth1) 
        exp_depth = depth2popbutton(expmabs.depth2);
    if(expmabs.depth3>expmabs.depth1) 
        exp_depth = depth2popbutton(expmabs.depth3);
    if(expmabs.depth4>expmabs.depth1) 
        exp_depth = depth2popbutton(expmabs.depth4);
    if(expmabs.depth5>expmabs.depth1) 
        exp_depth = depth2popbutton(expmabs.depth5);
    if(expmabs.depth6>expmabs.depth1) 
        exp_depth = depth2popbutton(expmabs.depth6);
    if(expmabs.depth7>expmabs.depth1) 
        exp_depth = depth2popbutton(expmabs.depth7);
    if(expmabs.depth8>expmabs.depth1) 
        exp_depth = depth2popbutton(expmabs.depth8);
        
    strcpy(dialog[DEPTH_PB].TextCast, smurf_vars->exp_dp_popup[exp_depth].TextCast);
    services->deselect_popup(&window, DEPTH_PB, DEPTH_CB);
    depth_popup.item = exp_depth;
    
    check_depth();
}



void check_depth(void)
{
    int enablemode;

    if(exp_depth==EXP_D1)
    {
        strcpy(dialog[PALMODE_PB].TextCast, "s/w");
        dialog[PALMODE_CB].ob_state |= DISABLED;
        dialog[PALMODE_PB].ob_state |= DISABLED;
        my_exp_conf.exp_colred = CR_SYSPAL;
    }
    else
    {
        strcpy(dialog[PALMODE_PB].TextCast, smurf_vars->colred_popup[my_exp_conf.exp_colred].TextCast);
        dialog[PALMODE_CB].ob_state &= ~DISABLED;
        dialog[PALMODE_PB].ob_state &= ~DISABLED;
    }
    
    services->deselect_popup(&window, PALMODE_PB, PALMODE_CB);
    

    if(exp_depth!=EXP_D16 && exp_depth>EXP_D1)
    {
        if(smurf_vars->ditmod_info[my_exp_conf.exp_dither-1]->pal_mode==FIXPAL)
        {
            palette_popup.item = CR_FIXPAL;
            my_exp_conf.exp_colred = CR_FIXPAL;
            strncpy(dialog[PALMODE_PB].TextCast, smurf_vars->colred_popup[my_exp_conf.exp_colred].TextCast, 15);
            enablemode=DISABLED;
        }
        else 
        {
            palette_popup.item = CR_SYSPAL;
            my_exp_conf.exp_colred = CR_SYSPAL;
            strncpy(dialog[PALMODE_PB].TextCast, smurf_vars->colred_popup[my_exp_conf.exp_colred].TextCast, 15);
            enablemode=ENABLED;
        }
        
        if(enablemode==DISABLED)
        {
            dialog[PALMODE_PB].ob_state |= DISABLED;
            dialog[PALMODE_CB].ob_state |= DISABLED;
        }
        else
        {
            dialog[PALMODE_PB].ob_state &= ~DISABLED;
            dialog[PALMODE_CB].ob_state &= ~DISABLED;
        }
        services->deselect_popup(&window, PALMODE_PB, PALMODE_CB);
    }
}


char *load_palfile(char *path, int *red, int *green, int *blue, int max_cols)
{
    int fsback;
    int *palbuf, *palcpy;
    int max_count, t;
    extern long f_len;
    char pal_loadpath[256];

    strcpy(pal_loadpath, path);
    fsback = smurf_functions->f_fsbox(pal_loadpath, "Palette laden", 0);

    if(fsback!=FALSE)
    {
        palbuf = (int*)fload(pal_loadpath, 0);
        max_count = (int)(f_len/6);
        if(max_count>max_cols)
        {
            services->f_alert(smurf_vars->alerts[PAL_DEPTHERR].TextCast, 0,0,0,1);
            return(NULL);
        }
        else
        {
            palcpy=palbuf;
            /* Mit der ersten Farbe im File die Palette ausnullen */
            for(t=0; t<256; t++)
            {
                red[t] = (int)(255L*(long)palcpy[0] / 1000L);
                green[t] = (int)(255L*(long)palcpy[1] / 1000L);
                blue[t] = (int)(255L*(long)palcpy[2] / 1000L);
            }

            /* und Åbertragen */
            for(t=0; t<max_count; t++)
            {
                red[t]= (int)(255L*(long)palcpy[t*3] / 1000L);
                green[t]= (int)(255L*(long)palcpy[t*3+1] / 1000L);
                blue[t]= (int)(255L*(long)palcpy[t*3+2] / 1000L);
            }
        }
        Mfree(palbuf);
        return(pal_loadpath);
    }

    return(NULL);
}

