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

GLOBL  get_standard_pix
GLOBL  getpix_std_1

*************************************************************** *
*   16 Standardformatpixel aus a0 in 16 Packed Pixels nach a1   *
*   schreiben.                                                  *
*   Deklaration:                                                *
*                                                               *
/*
void get_standard_pix(void *st_pic, void *buf16, int planes, long planelen);
*/
*************************************************************** *
get_standard_pix:
movem.l d3-d6/a2, -(sp)

subq.w #1, d0           ; planes-1 fÅr dbra
moveq   #15, d6


loop16:
    move.l  d0, d2      ; plane-counter 

    movea.l a0, a2      ; source kopieren

    moveq.l #0, d3
    moveq.l #1, d5

loopplane:
    move.w  (a2), d4

    btst    d6, d4
    beq *+4

    or.b    d5, d3

    adda.l  d1, a2          ; eine plane weiter
    lsl.b   #1, d5
    dbra d2, loopplane

    move.b d3, (a1)+

    dbra d6, loop16


movem.l (sp)+, d3-d6/a2
rts


*************************************************************** *
*   1 Standardformatpixel aus a0 in 1 Packed Pixel nach a1      *
*   schreiben.                                                  *
*   Deklaration:                                                *
*                                                               *
/*
void getpix_std_1(void *st_pic, int pixval, int planes, long planelen, in which);
                            a0          a1          d0             d1        d2
*/
*************************************************************** 

getpix_std_1:
movem.l d3-d4, -(sp)

    subq.w  #1,d0               ; planes - 1 fÅr dbra

    moveq.l #1,d4               ; sauberer als bset.l #0,d6
                                ; und kleiner und schneller ;-)

    moveq.l #7,d3               ; zu prÅfendes Bit
    sub.w   d2,d3               ; ist 7 - which

    clr.w   (a1)                ; Ziel nullen

looplane:
    ;-------------------------Schleifenanfang - 1 Pixel einer Plane fertigmachen
    btst.b  d3,(a0)             ; Bit d3 gesetzt?
    beq.b   *+4                 ; Nein? Setzen Åberspringen
    or.w    d4,(a1)             ; Bit in ziel setzen
    ;-------------------------zum Schleifenanfang

    adda.l  d1,a0               ; nÑchste Plane
    lsl.w   #1,d4               ; zu setzendes Bit verschieben

    dbra    d0,looplane

movem.l (sp)+, d3-d4
rts
