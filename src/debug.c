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

/************************************************************/
/* Routinen, die es erleichtern, ein Debugfile ausgeben zu	*/
/* lassen.													*/
/************************************************************/

#include <tos.h>
#include <ext.h>
#include <stdio.h>
#include <string.h>

#define SCREEN 1

void write_debug(char *message);

void write_debug(char *message)
{
	int file;

	long dummy;


	if(message == NULL)
	{
		dummy = Fcreate("DEBUG.LOG", 0);
		message = "Smurf Debug";
	}
	else
		dummy = Fopen("DEBUG.LOG", FO_WRITE);

	if(dummy >= 0)
	{
		file = (int)dummy;

		Fseek(0, file, 2);

		Fwrite(file, strlen(message), message);
		Fwrite(file, 2, "\r\n");

		Fclose(file);
	}
		
}