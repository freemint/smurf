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

#include <tos.h>
#include <stddef.h>
#include "..\..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\smurf\smurf_st.h"
#include "..\..\..\smurf\smurf.h"
#include "..\..\..\smurf\smurfine.h"
#include "..\..\..\smurf\globdefs.h"

void change_object(WINDOW *window, int object, int status, int redraw)
{
    extern SERVICE_FUNCTIONS *services;

    OBJECT *obj = window->resource_form;
    
    if(status==SEL) obj[object].ob_state|=SELECTED;
    else if(status==UNSEL)  obj[object].ob_state &= ~SELECTED;
    else if(status==DISABLED)   obj[object].ob_state |= DISABLED;
    else if(status==ENABLED)    obj[object].ob_state &= ~DISABLED;
    
    if(redraw)  services->redraw_window(window, NULL, object, 0);
}


/* -------------------------- File laden --------------------------------*/
char *fload(char *Path, int header)
{
    char *buf;
    int fh, attrib = 0;
    long dummy;
    extern long f_len;


    /* Mit einem Trick testen, ob die Datei vorhanden ist */
    if(Fattrib(Path, 0, attrib) >= 0)
    {
        if((dummy = Fopen(Path, FO_READ)) >= 0)     /* Datei îffnen */
        {
            fh = (int)dummy;

            f_len = Fseek(0, fh, 2) - header;           /* FilelÑnge ermitteln */
            Fseek(header, fh, 0);                       /* und Dateizeiger wieder auf Anfang */

            if((buf = Malloc(f_len)) == NULL)
            form_alert(1, "[3][Nicht genÅgend Speicher zum|Laden der Datei!][ Oh! ]");
            else
            {
                dummy = Fread(fh, f_len, buf);

                if(dummy < f_len)
                {
                    form_alert(1, "[3][Lesefehler beim Laden|der Datei!][ Oh! ]");
                    Mfree(buf);
                    buf = NULL;
                }
            }
            Fclose(fh);
        }
    }
    else
        buf = NULL;

    return(buf);
} /* fload */


int depth_button2depth(int button)
{
    if(button==EXP_D24)         return(24);
    else if(button==EXP_D16)    return(16);
    else if(button==EXP_D8)     return(8);
    else if(button==EXP_D4)     return(4);
    else if(button==EXP_D2)     return(2);
    else if(button==EXP_D1)     return(1);

    return(1);
}


/* So stelle ich mir die Implementierung der Bibliotheksfunktion strtok() vor. */
/* Mit dem Unterschied, daû weitere Aufrufe nicht mit NULL in s aufgerufen */
/* werden muû, sondern mystrtok() folgende Aufrufe selbst erkennt. */
/* Ein Problem ist allerdings, daû diese Erkennung nur funktioniert, wenn ein */
/* String zuende tokenisiert wurde. Ein vorheriger Abbruch bringt alles durcheinander. */
/* Deshalb gibt es die Mîglichkeit des manuellen Inits mit NULL als erstem Parameter. */
char *mystrtok(char *s, char c)
{
    static char *t = NULL;
    char *back;


    if(s == NULL)                   /* manueller Init */
    {
        t = NULL;
        return(NULL);
    }

    if(t == NULL)                   /* Nur am Stringanfang der Fall */
        t = s;

    if(*t == '\0')                  /* Wenn der String zu Ende tokenisiert wurde, */
    {                               /* also nach dem letzten gÅltigen Aufruf. */
        t = NULL;                   /* Und das, damit nachfolgende Strings auch noch */
        return(NULL);               /* gemacht werden kînnen. */
    }

    s = t;

    while(*s)
    {
        if(*s == c)
        {
            *s++ = '\0';
            break;
        }
        else
            s++;
    }

    back = t;                       /* Eingangsstring wegsichern */
    t = s;                          /* und neuen Anfang setzen */

    return(back);
} /* mystrtok */


/* Umwandlung der Buchstaben eines Strings in Kleinbuchstaben */
/* unter BerÅcksichtigung der deutschen Umlaute */
void lower(char *string)
{
    while(*string)
    {
        if(*string >= 'A' && *string <= 'Z')
            *string += 32;
        else
            if(*string == 'é')
                *string -= 10;
            else
                if(*string == 'ô')
                    *string -= 5;
                else
                    if(*string == 'ö')
                        *string -= 25;

        string++;
    }

    return;
} /* lower */
