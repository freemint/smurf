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

/*  ----------- Bump Mapping-Modul V1.0 --------------  */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include "..\..\..\..\sym_gem.h"
#include "..\..\..\import.h"
#include "..\..\..\..\src\smurfine.h"
#include "bumpmap.rsh"
#include "bumpmap.rh"


#define Obj_Selected(a) (main_form[a].ob_state & SELECTED)


#include <math.h>

int do_it(GARGAMEL *smurf_struct);

void f_init_sliders(void);
void f_init_window(int mod_id);
void f_default_sliders(void);

void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);


/*------ Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
    {
    "Bump Mapping V1.0",                        /* Name des Moduls */
    0x0100,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "Rot",
    "GrÅn",
    "Blau",
    "StÑrke %",
/* 4 CheckboxÅberschriften: */
    "",
    "",
    "",
    "",
/* 4 Edit-Objekt-öberschriften: */
    "",
    "",
    "",
    "",
/* min/max-Werte fÅr Slider */
    0,255,
    0,255,
    0,255,
    0,1000,
/* min/max fÅr Editobjekte */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaultwerte fÅr Slider, Check und Edit */
    255,255,255,100,
    0,0,0,0,
    0,0,0,0,
    };


/*--------------------- Was kann ich ? ----------------------*/
MOD_ABILITY  module_ability = {
                        24, 0, 0, 0, 0, 0, 0, 0,    /* Farbtiefen */
            /* Grafikmodi: */
                        FORM_PIXELPAK,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        FORM_BOTH,
                        0
                        };


char red, green, blue;
char gred, ggreen, gblue;
char diffuse_strengh, glanz_strengh;
char diffuse_hard, glanz_hard;
int  bm_strengh;

OBJECT *main_form;

WINDOW *my_window;

SLIDER  red_slider,
                green_slider,
                blue_slider,
                m_str_slider,
                m_hard_slider,
                bm_str_slider;


void (*set_slider)(SLIDER *sliderstruct, int value);  /* Funktion deklarieren */ 

/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
int (*get_window)(WINDOW *wind_struct);
int (*slider)(SLIDER *slider_struct);       /* Funktion deklarieren */
int module_id;
int SmurfMessage;
int t;
int next_edit;


int object, mousex, mousey;



SmurfMessage=smurf_struct->module_mode;

get_window = smurf_struct->services->f_module_window;
slider = smurf_struct->services->slider;
set_slider = smurf_struct->services->set_slider;

main_form=rs_trindex[BUMPMAIN];

/* Wenn das Modul aufgerufen wurde, */
if(SmurfMessage == MSTART)
{

    for(t=0; t<NUM_OBS; t++)
        rsrc_obfix(&rs_object[t], 0);


    module_id=smurf_struct->module_number;
    my_window=Malloc(sizeof(WINDOW));

    f_init_sliders();
    f_init_window(module_id);

    smurf_struct->wind_struct=my_window;

    get_window(my_window);

    f_default_sliders();
    
    smurf_struct->module_mode=M_WAITING;
    return;
}


else if(SmurfMessage==MBEVT)
{
    object=smurf_struct->event_par;
    mousex=smurf_struct->mousex;
    mousey=smurf_struct->mousey;

    form_button(main_form, object, 1, &next_edit);

    switch(object)
    {
    case DO_IT:         smurf_struct->module_mode = do_it(smurf_struct);
                                    return;
                                    break;

    case M_RED_SLIDE:       if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                                                    red=slider(&red_slider);
                                            else                gred=slider(&red_slider);               
                                            break;

    case M_GREEN_SLIDE: if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                                                    green=slider(&green_slider);
                                            else                ggreen=slider(&green_slider);
                                            break;

    case M_BLUE_SLIDE:  if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                                                    blue=slider(&blue_slider);
                                            else                gblue=slider(&blue_slider);
                                            break;
    case M_STRENGH_SLIDE:   if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                                                    diffuse_strengh=slider(&m_str_slider);
                                            else                glanz_strengh=slider(&m_str_slider);
                                            break;
    case M_HARD_SLIDE:  if(Obj_Selected(M_RADIO_DIFFUSE)) 
                                                                    diffuse_hard=slider(&m_hard_slider);
                                            else                glanz_hard=slider(&m_hard_slider);
                                            break;
    case BM_STRENGH_SLIDE:                  bm_strengh=slider(&bm_str_slider);
                                            break;
    case M_RADIO_DIFFUSE:           set_slider(&red_slider, red);
                                                        set_slider(&green_slider, green);
                                                        set_slider(&blue_slider, blue);
                                                        set_slider(&m_str_slider, diffuse_strengh);
                                                        set_slider(&m_hard_slider, diffuse_hard); 
                                            break;
    case M_RADIO_GLANZ:             set_slider(&red_slider, gred);
                                                        set_slider(&green_slider, ggreen);
                                                        set_slider(&blue_slider, gblue);
                                                        set_slider(&m_str_slider, glanz_strengh);
                                                        set_slider(&m_hard_slider, glanz_hard); 
                                            break;
    
    }

    smurf_struct->module_mode=M_WAITING;
    return;
}



