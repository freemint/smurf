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

/* =========================================================*/
/*                      Dummy-Exporter                      */
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include "..\..\sym_gem.h"
#include "..\import.h"
#include "..\..\src\smurfine.h"

char comp1[12]="Dies ist ein";
char comp2[12]="Dummy-Export";
char comp3[12]=" Zum Testen ";
char comp4[12]="des Export- ";
char comp5[12]=" Parts von  ";
char comp6[12]="SCHLUMPFINE ";


/* Infostruktur fÅr Hauptmodul */
MOD_INFO    module_info = {"Dummy-Exporter",
                        0x0010,
                        "Olaf",
                        "XXX", "YYY", "", "", "",
                        "", "", "", "", "",
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
                        0,0,0,0
                        };


MOD_ABILITY  module_ability = {
                        24, 0, 0, 0, 0, 0, 0, 0,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        0,
                        comp1,
                        comp2,
                        comp3,
                        comp4,
                        comp5,
                        comp6,
                        };


/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*              8 Bit, unkomprimiert                */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
    EXPORT_PIC *exp_pic;

    return(M_INVALID);      /* Hier macht Smurf nix. */
}
