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

/*  -------------Freier 5x5 Filter-Modul V0.5 ------------------    */
/*          FÅr SMURF Bildkonverter, 26.04.96           */
#include <tos.h>
#include <aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <ext.h>
#include <math.h>

#include "..\..\..\..\sym_gem.h"
#include "..\..\..\import.h"
#include "..\..\..\..\src\smurfine.h"
#include "f_filter.rsh"
#include "f_filter.rh" 

#define Obj_Selected(a) (main_form[a].ob_state & SELECTED)
#define GET_Edit_Object(x) (main_form[x].ob_spec.tedinfo->te_ptext)

void Set_Edit_Object(OBJECT *obj, int wert);


void Calc_filtervalues(void);
int do_it(GARGAMEL *smurf_struct);

/*-- Globale deklarationen fÅr 'Calc_filtervalues' --------*/
    int f_div,f_bias;
    long f_mul;
    char f_clip,f_invert;

    int fak_o,fak_f;

    long red,green,blue;
    char *offset;
/*----------------------------------------------------------*/

void f_init_sliders(void);
void f_init_window(int mod_id);
void f_default_sliders(void);

void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);

char p1string[20] = "Picture";


void *Load_Filter(char *path);
int Save_Filter(char *path);

char inpath[255];
char *file;

typedef struct
{
    char  das_kennung[8];
    char    das_matrix_name[20];        /* 19 Zeichen + Nullbyte Filtername */
    int     das_matrix[5][5];
    int das_div;
    int das_bias;
    char das_strength;
    char das_unknown1;
    char das_clipnv;
    char das_unknown2;

} DAS_FILTER;




/*------ Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
    {
    "5x5 Matrix Filter",                       /* Name des Moduls */
    0x0040,
    "Jîrg Dittmer",                                 /* Autor */
    "","","","","","","","","","",  /* 10 Extensionen fÅr Importer */
/* 4 SliderÅberschriften: max 8 */
    "",
    "",
    "",
    "",
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
    0,0,
    0,0,
    0,0,
    0,0,
/* min/max fÅr Editobjekte */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaultwerte fÅr Slider, Check und Edit */
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    1,  /* Anzahl der Bilder */
    p1string /* Bezeichnung fÅr Bilder */
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
                        0 /* Extra Flag */ 
                        };


unsigned long busycount, busymax, busycall;

int  strength;

OBJECT *main_form;

WINDOW *my_window;

SLIDER  strength_slider;


void (*set_slider)(SLIDER *sliderstruct, int value);  /* Funktion deklarieren */ 