/* Wenn das Modul gestartet wurde */
else if(SmurfMessage == MEXEC)
{
}






/* Wenn das Modul sich verpissen soll */
if(smurf_struct->module_mode==MTERM)
    {
    Mfree(my_window);
    smurf_struct->module_mode=M_EXIT;
    return;
    }

} /*ende*/




/*------------------------- BambmÑbbing ---------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture;
    int width, height;
    char *mainpic;
    
    char *greypic;
    char *coltab;
    char *wurztab;
    
    char *c_offset, *offset, *noffset, *y_offset, *y_goffset, *g_offset;
    
    int grey;
    int x,y;
    char red2, green2, blue2;
    char gred2, ggreen2, gblue2;
    int nred, ngreen, nblue;
    long n, t, bpl, gbpl;
    
    long col_size, coltab_size, coltab_size2;
    long glanz_size, glanztab_size;
    signed long glanz_x, glanz_y;
    long bump_fakt;

    signed int x_vekt, y_vekt, vekt; 
    char o_height; 
    signed long x_dif, y_dif;
    long abstand;


    long bs_diago;

    /*---Bilddaten auslesen----------------------*/
    picture = smurf_struct->smurf_pic;
    width   = picture->pic_width;
    height  = picture->pic_height;
    mainpic = picture->pic_data;

    bpl = width * 3L;
    gbpl = width;
    
    bs_diago = sqrt((long)width*width + (long)height*height); 

    /*---Slider umrechnen----------------------*/
    
    col_size = (long)((100L - diffuse_hard) * 6.0 + 50);
    glanz_size = (long)((100L - glanz_hard) / 2.04 + 1);
    
    red2   = (char)((long )red   * diffuse_strengh / 100);
    green2 = (char)((long )green * diffuse_strengh / 100);
    blue2  = (char)((long )blue  * diffuse_strengh / 100);
    
    gred2   = (char)((long )gred   * glanz_strengh / 100);
    ggreen2 = (char)((long )ggreen * glanz_strengh / 100);
    gblue2  = (char)((long )gblue  * glanz_strengh / 100);
    
    
    if(Obj_Selected(BM_SMOOTH))
            bump_fakt = (long )(bs_diago / 870F * (bm_strengh / 80F) * 1024F);
        else
            bump_fakt = (long )(bs_diago / 360F * (bm_strengh / 80F) * 1024F);
        
    
    /*-----------------------------------------------------*/ 
     
    coltab_size = (long)(bs_diago * col_size / 100L);
    coltab_size2 = coltab_size * coltab_size;
    glanztab_size = (long)(bs_diago * glanz_size/100L);

    glanz_x = (long )width/4;
    glanz_y = (long )height/4;
    
        
    /*---Speicher anfordern-----------------------------------*/
    if( (coltab = Malloc((long)coltab_size*3L)) == NULL) return(M_MEMORY);
    if( (greypic = Malloc((long)width * (long)height)) == NULL) 
    {
        Mfree(coltab);  
        return(M_MEMORY);
    }
    if(Obj_Selected(BM_SMOOTH))
        if( (wurztab = Malloc(256)) == NULL) 
        {   
            printf("Smooth !!!!");
            Mfree(coltab);
            Mfree(greypic);
            return(M_MEMORY);       
        }
    
    
    /*---Wurzeltabelle erzeugen---------------------------------*/
    
    if(Obj_Selected(BM_SMOOTH))
    {
        for(t=0;t<256;t++)
        {
            *(wurztab + (long)t) = (char)( (float)t * 0.707);
        }
    }
    
        
    /*---Graubild erzeugen------------------------------------*/
    
    offset = mainpic;
    noffset = greypic;
    if(Obj_Selected(HW_RADIO_GREY))
    {
        for(y=0; y<height; y++)
            for(x=0; x<width; x++)
            {
                grey = *(offset++);
                grey += *(offset++);
                grey += *(offset++);
                *(noffset++) = (char)(grey / 3);
            }
    }
    else
    {
        if(Obj_Selected(HW_RADIO_GREEN))                
            offset++;   
        else if(Obj_Selected(HW_RADIO_BLUE))
            offset+=2;  

        for(y=0; y<height; y++)
            for(x=0; x<width; x++)
            {
                *(noffset++) = *(offset++);
                offset +=2;
            }
    }
    
    
    /*---Farbtabelle generieren--------------------------------*/
        
        /*---Farbverlauf----------------------*/
    c_offset = coltab;
    for(t=coltab_size; t>=0; t--)
    {
        n = t*t;
        *(c_offset++) = (char)((long)red2   * (n) / coltab_size2);
        *(c_offset++) = (char)((long)green2 * (n) / coltab_size2);
        *(c_offset++) = (char)((long)blue2  * (n) / coltab_size2);
    }
    
        /*---Glanzpunkt-----------------------*/
    c_offset = coltab;
    for(t=0; t<glanztab_size; t++)
    {
        
        nred   =   (int) ((long)(*(c_offset + 0L))  
                     +         (long ) ( (long)gred2   * (glanztab_size-t) / glanztab_size));
        ngreen =   (int) ((long)(*(c_offset + 1L))  
                     +         (long ) ( (long)ggreen2 * (glanztab_size-t) / glanztab_size));
        nblue  =   (int) ((long)(*(c_offset + 2L))  
                     +         (long ) ( (long)gblue2  * (glanztab_size-t) / glanztab_size));
                            
        if(nred  >255) nred   = 255;
        if(ngreen>255) ngreen = 255;
        if(nblue >255) nblue  = 255;
        
        *(c_offset++) = (char)nred;
        *(c_offset++) = (char)ngreen;
        *(c_offset++) = (char)nblue;
    }   
    
    /*---Bump Mapping--------------------------------------------*/
    for (y=0; y<height; y++)
    {
        if(! (y%20)) smurf_struct->busybox(((long)y<<7L)/(long)height);
    
        y_offset = mainpic + y*bpl;
        y_goffset = greypic + y*gbpl;
        for(x=0; x<width;x++)
        {
            offset = y_offset + x*3L;
            g_offset = y_goffset + x;
            
            o_height = *(g_offset);
            
            x_vekt = 0;
            y_vekt = 0;
            
            if(x < (width-1))           
              x_vekt += o_height - *(g_offset + 1);
            if(y < (height-1))
                y_vekt += o_height - *(g_offset + gbpl);
            if(x > 0)   
                x_vekt -= o_height - *(g_offset - 1); 
            if(y > 0)   
                y_vekt -= o_height - *(g_offset - gbpl);
            
            if(Obj_Selected(BM_SMOOTH))
            {
                if( (x < (width-1)) & (y > 0))
                {
                    vekt = o_height - ( *(wurztab + ((long)*(g_offset - gbpl +1))));
                    x_vekt += vekt;
                    y_vekt -= vekt; 
                }
            
                if( (x < (width-1)) & (y < (height-1)) )
                {
                    vekt = o_height - (*(wurztab + ((long)*(g_offset + gbpl +1))));
                    x_vekt += vekt;
                    y_vekt += vekt; 
                }
                if( (x > 0) & (y < (height-1)) )
                {
                    vekt = o_height - (*(wurztab + ((long)*(g_offset + gbpl - 1))));
                    x_vekt -= vekt;
                    y_vekt += vekt; 
                }
                if( (x > 0) & (y > 0))
                {
                    vekt = o_height - (*(wurztab + ((long)*(g_offset - gbpl -1))));
                    x_vekt -= vekt;
                    y_vekt -= vekt; 
                }
            }
            
            x_vekt = ((signed long)x_vekt * bump_fakt) >>10;
            y_vekt = ((signed long)y_vekt * bump_fakt) >>10;
            
                    
            x_dif = (signed long)glanz_x - (x_vekt + x);
            y_dif = (signed long)glanz_y - (y_vekt + y);
            
            abstand = sqrt(x_dif*x_dif + y_dif*y_dif);
            
            if(abstand >= coltab_size) abstand = coltab_size-1;
            
            c_offset = coltab + abstand*3L;
            
            *(offset++) = *(c_offset++);
            *(offset++) = *(c_offset++);
            *(offset++) = *(c_offset++);
                    
        }
    }

        
    Mfree(coltab);
    Mfree(greypic);
    if(Obj_Selected(BM_SMOOTH))
            Mfree(wurztab);
    
    return(M_PICDONE);
}



