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

; **********************************************************
;                   SET_PIXL.S          
; **********************************************************
;
;   EnthÑlt zwei Routinen zum setzen der Pixel ins Standard-
;   und eine fÅr Pixelpacked-Format.
;
; **********************************************************



GLOBL   setpix_standard, setpix_standard_16, setpix_pp


;---------------------------------------------------
;       16 Pixel ins Standardformat setzen
;       auf 8 bit ausgerichtet!
;   Deklaration:    
;   int setpix_standard(char *buf16, char *dest, int depth, long planelen, int howmany);
;
;   buf16:  Buffer, in dem sich 16 Bytes (Pixel) Pixelpacked befinden
;   dest:   Zielspeicherbereich
;   depth:  Farbtiefe
;   howmany:    Wieviele Pixel sollen gesetzt werden (16>=howmany>=8!)
;   planelen:   LÑnge einer Plane des Zielbildes in Bytes
;
;   return: Anzahl an Bytes, die weitergegangen werden soll (1 oder 2)
;
;   Noch unoptimiert, nur zum austesten
;---------------------------------------------------
setpix_standard:
    movem.l d3-d6, -(sp)

    clr.w   d4
    subq.w  #1,d0

    move.w  #16, d5     ; Sprungoffset ausrechnen
    sub.w   d2, d5
    mulu.w  #8, d5      ; 8 Bytes fÅr die 4 Befehle

    moveq.l #0, d6
    
looplane:
    clr.w   d3
    bset.l  #15, d6

    jmp 2(pc, d5.w)

    ;-------------------------Schleifenanfang - 16 Pixel einer Plane fertigmachen
    REPT 16

    btst.b  d4,(a0)+        ; Bit d4 gesetzt?
    beq.b   *+4             ; Nein? auch in d3 keins setzen.
    or.w    d6, d3
    lsr.w   #1, d6

    ENDM

    ;-------------------------zum Schleifenanfang
    suba.w  d2,a0           ; pxl-tab zum Anfang

    ror.w   #8, d3          ; obere 8 Bits holen
    move.b  d3,(a1)         ; Byte schreiben

    cmp.w   #8, d2
    ble nomorethan8
        rol.w   #8, d3
        move.b  d3,1(a1)            ; Byte schreiben

nomorethan8:
    adda.l  d1,a1           ; nÑchste Plane

    addq.w  #1, d4          ; getestetes Bit ++

    dbra d0, looplane

    ;----- Returncode schreiben
    ;   wurden mehr als 8 Pixel geschrieben ist dieser 2, sonst 1
    move.w  #1, d0
    cmp.w   #8, d2
    ble.b return_1
    move.w  #2, d0

return_1:
    movem.l (sp)+, d3-d6
    rts
    


;---------------------------------------------------
; 16 Packed-Pixel aus a0 ins Standardformat nach 
; a1 schreiben, in d1 planes auf 16 Bit ausgerichtet
;
;   int setpix_standard_16(char *buf16, char *dest, int depth, long planelen, int howmany);
;
;   return: Anzahl an Bytes, die im Quellbild weitergegangen werden soll
;           (in diesem Fall immer 2)
;---------------------------------------------------
setpix_standard_16:
    movem.l     d3-d4/a2, -(sp)     ; save regs
    subq.w  #1,d0
    moveq   #0,d4
    move.l  a0, a2

loopplane2:
    clr.w d3        ; clear Word to write

    BIT=32768
    REPT 16

    btst.b  d4,(a0)+    ; is d2th Bit set in d1th Byte of buffer?
    beq.b   *+6         ; nope!
    ori.w   #BIT,d3     ; yep! set d6th Bit in d7!

    BIT = BIT>>1
    ENDM

    movea.l a2, a0
    move.w  d3,(a1)     ; write finished word.
    adda.l d1, a1       ; next plane
    addq.w #1, d4
    dbra d0, loopplane2     ; do this for #d0 Planes.

    moveq.l #2, d0          ; return: increment 2 Bytes!

    movem.l (sp)+, d3-d4/a2
    rts




;---------------------------------------------------
;           16 Pixel pixelpacked setzen
;   Es wird immer von einer Farbtiefe von 8 Bit ausgegangen
;   Deklaration: dieselbe wie setpix_standard
;   int setpix_pp(char *buf16, char *dest, int depth, long planelen, int howmany);
;   Return: Pixelincrement im Quellbild (8Bit -> 16, 4 Bit->8)
;---------------------------------------------------

setpix_pp:
    ;cmp.w  #4, d0
    ;beq    set4bit
    ;cmp.w  #8, d0
    ;beq    set8bit

    ;set4bit:
        ; heute aber nicht mehr....
    ;rts


set_8bit:
    move.w  #16, d1     ; Sprungoffset ausrechnen
    sub.w   d2, d1
    lsl.w   #1, d1      ; 2 Bytes fÅr den move

    jmp 2(pc, d1.w)
    REPT 16
        move.b (a0)+, (a1)+
    ENDM

    move.w  d2, d0
    rts