/*---------------------------  FUNCTION MAIN -----------------------------*/
void edit_module_main(GARGAMEL *smurf_struct)
{
int (*get_window)(WINDOW *wind_struct);
int (*slider)(SLIDER *slider_struct);       /* Funktion deklarieren */
static int module_id;
int SmurfMessage;
int t;
int next_edit;
int object, mousex, mousey;

DAS_FILTER  *WF5_filter;

SmurfMessage=smurf_struct->module_mode;

get_window = smurf_struct->services->f_module_window;
slider = smurf_struct->services->slider;

set_slider = smurf_struct->services->set_slider;

main_form = rs_trindex[FREE_FILTER];

/* Wenn das Modul aufgerufen wurde -----------------------------*/
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

/* Wenn Button-Event kommt ----------------------------------*/
else if(SmurfMessage==MBEVT)
{
    object=smurf_struct->event_par[0];

    switch(object)
    {
    case DO_IT:                 smurf_struct->module_mode = M_STARTED;
                                            return;
                                        /*  break;  */

    case STRENGTH_SLIDE:    strength=slider(&strength_slider);
                                            break;
    
    case SAVE_FILTER:    if(0 > Save_Filter("D:\\*.WF5"))
                                            {
                                                /* Fehler beim Speichern */
                                            }
                                            break;
                                             
    
    case LOAD_FILTER:   file = Load_Filter("D:\\*.WF5");
                                            if((long)file < 0L) 
                                            {
                                                break;
                                            }
                                            WF5_filter = (DAS_FILTER*)file;
                                            
                                            Set_Edit_Object(&main_form[MATRIX_00], WF5_filter->das_matrix[0][0]);
                                            Set_Edit_Object(&main_form[MATRIX_10], WF5_filter->das_matrix[0][1]);
                                            Set_Edit_Object(&main_form[MATRIX_20], WF5_filter->das_matrix[0][2]);
                                            Set_Edit_Object(&main_form[MATRIX_30], WF5_filter->das_matrix[0][3]);
                                            Set_Edit_Object(&main_form[MATRIX_40], WF5_filter->das_matrix[0][4]);
                                            
                                            Set_Edit_Object(&main_form[MATRIX_01], WF5_filter->das_matrix[1][0]);
                                            Set_Edit_Object(&main_form[MATRIX_11], WF5_filter->das_matrix[1][1]);
                                            Set_Edit_Object(&main_form[MATRIX_21], WF5_filter->das_matrix[1][2]);
                                            Set_Edit_Object(&main_form[MATRIX_31], WF5_filter->das_matrix[1][3]);
                                            Set_Edit_Object(&main_form[MATRIX_41], WF5_filter->das_matrix[1][4]);
                                            
                                            Set_Edit_Object(&main_form[MATRIX_02], WF5_filter->das_matrix[2][0]);
                                            Set_Edit_Object(&main_form[MATRIX_12], WF5_filter->das_matrix[2][1]);
                                            Set_Edit_Object(&main_form[MATRIX_22], WF5_filter->das_matrix[2][2]);
                                            Set_Edit_Object(&main_form[MATRIX_32], WF5_filter->das_matrix[2][3]);
                                            Set_Edit_Object(&main_form[MATRIX_42], WF5_filter->das_matrix[2][4]);
                                            
                                            Set_Edit_Object(&main_form[MATRIX_03], WF5_filter->das_matrix[3][0]);
                                            Set_Edit_Object(&main_form[MATRIX_13], WF5_filter->das_matrix[3][1]);
                                            Set_Edit_Object(&main_form[MATRIX_23], WF5_filter->das_matrix[3][2]);
                                            Set_Edit_Object(&main_form[MATRIX_33], WF5_filter->das_matrix[3][3]);
                                            Set_Edit_Object(&main_form[MATRIX_43], WF5_filter->das_matrix[3][4]);
                                            
                                            Set_Edit_Object(&main_form[MATRIX_04], WF5_filter->das_matrix[4][0]);
                                            Set_Edit_Object(&main_form[MATRIX_14], WF5_filter->das_matrix[4][1]);
                                            Set_Edit_Object(&main_form[MATRIX_24], WF5_filter->das_matrix[4][2]);
                                            Set_Edit_Object(&main_form[MATRIX_34], WF5_filter->das_matrix[4][3]);
                                            Set_Edit_Object(&main_form[MATRIX_44], WF5_filter->das_matrix[4][4]);

                                            Set_Edit_Object(&main_form[DIV], WF5_filter->das_div);
                                            Set_Edit_Object(&main_form[BIAS], WF5_filter->das_bias);
                                            
                                            strncpy(main_form[FILTER_NAME].ob_spec.tedinfo->te_ptext, WF5_filter->das_matrix_name, 19);
                                                                                
                                            if(WF5_filter->das_clipnv & 0x80)
                                                main_form[CLIP].ob_state |= SELECTED;
                                            else
                                                main_form[CLIP].ob_state &= ~SELECTED;
                                            
                                            if(WF5_filter->das_clipnv & 0x40)
                                                main_form[INVERT].ob_state |= SELECTED;
                                            else
                                                main_form[INVERT].ob_state &= ~SELECTED;
                                            
                                            strength = WF5_filter->das_strength;
                                            set_slider(&strength_slider, strength);
                                        
                                            smurf_struct->services->redraw_window(my_window, NULL, 0, 0);
                                        
                                            Mfree(WF5_filter);                                          
                                            break;
    }

    smurf_struct->module_mode=M_WAITING;
    return;
}


/* Wenn das Modul gestartet wurde ----------------------------*/
else if(SmurfMessage == MEXEC)
{
    smurf_struct->module_mode = do_it(smurf_struct);
    return;
}


/* Wenn das Modul sich verpissen soll --------------------------*/
if(SmurfMessage == MTERM)
    {
    Mfree(my_window);
    smurf_struct->module_mode=M_EXIT;
    return;
    }

} /*---ende-----------------------------------------------------*/
/*---------------------------------------------------------------*/



