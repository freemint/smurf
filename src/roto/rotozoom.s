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

;
;	(a0)=xstep
; 4(a0)=ystep
; 8(a0)=xystep
;12(a0)=yxstep
;14(a0)=paddr
;15(a0)=image

GLOBL rotozoom


rotozoom:
	movem.l d3-d7/a2-a6, -(sp)			; Register retten

	move.w d0, a2		; xstep

	move.w	d0, a3		; ystep
	add.w	a3, a3

	move.w	d1, a4		; xystep
	add.w	a4, a4

	neg.w d1
	move.w d1, a5		; yxstep


	; Interpolationscounter l”schen
	lsl.w #1, d2
	neg.w d2
	move.w d2, d4
	move.w d2, d5
	add.w #16384, d4
	
	move.w #80-1, ycount		; Y-Counter
	
	lea.l 20(a1), a6
	
loopy:

	move.w d4, d0
	move.w d5, d1

	moveq.l	#160/16-1, d6
loopx:
	clr.w d7

	BIT=1<<15;
	REPT 16
		move.w	d0, d2		; X-Offset
		move.w	d1, d3
		lsr.w	#8, d2	
		move.b	d2, d3		; Und die Offsets aufeinanderaddieren.

		tst.b (a0, d3.w)
		beq.b *+6				;nobit

		ori.w	#BIT, d7

		add.w	a2, d0.w	; xc+=xs
		add.w	a5, d1.w	; yc+=xys
	BIT=BIT>>1;
	ENDM

	move.w d7, (a6)+
	move.w d7, (a1)+	

	dbra	d6, loopx
	
	add.w a3, d5.w			; yc inc
	add.w a4, d4.w			; xc inc

	adda.l #160/8, a1
	adda.l #160/8, a6
	
	subq.w #1, ycount
	bpl	loopy







	movem.l (sp)+, d3-d7/a2-a6			; Register wiederherstellen
	rts
	
	
	
.BSS
xcount:	ds.w	1
ycount:	ds.w	1

