#ifndef _sym_gem_h_
#define _sym_gem_h_

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

/*  ---------------------------------------------------------------*/
/*  -------------------------- Globales ---------------------------*/
/*  ---------------------------------------------------------------*/
/* Scrollbares Listenfeld */
/* Ein Listenfeld muû folgendermaûen aufgebaut sein:        */
/*  Zuerst ein Parentobjekt fÅr die ListeneintrÑge aufziehen,   */
/*  z.B. eine BOX. In dieses dann max_entries TEXT-Objekte ein- */
/*  fÅgen, in denen bereits Platz fÅr den String ist (also z.B. */
/*  mit '_' auffÅllen). Die Text-Objcs mÅssen RADIOBUTTONS und  */
/*  eines davon selektiert sein. Neben dieses Objekt dann eine  */
/*  BOX als Parentobjekt fÅr den Slider (INDICATOR, SELECTED)   */
/*  aufziehen, darin eine BOX (INDICATOR) als Slider selbst.    */
/*  DarÅber und darunter je ein Boxchar (INDICATOR) mit einem   */
/*  Pfeil nach oben und nach unten setzen. Und das wars. Die    */
/*  Struktur mit den entsprechenden Werten fÅllen und bei Klick */
/*  auf eines der Objekte (Slider, Pfeile, Sliderparent, Listen-*/
/*  eintrag) dieses und die Struktur an f_listfield Åbergeben.  */
/*  Fertig. Die Funktion srollt und redrawed das Listenfeld     */
/*  selbstÑndig ud gibt den selektierten Listeneintrag          */
/*  (incl. Scrolloffset) zurÅck.                                */
typedef char**  LISTPTR;

/*#include <multiaes.h>*/
#include <aes.h>

#define FALSE   0
#define TRUE    1

typedef struct
{
    int number_entries;     /* Anzahl an EintrÑgen          */
    char *entry;            /* Zeiger auf ein Feld von EintrÑgen */
    int max_entries;        /* Anzahl an maximal auf einmal darzustellenden EintrÑgen */
    int parent_obj;         /* Index des Vaterobjektes der ListeneintrÑge */
    int slider_parent;      /* Parentobjekt des Sliders     */
    int slider_obj;         /* Sliderobjekt                 */
    int slar_up;            /* Slider-Arrow hoch            */
    int slar_dn;            /* Slider-Arrow runter          */
    int scroll_offset;      /* Scrolloffset der Liste       */
    char *autolocator;      /* Zeiger auf Autolocator-String */
    int auto_len;
} LIST_FIELD;


/* Popup-Struktur zu Verwaltung von Popups durch f_pop */
typedef struct
{
    int popup_tree;     /* Objektbaum des Popups */
    int item;               /* Item (=letzter Eintrag)      */
    OBJECT *display_tree;   /* Tree, in dems dargestellt werden soll */
    int display_obj;        /* Objekt in display_tree, bei dems dargestellt werden soll */
    int Cyclebutton;        /* Index des Cyclebuttons */
} POP_UP;



/*  ---------------------------------------------------------------*/
/*  ------------------- SYM_GEM Functions -------------------------*/
/*  ---------------------------------------------------------------*/

int     button_ev(OBJECT *tree);                                

int     f_numedit(int obj, OBJECT *tree, int deflt);                    
int     f_schiebe(int regler, int fhr, OBJECT *tree);           
int     f_pop(POP_UP *popup_struct, int mouseflag, int button, OBJECT *poptree);
void    f_drag(int obj, int parent, OBJECT *tree);          
void    f_hidetree(OBJECT *tree, int object);               
void    f_showtree(OBJECT *tree, int object);                   

int     get_selected_object(OBJECT *tree, int first_entry, int last_entry);

void f_generate_listfield(int uparrow, int dnarrow,int sliderparent, 
            int sliderobject, int listparent, char *listentries, 
            int num_entries, int max_entries, LIST_FIELD *listfield, int autoloc);

int f_listfield(long *pwindow, int klick_obj, int key_scancode, LIST_FIELD *lfstruct);

#endif