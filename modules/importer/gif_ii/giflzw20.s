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

;**********************************************
;		Hier gehz los (66 Bytes bis 'loop')
;**********************************************
EXPORT decode_fast020

;	0(a0) = datasize
;	2(a0) = clear
;	4(a0) = eoi
;	6(a0) = available
;	8(a0) = pCodeSize
;	10(a0) = entries
;	12(a0) = pCodeMask

decode_fast020:
	movem.l	d3-d7/a2-a6, -(sp)
	clr.l d4

	;---------------------------- Variablen holen 
	move.w	(a0)+,	datasize
	add.w	#1, datasize
	move.w	(a0)+, 	clear
	move.w	(a0)+,	eoi
	move.w	(a0)+,	d3				; available
	move.w	(a0)+,	d1				; pCodeSize
	move.w	(a0)+,	entries
	move.w	(a0)+, 	d4				; pCodeMask

	move.l	(a1)+,	a0				; src	
	move.l	(a1)+,	a3				; srclen 
	move.l	(a1)+,	a2
	move.l	(a1)+,	a1				; pdata

	;-------------------- Counter etc. vorinitialisieren
	clr.l	d7			; pBitsLeft
	clr.l	d6			; pDatum
	clr.l	d5			; pCount


loop:
		;bsr getcode
		getcode:
				cmp.w	d1, d7			; pBitsLeft<pCodeSize?
				bge	gc_end
		
				tst.w	d5						; pCount==0?
				bne.b endif1
				moveq	#0, d5
				move.b	(a1)+, d5				; pCount=*pdata++
				beq end
		endif1:
		
				clr.l	d0
				move.b	(a1)+, d0			; *pData++
				lsl.l	d7,	d0				; << pBitsLeft
				or.l	d0, d6				; pDatum|=...
				addq.w	#8, d7				; pBitsLeft +=8
				subq.w	#1, d5				; pCount --
		
				bne.b endif2				; pCount = 0?
				moveq	#0, d5
				move.b	(a1)+, d5			; pCount = *pdata++
				beq end
		endif2:
		
				clr.l	d0
				move.b	(a1)+, d0			; *Pdata
				lsl.l	d7,	d0				; << pBitsLeft
				or.l	d0, d6				; pDatum|=...
				subq.w	#1, d5				; pCount --
				addq.w	#8, d7				; pBitsLeft +=8
		gc_end:
		
				clr.l	d0
				move.w	d6, d0 				; code=pDatum...
				and.w	d4, d0				; ...&pCodeMask
				lsr.l	d1, d6				; pDatum>>=pCodeSize
				sub.w	d1, d7				; pBitsleft-=pCodeSize
		
		

		;---------------------------Clearcode?
		cmp.w clear, d0
		bne.b cc_end

		move.w	d0, d3			; available=clear+2
		addq.w	#2, d3
		move.w	datasize, d1

		moveq.l	#1, d4
		lsl.w	d1, d4
		move.w	d4, entries
		subq.w	#1, d4
	
		bsr getcode2

		move.b	d0, (a2)+			; *ziel++=code

		move.w	d0, oldcode
		move.w	#1, oldcodelen		; oldcodelen
		movea.l	#1, a5
		bra loop
		
cc_end:
		; von der Tabellenpflege hierher geholt, um a5 verwenden zu k”nnen
		move.w	a5, (a3, d3.l*2)		; oldcodelen...

		;--------------------------- End-of-information?
		cmp.w eoi, d0		; in getcode wird a6=eoi gesetzt
		beq end

		movea.l a2, a6				; dst=ziel

		;--------------------------- bekannter Code
		cmp.w d3, d0
		bge.b new_code
		
		suba.w	a5, a6				; dst-=oldcodelen
		
		move.w	(a3, d0.l*2), d2		; i=codelen=(int*)srclen[code]
		movea.w	d2, a5
		movea.l	(a0, d0.l*4), a4
		addq.l	#1, a5

		move.b (a4)+, (a2)+			; *ziel++=*out++
		dbra d2, *-2

		bra.b dcc_end

