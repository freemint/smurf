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

*******************************************************************
*
*   Makrodefinitionen fÅr die Assembler-Ditherroutinen
*
*   05.10.96 Olaf   
*
*******************************************************************


*-------------------------------------------------------------
*   Busybox-Aufruf
*-------------------------------------------------------------

MACRO   _Busybox

    move.l busybox, a6      ; busybox-adresse
    move.w 4(sp), d2        ; Wert...
    lsl.l #7, d2            ;...ausrechnen
    move.w height, d0
    divu.w d0,d2
    move.w #128, d0
    sub.w d2,d0
    movem.l d3-d7/a0-a6, -(sp)  ; register retten
    jsr (a6)                    ; busybox
    movem.l (sp)+, d3-d7/a0-a6  ; und regs zurÅck

ENDM



