;/*
; * ***** BEGIN LICENSE BLOCK *****
; *         
; * The contents of this file are subject to the GNU General Public License
; * Version 2 (the "License"); you may not use this file except in compliance
; * with the GPL. You may obtain a copy of the License at
; * http://www.gnu.org/copyleft/gpl.html or the file GPL.TXT from the program
; * or source code package.
; *         
; * Software distributed under the License is distributed on an "AS IS" basis,
; * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
; * the specific language governing rights and limitations under the GPL.
; *         
; * The Initial Developer of the Original Code is
; * Bjoern Spruck
; *
; * This code is a module for the programm SMURF developed by
; * Olaf Piesche, Christian Eyrich, Dale Russell and Joerg Dittmer
; *         
; * Contributor(s):
; *         
; *
; * ***** END LICENSE BLOCK *****
; */
;
;		Schnelle Punktkopierroutinen
;	2,4,16,256 Farben & TrueColor

.EXPORT fcopypoint
.EXPORT fcopypoint2
.EXPORT fcopypoint4
.EXPORT fcopypoint16
.EXPORT fcopypoint256
.EXPORT fcopypoint256PP
.EXPORT fcopypointTC16
.EXPORT fcopypointTC24

.TEXT

fcopypoint : dc.l	0;		der Zeiger

;	šbergabeparameter :
;		A0 - Zeiger auf folgende Struktur :
;			L	Adresse
;			W	X
;			L	XY
;		A1 - Zeiger auf folgende Struktur :
;			L	Adresse
;			W	X
;			L	XY

.MODULE fcopypoint2 :

	movem.l	D3/A2-A3,	-(sp)

	movea.l	(A0)+,	A2
	move.w	(A0)+,	D0
	move.w	D0,	D1
	and.w	#$FFF0,	D0	
	and.w	#$F,	D1	
	lsr.w	#3,	D0
	adda.w	D0,	A2
	moveq.l	#15,	D0
	sub.w	D1,	D0

	movea.l	(A1)+,	A3
	move.w	(A1)+,	D2
	move.w	D2,	D3
	and.w	#$FFF0,	D2	
	and.w	#$F,	D3	
	lsr.w	#3,	D2
	adda.w	D2,	A3
	moveq.l	#15,	D2
	sub.w	D3,	D2

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset
	bset.l	D2,	D3
	bra.b	tsch
notset:
	bclr.l	D2,	D3
tsch:
	move.w	D3,	(A3)

	movem.l	(sp)+,	D3/A2-A3

	rts
.ENDMOD

.MODULE fcopypoint4 :

	movem.l	D3/A2-A3,	-(sp)

	movea.l	(A0)+,	A2
	move.w	(A0)+,	D0
	move.w	D0,	D1
	and.w	#$FFF0,	D0	
	and.w	#$F,	D1	
	lsr.w	#3,	D0
	adda.w	D0,	A2
	moveq.l	#15,	D0
	sub.w	D1,	D0

	movea.l	(A1)+,	A3
	move.w	(A1)+,	D2
	move.w	D2,	D3
	and.w	#$FFF0,	D2	
	and.w	#$F,	D3	
	lsr.w	#3,	D2
	adda.w	D2,	A3
	moveq.l	#15,	D2
	sub.w	D3,	D2

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset1
	bset.l	D2,	D3
	bra.b	tsch1
notset1:
	bclr.l	D2,	D3
tsch1:
	move.w	D3,	(A3)

	adda.l	(A0),	A2
	adda.l	(A1),	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset2
	bset.l	D2,	D3
	bra.b	tsch2
notset2:
	bclr.l	D2,	D3
tsch2:
	move.w	D3,	(A3)

	movem.l	(sp)+,	D3/A2-A3

	rts
.ENDMOD

.MODULE fcopypoint16 :

	movem.l	D3-D5/A2-A3,	-(sp)

	movea.l	(A0)+,	A2
	move.w	(A0)+,	D0
	move.l	(A0),	D4
	move.w	D0,	D1
	and.w	#$FFF0,	D0	
	and.w	#$F,	D1	
	lsr.w	#3,	D0
	adda.w	D0,	A2
	moveq.l	#15,	D0
	sub.w	D1,	D0

	movea.l	(A1)+,	A3
	move.w	(A1)+,	D2
	move.l	(A1),	D5
	move.w	D2,	D3
	and.w	#$FFF0,	D2	
	and.w	#$F,	D3	
	lsr.w	#3,	D2
	adda.w	D2,	A3
	moveq.l	#15,	D2
	sub.w	D3,	D2

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset1
	bset.l	D2,	D3
	bra.b	tsch1
