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
        sub.w   d7,d4           ;-rf
        add.w   d4,d4
        move.w  (a1, d4.w),d4   ;Clipping

        move.b  (a0)+,d3        ;get green
        lsr.w   #3,d3           ;>>3
        sub.w   d6,d3           ;-gf
        add.w   d3,d3
        move.w  (a1, d3.w),d3   ;Clipping

        move.b  (a0)+,d2        ;get blue
        lsr.w   #3,d2           ;>>3
        sub.w   d5,d2           ;-bf
        add.w   d2,d2
        move.w  (a1, d2.w),d2   ;Clipping

        /*
        move.l  (a0), d0
        andi.l  #$F8F8F800, d0
        rol.l   #5, d0
        clr.w d4
        move.b  d0, d4
    sub.w   d7,d4           ;-rf
        add.w   d4, d4              ; Clip
        move.w  (a1, d4.w), d4      ; ->rot
        swap    d0
        clr.w d2
        move.b  d0, d2
    sub.w   d5,d2           ;-bf
        add.w   d2, d2              ; Clip
        move.w  (a1, d2.w), d2      ; ->blau
        ror.l   #8, d0
        clr.w d3
        move.b  d0, d3
    sub.w   d6,d3           ;-gf
        add.w   d3, d3              ; Clip
        move.w  (a1, d3.w), d3      ; ->grn
        addq.l  #3, a0
        */
        bra read_ready








read8bit:
        move.l  Palette,a6      ;Palette holen

        move.b  (a0)+,d2        ;*pic
        
        adda.l  d2, a6
        adda.l  d2, a6
        adda.l  d2, a6

        move.b  (a6)+,d4        ;get red
        lsr.b   #3, d4
        sub.w   d7,d4           ;-rf
        add.w d4,d4
        move.w  (a1, d4.w),d4   ;Clipping
        
        move.b  (a6)+,d3        ;get green
        lsr.b   #3, d3
        sub.w   d6,d3           ;-gf
        add.w d3,d3
        move.w  (a1, d3.w),d3   ;Clipping

        move.b  (a6)+,d2        ;get blue
        lsr.b   #3, d2
        sub.w   d5,d2           ;-bf
        add.w d2,d2
        move.w  (a1, d2.w),d2   ;Clipping

        move.l  rgbtab,a6       ; rgbtab wieder holen

        bra read_ready





read16bit:
        move.w  (a0)+, d4           ; pixel holen
        move.w  d4, d3              ; fr grn
        move.w  d4, d2              ; fr blau
    
        rol.w   #5,d4               ; rot fertigmachen
        and.w   #$001f, d4          ; rot ausmaskieren
        sub.w   d7, d4              ; -rf
        add.w   d4,d4
        move.w  (a1, d4.w), d4  ; clip

        lsr.w   #6,d3               ;grn fertigmachen
        and.w   #$001f, d3          ; grn ausmaskieren
        sub.w   d6, d3              ; -gf
        add.w   d3,d3
        move.w  (a1, d3.w), d3  ; clip
    
        and.w   #$001f, d2          ; blau ausmaskieren
        sub.w   d5, d2              ; -rf
        add.w   d2,d2
        move.w  (a1, d2.w), d2  ; clip

        bra read_ready


