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

;                               pixelins.s
;   Enth„lt die Routinen zum Einrechnen eines Blockpixels in ein
;   Bildpixel, in allen Blockmodi und mit Deckkraft.
;
;   Die Routinen holen s„mtliche Parameter vom Stack, die C-Deklaration:
;
;   void cdecl insertpixel_x(unsigned char idx, 
;                                   d0
;               unsigned int r, unsigned int g, unsigned int b, 
;                       d1          d2              d3
;               int depth, long opacity, void *pdata)
;                       d4          d5      a0
;
/*
GLOBL   insertpixel_replace, insertpixel_add, insertpixel_sub, insertpixel_clipadd
GLOBL   insertpixel_clipsub, insertpixel_mult



insertpixel_replace:
    lea regsave_end,a0
    movem.l d3-d7, -(a0)
    
    move.b  (sp)+, d0
    move.w  (sp)+, d1
    move.w  (sp)+, d2
    move.w  (sp)+, d3
    move.w  (sp)+, d4
    move.l  (sp)+, d5
    move.l  (sp)+, a0
    suba.l  #1+8+8, sp
    
    move.w  d4, depth
    move.l  #255,d4
    sub.l   d5,d4       ; opac2
    
    cmpi.w  #16,depth
    blt replace_8bit
    bgt replace_24bit

replace_16bit:
    move.w (a0),d7
    
    move.w  d7,d6
    and.w   #$F800,d6
    rol.w   #5,d6
    mulu.w  d4,d6
    lsr.w   #8,d6
    move.w  d6,red1
    
    move.w  d7,d6
    and.w   #$07E0,d6
    lsr.w   #6,d6
    mulu.w  d4,d6
    lsr.w   #8,d6
    move.w  d6,green1
    
    move.w  d7,d6
    and.w   #$001F,d6
    mulu.w  d4,d6
    lsr.w   #8,d6
    move.w  d6,blue1

    rol.w   #5,d1
    mulu.w  d5,d1
    lsr.w   #8,d1
    add.w   red1,d1
    lsr.w   #1,d1
    
    lsr.w   #6,d2
    mulu.w  d5,d2
    lsr.w   #8,d2
    add.w   green1,d2
    lsr.w   #1,d2
    
    mulu.w  d5,d3
    lsr.w   #8,d3
    add.w   blue1,d3
    lsr.w   #1,d3

    ror.w   #5,d1
    lsl.w   #6,d2
    or.w    d1,d3
    or.w    d2,d3
    move.w  d3, (a0)

    lea regsave, a0
    movem.l (a0)+, d3-d7
    rts

replace_8bit:
    clr.w   d7
    move.b  (a0),d7
    mulu.w  d4,d7
    lsr.w   #8,d7

    mulu.w  d5,d0
    lsr.w   #8,d0
    add.w   d7,d0
    lsr.w   #1,d0
    move.w  d0, (a0)

    lea regsave, a0
    movem.l (a0)+, d3-d7
    rts

replace_24bit:
    clr.w   d6
    move.b  (a0),d6
    mulu.w  d4,d6
    lsr.w   #8,d6
    move.w  d6,red1
    
    clr.w   d6
    move.b  (1,a0),d6
    mulu.w  d4,d6
    lsr.w   #8,d6
    move.w  d6,green1
    
    clr.w   d6
    move.b  (2,a0),d6
    mulu.w  d4,d6
    lsr.w   #8,d6
    move.w  d6,blue1
    
    mulu.w  d5,d1
    lsr.w   #8,d1
    add.w   red1,d1
    lsr.w   #1,d1
    move.b  d1,(a0)+
    
    mulu.w  d5,d2
    lsr.w   #8,d2
    add.w   green1,d2
    lsr.w   #1,d2
    move.b  d2,(a0)+

    mulu.w  d5,d3
    lsr.w   #8,d3
    add.w   blue1,d3
    lsr.w   #1,d3
    move.b  d3,(a0)+
    
    lea regsave, a0
    movem.l (a0)+, d3-d7
    rts


depth:  ds.w    1
idx:    ds.w    1
red1:   ds.w    1
green1: ds.w    1
blue1:  ds.w    1
regsave:    ds.l    5
regsave_end:
*/
