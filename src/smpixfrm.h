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

/*----------------------------------------------------------------------------------*/
/* smurfinterne Pixelformate fÅr Bitmaps zur Benutzung der NVDI 5 Rasterfunktion    */
/*----------------------------------------------------------------------------------*/

#define PX_SM1S     (PX_PACKED + PX_1COMP + PX_USES1 + PX_1BIT)
#define PX_SM2S     (PX_PLANES + PX_1COMP + PX_USES2 + PX_2BIT)
#define PX_SM4S     (PX_PLANES + PX_1COMP + PX_USES4 + PX_4BIT)
#define PX_SM8S     (PX_PLANES + PX_1COMP + PX_USES8 + PX_8BIT)
#define PX_SM8P     (PX_PACKED + PX_1COMP + PX_USES8 + PX_8BIT)
#define PX_SM16     (PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES16 + PX_16BIT)
#define PX_SM24     (PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES24 + PX_24BIT)
