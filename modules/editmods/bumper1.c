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

/*  ------------- Bumpmap mit Texture - Modul V1.0 ------------------   */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>

#include "..\..\sym_gem.h"
#include "..\import.h"
#include "..\..\smurf\smurfine.h"

#include "bumpmap.rsh"
#include "bumpmap.rh"
#include <math.h>

#define Obj_Selected(a) (main_form[a].ob_state & SELECTED)
#define TextCast    ob_spec.tedinfo->te_ptext

int do_it(GARGAMEL *smurf_struct);

void f_init_sliders(void);
void f_init_window(int mod_id);
void f_default_sliders(void);

void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);

#define BUMPMAP 0
#define TEXTURE 1

char p1string[20] = "Bumpmap";
char p2string[20] = "Textur";

/*------ Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
    {
    "Bump It Up",                       /* Name des Moduls */
    0x0100,
    "J. Dittmer/O. Piesche",                                    /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
    "",         /* 4 SliderÅberschriften: max 8 */
    "",
    "", 
    "",
    "",         /* 4 CheckboxÅberschriften: */
    "",
    "",
    "",
    "",         /* 4 Edit-Objekt-öberschriften: */
    "",
    "",
    "",
    0,0,                /* min/max-Werte fÅr Slider */
    0,0,
    0,0,
    0,0,
    0,0,                /* min/max fÅr Editobjekte */
    0,0,
    0,0,
    0,0,
    255,255,255,100,    /* Defaultwerte fÅr Slider, Check und Edit */
    0,0,0,0,
    0,0,0,0,
    2,                  /* max. Anzahl der Bilder */
    p1string,           /* Bezeichnung fÅr Bilder */
    p2string
    };


/*--------------------- Was kann ich ? ----------------------*/
MOD_ABILITY  module_ability = 
{
    24, 0, 0, 0, 0, 0, 0, 0,    /* Farbtiefen */
    FORM_PIXELPAK,
    FORM_BOTH, FORM_BOTH, FORM_BOTH, FORM_BOTH,
    FORM_BOTH, FORM_BOTH, FORM_BOTH,
    0 /* Extra Flag */ 
};


typedef struct configuration
{
    long version;
    char red, green, blue;
    char gred, ggreen, gblue;
    char diff_strength, gl_strength;
    char diff_pow, gl_pow;
    int bump_str;
    char invert, background, parallel, texture;
} BUMP_CONFIG;
          
char red, green, blue;
char gred, ggreen, gblue;
char diffuse_strengh, glanz_strengh;
char diffuse_hard, glanz_hard;
int  bm_strengh;
int kreuz_x, kreuz_y;

SMURF_PIC *input_pictures[3];

OBJECT *main_form;

WINDOW *my_window;
int module_id;

SLIDER  red_slider, green_slider, blue_slider, 
        m_str_slider, m_hard_slider, bm_str_slider;

void (*set_slider)(SLIDER *sliderstruct, long value);  /* Funktion deklarieren */ 

void *(*mconfLoad)(MOD_INFO *modinfo, int mod_id, char *name);
void (*mconfSave)(MOD_INFO *modinfo, int mod_id, void *confblock, long len, char *name);
void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);
int (*SMfree)(void *ptr);

void save_setting(void);
void load_setting(void);
void apply_setting(BUMP_CONFIG *myConfig);
void write_setting(BUMP_CONFIG *myConfig);