/*  ------------------- Fensterstruktur init -------------------    */
void f_init_window(int mod_id)
{
    my_window->whandlem=0;
    my_window->module=mod_id;
    my_window->wnum=1;
    my_window->wx=100;
    my_window->wy=50;
    my_window->ww=main_form->ob_width;
    my_window->wh=main_form->ob_height;
    strncpy(my_window->wtitle, "Nohow's Bump Mapper", 40);
    my_window->resource_form=main_form;
    my_window->picture=NULL;

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

    green_slider.regler=M_GREEN_SLIDE;
    green_slider.schiene=M_GREEN_FHR;
    green_slider.rtree=main_form;
    green_slider.txt_obj=M_GREEN_EDIT;
    green_slider.min_val=0;
    green_slider.max_val=255;

    blue_slider.regler=M_BLUE_SLIDE;
    blue_slider.schiene=M_BLUE_FHR;
    blue_slider.rtree=main_form;
    blue_slider.txt_obj=M_BLUE_EDIT;
    blue_slider.min_val=0;
    blue_slider.max_val=255;

    m_str_slider.regler=M_STRENGH_SLIDE;
    m_str_slider.schiene=M_STRENGH_FHR;
    m_str_slider.rtree=main_form;
    m_str_slider.txt_obj=M_STRENGH_EDIT;
    m_str_slider.min_val=0;
    m_str_slider.max_val=100;

    m_hard_slider.regler=M_HARD_SLIDE;
    m_hard_slider.schiene=M_HARD_FHR;
    m_hard_slider.rtree=main_form;
    m_hard_slider.txt_obj=M_HARD_EDIT;
    m_hard_slider.min_val=0;
    m_hard_slider.max_val=100;
 
  bm_str_slider.regler=BM_STRENGH_SLIDE;
    bm_str_slider.schiene=BM_STRENGH_FHR;
    bm_str_slider.rtree=main_form;
    bm_str_slider.txt_obj=BM_STRENGH_EDIT;
    bm_str_slider.min_val=0;
    bm_str_slider.max_val=999;
}


