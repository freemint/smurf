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

*********************************************************
*                       X-Loops
*   EnthÑlt die X-Schleifen fÅr alle nîtigen Farbtiefen
*   fÅr den Fast Diffusion Dither
*********************************************************

IMPORT seekcolor, rgbtab, scalex_inc, line_ready, Palette
GLOBL fdd_24bit, fdd_16bit, fdd_8bit


/*
 *  FÅr 24 Bit-Source
 */
fdd_24bit:
    move.b  (a0)+,d4        ; rot holen
    lsr.w   #3,d4           ;r>>3
    sub.w   d7,d4           ;-rf
    move.b  (a0)+,d3        ; grÅn holen
    add.w   d4,d4
    move.w  (a1, d4.w),d4   ; rot clippen
    
    lsr.w   #3,d3           ;g>>3
    sub.w   d6,d3           ;-gf
    move.b  (a0)+,d2        ; blau holen
    add.w   d3,d3
    move.w  (a1, d3.w),d3   ;grÅn clippen

    lsr.w   #3,d2           ;b>>3
    sub.w   d5,d2           ;-bf
    add.w   d2,d2
    move.w  (a1, d2.w),d2   ; blau clippen


    ;
    ;       Nearest Color table offset (3*5bit->15Bit-Wert)
    ;
    move.w  d3,d5           ;grÅn kopieren
    move.w  d4,d6           ;rot kopieren
    ror.w   #6,d6
    lsl.w   #5,d5           ;grÅn<<=5
    add.w   d2,d6           ;rot+=blau
    add.w   d5,d6           ;rot+=grÅn

    ;
    ;   Index aus NC-Table holen und Screenwert eintragen
    ;
    clr.w   d7
    move.b  (d6.w, a2), d7          ;Index des Screenwerts (NCT)

    cmp.w   d7, d1          ; war der Index in der NCT vorhanden?
    bne endseek24
        jsr seekcolor
    endseek24:

    move.b  (d7.w, a3), (a4)+       ;Screenwert eintragen (Planetable)


    ;
    ;   Fehler berechnen
    ;   Der Fehler wird zu 100% ins rechte Pixel Åbernommen
    ;
    move.l  rgbtab,a6           ; Palette (3*512 Bytes)
    lsl.w   #3, d7
    adda.l  d7, a6

    move.w  (a6)+, d5
    sub.w   d2, d5                  ;Blaufehler
    move.w  (a6)+, d6
    sub.w   d3, d6                  ;GrÅnfehler
    move.w  (a6)+, d7
    sub.w   d4, d7                  ;Rotfehler

    adda.w  scalex_inc, a0          ; X-Scaling 

    dbra d0, fdd_24bit
    bra line_ready



/*
 *  FÅr 16 Bit-Source
 */
fdd_16bit:
    move.w  (a0)+, d4           ; pixel holen
    move.w  d4, d3              ; fÅr grÅn
    move.w  d4, d2              ; fÅr blau

    rol.w   #5,d4               ; rot fertigmachen
    and.w   #$001f, d4          ; rot ausmaskieren
    sub.w   d7, d4              ; -rf
    add.w   d4,d4
    move.w  (a1, d4.w), d4  ; clip

    lsr.w   #6,d3               ;grÅn fertigmachen
    and.w   #$001f, d3          ; grÅn ausmaskieren
    sub.w   d6, d3              ; -gf
    add.w   d3,d3
    move.w  (a1, d3.w), d3  ; clip

    and.w   #$001f, d2          ; blau ausmaskieren
    sub.w   d5, d2              ; -rf
    add.w   d2,d2
    move.w  (a1, d2.w), d2  ; clip


    ;
    ;       Nearest Color table offset (3*5bit->15Bit-Wert)
    ;
    move.w  d3,d5           ;grÅn kopieren
    move.w  d4,d6           ;rot kopieren
    ror.w   #6,d6
    lsl.w   #5,d5           ;grÅn<<=5
    add.w   d2,d6           ;rot+=blau
    add.w   d5,d6           ;rot+=grÅn

    ;
    ;   Index aus NC-Table holen und Screenwert eintragen
    ;
    clr.w   d7
    move.b  (d6.w, a2), d7          ;Index des Screenwerts (NCT)

    cmp.w   d7, d1          ; war der Index in der NCT vorhanden?
    bne endseek16
        jsr seekcolor
    endseek16:

    move.b  (d7.w, a3), (a4)+       ;Screenwert eintragen (Planetable)


    ;
    ;   Fehler berechnen
    ;   Der Fehler wird zu 100% ins rechte Pixel Åbernommen
    ;
    move.l  rgbtab,a6           ; Palette (3*512 Bytes)
    lsl.w   #3, d7
    adda.l  d7, a6

    move.w  (a6)+, d5
    sub.w   d2, d5                  ;Blaufehler
    move.w  (a6)+, d6
    sub.w   d3, d6                  ;GrÅnfehler
    move.w  (a6)+, d7
    sub.w   d4, d7                  ;Rotfehler

    adda.w  scalex_inc, a0          ; X-Scaling 

    dbra d0, fdd_16bit
    bra line_ready



/*
 *  FÅr 8 Bit-Source
 */
fdd_8bit:
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

    ;
    ;       Nearest Color table offset (3*5bit->15Bit-Wert)
    ;
    move.w  d3,d5           ;grÅn kopieren
    move.w  d4,d6           ;rot kopieren
    ror.w   #6,d6
    lsl.w   #5,d5           ;grÅn<<=5
    add.w   d2,d6           ;rot+=blau
    add.w   d5,d6           ;rot+=grÅn

    ;
    ;   Index aus NC-Table holen und Screenwert eintragen
    ;
    clr.w   d7
    move.b  (d6.w, a2), d7          ;Index des Screenwerts (NCT)

    cmp.w   d7, d1          ; war der Index in der NCT vorhanden?
    bne endseek8
        jsr seekcolor
    endseek8:

    move.b  (d7.w, a3), (a4)+       ;Screenwert eintragen (Planetable)


    ;
    ;   Fehler berechnen
    ;   Der Fehler wird zu 100% ins rechte Pixel Åbernommen
    ;
    move.l  rgbtab,a6           ; Palette (3*512 Bytes)
    lsl.w   #3, d7
    adda.l  d7, a6

    move.w  (a6)+, d5
    sub.w   d2, d5                  ;Blaufehler
    move.w  (a6)+, d6
    sub.w   d3, d6                  ;GrÅnfehler
    move.w  (a6)+, d7
    sub.w   d4, d7                  ;Rotfehler

    adda.w  scalex_inc, a0          ; X-Scaling 

    dbra d0, fdd_8bit
    bra line_ready
