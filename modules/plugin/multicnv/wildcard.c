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

/*----------------------------------------------------------------
    Wildcard-Parser
    in gehirnzellenfressender Kleinstarbeit von Olaf am 23.03.98
    geschrieben. Am 24.03. morgens dann gebugfixt und fertiggestellt.
    Diese Routinen haben die Kraft der zwei Herzen:
    die Funktionen strn_q_cmp und strn_q_cmp, die Strings mit anderen
    vergleichen, und zwar unter BerÅcksichtigung beliebig vieler ?s
    jeweils im String wild.
-----------------------------------------------------------------*/
/*----------------------------------------------------------------
    noch'n Wilcard-Parser
    viel kÅrzer, Åbersichtlicher und gehirnzellenschonender von
    Christian am 22.7.98 erdacht und geschrieben. Aber frage mich
    bitte niemand, wie ich darauf kam - keine Ahnung.
-----------------------------------------------------------------*/
/*----------------------------------------------------------------
    und noch einer - hoffentlich der letzte
    noch kÅrzer, Åbersichtlicher und wenn man nichts gegen
    Rekursionen hat auch eingÑngiger
    Christian um den 20.7.99 herum erdacht und geschrieben
-----------------------------------------------------------------*/
#include <tos.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <ext.h>

void tokenize_wildcards(char *string);
char check_wildcard(char *filename, char *wildcard);
char *mystrtok(char *s, char c);


/* tokenize_wildcards----------------------------------------------------
    Schreibt die durch Kommata getrennten Wildcards in string als einzelne
    Wildcard-Strings ins Feld char *wildcards[].
    ---------------------------------------------------------------------*/
void tokenize_wildcards(char *string)
{
    char *curr_token;

    extern char *wildcards[64];
    extern int num_of_wildcards;


    /*
     * Mittels mystrtok wird der String in seine durch , getrennten Tokens unterteilt.
     * Dabei werden die , durch Nullbytes ersetzt, so daû die LÑnge der einzelnen
     * Tokens ohne weiteres mit strlen ausgemessen werden kann.
     */
    while((curr_token = mystrtok(string, ',')) != NULL)
    {
        wildcards[num_of_wildcards] = malloc(strlen(curr_token) + 1);   /* LÑnge ausmessen und anfordern */

        strcpy(wildcards[num_of_wildcards], curr_token);

        num_of_wildcards++;
    }

    return;
} /* tokenize_wildcards */


/* check_wildcard -----------------------------------------------
    öberprÅft, ob die Wildcard wildcard auf den Dateinamen filename
    zutrifft (0) oder nicht (1). Es sind beliebig viele '*' und '?'
    in beliebiger Kombination zulÑssig.
    ------------------------------------------------------------*/
char check_wildcard(char* filename, char* wildcard)
{
    while(*wildcard)
    {
        if((*wildcard == *filename) || (*wildcard == '?' && *filename))
        {
            wildcard++;
            filename++;
        }
        else
            if(*wildcard == '*')
            {
                for(;;)
                {
                    if(check_wildcard(filename, wildcard + 1))
                        return(1);

                    if(*filename)
                        filename++;
                    else
                        return(0);
                }
            }
            else
                return(0);
    }

    return(*filename == '\0');
} /* check_wildcard */
