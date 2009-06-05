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
 * Olaf Piesche, Christian Eyrich, Dale Russell and J”rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

#include <tos.h>
#include <string.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
/*#include <multiaes.h>*/
#include <aes.h>
#include <vdi.h>
#include <ext.h>
#include <math.h>
#include "demolib.h"
#include <errno.h>
#include "sym_gem.h"
#include "..\modules\import.h"
#include "smurf.h"
#include "smurf_st.h"
#include "smurfine.h"
#include "smurf_f.h"
#include "popdefin.h"

#include "xrsrc.h"
#include "globdefs.h"

#include "smurfobs.h"
#include "ext_obs.h"


/*------------------ Typedefs ----------------*/
typedef struct
{
	char cookie_id[4];
	long value;
} COOKIE;

typedef struct
{
	char rgb[3];
	char cmy[3];
	char hsv[3];
} COL;


typedef struct
{
	COL* (*open_colselect)(void);
} COLSEL_FUNCTIONS;



/*-------------------- Globale Variablen --------------*/
COLSEL_FUNCTIONS *csf;

void color_choose(void)
{
	unsigned long val;
	int ok;
	
	/* Farbauswahlcookie holen */
	ok = get_cookie('COLS', &val);
	
	if(!ok) 
	{
		Dialog.winAlert.openAlert(Dialog.winAlert.alerts[NO_COLS].TextCast, NULL, NULL, NULL, 1);
		return;
	}
	
	csf = (COLSEL_FUNCTIONS*)val;
	
	csf->open_colselect();

}