/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
    int (*get_window)(WINDOW *wind_struct);
    int (*slider)(SLIDER *slider_struct);       /* Funktion deklarieren */
    int SmurfMessage;
    int t;
    int next_edit;
    BUMP_CONFIG *Default;
    unsigned long w1=0;
    unsigned int w2=0;
    
    int object;
    
    SmurfMessage=smurf_struct->module_mode;
    
    get_window = smurf_struct->services->f_module_window;
    slider = smurf_struct->services->slider;
    set_slider = smurf_struct->services->set_slider;
    mconfLoad = smurf_struct->services->mconfLoad;
    mconfSave = smurf_struct->services->mconfSave;
    redraw_window = smurf_struct->services->redraw_window;
    SMfree = smurf_struct->services->SMfree;
    
    main_form=rs_trindex[BUMPMAIN];
    

    /* Wenn das Modul aufgerufen wurde, */
    if(SmurfMessage == MSTART)
    {
        for(t=0; t<NUM_OBS; t++) rsrc_obfix(&rs_object[t], 0);
    
        module_id = smurf_struct->module_number;
        my_window = malloc(sizeof(WINDOW));
    
        f_init_sliders();
        f_init_window(module_id);
    
        smurf_struct->wind_struct=my_window;
    
        get_window(my_window);
    
        f_default_sliders();
        strcpy(main_form[LOAD_SET].TextCast, "Laden");
        smurf_struct->module_mode=M_WAITING;
        return;
    }
    
    else if(SmurfMessage == MBEVT)
    {
        object=smurf_struct->event_par[0];
    
        switch(object)
        {
            case DO_IT:             smurf_struct->module_mode = M_STARTED;
                                    return;
                                    break;
        
            case M_RED_SLIDE:       if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                        red=slider(&red_slider);
                                    else
                                        gred=slider(&red_slider);               
                                    break;
        
            case M_GREEN_SLIDE:     if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                        green=slider(&green_slider);
                                    else
                                        ggreen=slider(&green_slider);
                                    break;
        
            case M_BLUE_SLIDE:      if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                        blue=slider(&blue_slider);
                                    else
                                        gblue=slider(&blue_slider);
                                    break;

            case M_STRENGH_SLIDE:   if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                        diffuse_strengh=slider(&m_str_slider);
                                    else
                                        glanz_strengh=slider(&m_str_slider);
                                    break;

            case M_HARD_SLIDE:      if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                        diffuse_hard=slider(&m_hard_slider);
                                    else
                                        glanz_hard=slider(&m_hard_slider);
                                    break;
                                    
            case BM_STRENGH_SLIDE:  bm_strengh=slider(&bm_str_slider);
                                    break;
                                    
            case M_RADIO_DIFFUSE:   set_slider(&red_slider, red);
                                    set_slider(&green_slider, green);
                                    set_slider(&blue_slider, blue);
                                    set_slider(&m_str_slider, diffuse_strengh);
                                    set_slider(&m_hard_slider, diffuse_hard); 
                                    break;
                                    
            case M_RADIO_GLANZ:     set_slider(&red_slider, gred);
                                    set_slider(&green_slider, ggreen);
                                    set_slider(&blue_slider, gblue);
                                    set_slider(&m_str_slider, glanz_strengh);
                                    set_slider(&m_hard_slider, glanz_hard); 
                                    break;

            case SAVE_SET:          save_setting();
                                    break;

            case LOAD_SET:          load_setting();
                                    break;
        }
        smurf_struct->module_mode = M_WAITING;
        return;
    }

    else if(SmurfMessage == MPICS)
    {
        switch(smurf_struct->event_par[0])
        {
            case TEXTURE:   if(Obj_Selected(BM_TEXTURE))
                            {
                                smurf_struct->event_par[0]=24;
                                smurf_struct->event_par[1]=FORM_PIXELPAK;
                                smurf_struct->event_par[2]=RGB;
                                smurf_struct->module_mode=M_PICTURE;
                            }
                            else smurf_struct->module_mode=M_WAITING;
                            break;
        
            case BUMPMAP:   smurf_struct->event_par[0]=24;
                            smurf_struct->event_par[1]=FORM_PIXELPAK;
                            smurf_struct->event_par[2]=RGB;
                            smurf_struct->module_mode=M_PICTURE;
                            break;
    
            default:    smurf_struct->module_mode=M_WAITING;
                        break;
        }
        return;
    }
    
    else if(SmurfMessage == MPICTURE)
    {
        /* Bild holen */
        input_pictures[smurf_struct->event_par[0]] = smurf_struct->smurf_pic;
    
        /* und weiter warten */
        smurf_struct->module_mode = M_WAITING;
        return;
    }
    
    /* Smurf fragt: Fadenkreuz? --------------*/
    else    if(SmurfMessage == MCROSSHAIR)
    {
        smurf_struct->event_par[0] = 1;
        smurf_struct->event_par[1] = 0;
        
        smurf_struct->module_mode = M_CROSSHAIR;
        return;
    }
     
    /* Smurf Åbergibt mir das Fadenkreuz -----------*/
    else    if(SmurfMessage == MCH_COORDS)
    {
        kreuz_x = smurf_struct->event_par[0];
        kreuz_y = smurf_struct->event_par[1];
        smurf_struct->module_mode = M_CH_COORDS;
        return;
    } 
    
    /* Wenn das Modul gestartet wurde 
     */
    else if(SmurfMessage == MEXEC)
    {
        smurf_struct->module_mode = do_it(smurf_struct);
        return;
    }
    
    /* Wenn das Modul sich verpissen soll
     */
    else if(smurf_struct->module_mode==MTERM)
    {
        free(my_window);
        smurf_struct->module_mode=M_EXIT;
        return;
    }

    else if(smurf_struct->module_mode==GETCONFIG)
    {
        Default = smurf_struct->services->SMalloc(sizeof(BUMP_CONFIG));
        write_setting(Default);
        smurf_struct->event_par[0] = (int)((unsigned long)Default>>16);
        smurf_struct->event_par[1] = (int)Default;
        smurf_struct->event_par[2] = (int)sizeof(BUMP_CONFIG);
        smurf_struct->module_mode = M_CONFIG;
    }

    else if(smurf_struct->module_mode==CONFIG_TRANSMIT)
    {
        w1 = (unsigned long)smurf_struct->event_par[0]<<16;
        w2 = smurf_struct->event_par[1];
        Default = (BUMP_CONFIG*)(w1|w2);
        apply_setting(Default);
        smurf_struct->module_mode = M_WAITING;
    }

} /*ende*/




