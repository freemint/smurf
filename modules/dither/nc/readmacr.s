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

*******************************************************************
*                           DIT_READ.S
*
*   Die Pixelauslese-Routinen fr die Ditherrouts 
*   als Subroutines
*
*   28.10.96 Olaf
*
*******************************************************************

GLOBL read24bit, read8bit, read16bit

IMPORT  read_ready

IMPORT  Palette, rgbtab




read24bit:
        move.b  (a0)+,d4        ;get red
        lsr.w   #3,d4           ;>>3

        move.b  (a0)+,d3        ;get green
        lsr.w   #3,d3           ;>>3

        move.b  (a0)+,d2        ;get blue
        lsr.w   #3,d2           ;>>3

        bra read_ready








read8bit:
        move.l  Palette,a6      ;Palette holen

        move.b  (a0)+,d2        ;*pic
        
        adda.l  d2, a6
        adda.l  d2, a6
        adda.l  d2, a6

        move.b  (a6)+,d4        ;get red
        lsr.b   #3, d4
        
        move.b  (a6)+,d3        ;get green
        lsr.b   #3, d3

        move.b  (a6)+,d2        ;get blue
        lsr.b   #3, d2

        move.l  rgbtab,a6       ; rgbtab wieder holen

        bra read_ready





read16bit:
        move.w  (a0)+, d4           ; pixel holen
        move.w  d4, d3              ; fr grn
        move.w  d4, d2              ; fr blau
    
        rol.w   #5,d4               ; rot fertigmachen
        and.w   #$001f, d4          ; rot ausmaskieren

        lsr.w   #6,d3               ;grn fertigmachen
        and.w   #$001f, d3          ; grn ausmaskieren
    
        and.w   #$001f, d2          ; blau ausmaskieren

        bra read_ready


