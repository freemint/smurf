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

;	Median-Cut Routinen
;	fÅr Smurf Atarimultifunktionsgrafiksoftware ;-)
;	Olaf, 05.11.1997


;------------------------- Definition fÅr die Strukturoffsets
;							der MC_BOX - Struktur
XBOX		equ	0
YBOX		equ	2
ZBOX		equ	4
BOXWID		equ	6
BOXHGT		equ	8
BOXDEPTH	equ	10


;-----------------------------------------------------
;	Aufsummieren der Farben einer yz-Plane aus dem FarbwÅrfel
;	Deklaration: ulong sumplane_yz(long *hist, MC_BOX box, int xposition);
GLOBL sumplane_yz

sumplane_yz:
	movem.l	d3-d7, -(sp)

	move.w	YBOX(a1), d1
	move.w	ZBOX(a1), d2
	move.w	BOXHGT(a1), d3
	move.w	BOXDEPTH(a1), d4

	move.w	d2, d5
	add.w	d4, d5			; Endwert fÅr die Z-Schleife

	clr.l d6
	move.w	d0, d6
	ror.w #6, d6		; rot an seinen Platz schieben
	lsl.w #5, d1		; blau an seinen Platz schieben
	add.w d1, d6		; und beide addieren

	subq.w	#1, d3		; fÅr dbra

	clr.l d0		; SummenzÑhler
	move.l	#32*4, d7


zloop:
	move.l	d6, d1
	add.w	d2, d1		; 3D-Index (15 Bit, r,g & b sind je eine Achse mit 5 Bit)
	lsl.l	#2, d1		; als long-offset


	move.w d3, d4		; y-counter
yloop:
	add.l	(d1.l, a0), d0
	add.l	d7, d1
	dbra	d4, yloop

	addq.w #1, d2
	cmp.w	d5, d2
	blt	zloop

	movem.l	(sp)+, d3-d7
	rts






;-----------------------------------------------------
;	Aufsummieren der Farben einer xz-Plane aus dem FarbwÅrfel
;	Deklaration: ulong sumplane_xz(long *hist, MC_BOX box, int yposition);
GLOBL sumplane_xz

sumplane_xz:
	movem.l	d3-d7, -(sp)

	move.w	XBOX(a1), d1
	move.w	ZBOX(a1), d2
	move.w	BOXWID(a1), d3
	move.w	BOXDEPTH(a1), d4

	move.w	d2, d5
	add.w	d4, d5			; Endwert fÅr die Z-Schleife

	clr.l d6
	move.w	d0, d6
	lsl.w #5, d6		; rot an seinen Platz schieben
	ror.w #6, d1		; blau an seinen Platz schieben
	add.w d1, d6		; und beide addieren

	subq.w	#1, d3		; fÅr dbra

	clr.l d0		; SummenzÑhler
	move.l	#1024*4, d7


zloop2:
	move.l	d6, d1
	add.w	d2, d1		; 3D-Index (15 Bit, r,g & b sind je eine Achse mit 5 Bit)
	lsl.l	#2, d1		; als long-offset


	move.w d3, d4		; y-counter
xloop:
	add.l	(d1.l, a0), d0
	add.l	d7, d1
	dbra	d4, xloop

	addq.w #1, d2
	cmp.w	d5, d2
	blt	zloop2

	movem.l	(sp)+, d3-d7
	rts



;-----------------------------------------------------
;	Aufsummieren der Farben einer xy-Plane aus dem FarbwÅrfel
;	Deklaration: ulong sumplane_xy(long *hist, MC_BOX box, int zposition);
GLOBL sumplane_xy

sumplane_xy:
	movem.l	d3-d7, -(sp)
	clr.l d2

	move.w	XBOX(a1), d1
	move.w	YBOX(a1), d2
	move.w	BOXWID(a1), d3
	move.w	BOXHGT(a1), d4

	move.w	d2, d5
	add.w	d4, d5			; Endwert fÅr die Z-Schleife

	clr.l d6
	move.w	d1, d6
	ror.w #6, d6		; rot an seinen Platz schieben
	add.w d0, d6		; und beide addieren

	subq.w	#1, d3		; fÅr dbra

	clr.l d0		; SummenzÑhler
	move.l	#1024*4, d7


yloop2:
	move.l	d2, d1
	lsl.w	#5, d1
	add.w	d6, d1		; 3D-Index (15 Bit, r,g & b sind je eine Achse mit 5 Bit)
	lsl.l	#2, d1		; als long-offset


	move.w d3, d4		; y-counter
xloop2:
	add.l	(d1.l, a0), d0
	add.l	d7, d1
	dbra	d4, xloop2

	addq.w #1, d2
	cmp.w	d5, d2
	blt	yloop2

	movem.l	(sp)+, d3-d7
	rts



;-----------------------------------------------------
;	Histogramm fÅr 24 Bit-Bild erstellen
; Deklaration: histogram_24bit(long *histogram, char *pdata, long pixlen)
GLOBL histogram_24bit
histogram_24bit:
	movem.l d3-d4, -(sp)

	clr.l	d1
	clr.l	d2
	clr.l	d3
	clr.l	d4

pixloop24:
	clr.l	d1
	clr.l	d2
	clr.l	d3
	move.b	(a1)+,	d1		; r
	lsr.b	#3, d1
	move.b	(a1)+,	d2		; g
	lsr.b	#3, d2
	move.b	(a1)+,	d3		; b
	lsr.b	#3, d3

	; 3D-Index ausrechnen
	ror.w 	#6,	d1
	lsl.w	#5,	d2
	add.w	d2,	d1
	add.w	d3,	d1
	
	lsl.l	#2,	d1			; long-index
	
	tst.l	(a0,d1.l)		; farbe zÑhlen?
	bne no_new_col
	addq.l #1, d4

no_new_col:
	addq.l	#1,	(a0, d1.l)	; und die Farbanzahl im Histogramm inkrementieren. 

	subq.l	#1,	d0
	bne	pixloop24
	
	move.l d4, d0
	movem.l	(sp)+,	d3-d4
	rts