/*------------------------- BambmÑbbing ---------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture,*texture;
    int width, height, twidth, theight;
    int tex_x, tex_y;
    char *mainpic,*texpic;
    
    char *greypic;
    char *coltab,*glanztab;
    char *wurztab;
    
    char *c_offset, *offset, *noffset,
             *y_offset, *y_goffset, *g_offset,
             *t_offset;
    
    int grey;
    int x,y;
    char red2, green2, blue2;
    char gred2, ggreen2, gblue2;
    int nred, ngreen, nblue;
    unsigned int tred,tgreen,tblue;
    long n, t, bpl, gbpl, tbpl;
    
    long col_size, coltab_size, coltab_size2;
    long glanz_size, glanztab_size;
    signed long glanz_x, glanz_y;
    long bump_fakt;

    signed int x_vekt, y_vekt, vekt; 
    char o_height; 
    signed long x_dif, y_dif;
    long abstand;

    long bs_diago;


    /* Bilddaten auslesen
     */
    picture = input_pictures[0];
    width   = picture->pic_width;
    height  = picture->pic_height;
    mainpic = picture->pic_data;

    bpl = width * 3L;
    gbpl = width;

    if(Obj_Selected(BM_TEXTURE))
    {
        texture = input_pictures[1];
        twidth = texture->pic_width;
        theight = texture->pic_height;
        texpic = texture->pic_data;     
        tbpl = twidth * 3L; 
    }
    
    bs_diago = sqrt((long)width*width + (long)height*height); 

    /* Slider umrechnen
     */
    col_size = (long)((100L - diffuse_hard) * 6.0 + 50);
    glanz_size = (long)((100L - glanz_hard) / 2.04 + 1);
    
    red2   = (char)((long )red   * diffuse_strengh / 100);
    green2 = (char)((long )green * diffuse_strengh / 100);
    blue2  = (char)((long )blue  * diffuse_strengh / 100);
    
    gred2   = (char)((long )gred   * glanz_strengh / 100);
    ggreen2 = (char)((long )ggreen * glanz_strengh / 100);
    gblue2  = (char)((long )gblue  * glanz_strengh / 100);
    
    bump_fakt = (long )(bs_diago / 360F * (bm_strengh / 80F) * 1024F);
        
    
    /*-----------------------------------------------------*/ 
     
    coltab_size = (long)(bs_diago * col_size / 100L);
    coltab_size2 = coltab_size * coltab_size;
    glanztab_size = (long)(bs_diago * glanz_size/100L);

    if(Obj_Selected(BM_PARALEL))
    {
        glanz_x = kreuz_x - width/2;
        glanz_y = kreuz_y - height/2;
    }
    else
    {
        glanz_x = kreuz_x;
        glanz_y = kreuz_y;
    }
    
        
    /* Speicher anfordern
     */
    if( (coltab = Malloc((long)coltab_size*3L)) == NULL) return(M_MEMORY);
    if( (glanztab = Malloc((long)coltab_size*3L)) == NULL) 
    {
        Mfree(coltab);  
        return(M_MEMORY);
    }
    if( (greypic = Malloc((long)width * (long)height)) == NULL) 
    {
        Mfree(glanztab);
        Mfree(coltab);  
        return(M_MEMORY);
    }
    
    
    /*---Glanztab lîschen------------------------*/
    g_offset = glanztab;
    for(t=0; t<(coltab_size*3L) ;t++) *(g_offset++) = 0;
    
        
    /*---Graubild erzeugen------------------------------------*/
    offset = mainpic;
    noffset = greypic;
    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            grey = *(offset++);
            grey += *(offset++);
            grey += *(offset++);
            if( Obj_Selected(INVERT) )
                *(noffset++) = 255-(char)(grey / 3);
            else
                *(noffset++) = (char)(grey / 3);
        }
    }
    
    
    
    /*---Farbtabelle generieren--------------------------------*/
    
    /*---Farbverlauf----------------------*/
    c_offset = coltab;
    for(t=coltab_size; t>0; t--)
    {
        n = t*t;
        *(c_offset++) = (char)((long)red2   * (n) / coltab_size2);
        *(c_offset++) = (char)((long)green2 * (n) / coltab_size2);
        *(c_offset++) = (char)((long)blue2  * (n) / coltab_size2);
    }
    
    if(Obj_Selected(BM_TEXTURE))
    {
        /*---Glanzpunkt-----------------------*/
        c_offset = glanztab;
        for(t=0; t<glanztab_size; t++)
        {
            n = glanztab_size-t;
            *(c_offset++) = (long ) ( (long)gred2   * n / glanztab_size);
            *(c_offset++) = (long ) ( (long)ggreen2 * n / glanztab_size);
            *(c_offset++) = (long ) ( (long)gblue2  * n / glanztab_size);               
        }
    }
    else
    {
        /*---Glanzpunkt-----------------------*/
        c_offset = coltab;
        for(t=0; t<glanztab_size; t++)
        {
            
            nred = (int)((long)(*(c_offset + 0L)) 
                        + (long)((long)gred2 * (glanztab_size-t) / glanztab_size));
            ngreen = (int)((long)(*(c_offset + 1L))
                        + (long) ((long)ggreen2 * (glanztab_size-t) / glanztab_size));
            nblue  = (int) ((long)(*(c_offset + 2L))  
                        + (long) ((long)gblue2  * (glanztab_size-t) / glanztab_size));
                                
            if(nred  >255) nred   = 255;
            if(ngreen>255) ngreen = 255;
            if(nblue >255) nblue  = 255;
            
            *(c_offset++) = (char)nred;
            *(c_offset++) = (char)ngreen;
            *(c_offset++) = (char)nblue;
        }   
    }   
    
    
    /*---Bump Mapping--------------------------------------------*/
    for (y=0; y<height; y++)
    {
        if(!(y&7)) smurf_struct->services->busybox(((long)y<<7L)/(long)height);
    
        offset = y_offset = mainpic + y*bpl;
        y_goffset = greypic + y*gbpl;
        
        if(Obj_Selected(BM_TEXTURE))
        {
            tex_y = y % (theight-1);
            tex_x = 0;
            t_offset = texpic + tex_y*tbpl;
        }
            
        for(x=0; x<width;x++)
        {
            g_offset = y_goffset + x;
            o_height = *(g_offset);
            
            /*  Soll der Hintergrund (Level 0) mit bearbeitet werden?
             */
            if(Obj_Selected(NORM_BAK) || o_height!=0)
            {
                x_vekt = 0;
                y_vekt = 0;
                
                if(x < (width-1))
                    x_vekt += o_height - *(g_offset + 1);
                if(y < (height-1))
                    y_vekt += o_height - *(g_offset + gbpl);
                /*if(x > 0)*/
                    x_vekt -= o_height - *(g_offset - 1); 
                /*if(y > 0)*/
                    y_vekt -= o_height - *(g_offset - gbpl);
                
                            
                x_vekt = ((signed long)x_vekt * bump_fakt) >>10;
                y_vekt = ((signed long)y_vekt * bump_fakt) >>10;
                
                if(Obj_Selected(BM_PARALEL))
                {
                    x_dif = (signed long)glanz_x - x_vekt;
                    y_dif = (signed long)glanz_y - y_vekt;
                } 
                else
                {       
                    x_dif = (signed long)glanz_x - (x_vekt + x);
                    y_dif = (signed long)glanz_y - (y_vekt + y);
                }
                abstand = sqrt(x_dif*x_dif + y_dif*y_dif);
                
                if(abstand >= coltab_size) abstand = coltab_size-1;
                
                c_offset = coltab + abstand*3L;
                
                if(Obj_Selected(BM_TEXTURE))
                {       
                    g_offset = glanztab + abstand*3L;   
                    tred   = ( ((unsigned int)*(t_offset++) * (unsigned int)*(c_offset++)) >>8) + (unsigned int)*(g_offset++);
                    tgreen = ( ((unsigned int)*(t_offset++) * (unsigned int)*(c_offset++)) >>8) + (unsigned int)*(g_offset++);
                    tblue  = ( ((unsigned int)*(t_offset++) * (unsigned int)*(c_offset++)) >>8) + (unsigned int)*(g_offset++);
                
                    if(tred > 255) tred = 255;
                    if(tgreen > 255) tgreen = 255;      
                    if(tblue > 255) tblue = 255;
                    
                    *(offset++) = (char)tred;
                    *(offset++) = (char)tgreen;
                    *(offset++) = (char)tblue;
                    
                    tex_x += 1;
                    if ( tex_x >= twidth)
                    {
                        tex_x -= twidth;
                        t_offset -= tbpl;
                    }           
                }
                else
                {
                    *(offset++) = *(c_offset++);
                    *(offset++) = *(c_offset++);
                    *(offset++) = *(c_offset++);
                }
            }/* Hintergrund gerendert*/

            /*  Hintergrund einfÑrben?
             */
            else
            {
                if(Obj_Selected(WHITE_BAK))
                {
                    *(offset++) = 255;
                    *(offset++) = 255;
                    *(offset++) = 255;
                }
                else if(Obj_Selected(BLACK_BAK))
                {
                    *(offset++) = 0;
                    *(offset++) = 0;
                    *(offset++) = 0;
                }
            }
                    
        }
    }
    

    Mfree(coltab);
    Mfree(glanztab);
    Mfree(greypic);
    
    smurf_struct->event_par[0] = 0;
    smurf_struct->event_par[1] = -1;
    
    return(M_PICDONE);
}