/*------------------------- Filtern ------------------------*/
int do_it(GARGAMEL *smurf_struct)
{
    SMURF_PIC *picture;
    int width, height;
    char *mainpic;

    int matrix[5][5];
    long matrix_start;
    int matrix_anz;
    long matrix_offsets[25];
    char *matrix_pntr[25];
    int matrix_fak[25];
    int matrix_koord_x[25];
    int matrix_koord_y[25];

    int used_lb;    
    char *line_buf[5];
    long lb_len;
    char *lb_offset;
    char line_buf_full[5];
    char *line_buf_addr[5];

    char *mp_offset;

    long bpl;   
    int i,x,y;
        


    /*---Bilddaten auslesen----------------------*/
    picture = smurf_struct->smurf_pic;
    width   = picture->pic_width;
    height  = picture->pic_height;
    mainpic = picture->pic_data;

    bpl = (long)width * 3L;
    lb_len = bpl;

    /*---Slider umrechnen----------------------*/
    
    
    fak_o = 256L * (255-strength) / 255;
    fak_f = 256L * strength / 255;


    /*---Einstellungen auslesen---------------------*/
    
    f_div = atoi(GET_Edit_Object(DIV)); 
    if(f_div == 0) f_div = 1;   
    f_mul = 16384 / f_div;
    
    f_bias = atoi(GET_Edit_Object(BIAS));
    
    if(Obj_Selected(CLIP)) 
        f_clip = 1;
    else
        f_clip = 0;
                
    if(Obj_Selected(INVERT)) 
        f_invert = 1;
    else
        f_invert = 0;
        

    /*--- Matrix einlesen ---------------------------*/
    
    matrix[0][0] = atoi(GET_Edit_Object(MATRIX_00)); 
    matrix[1][0] = atoi(GET_Edit_Object(MATRIX_10)); 
    matrix[2][0] = atoi(GET_Edit_Object(MATRIX_20)); 
    matrix[3][0] = atoi(GET_Edit_Object(MATRIX_30)); 
    matrix[4][0] = atoi(GET_Edit_Object(MATRIX_40)); 
    
    matrix[0][1] = atoi(GET_Edit_Object(MATRIX_01)); 
    matrix[1][1] = atoi(GET_Edit_Object(MATRIX_11)); 
    matrix[2][1] = atoi(GET_Edit_Object(MATRIX_21)); 
    matrix[3][1] = atoi(GET_Edit_Object(MATRIX_31)); 
    matrix[4][1] = atoi(GET_Edit_Object(MATRIX_41)); 
    
    matrix[0][2] = atoi(GET_Edit_Object(MATRIX_02)); 
    matrix[1][2] = atoi(GET_Edit_Object(MATRIX_12)); 
    matrix[2][2] = atoi(GET_Edit_Object(MATRIX_22)); 
    matrix[3][2] = atoi(GET_Edit_Object(MATRIX_32)); 
    matrix[4][2] = atoi(GET_Edit_Object(MATRIX_42)); 
    
    matrix[0][3] = atoi(GET_Edit_Object(MATRIX_03)); 
    matrix[1][3] = atoi(GET_Edit_Object(MATRIX_13)); 
    matrix[2][3] = atoi(GET_Edit_Object(MATRIX_23)); 
    matrix[3][3] = atoi(GET_Edit_Object(MATRIX_33)); 
    matrix[4][3] = atoi(GET_Edit_Object(MATRIX_43)); 
    
    matrix[0][4] = atoi(GET_Edit_Object(MATRIX_04)); 
    matrix[1][4] = atoi(GET_Edit_Object(MATRIX_14)); 
    matrix[2][4] = atoi(GET_Edit_Object(MATRIX_24)); 
    matrix[3][4] = atoi(GET_Edit_Object(MATRIX_34)); 
    matrix[4][4] = atoi(GET_Edit_Object(MATRIX_44)); 
    
    matrix_anz = 0;
    matrix_start = -2L*3L + -2L*bpl;
    for(y=0; y<5; y++)
    {
        for(x=0; x<5; x++)
        {
            if(matrix[x][y] != 0)
            {
                matrix_offsets[matrix_anz] = matrix_start +
                                                                        (long)x*3L + (long)y*bpl;
                matrix_fak[matrix_anz] = matrix[x][y];
                matrix_koord_x[matrix_anz] = x-2;
                matrix_koord_y[matrix_anz] = y-2;
                matrix_anz++;
            }
        }
    }
    
    if(matrix_anz == 0) /* Es gibt nix zu tun!!! */
    {
        return(M_MODERR);
    }
        
    
    /*---Speicher anfordern-----------------------------------*/

    i = 0;
    do
    {
      line_buf[i] = malloc(lb_len);
    } while ((++i < 5) && (line_buf[i-1] != NULL));

    if(i<5)
    {
        for(x=0; x<i; x++)
        {
            free(line_buf[x]);
        }
        return(M_MEMORY);
    }
    
    
    /*--- BusyBox Vorberechnungen ----------------------*/
    busycount = 0;
    busycall =  15;
    busymax = height-4;
    busymax = (busymax<<10) /127;
    
    
    /*---Filter Routine--------------------------------------------*/
    
    for(i=0;i<5;i++) line_buf_full[i] = 0;
    used_lb = 0;
    
/*--- Y-Schleife oben -----------------------------------------*/   
    for(y=0; y<2; y++)
    {
        offset = mainpic + (long)y*bpl;
        line_buf_addr[used_lb] = offset;
        lb_offset = line_buf[used_lb];
    
        for(i=0; i<matrix_anz; i++)
        {
            matrix_pntr[i] = offset + matrix_offsets[i];
        }   
    
        for(x=0; x<width; x++)
        {
            if( (matrix_koord_y[0] + y) < 0) 
                mp_offset = offset; 
            else
            {
                if( (matrix_koord_x[0] +x) < 0)
                    mp_offset = offset;
                else
                {
                    if( (matrix_koord_x[0] + x) >= width)
                        mp_offset = offset;
                    else
                    {
                        mp_offset = matrix_pntr[0];
                    }
                }
            }   
            matrix_pntr[0] += 3;
        
            red   = *(mp_offset++);
            green = *(mp_offset++);
            blue  = *(mp_offset++);
        
            for(i=1; i<matrix_anz; i++)
            {
                if( (matrix_koord_y[0] + y) < 0) 
                    mp_offset = offset; 
                else
                {
                    if( (matrix_koord_x[0] +x) < 0)
                        mp_offset = offset;
                    else
                    {
                        if( (matrix_koord_x[0] + x) >= width)
                            mp_offset = offset;
                        else
                        {
                            mp_offset = matrix_pntr[0];
                        }
                    }
                }
                    
                matrix_pntr[i] += 3;
                    
                red   += *(mp_offset++);
                green += *(mp_offset++);
                blue  += *(mp_offset++);
            }
            
            Calc_filtervalues();
            
            /*--- in Buffer schreiben ---------------------------------*/
            *lb_offset++ = red;
            *lb_offset++ = green;
            *lb_offset++ = blue;        
        }
        line_buf_full[used_lb] = 1;
        used_lb++;
        if(used_lb > 4) used_lb = 0;
    }
    
    
/*--- Main Y-Schleife -----------------------------------------*/   
    for (y=2; y<(height-2); y++)
    {
        busycount++;
        if(!(busycount & busycall)) smurf_struct->services->busybox((int)((busycount<<10) / busymax));
    
        if(1 == line_buf_full[used_lb]) /*---Buffer schreiben-----*/
        {
            memcpy(line_buf_addr[used_lb], line_buf[used_lb],lb_len);
            line_buf_full[used_lb] = 0;
        }
        
        offset = mainpic + (long)y*bpl;
        line_buf_addr[used_lb] = offset;
        lb_offset = line_buf[used_lb];
        
        for(i=0; i<matrix_anz; i++)
        {
            matrix_pntr[i] = offset + matrix_offsets[i];
        }
        
        for(x=0; x<2; x++)
        {
            if( (matrix_koord_x[0] + x) < 0) 
                mp_offset = offset; 
            else
                mp_offset = matrix_pntr[0];
            matrix_pntr[0] += 3;
        
            red   = *(mp_offset++);
            green = *(mp_offset++);
            blue  = *(mp_offset++);
        
            for(i=1; i<matrix_anz; i++)
            {
                if( (matrix_koord_x[i] + x) < 0) 
                    mp_offset = offset; 
                else
                    mp_offset = matrix_pntr[i];
                matrix_pntr[i] += 3;
                    
                red   += *(mp_offset++);
                green += *(mp_offset++);
                blue  += *(mp_offset++);
            }
            
            Calc_filtervalues();
            
            /*--- in Buffer schreiben ---------------------------------*/
            *lb_offset++ = red;
            *lb_offset++ = green;
            *lb_offset++ = blue;
            
        }
        
        
/*--- Main_Part---------------------------------------------------*/
        for(x=2; x<(width-2);x++)
        {
            red   = *(matrix_pntr[0]++) * matrix_fak[0];
            green = *(matrix_pntr[0]++) * matrix_fak[0];
            blue  = *(matrix_pntr[0]++) * matrix_fak[0];
            
            for(i=1; i<matrix_anz; i++)
            {
                red   += *(matrix_pntr[i]++) * matrix_fak[i];           
                green += *(matrix_pntr[i]++) * matrix_fak[i];           
                blue  += *(matrix_pntr[i]++) * matrix_fak[i];
            }           
            
            Calc_filtervalues();
            
            /*--- in Buffer schreiben ---------------------------------*/
            *lb_offset++ = red;
            *lb_offset++ = green;
            *lb_offset++ = blue;
        }
        /*------------------------------------------------------------*/
        
            for(x=(width-2); x<width; x++)
        {
            if( (matrix_koord_x[0] + x) >= width) 
                mp_offset = offset; 
            else
                mp_offset = matrix_pntr[0];
            matrix_pntr[0] += 3;
        
            red   = *(mp_offset++);
            green = *(mp_offset++);
            blue  = *(mp_offset++);
        
            for(i=1; i<matrix_anz; i++)
            {
                if( (matrix_koord_x[i] + x) >= width) 
                    mp_offset = offset; 
                else
                    mp_offset = matrix_pntr[i];
                matrix_pntr[i] += 3;
                    
                red   += *(mp_offset++);
                green += *(mp_offset++);
                blue  += *(mp_offset++);
            }
            
            Calc_filtervalues();
            
            /*--- in Buffer schreiben ---------------------------------*/
            *lb_offset++ = red;
            *lb_offset++ = green;
            *lb_offset++ = blue;    
        }
            
        line_buf_full[used_lb] = 1;
        used_lb++;
        if(used_lb > 4) used_lb = 0;
    }
    
    /*--- Y-Schleife unten -----------------------------------------*/  
    for(y=(height-2); y<height; y++)
    {
        if(1 == line_buf_full[used_lb]) /*---Buffer schreiben-----*/
        {
            memcpy(line_buf_addr[used_lb], line_buf[used_lb],lb_len);
            line_buf_full[used_lb] = 0;
        }
        
        offset = mainpic + (long)y*bpl;
        line_buf_addr[used_lb] = offset;
        lb_offset = line_buf[used_lb];
    
        for(i=0; i<matrix_anz; i++)
        {
            matrix_pntr[i] = offset + matrix_offsets[i];
        }   
    
        for(x=0; x<width; x++)
        {
            if( (matrix_koord_y[0] + y) >= height) 
                mp_offset = offset; 
            else
            {
                if( (matrix_koord_x[0] +x) < 0)
                    mp_offset = offset;
                else
                {
                    if( (matrix_koord_x[0] + x) >= width)
                        mp_offset = offset;
                    else
                    {
                        mp_offset = matrix_pntr[0];
                    }
                }
            }   
            matrix_pntr[0] += 3;
        
            red   = *(mp_offset++);
            green = *(mp_offset++);
            blue  = *(mp_offset++);
        
            for(i=1; i<matrix_anz; i++)
            {
                if( (matrix_koord_y[0] + y) < 0) 
                    mp_offset = offset; 
                else
                {
                    if( (matrix_koord_x[0] +x) < 0)
                        mp_offset = offset;
                    else
                    {
                        if( (matrix_koord_x[0] + x) >= width)
                            mp_offset = offset;
                        else
                        {
                            mp_offset = matrix_pntr[0];
                        }
                    }
                }
                    
                matrix_pntr[i] += 3;
                    
                red   += *(mp_offset++);
                green += *(mp_offset++);
                blue  += *(mp_offset++);
            }
            
            Calc_filtervalues();
            
            /*--- in Buffer schreiben ---------------------------------*/
            *lb_offset++ = red;
            *lb_offset++ = green;
            *lb_offset++ = blue;        
        }
        line_buf_full[used_lb] = 1;
        used_lb++;
        if(used_lb > 4) used_lb = 0;
    }
    
    
    
    
    for(i=0; i<5; i++) /*--restliche Buffer zurÅckschreiben-----*/
    {
        if(line_buf_full[i] == 1)
            memcpy(line_buf_addr[i], line_buf[i],lb_len);
    }
        
    for(i=0; i<5; i++)
        free(line_buf[i]);
        
    return(M_PICDONE);
}

