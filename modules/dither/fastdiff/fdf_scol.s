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

*******************************************************
*       Farbsuche fÅr 1 Farbe
*   fÅr den FAST-DIFFUSION - Algorithmus
*******************************************************
IMPORT endseek, red,grn,blu, max_col

GLOBL seekcolor

seekcolor:
    movem.l a1-a4/d0-d6, -(sp)

    movea.l red, a1     ; rpal
    movea.l grn, a2     ; gpal  
    movea.l blu, a3     ; bpal

    clr.w   d0
    move.w  max_col, d0
    subq.w  #1, d0      ; wg. dbra

    move.l #32767, d7
    
loopcol:
    move.w (a1)+, d1            ; Rot-differenz
    sub.w   d4, d1
    bpl.b *+4               ;rok
    neg.w d1

    move.w (a2)+, d5            ; GrÅn-Differenz
    sub.w   d3, d5
    bpl.b *+4               ;gok
    neg.w d5

    move.w (a3)+, d6            ; Blau-Differenz
    sub.w   d2, d6
    bpl.b *+4               ;bok
    neg.w d6

    add.w   d6, d5              ; Gesamtdifferenz
    add.w   d5, d1
    
    cmp.w   d7, d1              ; Speichern?
    bge coldiffok               ;coldiffok
        move.w  d1, d7
        move.w  d0, a4
    coldiffok:

    dbra d0, loopcol

    clr.l d7
    move.w  max_col, d7     ; RÅckgabe-Index ausrechnen 
    subq.w  #1, d7
    sub.w   a4, d7

    movem.l (sp)+, a1-a4/d0-d6

    move.b  d7, (d6.w, a2)

    rts
/*  bra endseek*/




    