/*  ------------------- Fensterstruktur init -------------------    */
void f_init_window(int mod_id)
{
    memset(my_window, 0, sizeof(WINDOW));
    my_window->whandlem = 0;
    my_window->module = mod_id;
    my_window->wnum = 1;
    my_window->wx = -1;
    my_window->wy = -1;
    my_window->ww = main_form->ob_width;
    my_window->wh = main_form->ob_height;
    strncpy(my_window->wtitle, "Bump It Up  V0.1", 40);
    my_window->resource_form = main_form;
    my_window->picture = NULL;
    my_window->pflag = 0;
    my_window-> editob = 0;
}



/*--------------------- Slider initialisieren -------------------*/
void f_init_sliders(void)
{
    red_slider.regler=M_RED_SLIDE;
    red_slider.schiene=M_RED_FHR;
    red_slider.rtree=main_form;
    red_slider.txt_obj=M_RED_EDIT;
    red_slider.min_val=0;
    red_slider.max_val=255;
    red_slider.window=my_window;

    green_slider.regler=M_GREEN_SLIDE;
    green_slider.schiene=M_GREEN_FHR;
    green_slider.rtree=main_form;
    green_slider.txt_obj=M_GREEN_EDIT;
    green_slider.min_val=0;
    green_slider.max_val=255;
    green_slider.window=my_window;

    blue_slider.regler=M_BLUE_SLIDE;
    blue_slider.schiene=M_BLUE_FHR;
    blue_slider.rtree=main_form;
    blue_slider.txt_obj=M_BLUE_EDIT;
    blue_slider.min_val=0;
    blue_slider.max_val=255;
    blue_slider.window=my_window;

    m_str_slider.regler=M_STRENGH_SLIDE;
    m_str_slider.schiene=M_STRENGH_FHR;
    m_str_slider.rtree=main_form;
    m_str_slider.txt_obj=M_STRENGH_EDIT;
    m_str_slider.min_val=0;
    m_str_slider.max_val=100;
    m_str_slider.window=my_window;

    m_hard_slider.regler=M_HARD_SLIDE;
    m_hard_slider.schiene=M_HARD_FHR;
    m_hard_slider.rtree=main_form;
    m_hard_slider.txt_obj=M_HARD_EDIT;
    m_hard_slider.min_val=0;
    m_hard_slider.max_val=100;
    m_hard_slider.window=my_window;
 
    bm_str_slider.regler=BM_STRENGH_SLIDE;
    bm_str_slider.schiene=BM_STRENGH_FHR;
    bm_str_slider.rtree=main_form;
    bm_str_slider.txt_obj=BM_STRENGH_EDIT;
    bm_str_slider.min_val=0;
    bm_str_slider.max_val=999;
    bm_str_slider.window=my_window;
}