/*--------------------- Slider auf defaults setzen -------------------*/
void f_default_sliders(void)
{
    red   = 255;
    green = 0;
    blue  = 0;
    diffuse_strengh = 100;
    diffuse_hard = 80;
    
    gred   = 255;
    ggreen = 255;
    gblue  = 255;
    glanz_strengh = 90;
    glanz_hard = 80;
    
    bm_strengh = 200;
    
    set_slider(&red_slider, red);
    set_slider(&green_slider, green);
    set_slider(&blue_slider, blue);
    set_slider(&m_str_slider, diffuse_strengh);
    set_slider(&m_hard_slider, diffuse_hard);
    
    set_slider(&bm_str_slider, bm_strengh);
}




/*------ Previewfunktion - wird von Smurf bei Klick aufs Preview aufgerufen.------- */
/* Diese Funktion sollte ein 64*64 Pixel groûes Preview des Bildes nach             */
/* Abarbeitung der Modulfunktion erzeugen. Das Preview muû in der SMURF_PIC-        */
/* Struktur *preview abgelegt werden. Dithering und Darstellung werden von Smurf    */
/* Åbernommen.                                                                      */
/* In prev_struct->smurf_pic liegt das unbearbeitete Bild. Das Modul muû nun        */
/* ein Previewbild in *preview erzeugen. Speicher wurde hierfÅr bereits von Smurf   */
/* angefordert. Das Preview (im Smurf-Standardformat) wird dann vom Hauptprogramm   */
/* fÅr die Screen-Farbtiefe gedithert und im Einstellformular dargestellt.          */

void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview){

    return;     /* Ich mach' noch nix. */
}