/*-- Funktion um die Werte einzurechnen (Div, Bias, Clip, Invert) -*/
void Calc_filtervalues(void)
{
            /*---DIVisor einrechen--------------*/      
            if(f_div != 1)
            {
                red   = (red  *f_mul)>>14;
                green = (green*f_mul)>>14;
                blue  = (blue *f_mul)>>14;
            }
            
            /*---BIAS einrechen ---------------------------*/
            if(f_bias != 0)
            {
                red   += f_bias;
                green += f_bias;
                blue  += f_bias;
            }
            
            /*---Cliping ---------------------------*/
            if(f_clip == 1)
            {
                if(red<0)               red = 0;
                else if(red>255)    red = 255;
                
                if(green<0)                 green = 0;
                else if(green>255)  green = 255;
                
                if(blue<0)              blue = 0;
                else if(blue>255) blue = 255;
            }
            else    /*-------- kein cliping ---------*/
            {
                red   = (char)red;
                green = (char)green;
                blue  = (char)blue;
            }
            
            /*----------- Invertieren ----------------------*/ 
            if(f_invert == 1)
            {   
                red = ~red;
                green = ~green;
                blue = ~blue;
            }   
                
            /*--- mit Original filtern --------------------------------*/
            red   =  (*(offset++) * fak_o + red   * fak_f) >>8;
            green =  (*(offset++) * fak_o + green * fak_f) >>8;
            blue  =  (*(offset++) * fak_o + blue  * fak_f) >>8;
    return;
}


