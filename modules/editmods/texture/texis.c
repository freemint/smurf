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

/*              FÅr SMURF Bildkonverter, 19.10.95           */

#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdi.h>
#include <aes.h>
#include <screen.h>
#include <math.h>
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\smurf\smurfine.h"

#include "texture.rsh"
#include "texture.rh"


#define Clip(a,b)   (a>b? 0 : a)
#define Abs(a)      (a<0? -a : a)


/*----- Typendefinitionen -------------*/
    typedef struct
    {
        char *map;
        int map_wid, map_hgt;
        int xscale, yscale;
        int turbulence;
        char turbulence_mode;
        char map_mode;
    } TEXTURE_DEF;




/*----- Modulinterne Funktionen -------------*/
    void open_window(GARGAMEL *smurf_struct);
    void load_mapfile(void);
    void make_sliders(void);
    void make_popups(void);
    void start_texture(GARGAMEL *smurf_struct);

    void do_turbulence(char *pixel, int *xp, int *yp);
    void do_scaling(int *xp, int *yp);
    void fill_colourtable(void);



/*---------- Blinddeklarationen fÅr Smurf-Funktionen -----------------*/
    int     (*get_window)(WINDOW *wind);        
    int     (*popup)(POP_UP *popup_struct, int mouseflag, int button, OBJECT *poptree);         
    int     (*slider)(SLIDER *slider_struct);       
    void    (*set_slider)(SLIDER *sliderstruct, int value);
    void    (*listfield)(OBJECT *tree, int klick_obj, LIST_FIELD *lfstruct);
    void    (*busybox)(int lft);
    char *  (*file_load)(char *ppath, char *ltext);
    BITBLK *prev(GARGAMEL *prev_struct);        /* Previewfunktion */

    void (*deselect_popup)(WINDOW *wind, int ob1, int ob2);




/*--------------------- globale Variablen ------------------------*/
    WINDOW *mwindow;
    OBJECT *maintree;
    char *map, *mapdata;
    int map_height, map_width;
    int scaled_wid, scaled_hgt;
    char *redtab, *greentab, *bluetab;
    int par1, par2, par3, par4;
    float fpar1, fpar2, fpar3, fpar4;
    int mmpar1=0, mmpar2=0, mmpar3=0, mmpar4=0;

    int inter_startup=1;

/*-------------------- Strukturen --------------------------------*/
    SLIDER red_slider;
    SLIDER green_slider;
    SLIDER blue_slider;
    
    SLIDER xscale_slider;
    SLIDER yscale_slider;

    SLIDER turb_slider;
    
    POP_UP  turbulence_popup;


    TEXTURE_DEF texture_define;


/* Infostruktur fÅr Hauptmodul */
MOD_INFO    module_info={
                        "ProTexiS",
                        0x0030,
                        "Olaf Piesche",
                        "","","","","",
                        "","","","","",
                        "Slider 1",
                        "Slider 2",
                        "Slider 3",
                        "Slider 4",
                        "Checkbox 1",
                        "Checkbox 2",
                        "Checkbox 3",
                        "Checkbox 4",
                        "Edit 1",
                        "Edit 2",
                        "Edit 3",
                        "Edit 4",
                        0,128,
                        0,128,
                        0,128,
                        0,128,
                        0,10,
                        0,10,
                        0,10,
                        0,10,
                        0,0,0,0,
                        0,0,0,0,
                        0,0,0,0,
                        1,
                        "","","","","",""
                        };



