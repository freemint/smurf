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
#include "..\..\..\..\sym_gem.h"
#include "..\..\..\import.h"
#include "..\..\..\..\smuf\smurfine.h"
#include "f_filter.rsh"
#include "f_filter.rh"
 
#define Obj_Selected(a) (main_form[a].ob_state & SELECTED)

#define GET_Edit_Object(x) (main_form[x].ob_spec.tedinfo->te_ptext)

#include <math.h>

int do_it(GARGAMEL *smurf_struct);

void f_init_sliders(void);
void f_init_window(int mod_id);
void f_default_sliders(void);

void prev(SMURF_PIC *smurfpic, SMURF_PIC *preview);

char p1string[20] = "Picture";

/*------ Infostruktur fÅr Hauptprogramm -----*/
MOD_INFO    module_info=
    {
    "Freier Filter",                        /* Name des Moduls */
    0x0020,
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
                        0
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

SmurfMessage=smurf_struct->module_mode;

get_window = smurf_struct->f_module_window;
slider = smurf_struct->slider;

set_slider = smurf_struct->set_slider;

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
    mousex=smurf_struct->mousex;
    mousey=smurf_struct->mousey;

    switch(object)
    {
    case DO_IT:                 smurf_struct->module_mode = M_STARTED;
                                            return;
                                            break;

    case STRENGTH_SLIDE:    strength=slider(&strength_slider);
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
    char *mainpic, *offset;

    int matrix[5][5];
    long matrix_start;
    int matrix_anz;
    long matrix_offsets[25];
    char *matrix_pntr[25];
    int matrix_fak[25];

    int used_lb;    
    char *line_buf[5];
    long lb_len;
    char *lb_offset;
    char line_buf_full[5];
    char *line_buf_addr[5];

    int div,bias;
    long mul;
    char clip,invert;

    int fak_o,fak_f;
    long bpl;   
    int i,x,y;
        
    long red,green,blue;


    /*---Bilddaten auslesen----------------------*/
    picture = smurf_struct->smurf_pic;
    width   = picture->pic_width;
    height  = picture->pic_height;
    mainpic = picture->pic_data;

    bpl = width * 3L;
    lb_len = bpl - 12L;

    /*---Slider umrechnen----------------------*/
    
    fak_o = 256 * (100-strength) / 100;
    fak_f = 256 * strength / 100;

    /*---Einstellungen auslesen---------------------*/
    
    div = atoi(GET_Edit_Object(DIV));   
    if(div == 0) div = 1;   
    mul = 16384 / div;
    
    bias = atoi(GET_Edit_Object(BIAS));
    
    if(Obj_Selected(CLIP)) 
        clip = 1;
    else
        clip = 0;
                
    if(Obj_Selected(INVERT)) 
        invert = 1;
    else
        invert = 0;
        

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
      line_buf[i] = malloc(bpl);
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
    
    for (y=2; y<(height-2); y++)
    {
        busycount++;
        if(!(busycount & busycall)) smurf_struct->busybox((int)((busycount<<10) / busymax));
    
        if(1 == line_buf_full[used_lb]) /*---Buffer schreiben-----*/
        {
            memcpy(line_buf_addr[used_lb], line_buf[used_lb],lb_len);
            line_buf_full[used_lb] = 0;
        }
        
        offset = mainpic + (long)y*bpl+6L;
        line_buf_addr[used_lb] = offset;
        lb_offset = line_buf[used_lb];
        
        for(i=0; i<matrix_anz; i++)
        {
            matrix_pntr[_START 8VOIES 0000 0000 0000 DSP56000 4.1.1 

_DATA X 0000
000000 000000 000000 000000 000000 000000 000000 000000 
000000 000000 000000 000000 000000 000000 000000 000000 
_DATA P 0000
0AF080 000040 
_DATA P 0010
08DDEF 
_DATA P 0012
08DDEF 
_DATA P 0040
08442B 0500BB 08F4AC 004100 08F4A1 0001F8 0BAE24 08F4BE 
000000 08F4BF 003000 08F4A0 000001 240000 4C0000 200013 
300000 221400 44F400 001F40 06C400 000057 5E5800 5E5800 
65F400 001F40 05F424 003E7F 0465A4 08F4A0 000001 0AA980 
00005F 00FCB8 08F4AD 005800 0AF080 000068 0AA980 000066 
085A2B 0ADA2F 0000F0 0ADA0E 000076 0AD520 00006D 0AD500 
00006F 0AD520 000071 340000 65F400 001F40 0ADA4E 300000 
331000 45F400 008000 44F400 0003E8 209B00 06DA00 0000A3 
0AA980 000080 08462B 4658D0 0AA981 000084 08CC2B 227100 
44F400 000001 200023 200040 218600 0AA980 00008D 08472B 
2000F0 218700 2000F8 515900 06C600 00009C 0AA980 000096 
08472B 5059F0 218700 2000F8 515900 505900 044B13 44F400 
0003E8 0AA980 0000A1 0858AB 0AA823 74F400 000002 300000 
321000 229600 0BF080 0000D3 62F400 0003F8 22D400 0BF080 
0000E1 62F400 0007E0 22D400 0BF080 0000E1 62F400 000BC8 
22D400 0BF080 0000E1 045E16 62F400 000FB0 22D400 0BF080 
0000D3 62F400 001398 22D400 0BF080 0000E1 62F400 001780 
22D400 0BF080 0000E1 62F400 001B68 22D400 0BF080 0000E1 
045414 0AA803 0C0066 72F41B 0003E8 45F413 000000 55D800 
224C3B 21A400 46D800 06DA00 0000DF 47E220 2192B8 5F4C00 
00000C 72F41B 0003E8 45F413 000000 55D800 224C3B 21A400 
46D800 06DA00 0000EE 47E220 4FE4B8 219278 5F4C1B 00000C 
300013 44F400 001F40 06C400 0000F6 5E5800 5E5800 0C0066 
_SYMBOL X
period_vol           I 000000
data_sam             I 000010
_SYMBOL Y
buffer_sam           I 000000
_SYMBOL P
Reset                I 000040
eff                  I 000058
start                I 000066
start2               I 000068
begin                I 000076
load_sam             I 00009D
load_period          I 0000A4
Calcule              I 0000A4
Put_one_voice        I 0000D3
put_v                I 0000E0
Mix_one_voice        I 0000E1
mix_v                I 0000EF
coupe                I 0000F0
eff2                 I 0000F7

_END 0000
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
