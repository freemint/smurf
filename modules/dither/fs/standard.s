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

* Pixel Packed to Interleaved Bitplane - Transformer
* fÅr SMURF
* a0=16 Bytes Buffer, Bilddaten
* a1= Ziel-Adresse
* d0= Anzahl Bitplanes
*
* 13.11.95 Olaf Piesche
*


GLOBL setpix162, get_standard_pix



*****************************************************************
* 16 Packed-Pixel aus a0 ins Standardformat nach a1 schreiben   *
* in d1 planes                                                  *
*****************************************************************
setpix162:
    movem.l     d3-d5, -(sp)        ; save regs
    moveq   #16,d4
    subq.w  #1,d0
    moveq   #0,d5

loopplane2:
    moveq #0, d3        ; Word to write
    moveq #15, d2       ; Bit val

    loopp:
        btst.b  d5,(a0)+    ; is d2th Bit set in d1th Byte of buffer?
        beq.b   *+4         ; nope!
        bset.l  d2,d3       ; yep! set d6th Bit in d7!
        dbra    d2,loopp

    suba.l d4, a0           ; goto buffer start
    move.w  d3,(a1)     ; write finished word.
    adda.l d1, a1       ; next plane
    addq.w #1, d5

    dbra d0, loopplane2     ; do this for #d0 Planes.


movem.l (sp)+, d3-d5
rts





*************************************************************** *
*   16 Standardformatpixel aus a0 in 16 Packed Pixels nach a1   *
*   schreiben.                                                  *
*   Deklaration:                                                *
*                                                               *
/*
void get_standard_pix(void *st_pic, void *buf16, int planes, long planelen, int zoom);
*/
*************************************************************** *
get_standard_pix:
movem.l d3-d6/a2, -(sp)

subq.w #1, d0           ; planes-1 fÅr dbra
moveq   #15, d6

/*tst.l d2
bne get_zoom*/

;--------------------------- normales auslesen (zoom=0)
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



/*
;--------------------------- gezoomtes auslesen (1<=zoom<=15)
get_zoom:
IMPORT  st_zoom

    move.l a2, old_src

    move.l d7, -(sp)
    clr.l d7

    move.w d2, d7
    move.w d2, zoomcounter


loop16z:
    move.l  d0, d2      ; plane-counter 

    movea.l a0, a2      ; source kopieren

    moveq.l #0, d3
    moveq.l #1, d5

    ;------X-scaling
    subq.w  #1, d7
    cmp.w   zoomcounter, d7
    bge end_planeloop
        move.w zoomcounter, d7

loopplanez:
    move.w  (a2), d4

    btst    d6, d4
    beq *+4

    or.b    d5, d3

    adda.l  d1, a2          ; eine plane weiter
    lsl.b   #1, d5
    dbra d2, loopplanez

    move.b d3, (a1)+
    add.w   #1, written

end_planeloop:
    dbra d6, loop16z

    cmp.w #16, written      ;--- ende der 16 Lesepixel: schon 16 geschrieben?
    beq end
    move.w  #15, d6
    move.l old_src, a2
    add.l   #2, a2
    bra loop16z

end:
    movem.l (sp)+, d3-d7/a2
    rts

zoomcounter:    ds.w    1
old_src:        ds.l    1
written:        ds.w    1
*/
