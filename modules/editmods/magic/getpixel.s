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
;		Schnelle Punktinformationsroutinen
;		2, 4, 16, 256 Farben & 16 / 24 Bit TrueColor

.EXPORT gp_fak2

.EXPORT get_point
.EXPORT get_point2
.EXPORT get_point2_fak
.EXPORT get_point4
.EXPORT get_point16
.EXPORT get_point256
.EXPORT get_point256PP
.EXPORT get_pointTC16
.EXPORT get_pointTC24

.EXPORT get_pointTC16_2
.export fuck_tab

.DATA

gp_fak2 : 	dc.l 10

.BSS
fuck_tab:	ds.b	256

.TEXT

get_point : dc.l	0;		der Zeiger

;	öbergabeparameter :
;		A0 - Adresse
;		A1 - max Y
;		D0	max X
;		D1	X
;		D2	Y
;	RÅckgabe in D0
;

.MODULE get_point2:

	mulu.w	D0,	D2;
	move.w	D1,	D0
	and.l	#$FFF0,	D1	
	and.l	#$F,	D0
	lsr.w	#3,	D1	
	add.l	D1,	D2
	adda.l	D2,	A0	
	moveq.l	#15,	D2
	sub.w	D0,	D2

	move.w	(A0),	D1
	btst.l	D2,	D1
	beq.b	.notset
	moveq.l	#1,	D0
	rts
.notset:
	moveq.l	#0,	D0
	rts
.ENDMOD

.MODULE get_point2_fak:

	mulu.w	D0,	D2;
	move.w	D1,	D0
	and.l	#$FFF0,	D1	
	and.l	#$F,	D0
	lsr.w	#3,	D1	
	add.l	D1,	D2
	adda.l	D2,	A0	
	moveq.l	#15,	D2
	sub.w	D0,	D2

	move.w	(A0),	D1
	btst.l	D2,	D1
	beq.b	.notset
	move.l	gp_fak2,	D0
	rts
.notset:
	moveq.l	#0,	D0
	rts
.ENDMOD

.MODULE get_point4:
	move.l	D3,	-(sp)

	mulu.w	D0,	D2;
	move.w	D1,	D3
	and.l	#$FFF0,	D1	
	and.l	#$F,	D3
	lsr.w	#3,	D1	
	add.l	D1,	D2
	adda.l	D2,	A0	
	moveq.l	#15,	D0
	sub.w	D3,	D0
	moveq.l	#0,	D3

	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset
	bset.l	#0,	D3
.notset:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset2
	bset.l	#1,	D3
.notset2:
	move.w	D3,	D0

	move.l	(sp)+,	D3
	rts
.ENDMOD

.MODULE get_point16:
	move.l	D3,	-(sp)

	mulu.w	D0,	D2;
	move.w	D1,	D3
	and.l	#$FFF0,	D1	
	and.l	#$F,	D3
	lsr.w	#3,	D1	
	add.l	D1,	D2
	adda.l	D2,	A0	
	moveq.l	#15,	D0
	sub.w	D3,	D0
	moveq.l	#0,	D3

	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset
	bset.l	#0,	D3
.notset:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset2
	bset.l	#1,	D3
.notset2:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset3
	bset.l	#2,	D3
.notset3:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset4
	bset.l	#3,	D3
.notset4:
	move.w	D3,	D0

	move.l	(sp)+,	D3
	rts
.ENDMOD

.MODULE get_point256:
	move.l	D3,	-(sp)

	mulu.w	D0,	D2;
	move.w	D1,	D3
	and.l	#$FFF0,	D1	
	and.l	#$F,	D3
	lsr.w	#3,	D1	
	add.l	D1,	D2
	adda.l	D2,	A0	
	moveq.l	#15,	D0;
	sub.w	D3,	D0
	moveq.l	#0,	D3

	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset
	bset.l	#0,	D3
.notset:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset2
	bset.l	#1,	D3
.notset2:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset3
	bset.l	#2,	D3
.notset3:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset4
	bset.l	#3,	D3
.notset4:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset5
	bset.l	#4,	D3
.notset5:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset6
	bset.l	#5,	D3
.notset6:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset7
	bset.l	#6,	D3
.notset7:
	adda.l	A1,	A0
	move.w	(A0),	D1
	btst.l	D0,	D1
	beq.b	.notset8
	bset.l	#7,	D3
.notset8:
	move.w	D3,	D0

	move.l	(sp)+,	D3
	rts
.ENDMOD

.MODULE get_point256PP:
	mulu.w	D0,	D2
	adda.l	D2,	A0
	lea		fuck_tab(pc),	A1
	adda.w	D1,	A0
	moveq.l	#0,	D0
	move.b	(A0),	D0
	move.b	(A1,D0.w),	D0; Skaliertabelle
	rts
.ENDMOD

.MODULE get_pointTC16:;*** hier wird nur das letzte Byte Åbergeben !!!
	lsl.w	#1,	D0
	mulu.w	D0,	D2
	adda.l	D2,	A0
	add.w	D1, D1
	adda.w	D1,	A0
	moveq.l	#0,	D0
	move.w	(A0),	D0
;	and.w	#$FF,	D0;  !!! nur das letzte Byte
	rts
.ENDMOD

.MODULE get_pointTC24:
	move.w	D0,	-(sp)
	lsl.w	#1,	D0
	add.w	(sp)+,	D0
	
	mulu.w	D0,	D2
	adda.l	D2,	A0
	move.w	D1,	D0
	add.w	D1,	D1
	add.w	D0,	D1
	adda.w	D1,	A0

	moveq.l	#0,	D0
	move.b	(A0)+,	D0
	lsl.l	#8,	D0
	add.b	(A0)+,	D0
	lsl.l	#8,	D0
	add.b	(A0),	D0
	rts
.ENDMOD


.MODULE get_pointTC16_2:	;*** hier werden die FARBEN wie bei 24Bit Åbergeben !!!
						;*** long = 0,ROT,GRöN,BLAU
	lsl.w	#1,	D0
	mulu.w	D0,	D2
	adda.l	D2,	A0
	add.w	D1,	D1
	adda.w	D1,	A0
	move.w	(A0),	D2
	clr.l	D0

	move.w	D2,	D1
	andi.w	#$1F,	D1; Blauwert isolieren
	or.w	D1,	D0

	lsl.l	#2,	D2
	move.w	D2,	D1
	andi.w	#$1F00,	D1; GrÅnwert isolieren
	or.w	D1,	D0

	lsl.l	#3,	D2
	andi.l	#$1F0000,	D2; Rotwert isolieren
	or.l	D2,	D0

	rts
.ENDMOD

.END