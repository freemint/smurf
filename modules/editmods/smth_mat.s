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

*    counter
*    Source Data
*    Dest Data
*    Offset table
*

GLOBL smooth_me

smooth_me:
    movem.l d3-d5/a2, -(sp)

move.l (a0)+, d0        ; counter
move.l (a0)+, a1        ; Source
move.l (a0)+, a2        ; Dest
move.l (a0)+, a0        ; Offsets

clr.l   d1
clr.l   d2
clr.l   d3
move.l  d0, d4
subq.l  #1, d0


*----------- Aufaddieren der Werte
loop:
    add.l   (a0)+, a1

    move.b (a1)+, d5        ; rot
    add.l   d5.b, d1.l
    move.b (a1)+, d5        ; grÅn
    add.l   d5.b, d2.l
    move.b (a1), d5         ; blau
    add.l   d5.b, d3.l

    dbra d0, loop

    *------ Durchschnitt berechnen und Daten schreiben
    divu.w  d4, d1
    move.b d1, (a2)+

    divu.w  d4, d2
    move.b d2, (a2)+

    divu.w  d4, d3
    move.b d3, (a2)

    movem.l (sp)+, d3-d5/a2


rts
