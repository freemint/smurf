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

/* ******************************************** */
/*                                              */
/*                                              */
/* ******************************************** */



#include <tos.h>
#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <stdlib.h>
#include <ext.h>

#include "..\import.h"
#include "..\..\src\smurfine.h"


char *find_numpoints(char *data_pos, int *points);

char *get_poly_points(char *data_pos, int points, 
                        long *pointx, long *pointy, 
                        long *poly_point_number, long *poly_defs);


MOD_INFO module_info=
{
    "NeoN-Shapefile-Importer",
    0x0010,
    "Olaf Piesche",
/* Extensionen */
    "SHP","","","","","","","","","",

/* Slider */
    "","","","",
/* Editfelder */
    "","","","",
/* Checkboxen */
    "","","","",

/* Minima + Maxima */
/* Slider */
    0,0,
    0,0,
    0,0,
    0,0,
/* Edits */
    0,0,
    0,0,
    0,0,
    0,0,
/* Defaults */
    0,0,0,0,
    0,0,0,0,
    0,0,0,0
};




long number_of_points;      /* Punkte im ganzen File? */
long number_of_polys;       /* Wieviele Polygone? */ 


int imp_module_main(GARGAMEL *smurf_struct)
{
long *pointx;               /* X-Positionen */
long *pointy;               /* Y-Positionen */

long *poly_point_number;    /* Wieviele Punkte im Polygon n? */
long *poly_defs;            /* Polygondefinition */

char *file_data;
char *data_pos, *dp2;

int points;
int t;

pointx=Malloc(40000);
pointy=Malloc(40000);
poly_point_number=Malloc(20000);


file_data=smurf_struct->smurf_pic->pic_data;

if(smurf_struct->module_mode==MSTART)       /* geht's los, geht's los? */
{
    /* Kennung(en) prÅfen */
    if(strncmp(file_data, "RIP-2D-Shape(ascii)", 19) !=0 ) return(M_INVALID);
    /*
    if(strncmp(file_data+22, "Splineversion", 13) !=0 ) return(M_INVALID);
    if(strncmp(file_data+37, "Params: ", 8) !=0 ) return(M_INVALID);
    */

/*Goto_pos(1,1);*/

    /* Und los. */
    while(data_pos!=NULL)
    {
/*      printf("\nSuche nach numpoints...");    */
        dp2=data_pos;       /* Das ist nîtig, sonst funktionierts in der Schleife nicht */
        data_pos=find_numpoints(dp2, &points);      /* Poly-Punktzahl suchen */
/*      printf("\nPoly mit %i points gefunden, datapos=%li", points, data_pos); */
    
        if(data_pos==NULL) break;
    
        dp2=data_pos;       /* Das ist nîtig, sonst funktionierts in der Schleife nicht */
        data_pos=get_poly_points(dp2, points, 
                                pointx, pointy,
                                poly_point_number, poly_defs);
    }

    Mshrink(0, poly_point_number, number_of_polys*4L);
    Mshrink(0, pointx, number_of_points*4L);
    Mshrink(0, pointy, number_of_points*4L);

    /*
    printf("\n\n Polygone gesamt: %li", number_of_polys);
    printf("\n Punkte gesamt: %li", number_of_points);
    getch();
    */
}



smurf_struct->smurf_pic->pic_data=NULL;
smurf_struct->smurf_pic->depth=16;

strncpy(smurf_struct->smurf_pic->format_name, "NeoN Shape", 21);
smurf_struct->smurf_pic->pic_width=0;
smurf_struct->smurf_pic->pic_height=0;
smurf_struct->smurf_pic->x_coord=pointx;
smurf_struct->smurf_pic->y_coord=pointy;
smurf_struct->smurf_pic->number_of_polys=number_of_polys;
smurf_struct->smurf_pic->poly_point_number=poly_point_number;
smurf_struct->smurf_pic->this_is_a_vector=1;

return(M_PICDONE);  /* das wars. */
}




/*------------------------------------------------------------- */
/*  Liest alle Punkte eines Polygons aus dem File aus.          */
/*  ZurÅckgeliefert wird ein Zeiger auf das Ende des Polygons,  */
/*  poly_point_number und poly_defs werden gefÅllt.             */
/*------------------------------------------------------------- */
char *get_poly_points(char *data_pos, int points, 
                            long *pointx, long *pointy, 
                            long *poly_point_number, long *poly_defs)
{
char string[32];
int points_read;
char *endpos;
long xposition, yposition;

long len_of_number;
double orig_number;
int t;

number_of_polys++;  /* ein Polygon mehr... */
poly_point_number[number_of_polys]=points;  /* Punktzahl eintragen */

/*printf("\n Polygon %li, %i Punkte...", number_of_polys, points);*/

for(t=0; t<points; t++)
{
    endpos=strchr(data_pos, '\t');          /* Ende der Zahl suchen */
    len_of_number=endpos-data_pos;          /* LÑnge der Zahl ermitteln */
    strncpy(string, data_pos, len_of_number);
    orig_number=atof(string);
    pointx[number_of_points]=(long)(orig_number*(long)(2L<<20L));

/*printf("\nXpos: %.8f", orig_number);*/
    data_pos+=len_of_number;
    data_pos+=1;                            /* zur Y-Pos gehen */


    endpos=strchr(data_pos, '\t');          /* Ende der Zahl suchen */
    len_of_number=endpos-data_pos;          /* LÑnge der Zahl ermitteln */
    strncpy(string, data_pos, len_of_number);
    orig_number=atof(string);
    pointy[number_of_points]=(long)(orig_number*(long)(2L<<20L));

/*printf("\tYpos: %.8f", orig_number);*/
    data_pos+=len_of_number;
    data_pos+=4;                            /* tab, sp-Flag und \n Åberspringen */

    number_of_points++;
}

return(data_pos);
}





/*------------------------------------------------------------- */
/*  sucht nach einem "numpoints"-Tag, schreibt numpoints nach   */
/*  *points und liefert einen Zeiger auf die X-Koordinate des   */
/*  ersten Punktes zurÅck.                                      */
/*------------------------------------------------------------- */
char *find_numpoints(char *data_pos, int *points)
{
char pts[6];
char *endpos;
int len_of_number;

    data_pos=strstr(data_pos, "numpoints");
    
    if(data_pos==NULL) return(NULL);
    
    endpos=strstr(data_pos, "\n");  /* Ende der Zahl suchen */
    data_pos+=10;                   /* data_pos auf die Zahl ausrichten */
    
    len_of_number=endpos-data_pos;  /* LÑnge der Zahl ermitteln */
    
    strncpy(pts, data_pos, len_of_number);
    *(points) = atoi(pts);          /* Zahl Åbernehmen */
    
    data_pos+=len_of_number;        /* ans Ende der Zahl */
    data_pos+=4;                    /* 3 Returns+1 Tab Åberspringen. */

    return(data_pos);
}
