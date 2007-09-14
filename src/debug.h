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

/* Headerfile zu DEBUG.C */

#define __c_va_alist				, ...
#define __c_va_dcl
#define __c_va_list				va_list
#define __c_va_start(pvar, prev) 	va_start(pvar, prev)
#define __c_va_arg(pvar, type)	va_arg(pvar, type)
#define __c_va_end(pvar) 			va_end(pvar)


#ifdef DEBUG
/*	extern _BOOL is_DEBUGGING;

	#define DEBUG_MSG(x) ((void)(!is_DEBUGGING || (DebugMsg x, 0)))
*/
	#define DEBUG_MSG(x) ((void)((DebugMsg x, 0)))

#else
	#define DEBUG_MSG(x)
#endif

void DebugMsg( char *str, ... );
