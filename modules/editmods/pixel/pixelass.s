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
 * Olaf Piesche, Christian Eyrich
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

;
; Mosaik-Effekt - Assemblerroutine
;
; Deklaration:
;	void do_block(unsigned char *data, unsigned long *pixtab, int blocksize);

GLOBL do_block

do_block:
	movem.l d3-d5/a2-a3, -(sp)

	clr.l d1				; blockvalr,
	clr.l d2				; blockvalg und
	clr.l d3				; blockvalb auf Null
	clr.l d4				; Zwischenspeicher fÅr Farbwert
	clr.l d5				; Divisor Long lîschen

	move.w d0, d5			; Schleifenmaximum nach d5 sichern
							; nur word, blocksize ist als INT deklariert!
	movea.l a1, a3			; Pixtab sichern ( addq #4, a1 -> (a1)+ )

	
	subq.l #1, d0			; wg. dbra
						

loop1:
	movea.l a0, a2			; data neu holen
	adda.l (a1)+, a2		; Offset aus Pixtab aufrechnen ...

	move.b (a2)+, d4		; Byte holen
	add.l	d4.b, d1.l		; und in die vollen 32Bit von d1 addieren
	move.b (a2)+, d4
	add.l	d4.b, d2.l
	move.b (a2)+, d4
	add.l	d4.b, d3.l

	dbra d0, loop1			; Test und zurÅckschleifen


	;*********** Durchschnitt ausrechnen *****************
	divu.w d5, d1			; Durchschnitte fÅr Rot
	divu.w d5, d2			; GrÅn und
	divu.w d5, d3			; Blau ausrechnen




	;********* Und jetzt den durchschnittswert in den Block eintragen
	move.l d5, d0			; Schleifenmaximum aus d5 holen
	subq.l #1, d0			; wg. dbra
	movea.l a3, a1			; Pixtab wieder holen


loop2:
	movea.l a0, a2			; data neu holen
	adda.l (a1)+, a2		; Offset aus Pixtab aufrechnen ...

	move.b d1, (a2)+		; Rot,
	move.b d2, (a2)+		; GrÅn und
	move.b d3, (a2)+		; Blau im ganzen Block setzen

	dbra d0, loop2			; Test und zurÅckschleifen

	movem.l (sp)+, d3-d5/a2-a3
	rts


/*
		Blockdurchschnitt berechnen
			blockvalr = blockvalg = blockvalb = 0;
			b = 0;
			do
			{
				blockvalr += *(data + pixtab[b]);
				blockvalg += *(data + pixtab[b] + 1);
				blockvalb += *(data + pixtab[b] + 2);
			} while(++b < blocksize);
			blockvalr /= blocksize;
			blockvalg /= blocksize;
			blockvalb /= blocksize;

		Blockdurchschnitt im Block setzen
			b = 0;
			do
			{
				*(data + pixtab[b]) = blockvalr;
				*(data + pixtab[b] + 1) = blockvalg;
				*(data + pixtab[b] + 2) = blockvalb;
			} while(++b < blocksize);
*/