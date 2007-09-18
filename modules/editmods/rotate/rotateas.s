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

;	Assemblerroutine fr antialiasendes Drehen
;
;	Deklaration:
;
;	void rotate_xschleife(long *par, int width);
;
;	long par[4]:
;		par[0] = fracnew0;		unsigned int
;		par[1] = omfracnew0;	unsigned int
;		par[2] = ziel;			unsigned char
;		par[3] = buffer;		unsigned char
;

GLOBL rotate_xschleife

rotate_xschleife:
    movem.l d3-d7, -(sp)	; Register retten

    ; Parameter holen
    move.l (a0)+, d4	; fracnew0
    move.l (a0)+, d5	; omfracnew0
    move.l (a0)+, a1	; ziel
    move.l (a0), a0		; buffer

    
    subq.l #1, d0		; wg. dbra
    moveq.l #12, d7		; Shift-Wert fr lsr.l

    clr.l  d1			; prevr initial auf Hintergrund
    clr.l  d2			; prevg initial auf Hintergrund
    clr.l  d3			; prevb initial auf Hintergrund

xloop:
	clr.l d6
    ;-------------- Rot
	mulu.w d4, d1		; prevr * fracnew0
	move.b (a0), d6		; Bufferpixel nach d6
	mulu.w d5, d6		; buffer * omfracnew0
	add.l d6, d1		; die beiden addieren
	add.l #2048, d1		; 2048 drauf
	lsr.l d7, d1		; und um 12 Bits runtershiften
	move.b d1, (a1)+	; und nach ziel schreiben

;	clr.l  d1			; prevr LONG l”schen = Hintergrund
	move.b (a0)+, d1	; und buffer holen


	clr.l d6
	;-------------- Grn
	mulu.w d4, d2		; prevg * fracnew0
	move.b (a0), d6		; Bufferpixel nach d6
	mulu.w d5, d6		; buffer * omfracnew0
	add.l d6, d2		; die beiden addieren
	add.l #2048, d2		; 2048 drauf
	lsr.l d7, d2		; und um 12 Bits runtershiften
	move.b d2, (a1)+	; und nach ziel schreiben

;	clr.l  d2			; prevg LONG l”schen = Hintergrund
	move.b (a0)+, d2	; und buffer holen

    
	clr.l d6
	;-------------- Blau
	mulu.w d4, d3		; prevb * fracnew0
	move.b (a0), d6		; Bufferpixel nach d6
	mulu.w d5, d6		; buffer * omfracnew0
	add.l d6, d3		; die beiden addieren
	add.l #2048, d3		; 2048 drauf
	lsr.l d7, d3		; und um 12 Bits runtershiften
	move.b d3, (a1)+	; und nach ziel schreiben

;	clr.l  d3			; prevb LONG l”schen = Hintergrund
	move.b (a0)+, d3	; und buffer holen
dbra d0, xloop

tst.w d4
beq xnofracnew0			; if(fracnew0), einen weiteren Pixel

    clr.l d6

	;-------------- Rot
	mulu.w d4, d1		; prevr * fracnew0
	add.l #2048, d1		; 2048 drauf
	lsr.l d7, d1		; und um 12 Bits runtershiften
	move.b d1, (a1)+	; und nach ziel schreiben

	;-------------- Grn
	mulu.w d4, d2		; prevg * fracnew0
	add.l #2048, d2		; 2048 drauf
	lsr.l d7, d2		; und um 12 Bits runtershiften     
	move.b d2, (a1)+	; und nach ziel schreiben
    
	;-------------- Blau
	mulu.w d4, d3		; prevb * fracnew0
	add.l #2048, d3		; 2048 drauf
	lsr.l d7, d3		; und um 12 Bits runtershiften
	move.b d3, (a1)		; und nach ziel schreiben

xnofracnew0:

    movem.l (sp)+, d3-d7	; Register restore
    rts						; und tschss


/*
	prevr = prevg = prevb = 0x0;

	x = 0;
	do
	{
		*ziel++ = (char)((prevr * fracnew0 + (unsigned long)*buffer * omfracnew0 + 2048L) >> 12);
		*ziel++ = (char)((prevg * fracnew0 + (unsigned long)*(buffer + 1) * omfracnew0 + 2048L) >> 12);
		*ziel++ = (char)((prevb * fracnew0 + (unsigned long)*(buffer + 2) * omfracnew0 + 2048L) >> 12);

		prevr = (unsigned long)*buffer++;
		prevg = (unsigned long)*buffer++;
		prevb = (unsigned long)*buffer++;
	} while(++x < width);

	if(fracnew0 && intnew0 + width < tempwidth)
	{
		*ziel++ = (char)((prevr * fracnew0 + 2048L) >> 12);
		*ziel++ = (char)((prevg * fracnew0 + 2048L) >> 12);
		*ziel++ = (char)((prevb * fracnew0 + 2048L) >> 12);

		ziel += (newwidth - width - intnew0 - 1) * 3;
	}
	else
		ziel += (newwidth - width - intnew0) * 3;
*/