notset1:
	bclr.l	D2,	D3
tsch1:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset2
	bset.l	D2,	D3
	bra.b	tsch2
notset2:
	bclr.l	D2,	D3
tsch2:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset3
	bset.l	D2,	D3
	bra.b	tsch3
notset3:
	bclr.l	D2,	D3
tsch3:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset4
	bset.l	D2,	D3
	bra.b	tsch4
notset4:
	bclr.l	D2,	D3
tsch4:
	move.w	D3,	(A3)

	movem.l	(sp)+,	D3-D5/A2-A3

	rts
.ENDMOD

.MODULE fcopypoint256 :

	movem.l	D3-D5/A2-A3,	-(sp)

	movea.l	(A0)+,	A2
	move.w	(A0)+,	D0
	move.l	(A0),	D4
	move.w	D0,	D1
	and.w	#$FFF0,	D0	
	and.w	#$F,	D1	
	lsr.w	#3,	D0
	adda.w	D0,	A2
	moveq.l	#15,	D0
	sub.w	D1,	D0

	movea.l	(A1)+,	A3
	move.w	(A1)+,	D2
	move.l	(A1),	D5
	move.w	D2,	D3
	and.w	#$FFF0,	D2	
	and.w	#$F,	D3	
	lsr.w	#3,	D2
	adda.w	D2,	A3
	moveq.l	#15,	D2
	sub.w	D3,	D2

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset1
	bset.l	D2,	D3
	bra.b	tsch1
notset1:
	bclr.l	D2,	D3
tsch1:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset2
	bset.l	D2,	D3
	bra.b	tsch2
notset2:
	bclr.l	D2,	D3
tsch2:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset3
	bset.l	D2,	D3
	bra.b	tsch3
notset3:
	bclr.l	D2,	D3
tsch3:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset4
	bset.l	D2,	D3
	bra.b	tsch4
notset4:
	bclr.l	D2,	D3
tsch4:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset5
	bset.l	D2,	D3
	bra.b	tsch5
notset5:
	bclr.l	D2,	D3
tsch5:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset6
	bset.l	D2,	D3
	bra.b	tsch6
notset6:
	bclr.l	D2,	D3
tsch6:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset7
	bset.l	D2,	D3
	bra.b	tsch7
notset7:
	bclr.l	D2,	D3
tsch7:
	move.w	D3,	(A3)

	adda.l	D4,	A2
	adda.l	D5,	A3

	move.w	(A2),	D1
	move.w	(A3),	D3
	btst.l	D0,	D1
	beq.b	notset8
	bset.l	D2,	D3
	bra.b	tsch8
notset8:
	bclr.l	D2,	D3
tsch8:
	move.w	D3,	(A3)

	movem.l	(sp)+,	D3-D5/A2-A3

	rts
.ENDMOD

.MODULE fcopypoint256PP

	move.l	(A0)+,	D1
	move.w	(A0),	D2
	movea.l	(A1)+,	A0
	move.w	(A1),	D0
	adda.w	D0,	A0
	move.l	D1,	A1
	adda.w	D2,	A1

	move.b	(A1),	(A0)

	rts
.ENDMOD

.MODULE fcopypointTC16

	move.l	(A0)+,	D1
	move.w	(A0),	D2
	movea.l	(A1)+,	A0
	move.w	(A1),	D0
	adda.w	D0,	A0
	adda.w	D0,	A0
	move.l	D1,	A1
	adda.w	D2,	A1
	adda.w	D2,	A1

	move.w	(A1),	(A0)

	rts
.ENDMOD

MODULE fcopypointTC24

	move.l	(A0)+,	D1
	move.w	(A0),	D2
	movea.l	(A1)+,	A0
	move.w	(A1),	D0
	adda.w	D0,	A0
	adda.w	D0,	A0
	adda.w	D0,	A0
	movea.l	D1,	A1
	adda.w	D2,	A1
	adda.w	D2,	A1
	adda.w	D2,	A1

	move.b	(A1)+,	(A0)+
	move.b	(A1)+,	(A0)+
	move.b	(A1),	(A0)

	rts
.ENDMOD

.END