/*--------------------- Slider auf defaults setzen -------------------*/
void f_default_sliders(void)
{
    red   = 255;
    green = 255;
    blue  = 255;
    diffuse_strengh = 100;
    diffuse_hard = 70;
    
    gred   = 255;
    ggreen = 255;
    gblue  = 255;
    glanz_strengh = 100;
    glanz_hard = 80;
    
    bm_strengh = 200;
    
    set_slider(&red_slider, red);
    set_slider(&green_slider, green);
    set_slider(&blue_slider, blue);
    set_slider(&m_str_slider, diffuse_strengh);
    set_slider(&m_hard_slider, diffuse_hard);
    
    set_slider(&bm_str_slider, bm_strengh);
}




void save_setting(void)
{
    BUMP_CONFIG myConfig;
    char name[33];

    write_setting(&myConfig);
    
    memset(name, 0, 33);
    mconfSave(&module_info, module_id, &myConfig, sizeof(BUMP_CONFIG), name);

    strcpy(main_form[LOAD_SET].TextCast, name);
    redraw_window(my_window, NULL, LOAD_SET, 0);
}


void load_setting(void)
{
    BUMP_CONFIG *myConfig;
    char name[33];

    memset(name, 0, 33);

    myConfig = mconfLoad(&module_info, module_id, name);
    strcpy(main_form[LOAD_SET].TextCast, name);
    redraw_window(my_window, NULL, LOAD_SET, 0);

    apply_setting(myConfig);    
    SMfree(myConfig);
}