;	Deklaration:
;
;	void rotate_yschleife(long *par, int height);
;
;	long par[4]:
;		par[0] = fracnew0;		unsigned int
;		par[1] = omfracnew0;	unsigned int
;		par[2] = realtempwidth;	unsigned long
;		par[3] = newheight;		unsigned long
;		par[4] = ziel;			unsigned char
;		par[5] = buffer;		unsigned char
;

GLOBL rotate_yschleife

rotate_yschleife:
	movem.l d3-d7, -(sp)	; Register retten

	; Parameter holen
	move.l (a0)+, d4	; fracnew0
	move.l (a0)+, d5	; omfracnew0
	move.l (a0)+, a2	; realtempwidth
	move.l (a0)+, a3	; newheight
	move.l (a0)+, a1	; ziel
	move.l (a0), a0		; buffer

    
	subq.l #1, d0		; wg. dbra
	moveq.l #12, d7		; Shift-Wert fr lsr.l

	clr.l  d1			; prevr initial auf Hintergrund
	clr.l  d2			; prevg initial auf Hintergrund
	clr.l  d3			; prevb initial auf Hintergrund

yloop:

beq nonew				; if(new >= 0 && new < newheight), einen weiteren Pixel
	clr.l d6
	;-------------- Rot
	mulu.w d4, d1		; prevr * fracnew0
	move.b (a0), d6		; Bufferpixel nach d6
	mulu.w d5, d6		; buffer * omfracnew0
	add.l d6, d1		; die beiden addieren
	add.l #2048, d1		; 2048 drauf
	lsr.l d7, d1		; und um 12 Bits runtershiften
	move.b d1, (a1)		; und nach ziel schreiben

;	clr.l  d1			; prevr LONG l”schen = Hintergrund
	move.b (a0), d1		; und buffer holen


	clr.l d6
	;-------------- Grn
	mulu.w d4, d2		; prevg * fracnew0
	move.b 1(a0), d6	; Bufferpixel nach d6
	mulu.w d5, d6		; buffer * omfracnew0
	add.l d6, d2		; die beiden addieren
	add.l #2048, d2		; 2048 drauf
	lsr.l d7, d2		; und um 12 Bits runtershiften
	move.b d2, 1(a1)	; und nach ziel schreiben

;	clr.l  d2			; prevg LONG l”schen = Hintergrund
	move.b 1(a0), d2	; und buffer holen

    
	clr.l d6
	;-------------- Blau
	mulu.w d4, d3		; prevb * fracnew0
	move.b 2(a0), d6	; Bufferpixel nach d6
	mulu.w d5, d6		; buffer * omfracnew0
	add.l d6, d3		; die beiden addieren
	add.l #2048, d3		; 2048 drauf
	lsr.l d7, d3		; und um 12 Bits runtershiften
	move.b d3, 2(a1)	; und nach ziel schreiben

;	clr.l  d3			; prevb LONG l”schen = Hintergrund
	move.b 2(a0), d3	; und buffer holen

nonew:

	add.l a2, a0		; buffer += realtempwidth;
dbra d0, yloop

tst.w d4
beq ynofracnew0			; if(fracnew0), einen weiteren Pixel

	clr.l d6

	;-------------- Rot
	mulu.w d4, d1		; prevr * fracnew0
	add.l #2048, d1		; 2048 drauf
	lsr.l d7, d1		; und um 12 Bits runtershiften
	move.b d1, (a1)+	; und nach ziel schreiben

	;-------------- Grn
	mulu.w d4, d2		; prevg * fracnew0
	add.l #2048, d2		; 2048 drauf
	lsr.l d7, d2		; und um 12 Bits runtershiften     
	move.b d2, (a1)+	; und nach ziel schreiben
    
	;-------------- Blau
	mulu.w d4, d3		; prevb * fracnew0
	add.l #2048, d3		; 2048 drauf
	lsr.l d7, d3		; und um 12 Bits runtershiften
	move.b d3, (a1)		; und nach ziel schreiben

ynofracnew0:

    movem.l (sp)+, d3-d7	; Register restore
    rts						; und tschss


/*
		prevr = prevg = prevb = 0x0;

		y = 0;
		do
		{
			new = y + intnew0;
			if(new >= 0 && new < newheight)
			{
				off = new * realtempwidth;
				*(ziel + off) = (char)((prevr * fracnew0 + (unsigned long)*buffer * omfracnew0 + 2048) >> 12);
				*(ziel + off + 1) = (char)((prevg * fracnew0 + (unsigned long)*(buffer + 1) * omfracnew0 + 2048) >> 12);
				*(ziel + off + 2) = (char)((prevb * fracnew0 + (unsigned long)*(buffer + 2) * omfracnew0 + 2048) >> 12);

				prevr = (unsigned long)*buffer;
				prevg = (unsigned long)*(buffer + 1);
				prevb = (unsigned long)*(buffer + 2);
			}

			buffer += realtempwidth;
		} while(++y < height);

		if(fracnew0 && intnew0 + height < newheight)
		{
			ziel += off + realtempwidth;
			*ziel++ = (char)((prevr * fracnew0 + 2048) >> 12);
			*ziel++ = (char)((prevg * fracnew0 + 2048) >> 12);
			*ziel++ = (char)((prevb * fracnew0 + 2048) >> 12);
		}
*/