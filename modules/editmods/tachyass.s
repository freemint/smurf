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

;
; Tachyon-Effekt - Assemblerroutine
;
; Deklaration:
;	void tachyon_pixel(char *data, char *cliptab, unsigned int width);


GLOBL tachyon_pixel

tachyon_pixel:
movem.l d3-d7/a2-a6, -(sp)

move.l	a0,a2					;data
subq.w	#1,d0					;width-1 fÅr dbra

moveq.l #12,d1					;Shiftwert in Register packen
move.l #1050624,d7				;a2 als Datenregister miûbrauchen
								;1048576+2048 (256,5<<12)

_loopx:
;---------------------------
; Luminanz holen und aufbereiten
;---------------------------
	moveq.l	#$0,d4				;zeroing
	move.b 	(a0)+,d4			;R holen
	move.l	d4,d3				;und zwischenspeichern
	mulu.w  #1225,d3			;*1225L

	moveq.l	#$0,d5				;zeroing
	move.b 	(a0)+,d5			;G holen
	move.l	d5,d2				;und zwischenspeichern
	mulu.w  #2404,d2			;*2404L
	add.l	d2,d3				;R+G

	moveq.l	#$0,d6				;zeroing
	move.b 	(a0)+,d6			;B holen
	move.l	d6,d2				;und zwischenspeichern
	mulu.w  #467,d2				;*467L
	add.l	d2,d3				;R+G+B

	add.l	d3,d3				;l<<1 nur etwas schneller
	move.l	d7,d2				;
	sub.l	d3,d2				;1050624-l

;---------------------------
; Rot bearbeiten
;---------------------------
	lsl.l	d1,d4				;R<<12L
	add.l	d2,d4				;R+l
	asr.l   d1,d4				;R>>12L

	move.b	127(d4,a1),(a2)+	;holt Byte vom Offset 128 +
								;Inhalt von d4 aus der Cliptab
								;und speichert's gleich

;---------------------------
; GrÅn bearbeiten
;---------------------------
	lsl.l	d1,d5				;G<<12L
	add.l	d2,d5				;G+l
	asr.l   d1,d5				;G>>12L

	move.b	127(d5,a1),(a2)+	;holt Byte vom Offset 128 +
								;Inhalt von d4 aus der Cliptab
								;und speichert's gleich

;---------------------------
; Blau bearbeiten
;---------------------------
	lsl.l	d1,d6				;B<<12L
	add.l	d2,d6				;B+l
	asr.l   d1,d6				;B>>12L

	move.b	127(d6,a1),(a2)+	;holt Byte vom Offset 128 +
								;Inhalt von d4 aus der Cliptab
								;und speichert's gleich

dbra d0, _loopx

movem.l (sp)+,d3-d7/a2-a6
rts