/*  ------------------- Fensterstruktur init -------------------    */
void f_init_window(int mod_id)
{
    memset(my_window, 0, sizeof(WINDOW));
    my_window->whandlem=0;
    my_window->module=mod_id;
    my_window->wnum=1;
    my_window->wx=100;
    my_window->wy=50;
    my_window->ww=main_form->ob_width;
    my_window->wh=main_form->ob_height;
    my_window->editob = DIV;
    strncpy(my_window->wtitle, "Freier 5x5 Filter", 40);
    my_window->resource_form=main_form;
    my_window->picture=NULL;

}



/*--------------------- Slider initialisieren -------------------*/

void f_init_sliders(void)
{
    strength_slider.regler=STRENGTH_SLIDE;
    strength_slider.schiene=STRENGTH_FHR;
    strength_slider.rtree=main_form;
    strength_slider.txt_obj=STRENGTH_EDIT;
    strength_slider.min_val=0;
    strength_slider.max_val=255;
    strength_slider.window=my_window;
}


/*--------------------- Slider auf defaults setzen -------------------*/
void f_default_sliders(void)
{
    strength = 255;
    
    set_slider(&strength_slider, strength);
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



void *Load_Filter(char *path)
{
    void *infile;

    char filename[15];
    char *namepos;
    char loadpath[255];
    int exitbutton;
    char headline[] = "Matrix laden";
    
    char kennung[] = "WINFILT5";
    char file_kennung[15];
    int k_cmp;
    
    int err;
    long len;
    
    long back;
    int handle;
    
    if(strlen(inpath) == 0)
        strcpy(inpath,path);
    
    strcpy(filename,"");
    
    len = sizeof(DAS_FILTER);
    
    err = fsel_exinput(inpath, filename, &exitbutton, headline); 

    if(err == 0)
    {
        return((void *)-1);
    }
    if(exitbutton == 0)
    {
        return((void *)-2);
    }
    else
    {
        strcpy(loadpath,inpath);
        namepos = strrchr(loadpath, '\\');
        strcpy(namepos+1,filename);

        back = Fopen(loadpath,FO_READ);
        if(back < 0L)
        {
            return((void *)-3);
        }
        else
        {
            handle = (int)back;
            Fread(handle,8,file_kennung);
            
            k_cmp = strncmp(kennung, file_kennung, 8);
            if(k_cmp != 0)
            {
                return((void *)-4);
            }
            else
            {
                infile = Malloc(len);
                if(infile == NULL)
                {
                    return((void *)-5);
                }               
                else
                {
                    Fseek(0,handle,0); 
                    if(len != Fread(handle,len,infile))
                    {
                        Fclose(handle);
                        return((void *)-6);
                    }
                    else
                    {
                        Fclose(handle);
                    }
                }
            }
        }
    }

    return(infile);
}

void Set_Edit_Object(OBJECT *obj, int wert)
{
    char string[4];
    
    itoa(wert, string, 10);
    
    strncpy(obj->ob_spec.tedinfo->te_ptext, string, 4);
}

int Save_Filter(char *path)
{
    DAS_FILTER *outfile;

    char filename[15];
    char *namepos;
    char loadpath[255];
    int exitbutton;
    char headline[] = "Matrix sichern";
    
    char kennung[] = "WINFILT5";
    
    int err;
    long len;
    
    long back;
    int handle;
    
    if(strlen(inpath) == 0)
        strcpy(inpath,path);
    
    strcpy(filename,"");
    
    len = sizeof(DAS_FILTER);
    
    err = fsel_exinput(inpath, filename, &exitbutton, headline); 

    if(err == 0)
    {
        return(-1);
    }
    if(exitbutton == 0)
    {
        return(-2);
    }
    else
    {
        strcpy(loadpath,inpath);
        namepos = strrchr(loadpath, '\\');
        strcpy(namepos+1,filename);

        outfile = Malloc(len);
        if(outfile == NULL)
        {
            return(-5);
        }               
        else
        {
            strncpy(outfile->das_kennung, kennung, 8);

            strncpy(outfile->das_matrix_name, "___________________", 20);
            strncpy(outfile->das_matrix_name, GET_Edit_Object(FILTER_NAME), 20);

            outfile->das_matrix[0][0] = atoi(GET_Edit_Object(MATRIX_00));
            outfile->das_matrix[0][1] = atoi(GET_Edit_Object(MATRIX_10));
            outfile->das_matrix[0][2] = atoi(GET_Edit_Object(MATRIX_20));
            outfile->das_matrix[0][3] = atoi(GET_Edit_Object(MATRIX_30));
            outfile->das_matrix[0][4] = atoi(GET_Edit_Object(MATRIX_40));

            outfile->das_matrix[1][0] = atoi(GET_Edit_Object(MATRIX_01));
            outfile->das_matrix[1][1] = atoi(GET_Edit_Object(MATRIX_11));
            outfile->das_matrix[1][2] = atoi(GET_Edit_Object(MATRIX_21));
            outfile->das_matrix[1][3] = atoi(GET_Edit_Object(MATRIX_31));
            outfile->das_matrix[1][4] = atoi(GET_Edit_Object(MATRIX_41));

            outfile->das_matrix[2][0] = atoi(GET_Edit_Object(MATRIX_02));
            outfile->das_matrix[2][1] = atoi(GET_Edit_Object(MATRIX_12));
            outfile->das_matrix[2][2] = atoi(GET_Edit_Object(MATRIX_22));
            outfile->das_matrix[2][3] = atoi(GET_Edit_Object(MATRIX_32));
            outfile->das_matrix[2][4] = atoi(GET_Edit_Object(MATRIX_42));
            
            outfile->das_matrix[3][0] = atoi(GET_Edit_Object(MATRIX_03));
            outfile->das_matrix[3][1] = atoi(GET_Edit_Object(MATRIX_13));
            outfile->das_matrix[3][2] = atoi(GET_Edit_Object(MATRIX_23));
            outfile->das_matrix[3][3] = atoi(GET_Edit_Object(MATRIX_33));
            outfile->das_matrix[3][4] = atoi(GET_Edit_Object(MATRIX_43));
            
            outfile->das_matrix[4][0] = atoi(GET_Edit_Object(MATRIX_04));
            outfile->das_matrix[4][1] = atoi(GET_Edit_Object(MATRIX_14));
            outfile->das_matrix[4][2] = atoi(GET_Edit_Object(MATRIX_24));
            outfile->das_matrix[4][3] = atoi(GET_Edit_Object(MATRIX_34));
            outfile->das_matrix[4][4] = atoi(GET_Edit_Object(MATRIX_44));
            
            outfile->das_div  = atoi(GET_Edit_Object(DIV));
            outfile->das_bias = atoi(GET_Edit_Object(BIAS));
            
            outfile->das_strength = strength;
            
            outfile->das_unknown1 = 0;
            
            if(Obj_Selected(CLIP))
                outfile->das_clipnv = 0x80;
            else
                outfile->das_clipnv = 0;
            
            if(Obj_Selected(INVERT))
                outfile->das_clipnv |= 0x40;    
            
            outfile->das_unknown2 = 0;  
                
            
            
            back = Fcreate(loadpath,0);
            if(back < 0L)
            {
                return(-3);
            }
            else
            {
                handle = (int)back;
                
                if(len != Fwrite(handle, len, outfile)) 
                    err = -7;
                else
                    err = 0;
                
                Fclose(handle);
            }
            
            Mfree(outfile);
        }
    }
    
    return(err);
}
