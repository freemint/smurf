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

;*****************************************************************
; Assembler Routinen fr Schlumpf's Fli-Viewer
; Neue Versionen, May 1996
; (c) 1996 Coke of Therapy 
; Stand im Februar 1996:
; Bisher alle FLI-Animationen fehlerfrei abspielbar,
; Geschwindigkeit: Durchschnittlich ca. 25 Bilder pro Sekunde
;*****************************************************************

GLOBL ByteRun, DeltaFli, Palette64NEW, Palette256


/*******************************************/
/*      Fill Palette    64                 */
/*******************************************/
Palette64NEW:
movem.l d3-d7/a2-a6,-(sp)
move.w  d0,d6
moveq   #11,d7
moveq   #0,d2
loopfill:
    moveq   #0,d0
    move.b  (a0)+,d0        ;get red
    lsr.w   #1,d0           ;6 Bit >> 1 = 5 Bit !
    lsl.w   d7,d0           ;<<11
    moveq   #0,d1           ;the same with
    move.b  (a0)+,d1        ;green
    lsl.w   #5,d1           ;Green is 6 Bit, already!
    or.w    d1,d0           ;add it to 16 Bit Color
    moveq   #0,d1           ;the same with
    move.b  (a0)+,d2        ;blue
    lsr.b   #1,d2           ;blue ist 6 Bit, therefore shift it!
    or.b    d2,d0           ;add it to 16 Bit Color
    move.w  d0,(a1)+        ;store it as 16 Bit HI-COLOR
dbra d6,loopfill

movem.l (sp)+,d3-d7/a2-a6
rts
/*******************************************/
/*      Fill Palette    256                */
/*******************************************/
Palette256:
movem.l d3-d7/a2-a6,-(sp)
move.w  d0,d6
moveq   #11,d7
moveq   #0,d2
loopfill256:
    moveq   #0,d0
    move.b  (a0)+,d0        ;get red
    lsr.b   #3,d0           ;red is 8 Bit -> make it to 5 Bit
    lsl.w   d7,d0           ;<<11
    moveq   #0,d1           ;the same with
    move.b  (a0)+,d1        ;green
    lsr.b   #2,d1           ;green is 8 Bit -> make it to 6 Bit
    lsl.w   #5,d1           ;<<5
    add.w   d1,d0           ;add it to 16 Bit Color
    moveq   #0,d1           ;the same with
    move.b  (a0)+,d2        ;blue
    lsr.b   #3,d2           ;blue is 8 Bit -> make it to 5 Bit
    or.b    d2,d0           ;add it to 16 Bit Color
    move.w  d0,(a1)+        ;store it as 16 Bit HI-Color
dbra d6,loopfill256

movem.l (sp)+,d3-d7/a2-a6
rts
/********************************************/
/*      ByteRun                             */
/********************************************/
ByteRun:
movem.l d3-d7/a2-a6,-(sp)

; a0 = buffer
; a1 = palette
; a2 = paddr
; d0 = height
; d5 = Width

move.l  (a0)+,a1
move.l  (a0)+,a2
move.l  (a0)+,d0
move.l  (a0)+,d5
move.l  (a0)+,physx
move.l  (a0),a0

;add.w  d5,d5               ;width *2
move.l  physx,d5            ;Bildschirmoffset
add.w   d5,d5
subq.w  #1,d0               ;Height-1
move.l  #$00ff,d7           ;faktor
moveq   #$007f,d6           ;compare

BRloopy:
    move.l  a2,a3           ;copy
    moveq   #0,d1
    move.b  (a0)+,d1        ;anzahl
    ble.b   StepInBR
    subq.w  #1,d1   

    BRloopanz:
    moveq   #0,d2
    moveq   #0,d4
    move.b  (a0)+,d2    ;get n
    cmp.w   d6,d2       ;if n<=0x7f
    ble.b   BRotherwise ;trace over
    move.w  d7,d3       ;get 0x100
    sub.w   d2,d3       ;-n
    blt.b   JumpOver

        BRloopn:
            move.b (a0)+,d4             ;m-Offset
            move.w  d4.w*2(a1),(a3)+    ;paddr
        dbra d3,BRloopn
JumpOver:       
    dbra d1,BRloopanz
StepInBR:   
 adda.l d5,a2
dbra d0,BRloopy

    bra.b   END

    BRotherwise:
    move.b  (a0)+,d4
    move.w  d4.w*2(a1),d4
    subq.w  #1,d2
    blt.b   Endloopx
    
        BRloopn2:
            move.w  d4,(a3)+
        dbra d2,BRloopn2    
Endloopx:       
    dbra d1,BRloopanz
 adda.l d5,a2
dbra d0,BRloopy

END: 
movem.l (sp)+,d3-d7/a2-a6
rts

/********************************************/
/*          Delta-Fli                       */
/********************************************/
DeltaFli:
movem.l d3-d7/a2-a6,-(sp)

; a0 = buffer
; a1 = palette
; a2 = paddr
; d0 = height
; d5 = Width
move.l  (a0)+,a1
move.l  (a0)+,a2
move.l  (a0)+,d0
move.l  (a0)+,d5
move.l  (a0)+,physx
move.l  (a0),a0

;add.w  d5,d5               ;width*2
move.l  physx,d5            ;Bildschirmoffset
add.w   d5,d5
moveq   #0,d0               ;clear reg
moveq   #0,d1               ;clear reg
move.b  (a0)+,d0            ;lo
move.b  (a0)+,d1            ;hi
lsl.w   #8,d1               ;set hi
add.w   d1,d0               ; Start Line
muls.w  d5,d0               ; *Width
adda.l  d0,a2               ; set on paddr

moveq   #0,d0               ;clear reg
moveq   #0,d1               ;clear reg
move.b  (a0)+,d0            ;get intel lo
move.b  (a0)+,d1            ;get intel hi
lsl.w   #8,d1               ; * 256
add.w   d1,d0               ; Line-Counter
ble.w   ENDDF               ;=0 : Ende
subq.w  #1,d0               ;-1 for dbra

move.w  #$00ff,d7           ;faktor
moveq   #$007f,d6           ;compare
moveq   #0,d4               ;clear color reg

DFloopy:
    move.l  a2,a3           ;copy
    moveq   #0,d1           ;clear y-counter
    move.b  (a0)+,d1        ;anzahl
    ble.b   StepIn          ;<= 0
    subq.w  #1,d1           ;dbra

DFloopanz:
    move.b  (a0)+,d4
    lea     d4.w*2(a3),a3
    moveq   #0,d2           ;clear 
    move.b  (a0)+,d2        ;n
    cmp.w   d6,d2           ;< $7f
    ble.b   DFotherwise

;
;   Fall1a
;

move.b  (a0)+,d4            ;m-Offset
move.w  d7,d3               ;get 0x100
sub.w   d2,d3               ;-n
blt.w   TraceIn             ;jump over
move.w  d4.w*2(a1),d2       ;color

        DFloopn1a:
            move.w  d2,(a3)+    ;paddr
        dbra d3,DFloopn1a
TraceIn:
    dbra d1,DFloopanz
StepIn: 
  adda.l    d5,a2
dbra d0,DFloopy
bra.b   ENDDF

DFotherwise:
subq.w  #1,d2
blt.b   NotAgain
        DFloopn2a:
            move.b  (a0)+,d4
            move.w  d4.w*2(a1),(a3)+
        dbra d2,DFloopn2a
NotAgain:       
    dbra d1,DFloopanz
 adda.l d5,a2
dbra d0,DFloopy

ENDDF: 
movem.l (sp)+,d3-d7/a2-a6
rts

physx: ds.l 1
