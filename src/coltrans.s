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

* Routinen zur Transformation von Farbsystemen
*
*	Transformation von und nach:
*
*		rgb
*		bgr
*		cmy
*		ycbcr
*
*
*	 (a0): pic_data
*	4(a0): L„nge des Bildes in Pixel
*	8(a0): color Format
*
*	29.02.96 Olaf

GLOBL	cs_transform
cs_transform:

/* Farbsysteme */
RGB			equ 0
BGR			equ 1
CMYK		equ 2
YCBCR		equ 3
YIQ			equ 4
WURSCHT		equ 5

Red			set d0
Green		set d1
Blue		set d2

Lcount			set d3
SrcColForm		set d4
DestColForm		set d5


move.l	4(a0), Lcount	; L„nge holen
move.l	8(a0), SrcColForm	; Quell-Farbsystem
move.l	8(a0), DestColForm	; Ziel-Farbsystem

movea.l a0, a1
movea.l a0, a2


loop:

	cmp.l RGB, SrcColForm
	bne.b l_bgr
		move.b	(a1)+, Red
		move.b	(a1)+, Green
		move.b	(a1)+, Blue

	l_bgr:
	cmp.l BGR, SrcColForm
	bne.b l_cmy
		move.b	(a1)+, Blue
		move.b	(a1)+, Green
		move.b	(a1)+, Red
	l_cmy:


		move.w (a2)+, Red
		move.w (a2)+, Green
		move.w (a2)+, Blue

dbra Lcount, loop
