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
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"


/* Infostruktur fÅr Hauptmodul */
MOD_INFO    module_info={"Speichertestmodul",
                        0x0010,
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
                        0,10
                        };
#define DEBUG           0
#define TRUE            1
#define FALSE           !TRUE
/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*      Modul zum Testen der Speicherverwaltung     */
/*      nach PEXEC 3/4 ohne unsauberen Sprung       */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
char *buffer;
long free_mem;

Goto_pos(0,0);
free_mem=(long)Mxalloc(-1, 2);
printf("\n-   Mod gestartet: %li Bytes.", free_mem);

buffer=Malloc(100000);
free_mem=(long)Mxalloc(-1, 2);
printf("\n-100K angefordert: %li Bytes.", free_mem);

Mfree(buffer);
free_mem=(long)Mxalloc(-1, 2);
printf("\n- und freigegeben: %li Bytes.", free_mem);
puts("\n\n Taste...");
getch();

return(M_INVALID);
}
