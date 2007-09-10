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
 * The Initial Developer of the Original Code is
 * Olaf Piesche
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

***************************************************************	*
*	Get interleaved bitplane line								*
*	wandelt eine Zeile in 8 Bitplanes aus IBPL nach PP.			*
*	a0 = zielpuffer												*
*	a1 = quellpuffer											*
*	d0 = L„nge einer Zeile einer Plane in Bytes					*
*	Deklaration:												*
*	void get_IBPLine(char *dest, char *src, long lineplanelen);	*
*																*
*	Die oberste Plane ist das niederwertigste Bit				*
***************************************************************	*
GLOBL get_IBPLine8
get_IBPLine8:
movem.l d3-d7/a2, -(sp)

move.l d0, d5
subq.l #1, d0

loopline:
	moveq	#7, d6
	moveq	#0, d7

	;-------------- 8 Pixel...
	loop8:
		moveq.l	#8, d1		; plane-counter 
		movea.l a1, a2		; source kopieren
	
		moveq.l	#0, d3
		moveq.l	#1, d4

		loopplane:
			move.b	(a2), d2		; Byte holen
	
			btst	d6,	d2
			beq *+4
			or.l	d4, d3
	
			adda.l	d5, a2			; eine plane weiter
			lsl.l	#1, d4
		dbra d1, loopplane
	
/*		lsl.l	#8, d3 */
		rol.w	#8, d3
		rol.l	#8, d3
		rol.l	#8, d3
		rol.w	#8, d3

		move.l	d3, (a0)+
		subq.l	#1, a0

		addq.l	#1, d7
	dbra d6, loop8

addq.l	#1, a1
dbra d0, loopline


movem.l (sp)+, d3-d7/a2
rts