MOD_ABILITY  module_ability = {
                        24, 0, 0, 0, 0, 0, 0, 0,
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

/* **************************************************************** */
/*                                                                  */
/*  -----------------------  FUNCTION MAIN -----------------------  */
/*                                                                  */
/* **************************************************************** */
void edit_module_main(GARGAMEL *smurf_struct)
{


/*---------- Anmeldevariablen ---------------------------------------*/
    int     mod_id;

/*---------- Modulvariablen ---------------------------------------*/
    int SmurfMessage;
    int Button, Mousex, Mousey;

    int t, back;





/* Message auslesen ----------------------------------------    */
    SmurfMessage=smurf_struct->module_mode;

/* Funktionen holen ----------------------------------------    */
    get_window=smurf_struct->services->f_module_window;   
    popup=smurf_struct->services->popup;  
    slider=smurf_struct->services->slider;    
    set_slider=smurf_struct->services->set_slider;    
    busybox=smurf_struct->services->busybox;  
    deselect_popup=smurf_struct->services->deselect_popup;    

/* Resource holen ------------------------------------------    */
    maintree=(OBJECT *)rs_trindex[TEX_MAIN];






/* -----------------------------------------------------------  */
/* Wenn das Modul zum ersten Mal gestartet wurde,               */
/* -----------------------------------------------------------  */


    if(SmurfMessage==MSTART)
    {
    
    /* Resource fix ----------------------------------------    */
        for (t=0; t<NUM_OBS; t++)
            rsrc_obfix (&rs_object[t], 0);
    
        mod_id=smurf_struct->module_number;         /* Welche ID habe ich? */
    
        open_window(smurf_struct);
        make_sliders();
        make_popups();
        fill_colourtable();

        back=get_window(mwindow);

        set_slider(&xscale_slider, 100);
        set_slider(&yscale_slider, 100);
        set_slider(&turb_slider, 50);

        texture_define.xscale=100;
        texture_define.yscale=100;
        texture_define.turbulence=50;
        
        
        if(back==-1) smurf_struct->module_mode=M_EXIT;
        else smurf_struct->module_mode=M_WAITING;
        
        load_mapfile();

        return;
    }







/* -----------------------------------------------------------  */
/*              Buttonevent im Modulfenster                     */
/* -----------------------------------------------------------  */
    else if(SmurfMessage==MBEVT)
    {
        Button=smurf_struct->event_par[0];
        Mousex=smurf_struct->mousex;
        Mousey=smurf_struct->mousey;
    
    

        switch(Button)
        {
            case XS_SLIDER: texture_define.xscale = slider(&xscale_slider); break;
            case YS_SLIDER: texture_define.yscale = slider(&yscale_slider); break;
            case TURB_SLIDER:   texture_define.turbulence = slider(&turb_slider); break;
            
            case TURB_PB:
            case TURB_CB:   texture_define.turbulence_mode = popup(&turbulence_popup, 0, Button, rs_trindex[TURB_POPUP]);
                            deselect_popup(mwindow, TURB_PB, TURB_CB);
                            break;

            case START:     smurf_struct->module_mode=M_STARTED;
                            return;
                            break;
        }
    
    
        smurf_struct->module_mode=M_WAITING;
        return;
    }
    



/* -----------------------------------------------------------  */
/*                      Modul kann loslegen                     */
/* -----------------------------------------------------------  */
    else if(SmurfMessage==MEXEC)
    {
        start_texture(smurf_struct);
        smurf_struct->module_mode=M_PICDONE;
        return;
    }








/* -----------------------------------------------------------  */
/*      Mterm empfangen - Speicher freigeben und beenden        */
/* -----------------------------------------------------------  */
    else if(SmurfMessage==MTERM)
    {
        Mfree(map);
        Mfree(mwindow);
        Mfree(redtab);
        Mfree(greentab);
        Mfree(bluetab);
        smurf_struct->module_mode=M_EXIT;
        return;
    }






}   /* ********************** Main Ende *********************** */









/* **************************************************************** */
/*                          Textur erzeugen                         */
/* **************************************************************** */
void start_texture(GARGAMEL *smurf_struct)
{
    int map_xpos=0, map_ypos;
    int cur_map_xpos=0, cur_map_ypos=0;
    int x,y;
    int width, height;
    char red, green, blue;
    char mappixel=0, oldp;
    char *pic_data, *mapcopy;
    long xoffset, yoffset;
    int stx, ox, nx=0, xdist, greyd;
    char pix1, pix2;


    width = smurf_struct->smurf_pic->pic_width;
    height = smurf_struct->smurf_pic->pic_height;
    pic_data = smurf_struct->smurf_pic->pic_data;
    mapcopy=texture_define.map;
    scaled_wid=map_width;
    scaled_hgt=map_height;

    for(y=0; y<height; y++)
    {
        if(!(y&7)) busybox(((long)y<<7L)/(long)height);
        
        map_xpos=0;
        nx=0;
        inter_startup=1;

        yoffset=(long)y*(long)width*3L;         

        for(x=0; x<width; x++)
        {
            cur_map_xpos=map_xpos;
            cur_map_ypos=map_ypos;
            
            mappixel=0;

            do_scaling(&cur_map_xpos, &cur_map_ypos);

            mappixel=*(mapcopy+cur_map_xpos+(cur_map_ypos*map_width));


            if(cur_map_xpos == nx || inter_startup==1)
            {
                inter_startup=0;
                nx=cur_map_xpos+1;

                pix1=mappixel;
                pix2=*(mapcopy+nx+(cur_map_ypos*map_width));
                greyd=pix2-pix1;
                
                xdist=texture_define.xscale/100;

                stx=greyd/xdist;
            }

            pix1+=stx;
            mappixel=pix1;


            do_turbulence(&mappixel, &cur_map_xpos, &cur_map_ypos); 

            map_xpos++;
            if(map_xpos>scaled_wid)
            { 
                map_xpos-=scaled_wid;
                inter_startup=1;
            }
            
            xoffset=x+x+x;
            *(pic_data+xoffset+yoffset)=redtab[mappixel];
            *(pic_data+xoffset+yoffset+1)=greentab[mappixel];
            *(pic_data+xoffset+yoffset+2)=bluetab[mappixel];
        }
        

        map_ypos++;
        if(map_ypos>scaled_hgt) map_ypos-=scaled_hgt;
    }






}







/* **************************************************************** */
/*                          Turbulenz                               */
/* **************************************************************** */
void do_turbulence(char *pixel, int *xp, int *yp)
{
int xpos, ypos;
long mapoff;
int wid, hgt, owid, ohgt;
int newpixel;
char *map_data;
double two_pi;

    two_pi=2*M_PI;

    xpos=*xp;
    ypos=*yp;
    newpixel=*pixel;

    owid=texture_define.map_wid;
    ohgt=texture_define.map_hgt;
    wid=scaled_wid;
    hgt=scaled_hgt;
    
    map_data=texture_define.map;

    if( texture_define.turbulence_mode==TURB_ZOOM )
    {
        xpos *= 2;
        ypos *= 2;
        
        if(xpos>owid) xpos-=owid;
        if(ypos>ohgt) ypos-=ohgt;

        mapoff=(long)ypos*(long)owid;
    
        newpixel += *(map_data+xpos+mapoff);
        newpixel>>=1; 
        *pixel=newpixel;
    }

    else if( texture_define.turbulence_mode==TURB_SIN )
    {
        xpos += sin(fpar1)*(owid*texture_define.turbulence/100);
        ypos += sin(fpar1)*(ohgt*texture_define.turbulence/100);

        fpar1+=0.001;
        if(fpar1 > two_pi) fpar1=0;

        if(xpos>owid) xpos-=owid;
        if(ypos>ohgt) ypos-=ohgt;
        if(xpos<0) xpos+=owid;
        if(ypos<0) ypos+=ohgt;
        
        mapoff=(long)ypos*(long)owid;
        
        newpixel = *(map_data+xpos+mapoff);
        *pixel=newpixel;
    }




}



/* **************************************************************** */
/*                              Scaling                             */
/* **************************************************************** */
void do_scaling(int *xp, int *yp)
{
int xpos, ypos;
int xsc, ysc;
int owid, ohgt;

    xsc=texture_define.xscale;
    ysc=texture_define.yscale;
    
    owid=texture_define.map_wid;
    ohgt=texture_define.map_hgt;

    xpos=(*xp);
    ypos=(*yp);
    scaled_wid=(int)(((long)texture_define.map_wid*(long)xsc)/100L);
    scaled_hgt=(int)(((long)texture_define.map_hgt*(long)ysc)/100L);

    xpos*=100L;
    ypos*=100L;

    xpos /= xsc;
    ypos /= ysc;

    if(xpos>owid)
    {
        xpos-=owid;
        inter_startup=1;
    }
    if(ypos>ohgt) ypos-=ohgt;

    *xp=xpos;
    *yp=ypos;
}





/* **************************************************************** */
/*                      Farbtabelle fÅllen                          */
/* **************************************************************** */
void fill_colourtable(void)
{
int t;

    redtab=Malloc(257);
    greentab=Malloc(257);
    bluetab=Malloc(257);

    for(t=0; t<255; t++)
    {
        redtab[t] = t;
        greentab[t] = t;
        bluetab[t] = t;
    }

}




/* **************************************************************** */
/*                          Mapfile laden                           */
/* **************************************************************** */
void load_mapfile(void)
{
    char mappath[]="F:\\DALEMAN\\SMURF\\MOD_DEVL\\EDITMODS\\TEXTURE\\RANDOM.MAP";
    int handle;
    long f_len;
    
    handle=Fopen(mappath, FO_READ);
    
    if(handle)
    {
        f_len=Fseek(0, handle, 2);
        Fseek(0, handle, 0);
        map=Malloc(f_len);
        Fread(handle, f_len, map);
        Fclose(handle);
    }
    else form_alert(1, "[0][   Konnte Map   |   nicht laden!!   ][ Uups. ]");
    
    texture_define.map_wid = map_width=*(map+18);
    texture_define.map_hgt = map_height=*(map+20);
    texture_define.map=mapdata = map+24;
}






/* **************************************************************** */
/*                  Sliderstrukturen vorbereiten                    */
/* **************************************************************** */
void make_sliders(void)
{
    xscale_slider.regler=XS_SLIDER;
    xscale_slider.schiene=XS_PAR; 
    xscale_slider.rtree=maintree;
    xscale_slider.txt_obj=XS_A;
    xscale_slider.min_val=1;
    xscale_slider.max_val=1000;
    xscale_slider.window=mwindow;

    yscale_slider.regler=YS_SLIDER;
    yscale_slider.schiene=YS_PAR; 
    yscale_slider.rtree=maintree;
    yscale_slider.txt_obj=YS_A;
    yscale_slider.min_val=1;
    yscale_slider.max_val=1000;
    yscale_slider.window=mwindow;
    
    turb_slider.regler=TURB_SLIDER;
    turb_slider.schiene=TS_PAR; 
    turb_slider.rtree=maintree;
    turb_slider.txt_obj=TS_A;
    turb_slider.min_val=0;
    turb_slider.max_val=100;
    turb_slider.window=mwindow;
}




/* **************************************************************** */
/*                  Popupstrukturen vorbereiten                     */
/* **************************************************************** */
void make_popups(void)
{
    turbulence_popup.popup_tree=TURB_POPUP;     /* Objektbaum-Index des Popups */
    turbulence_popup.item=TURB_NONE;            /* Item (=letzter Eintrag)     */
    turbulence_popup.display_tree=maintree;     /* Tree, in dems dargestellt werden soll */
    turbulence_popup.display_obj=TURB_PB;       /* Objekt in display_tree, bei dems dargestellt werden soll */
    turbulence_popup.Cyclebutton=TURB_CB;       /* Index des Cyclebuttons */
}




/* **************************************************************** */
/*          Window-Struktur fÅllen und Fenster anfordern            */
/* **************************************************************** */
void open_window(GARGAMEL *smurf_struct)
{
char    wt[]="Procedural Textures in Smurf V0.3";

    /* Window-Struktur fÅllen ------------------------------    */
        mwindow = (WINDOW*)Malloc(sizeof(WINDOW));
        smurf_struct->wind_struct = mwindow;
        strncpy(mwindow->wtitle, wt, 40);

        mwindow->whandlem = 0;                          /* evtl. Handle lîschen */
        mwindow->module =   smurf_struct->module_number;            /* Welche ID habe ich? */
        mwindow->wnum =     1;
        mwindow->wx =   -1;
        mwindow->wy =   -1;
        mwindow->ww =   maintree->ob_width;
        mwindow->wh =   maintree->ob_height;
        mwindow->resource_form =    maintree;
        mwindow->picture =  0;
        mwindow->xoffset =  0;
        mwindow->yoffset =  0;
        mwindow->pic_xpos = 0;
        mwindow->pic_ypos = 0;
        mwindow->editob =   0;
        mwindow->nextedit = 0;
        mwindow->editx =    0;
}







BITBLK *prev(GARGAMEL *prev_struct)
{
    return(0);
}
