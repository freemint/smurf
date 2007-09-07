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
 * Christian Eyrich, Olaf Piesche
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

GLOBL PCD_decode00

PCD_decode00:
movem.l d3-d7/a2-a6, -(sp)
move.l	(a0)+, a1				; ziel1
move.l	(a0)+, a2				; T_B
move.l	(a0)+, a3				; T_G
move.l	(a0)+, a4				; T_R
move.l	(a0)+, a5				; T_g
move.l	(a0)+, a6				; T_L
move.l	(a0)+, d0				; width
move.l	(a0)+, d1				; height
move.l	(a0)+, a0				; RGB_corr

move.l	#$03ff,d7				; compare value
subq.w	#1,d0					; width - 1 : dbra
subq.w	#1,d1					; height - 1 : dbra
moveq	#0,d5					; zeroing
moveq	#0,d6					; zeroing

move.w d1, ycount
moveq.l #10, d1

_loopy:
move.w	d0,d2					; width - counter
_loopx:
;---------------------------
; Luminanz holen
;---------------------------
	moveq	#0,d3				;zeroing
	move.b 	(a1),d3				;*ziel
	lsl.w	#2, d3
	move.l	(a6,d3.w),d3		;L = T_L[*ziel]
	moveq	#0,d5				;zeroing
	move.b	1(a1),d5			;cb = *(ziel + 1)
	moveq	#0,d6				;zeroing
	move.b	2(a1),d6			;cr = *(ziel + 2)
;---------------------------
; Rotwert holen	
;---------------------------
	lsl.w	#2, d6
	move.l	(a4,d6.w),d4		;get T_R2[c2]
	add.l	d3,d4				;+ L
	lsr.l	d1,d4				;>>= 10
	move.b	(a0,d4.w),d4		;PCDMap[r]
	move.b	d4,(a1)+			;speichern
;---------------------------
; Grnwert holen	
;---------------------------
	lsl.w 	#2, d5
	move.l	(a3,d5.w),d4		;get T_G1[c1]
	add.l	d3,d4				;+ L
	add.l	(a5,d6.w),d4		;+ T_G2[c2]
	lsr.l	d1,d4				;>>= 10
	move.b	(a0,d4.w),d4		;PCDMap[g]
	move.b	d4,(a1)+			;speichern
;---------------------------
; Blauwert holen
;---------------------------
	move.l	(a2,d5.w),d4		;get T_B1[c1]
	add.l	d3,d4				;+ L
	lsr.l	d1,d4				;>>= 10
	move.b	(a0,d4.w),d4		;PCDMap[b]
	move.b	d4,(a1)+			;speichern
dbra d2, _loopx

subq.w #1, ycount
bge	_loopy

movem.l (sp)+,d3-d7/a2-a6
rts

ycount:	ds.w 1