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
 * Olaf Piesche, Christian Eyrich, Dale Russell and J排g Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

*********************************************************
*                   Nearest Color X-Loops
*   Enth�lt die X-Schleifen f〉 den Nearest-Color-Dither
*********************************************************

IMPORT  Palette, rgbtab, line_ready
EXPORT loopx24, loopx16, loopx8

;----------------- 24 Bit
loopx24:
    move.b  (a0)+,d4        ;get red
    lsr.w   #3,d4           ;>>3

    move.b  (a0)+,d3        ;get green
    lsr.w   #3,d3           ;>>3

    move.b  (a0)+,d2        ;get blue
    lsr.w   #3,d2           ;>>3

    ;
    ;       Nearest Color table offset (3*5bit->15Bit-Wert)
    ;
    move.w  d3,d5           ;gr］ kopieren
    move.w  d4,d6           ;rot kopieren
    ror.w   #6,d6
    lsl.w   #5,d5           ;gr］<<=5
    add.w   d2,d6           ;rot+=blau
    add.w   d5,d6           ;rot+=gr］

    ;
    ;   Index aus NC-Table holen und Screenwert eintragen
    ;
    clr.w   d7
    move.b  (d6.w, a2), d7          ;Index des Screenwerts (NCT)
    move.b  (d7.w, a3), (a4)+       ;Screenwert eintragen (Planetable)

    adda.w  d1, a0          ; X-Scaling 

    dbra d0, loopx24
    jmp line_ready
    

;----------------- 16 Bit
loopx16:
    move.w  (a0)+, d4           ; pixel holen
    move.w  d4, d3              ; f〉 gr］
    move.w  d4, d2              ; f〉 blau
    
    rol.w   #5,d4               ; rot fertigmachen
    and.w   #$001f, d4          ; rot ausmaskieren
    
    lsr.w   #6,d3               ;gr］ fertigmachen
    and.w   #$001f, d3          ; gr］ ausmaskieren
    
    and.w   #$001f, d2          ; blau ausmaskieren
    
    ;
    ;       Nearest Color table offset (3*5bit->15Bit-Wert)
    ;
    move.w  d3,d5           ;gr］ kopieren
    move.w  d4,d6           ;rot kopieren
    ror.w   #6,d6
    lsl.w   #5,d5           ;gr］<<=5
    add.w   d2,d6           ;rot+=blau
    add.w   d5,d6           ;rot+=gr］

    ;
    ;   Index aus NC-Table holen und Screenwert eintragen
    ;
    clr.w   d7
    move.b  (d6.w, a2), d7          ;Index des Screenwerts (NCT)
    move.b  (d7.w, a3), (a4)+       ;Screenwert eintragen (Planetable)

    adda.w  d1, a0          ; X-Scaling 

    dbra d0, loopx16
    jmp line_ready
    

;----------------- 8 Bit
loopx8:
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
    
    ;
    ;       Nearest Color table offset (3*5bit->15Bit-Wert)
    ;
    move.w  d3,d5           ;gr］ kopieren
    move.w  d4,d6           ;rot kopieren
    ror.w   #6,d6
    lsl.w   #5,d5           ;gr］<<=5
    add.w   d2,d6           ;rot+=blau
    add.w   d5,d6           ;rot+=gr］

    ;
    ;   Index aus NC-Table holen und Screenwert eintragen
    ;
    clr.w   d7
    move.b  (d6.w, a2), d7          ;Index des Screenwerts (NCT)
    move.b  (d7.w, a3), (a4)+       ;Screenwert eintragen (Planetable)

    adda.w  d1, a0          ; X-Scaling 

    dbra d0, loopx8
    jmp line_ready