new_code:
		;----------------------------neuer Code?
		move.w	oldcode, d2
		move.l	(a0, d2.w*4), a4
		move.w	(a3, d2.w*2), d2	; i=codelen=(int*)srclen[code]
		move.w	d2, a5

		move.b	(a4)+, (a2)+			; *ziel++=*out++
		dbra d2, *-2

		move.w	oldcode, d2
		move.l	(a0, d2.w*4), a4
		move.b	(a4), (a2)+
		addq.w	#2, a5
dcc_end:

		;-------------------------- Tabellenpflege, etc.
		move.w	d0, oldcode
		move.l	a6, (a0, d3.l*4)

		addq.w	#1, d3				;available
		move.w	a5, oldcodelen		; ...,oldcodelen

		;--------------------------- evtl. Codegr”že erh”hen
		cmp.w	entries, d3
		blt	loop
		cmp.b	#12, d1
		bge loop
		addq.b	#1, d1
		add.w	d3, d4
		asl.w	entries

		bra loop

/*
;**********************************************
;		Subroutine zum Code holen
;**********************************************
getcode:
		move.w eoi, a6
		cmp.w	d1, d7			; pBitsLeft<pCodeSize?
		bge	gc_end

		tst.w	d5						; pCount==0?
		bne.b endif1
		moveq	#0, d5
		move.b	(a1)+, d5				; pCount=*pdata++
		beq.b exit_decoder
endif1:

		clr.l	d0
		move.b	(a1)+, d0			; *pData++
		lsl.l	d7,	d0				; << pBitsLeft
		or.l	d0, d6				; pDatum|=...
		addq.w	#8, d7				; pBitsLeft +=8
		subq.w	#1, d5				; pCount --

		bne.b endif2				; pCount = 0?
		moveq	#0, d5
		move.b	(a1)+, d5			; pCount = *pdata++
		beq.b exit_decoder
endif2:

		clr.l	d0
		move.b	(a1)+, d0			; *Pdata
		lsl.l	d7,	d0				; << pBitsLeft
		or.l	d0, d6				; pDatum|=...
		subq.w	#1, d5				; pCount --
		addq.w	#8, d7				; pBitsLeft +=8
gc_end:

		clr.l	d0
		move.w	d6, d0 				; code=pDatum...
		and.w	d4, d0				; ...&pCodeMask
		lsr.l	d1, d6				; pDatum>>=pCodeSize
		sub.w	d1, d7				; pBitsleft-=pCodeSize
		rts 

exit_decoder:
		move.w a6, d0
		rts
*/


;**********************************************
;		2. Subroutine zum Code holen
;**********************************************
getcode2:
		cmp.w d1, d7			; pBitsLeft<pCodeSize?
		bge gc_end2

		tst	d5						; pCount==0?
		bne.b *+4
		move.b (a1)+, d5			; pCount=*pdata++

		clr.l	d0
		move.b (a1)+, d0
		lsl.l	d7,	d0
		or.l	d0, d6				; pDatum|=...
		addq.w	#8, d7
		subq.w	#1, d5

		bne.b *+4
		move.b (a1)+, d5			; pCount=*pdata++

		clr.l	d0
		move.b (a1)+, d0
		lsl.l	d7,	d0
		or.l	d0, d6				; pDatum|=...
		subq.w	#1, d5
		addq.w	#8, d7
gc_end2:
		clr.l	d0
		move.w	d6, d0 				; code=pDatum&pCodeMask
		and.l	d4, d0
		lsr.l	d1, d6				; pDatum>>=pCodeSize
		sub.w	d1, d7				; pBitsleft-=pCodeSize
		rts


end:
	movem.l	(sp)+, d3-d7/a2-a6
	rts


;-------------------------------------------------------------
.bss
clear:		ds.w	1
datasize:	ds.w	1
entries:	ds.w	1
eoi:		ds.w	1
oldcode:	ds.w	1
oldcodelen:	ds.w	1