void apply_setting(BUMP_CONFIG *myConfig)
{
    
    red = myConfig->red;
    green = myConfig->green;
    blue = myConfig->blue;
    gred = myConfig->gred;
    ggreen = myConfig->ggreen;
    gblue = myConfig->gblue;
    diffuse_strengh = myConfig->diff_strength;
    glanz_strengh = myConfig->gl_strength;
    diffuse_hard = myConfig->diff_pow;
    glanz_hard = myConfig->gl_pow;
    bm_strengh = myConfig->bump_str;
    set_slider(&bm_str_slider, bm_strengh);
    
    main_form[INVERT].ob_state = myConfig->invert;
    main_form[BM_TEXTURE].ob_state = myConfig->texture;
    main_form[BLACK_BAK].ob_state &= ~SELECTED;
    main_form[WHITE_BAK].ob_state &= ~SELECTED;
    main_form[NORM_BAK].ob_state &= ~SELECTED;

    if(myConfig->background == BLACK_BAK)
        main_form[BLACK_BAK].ob_state |= SELECTED;
    else if(myConfig->background == WHITE_BAK)
        main_form[WHITE_BAK].ob_state |= SELECTED;
    else if(myConfig->background == NORM_BAK)
        main_form[NORM_BAK].ob_state |= SELECTED;

    main_form[BM_PARALEL].ob_state = myConfig->parallel;
    
    redraw_window(my_window, NULL, BLACK_BAK, 0);
    redraw_window(my_window, NULL, WHITE_BAK, 0);
    redraw_window(my_window, NULL, NORM_BAK, 0);
    redraw_window(my_window, NULL, BM_PARALEL, 0);
    redraw_window(my_window, NULL, BM_TEXTURE, 0);
    redraw_window(my_window, NULL, INVERT, 0);
    
    if(main_form[M_RADIO_DIFFUSE].ob_state&SELECTED)
    {
        set_slider(&red_slider, red);
        set_slider(&green_slider, green);
        set_slider(&blue_slider, blue);
        set_slider(&m_str_slider, diffuse_strengh);
        set_slider(&m_hard_slider, diffuse_hard); 
    }
    else
    {
        set_slider(&red_slider, gred);
        set_slider(&green_slider, ggreen);
        set_slider(&blue_slider, gblue);
        set_slider(&m_str_slider, glanz_strengh);
        set_slider(&m_hard_slider, glanz_hard);
    }
}

void write_setting(BUMP_CONFIG *myConfig)
{
    myConfig->version = 0x0100;
    myConfig->red = red;
    myConfig->green = green;
    myConfig->blue = blue;
    myConfig->gred = gred;
    myConfig->ggreen = ggreen;
    myConfig->gblue = gblue;
    myConfig->diff_strength = diffuse_strengh;
    myConfig->gl_strength = glanz_strengh;
    myConfig->diff_pow = diffuse_hard;
    myConfig->gl_pow = glanz_hard;
    myConfig->bump_str = bm_strengh;
    
    if(main_form[BLACK_BAK].ob_state&SELECTED)
        myConfig->background = BLACK_BAK;
    else if(main_form[WHITE_BAK].ob_state&SELECTED)
        myConfig->background = WHITE_BAK;
    else if(main_form[NORM_BAK].ob_state&SELECTED)
        myConfig->background = NORM_BAK;

    myConfig->parallel = main_form[BM_PARALEL].ob_state;
    myConfig->texture = main_form[BM_TEXTURE].ob_state;
    myConfig->invert = main_form[INVERT].ob_state;
}
