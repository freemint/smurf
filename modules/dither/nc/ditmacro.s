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
*                           DITMACRO.S
*
*   Makrodefinitionen fÅr die Assembler-Ditherroutinen
*
*   05.10.96 Olaf   
*
*******************************************************************



*************************************************************** *
*   16 Standardformatpixel aus a0 in 16 Packed Pixels nach a1   *
*   schreiben.                                                  *
*   Deklaration:                                                *
*                                                               *
/*

void get_standard_pix(void *st_pic, void *buf16, int planes, long planelen);

*/
*************************************************************** *
MACRO ReadPixel_standard

    LOCAL loop16_std, loopplane_std
    IMPORT scalex_inc;

    subq.w #1, d0           ; planes-1 fÅr dbra
    moveq   #15, d6


    loop16_std:
        move.l  d0, d2      ; plane-counter 
    
        movea.l a0, a2      ; source kopieren
    
        moveq.l #0, d3
        moveq.l #0, d5
    
    loopplane_std:
        move.w  (a2), d4
    
        btst d6, d4
        beq.b *+4
    
        bset.l  d5.w, d3
    
        adda.l  d1, a2          ; eine plane weiter
        addq.w  #1, d5
        dbra d2, loopplane_std
    
        move.b d3, (a6)+
    
    
        dbra d6, loop16_std


ENDM






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
    movem.l d0-d7/a0-a6, -(sp)  ; register retten
    jsr (a6)                    ; busybox
    movem.l (sp)+, d0-d7/a0-a6  ; und regs zurÅck

ENDM



